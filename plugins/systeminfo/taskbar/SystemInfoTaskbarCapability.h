#pragma once

#include "ITaskbarCapability.h"

namespace Vitals {

class SystemInfoMonitorCapability;

class SystemInfoTaskbarCapability : public ITaskbarCapability
{
public:
    explicit SystemInfoTaskbarCapability(const SystemInfoMonitorCapability* monitorCapability);

    QString displayText(const QHash<QString, MetricValue>& latestValues) const override;
    QString tooltip(const QHash<QString, MetricValue>& latestValues) const override;
    bool isEnabledByDefault() const override;
    TaskbarDetailContent detailContent(const QHash<QString, MetricValue>& latestValues) const override;

private:
    const SystemInfoMonitorCapability* m_monitorCapability = nullptr;
};

} // namespace Vitals
