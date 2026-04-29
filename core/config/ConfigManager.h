#pragma once

#include <QString>

namespace Vitals {

class ConfigManager
{
public:
    explicit ConfigManager(QString rootPath = QString());

    QString rootPath() const;
    QString pluginConfigPath(const QString& pluginId) const;

private:
    QString m_rootPath;
};

} // namespace Vitals

