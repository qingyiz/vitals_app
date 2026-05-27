#pragma once

#include "ITaskbarCapability.h"

namespace Vitals {

class SystemInfoMonitorCapability;
class IAppContext;

class SystemInfoTaskbarCapability : public ITaskbarCapability
{
public:
    SystemInfoTaskbarCapability(const SystemInfoMonitorCapability* monitorCapability, IAppContext* context);

    QString displayText(const QHash<QString, MetricValue>& latestValues) const override;
    QString tooltip(const QHash<QString, MetricValue>& latestValues) const override;
    bool isEnabledByDefault() const override;
    TaskbarDetailContent detailContent(const QHash<QString, MetricValue>& latestValues) const override;

private:
    QString text(const QString& key, const QString& fallback) const;

    const SystemInfoMonitorCapability* m_monitorCapability = nullptr;
    IAppContext* m_context = nullptr;
};

} // namespace Vitals
