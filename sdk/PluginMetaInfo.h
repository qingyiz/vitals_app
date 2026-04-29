#pragma once

#include <QString>
#include <QStringList>

namespace Vitals {

struct PluginMetaInfo
{
    QString id;
    QString name;
    QString description;
    QString version;
    QString author;
    QString category;
    QStringList supportedPlatforms;
    QString requiredHostVersion;
};

} // namespace Vitals

