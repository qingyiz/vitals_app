#pragma once

#include "ITaskbarCapability.h"

namespace Vitals {

class CpuMonitorCapability;
class IAppContext;

class CpuTaskbarCapability : public ITaskbarCapability
{
public:
    CpuTaskbarCapability(const CpuMonitorCapability* monitorCapability, IAppContext* context);

    QString displayText(const QHash<QString, MetricValue>& latestValues) const override;
    QString tooltip(const QHash<QString, MetricValue>& latestValues) const override;
    bool isEnabledByDefault() const override;
    bool supportsCustomTaskbarLabel() const override;
    QString defaultTaskbarLabel() const override;
    TaskbarDetailContent detailContent(const QHash<QString, MetricValue>& latestValues) const override;

private:
    QString text(const QString& key, const QString& fallback) const;

    const CpuMonitorCapability* m_monitorCapability = nullptr;
    IAppContext* m_context = nullptr;
};

} // namespace Vitals
