#pragma once

#include "ITaskbarCapability.h"

namespace Vitals {

class NetworkMonitorCapability;
class IAppContext;

class NetworkTaskbarCapability : public ITaskbarCapability
{
public:
    NetworkTaskbarCapability(const NetworkMonitorCapability* monitorCapability, IAppContext* context);

    QString displayText(const QHash<QString, MetricValue>& latestValues) const override;
    QString tooltip(const QHash<QString, MetricValue>& latestValues) const override;
    bool isEnabledByDefault() const override;
    QList<TaskbarLabelDescriptor> taskbarLabelDescriptors() const override;
    TaskbarDetailContent detailContent(const QHash<QString, MetricValue>& latestValues) const override;

private:
    QString text(const QString& key, const QString& fallback) const;

    const NetworkMonitorCapability* m_monitorCapability = nullptr;
    IAppContext* m_context = nullptr;
};

} // namespace Vitals
