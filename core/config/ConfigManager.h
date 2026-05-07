#pragma once

#include <QJsonObject>
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
    bool isPluginTaskbarEnabled(const QString& pluginId, const QString& filePath, bool defaultEnabled) const;
    bool setPluginTaskbarEnabled(const QString& pluginId, const QString& filePath, bool enabled);

private:
    QJsonObject loadAppConfig() const;
    bool saveAppConfig(const QJsonObject& rootObject) const;
    QString pluginConfigKey(const QString& pluginId, const QString& filePath) const;
    QString m_rootPath;
};

} // namespace Vitals
