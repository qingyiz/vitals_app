#pragma once

#include "MetricData.h"

#include <QHash>
#include <QList>
#include <QString>

namespace Vitals {

struct TaskbarDetailBadge
{
    QString label;
    QString value;
};

struct TaskbarDetailRow
{
    QString label;
    QString value;
    QString detail;
    double progress = -1.0;
    QString accentColor;
};

struct TaskbarDetailSection
{
    QString title;
    QList<TaskbarDetailRow> rows;
};

struct TaskbarDetailContent
{
    QString title;
    QString subtitle;
    QString primaryLabel;
    QString primaryValue;
    QString accentColor;
    QList<TaskbarDetailBadge> badges;
    QList<TaskbarDetailSection> sections;

    bool isEmpty() const
    {
        return title.isEmpty() && primaryValue.isEmpty() && sections.isEmpty();
    }
};

class ITaskbarDetailPlugin
{
public:
    virtual ~ITaskbarDetailPlugin() = default;

    virtual TaskbarDetailContent taskbarDetailContent(
        const QHash<QString, MetricValue>& latestValues) const = 0;
};

} // namespace Vitals

#define Vitals_ITaskbarDetailPlugin_iid "com.vitals.plugin.ITaskbarDetailPlugin/1.0"
Q_DECLARE_INTERFACE(Vitals::ITaskbarDetailPlugin, Vitals_ITaskbarDetailPlugin_iid)
