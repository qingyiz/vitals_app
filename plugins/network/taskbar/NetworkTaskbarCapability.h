#pragma once

#include "ITaskbarCapability.h"

namespace Vitals {

class NetworkMonitorCapability;

class NetworkTaskbarCapability : public ITaskbarCapability
{
public:
    explicit NetworkTaskbarCapability(const NetworkMonitorCapability* monitorCapability);

    QString displayText(const QHash<QString, MetricValue>& latestValues) const override;
    QString tooltip(const QHash<QString, MetricValue>& latestValues) const override;
    bool isEnabledByDefault() const override;
    TaskbarDetailContent detailContent(const QHash<QString, MetricValue>& latestValues) const override;

private:
    const NetworkMonitorCapability* m_monitorCapability = nullptr;
};

} // namespace Vitals
