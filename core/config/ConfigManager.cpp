#include "config/ConfigManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

namespace Vitals {

ConfigManager::ConfigManager(QString rootPath)
    : m_rootPath(rootPath.isEmpty()
        ? QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("config"))
        : std::move(rootPath))
{
}

QString ConfigManager::rootPath() const
{
    return m_rootPath;
}

QString ConfigManager::appConfigPath() const
{
    QDir dir(m_rootPath);
    return dir.filePath(QStringLiteral("app.json"));
}

QString ConfigManager::pluginConfigPath(const QString& pluginId) const
{
    QDir dir(m_rootPath);
    return dir.filePath(QStringLiteral("plugins/%1.json").arg(pluginId));
}

bool ConfigManager::isPluginEnabled(const QString& pluginId, const QString& filePath) const
{
    const QJsonObject root = loadAppConfig();
    const QJsonObject pluginsObject = root.value(QStringLiteral("plugins")).toObject();
    const QString key = pluginConfigKey(pluginId, filePath);
    if (!pluginsObject.contains(key)) {
        return true;
    }

    return pluginsObject.value(key).toObject().value(QStringLiteral("enabled")).toBool(true);
}

bool ConfigManager::setPluginEnabled(const QString& pluginId, const QString& filePath, bool enabled)
{
    QDir rootDir(m_rootPath);
    if (!rootDir.exists() && !rootDir.mkpath(QStringLiteral("."))) {
        return false;
    }

    QJsonObject rootObject = loadAppConfig();
    QJsonObject pluginsObject = rootObject.value(QStringLiteral("plugins")).toObject();
    const QString key = pluginConfigKey(pluginId, filePath);

    QJsonObject pluginState = pluginsObject.value(key).toObject();
    pluginState.insert(QStringLiteral("enabled"), enabled);
    pluginState.insert(QStringLiteral("pluginId"), pluginId);
    pluginState.insert(QStringLiteral("filePath"), filePath);
    pluginsObject.insert(key, pluginState);
    rootObject.insert(QStringLiteral("plugins"), pluginsObject);

    return saveAppConfig(rootObject);
}

bool ConfigManager::isPluginTaskbarEnabled(const QString& pluginId, const QString& filePath, bool defaultEnabled) const
{
    const QJsonObject root = loadAppConfig();
    const QJsonObject pluginsObject = root.value(QStringLiteral("plugins")).toObject();
    const QString key = pluginConfigKey(pluginId, filePath);
    if (!pluginsObject.contains(key)) {
        return defaultEnabled;
    }

    return pluginsObject.value(key).toObject().value(QStringLiteral("taskbarEnabled")).toBool(defaultEnabled);
}

bool ConfigManager::setPluginTaskbarEnabled(const QString& pluginId, const QString& filePath, bool enabled)
{
    QDir rootDir(m_rootPath);
    if (!rootDir.exists() && !rootDir.mkpath(QStringLiteral("."))) {
        return false;
    }

    QJsonObject rootObject = loadAppConfig();
    QJsonObject pluginsObject = rootObject.value(QStringLiteral("plugins")).toObject();
    const QString key = pluginConfigKey(pluginId, filePath);

    QJsonObject pluginState = pluginsObject.value(key).toObject();
    pluginState.insert(QStringLiteral("taskbarEnabled"), enabled);
    pluginState.insert(QStringLiteral("pluginId"), pluginId);
    pluginState.insert(QStringLiteral("filePath"), filePath);
    pluginsObject.insert(key, pluginState);
    rootObject.insert(QStringLiteral("plugins"), pluginsObject);

    return saveAppConfig(rootObject);
}

QJsonObject ConfigManager::loadAppConfig() const
{
    QFile file(appConfigPath());
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return {};
    }

    return QJsonDocument::fromJson(file.readAll()).object();
}

bool ConfigManager::saveAppConfig(const QJsonObject& rootObject) const
{
    QFile file(appConfigPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }

    file.write(QJsonDocument(rootObject).toJson(QJsonDocument::Indented));
    return true;
}

QString ConfigManager::pluginConfigKey(const QString& pluginId, const QString& filePath) const
{
    if (!pluginId.isEmpty()) {
        return pluginId;
    }
    return QFileInfo(filePath).completeBaseName();
}

} // namespace Vitals
