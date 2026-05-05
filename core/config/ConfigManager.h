#pragma once

#include <QString>

namespace Vitals {

class ConfigManager
{
public:
    explicit ConfigManager(QString rootPath = QString());

    QString rootPath() const;
    QString appConfigPath() const;
    QString pluginConfigPath(const QString& pluginId) const;
    bool isPluginEnabled(const QString& pluginId, const QString& filePath) const;
    bool setPluginEnabled(const QString& pluginId, const QString& filePath, bool enabled);

private:
    QString pluginConfigKey(const QString& pluginId, const QString& filePath) const;
    QString m_rootPath;
};

} // namespace Vitals
