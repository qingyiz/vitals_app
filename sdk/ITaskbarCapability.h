#pragma once

#include "ITaskbarDetailPlugin.h"

#include <QHash>

namespace Vitals {

class ITaskbarCapability
{
public:
    virtual ~ITaskbarCapability() = default;

    virtual QString displayText(const QHash<QString, MetricValue>& latestValues) const = 0;
    virtual QString tooltip(const QHash<QString, MetricValue>& latestValues) const = 0;
    virtual bool isEnabledByDefault() const
    {
        return true;
    }
    virtual TaskbarDetailContent detailContent(
        const QHash<QString, MetricValue>& latestValues) const = 0;
};

} // namespace Vitals
