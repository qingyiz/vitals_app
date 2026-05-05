#include "plugin/PluginManager.h"

#include "IPlugin.h"
#include "PluginMetaInfo.h"
#include "config/ConfigManager.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QPluginLoader>

namespace Vitals {

PluginManager::PluginManager(QObject* parent)
    : QObject(parent)
{
}

PluginManager::~PluginManager()
{
    shutdownAll();
}

void PluginManager::loadAllPlugins(const QString& pluginsDir, IAppContext* context, const ConfigManager* configManager)
{
    m_pluginInfos.clear();

    QDir dir(pluginsDir);
    if (!dir.exists()) {
        qWarning() << "Plugins directory does not exist:" << pluginsDir;
        return;
    }

    const auto files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo& fileInfo : files) {
        if (!isPluginFile(fileInfo.fileName())) {
            continue;
        }

        const QString filePath = fileInfo.absoluteFilePath();
        auto loader = QSharedPointer<QPluginLoader>::create(filePath);
        PluginRuntimeInfo runtimeInfo;
        runtimeInfo.filePath = filePath;
        runtimeInfo.metaInfo = metaInfoFromLoader(*loader);

        if (configManager
            && !configManager->isPluginEnabled(runtimeInfo.metaInfo.id, filePath)) {
            runtimeInfo.status = PluginRuntimeInfo::Status::Disabled;
            runtimeInfo.reason = QStringLiteral("Disabled in app config");
            m_pluginInfos.push_back(runtimeInfo);
            continue;
        }

        QString skipReason;
        if (!supportsCurrentPlatform(*loader, &skipReason)) {
            qInfo() << "Skipped plugin:" << filePath << skipReason;
            runtimeInfo.status = PluginRuntimeInfo::Status::Skipped;
            runtimeInfo.reason = skipReason;
            m_pluginInfos.push_back(runtimeInfo);
            Q_EMIT pluginSkipped(filePath, skipReason);
            continue;
        }

        QObject* instance = loader->instance();
        if (!instance) {
            const QString reason = loader->errorString();
            qWarning() << "Failed to load plugin:" << filePath << reason;
            runtimeInfo.status = PluginRuntimeInfo::Status::Failed;
            runtimeInfo.reason = reason;
            m_pluginInfos.push_back(runtimeInfo);
            Q_EMIT pluginLoadFailed(filePath, reason);
            continue;
        }

        auto* plugin = qobject_cast<IPlugin*>(instance);
        if (!plugin) {
            const QString reason = QStringLiteral("Object does not implement IPlugin");
            qWarning() << "Invalid plugin:" << filePath << reason;
            loader->unload();
            runtimeInfo.status = PluginRuntimeInfo::Status::Failed;
            runtimeInfo.reason = reason;
            m_pluginInfos.push_back(runtimeInfo);
            Q_EMIT pluginLoadFailed(filePath, reason);
            continue;
        }

        if (!plugin->initialize(context)) {
            const QString reason = QStringLiteral("Plugin initialize returned false");
            qWarning() << "Plugin initialize failed:" << filePath;
            loader->unload();
            runtimeInfo.status = PluginRuntimeInfo::Status::Failed;
            runtimeInfo.reason = reason;
            m_pluginInfos.push_back(runtimeInfo);
            Q_EMIT pluginLoadFailed(filePath, reason);
            continue;
        }

        const PluginMetaInfo meta = plugin->metaInfo();
        runtimeInfo.metaInfo = meta;
        runtimeInfo.status = PluginRuntimeInfo::Status::Loaded;
        m_plugins.push_back({loader, plugin});
        m_pluginInfos.push_back(runtimeInfo);
        Q_EMIT pluginLoaded(meta.id, meta.name);
    }
}

void PluginManager::startAll()
{
    for (const LoadedPlugin& loaded : m_plugins) {
        loaded.plugin->start();
    }
}

void PluginManager::stopAll()
{
    for (const LoadedPlugin& loaded : m_plugins) {
        loaded.plugin->stop();
    }
}

