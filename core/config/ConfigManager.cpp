#include "config/ConfigManager.h"

#include <QCoreApplication>
#include <QDir>

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

QString ConfigManager::pluginConfigPath(const QString& pluginId) const
{
    QDir dir(m_rootPath);
    return dir.filePath(QStringLiteral("plugins/%1.json").arg(pluginId));
}

} // namespace Vitals

