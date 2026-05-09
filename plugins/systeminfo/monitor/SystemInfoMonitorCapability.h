#pragma once

#include "IMonitorCapability.h"
#include "ISystemInfoCollector.h"

#include <QObject>
#include <memory>

class QTimer;

namespace Vitals {

class IAppContext;

class SystemInfoMonitorCapability : public QObject, public IMonitorCapability
{
    Q_OBJECT

public:
    explicit SystemInfoMonitorCapability(QObject* parent = nullptr);
    ~SystemInfoMonitorCapability() override;

    bool initialize(IAppContext* context);

    QList<MetricDescriptor> metricDescriptors() const override;
    int defaultIntervalMs() const override;
    void setIntervalMs(int intervalMs) override;
    void startMonitoring() override;
    void stopMonitoring() override;

    int intervalMs() const;
    const SystemInfoSnapshot& lastSnapshot() const;

Q_SIGNALS:
    void snapshotUpdated(const Vitals::SystemInfoSnapshot& snapshot);

private:
    void collectAndPublish();
    void publishSnapshot(const SystemInfoSnapshot& snapshot) const;

    IAppContext* m_context = nullptr;
    std::unique_ptr<ISystemInfoCollector> m_collector;
    QTimer* m_timer = nullptr;
    int m_intervalMs = 5000;
    SystemInfoSnapshot m_lastSnapshot;
};

} // namespace Vitals