void PluginManager::shutdownAll()
{
    for (const LoadedPlugin& loaded : m_plugins) {
        if (loaded.plugin) {
            loaded.plugin->stop();
            loaded.plugin->shutdown();
        }
        if (loaded.loader) {
            loaded.loader->unload();
        }
    }
    m_plugins.clear();
}

QList<IPlugin*> PluginManager::plugins() const
{
    QList<IPlugin*> result;
    for (const LoadedPlugin& loaded : m_plugins) {
        result.append(loaded.plugin);
    }
    return result;
}

int PluginManager::loadedPluginCount() const
{
    return m_plugins.size();
}

QList<PluginRuntimeInfo> PluginManager::pluginInfos() const
{
    return m_pluginInfos.toList();
}

bool PluginManager::isPluginFile(const QString& fileName)
{
#if defined(Q_OS_WIN)
    return fileName.endsWith(QStringLiteral(".dll"), Qt::CaseInsensitive);
#elif defined(Q_OS_MAC)
    return fileName.endsWith(QStringLiteral(".dylib"), Qt::CaseInsensitive)
        || fileName.endsWith(QStringLiteral(".so"), Qt::CaseInsensitive);
#else
    return fileName.endsWith(QStringLiteral(".so"), Qt::CaseInsensitive);
#endif
}

QString PluginManager::currentPlatformId()
{
#if defined(Q_OS_WIN)
    return QStringLiteral("windows");
#elif defined(Q_OS_MAC)
    return QStringLiteral("macos");
#elif defined(Q_OS_LINUX)
    return QStringLiteral("linux");
#else
    return QStringLiteral("unknown");
#endif
}

bool PluginManager::supportsCurrentPlatform(const QPluginLoader& loader, QString* reason)
{
    const QJsonObject root = loader.metaData();
    const QJsonObject pluginMeta = root.value(QStringLiteral("MetaData")).toObject();
    const QJsonValue supportedPlatformsValue = pluginMeta.value(QStringLiteral("supportedPlatforms"));

    if (!supportedPlatformsValue.isArray()) {
        if (reason) {
            *reason = QStringLiteral("Missing supportedPlatforms metadata; loading as legacy-compatible plugin");
        }
        return true;
    }

    const QString currentPlatform = currentPlatformId();
    const QJsonArray supportedPlatforms = supportedPlatformsValue.toArray();
    for (const QJsonValue& value : supportedPlatforms) {
        const QString platform = value.toString().trimmed().toLower();
        if (platform == currentPlatform
            || platform == QStringLiteral("all")
            || platform == QStringLiteral("*")) {
            return true;
        }
    }

    if (reason) {
        QStringList platforms;
        for (const QJsonValue& value : supportedPlatforms) {
            platforms.append(value.toString());
        }
        *reason = QStringLiteral("Current platform '%1' is not in supportedPlatforms [%2]")
            .arg(currentPlatform, platforms.join(QStringLiteral(", ")));
    }
    return false;
}

PluginMetaInfo PluginManager::metaInfoFromLoader(const QPluginLoader& loader)
{
    const QJsonObject root = loader.metaData();
    const QJsonObject pluginMeta = root.value(QStringLiteral("MetaData")).toObject();

    PluginMetaInfo metaInfo;
    metaInfo.id = pluginMeta.value(QStringLiteral("id")).toString();
    metaInfo.name = pluginMeta.value(QStringLiteral("name")).toString();
    metaInfo.description = pluginMeta.value(QStringLiteral("description")).toString();
    metaInfo.version = pluginMeta.value(QStringLiteral("version")).toString();
    metaInfo.author = pluginMeta.value(QStringLiteral("author")).toString();
    metaInfo.category = pluginMeta.value(QStringLiteral("category")).toString();
    metaInfo.requiredHostVersion = pluginMeta.value(QStringLiteral("requiredHostVersion")).toString();

    const QJsonArray supportedPlatforms = pluginMeta.value(QStringLiteral("supportedPlatforms")).toArray();
    for (const QJsonValue& value : supportedPlatforms) {
        metaInfo.supportedPlatforms.append(value.toString());
    }

    return metaInfo;
}

} // namespace Vitals
