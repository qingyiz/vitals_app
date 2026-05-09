#pragma once

#include "ICpuCollector.h"
#include "IMonitorCapability.h"

#include <QObject>
#include <memory>

class QTimer;

namespace Vitals {

class IAppContext;

class CpuMonitorCapability : public QObject, public IMonitorCapability
{
    Q_OBJECT

public:
    explicit CpuMonitorCapability(QObject* parent = nullptr);
    ~CpuMonitorCapability() override;

    bool initialize(IAppContext* context);

    QList<MetricDescriptor> metricDescriptors() const override;
    int defaultIntervalMs() const override;
    void setIntervalMs(int intervalMs) override;
    void startMonitoring() override;
    void stopMonitoring() override;

    int intervalMs() const;
    const CpuSnapshot& lastSnapshot() const;

Q_SIGNALS:
    void snapshotUpdated(const Vitals::CpuSnapshot& snapshot);

private:
    void collectAndPublish();
    void publishSnapshot(const CpuSnapshot& snapshot) const;

    IAppContext* m_context = nullptr;
    std::unique_ptr<ICpuCollector> m_collector;
    QTimer* m_timer = nullptr;
    int m_intervalMs = 2000;
    CpuSnapshot m_lastSnapshot;
};

} // namespace Vitals
