#pragma once

#include "ITaskbarCapability.h"

namespace Vitals {

class MemoryMonitorCapability;

class MemoryTaskbarCapability : public ITaskbarCapability
{
public:
    explicit MemoryTaskbarCapability(const MemoryMonitorCapability* monitorCapability);

    QString displayText(const QHash<QString, MetricValue>& latestValues) const override;
    QString tooltip(const QHash<QString, MetricValue>& latestValues) const override;
    bool isEnabledByDefault() const override;
    TaskbarDetailContent detailContent(const QHash<QString, MetricValue>& latestValues) const override;

private:
    const MemoryMonitorCapability* m_monitorCapability = nullptr;
};

} // namespace Vitals
