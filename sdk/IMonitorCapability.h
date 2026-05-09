#pragma once

#include "MetricData.h"

namespace Vitals {

class IMonitorCapability
{
public:
    virtual ~IMonitorCapability() = default;

    virtual QList<MetricDescriptor> metricDescriptors() const = 0;
    virtual int defaultIntervalMs() const = 0;
    virtual void setIntervalMs(int intervalMs) = 0;
    virtual void startMonitoring() = 0;
    virtual void stopMonitoring() = 0;
};

} // namespace Vitals
