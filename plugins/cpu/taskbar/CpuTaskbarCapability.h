#pragma once

#include "ITaskbarCapability.h"

namespace Vitals {

class CpuMonitorCapability;

class CpuTaskbarCapability : public ITaskbarCapability
{
public:
    explicit CpuTaskbarCapability(const CpuMonitorCapability* monitorCapability);

    QString displayText(const QHash<QString, MetricValue>& latestValues) const override;
    QString tooltip(const QHash<QString, MetricValue>& latestValues) const override;
    bool isEnabledByDefault() const override;
    TaskbarDetailContent detailContent(const QHash<QString, MetricValue>& latestValues) const override;

private:
    const CpuMonitorCapability* m_monitorCapability = nullptr;
};

} // namespace Vitals
