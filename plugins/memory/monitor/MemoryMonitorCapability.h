#pragma once

#include "IMemoryCollector.h"
#include "IMonitorCapability.h"

#include <QObject>
#include <memory>

class QTimer;

namespace Vitals {

class IAppContext;

class MemoryMonitorCapability : public QObject, public IMonitorCapability
{
    Q_OBJECT

public:
    explicit MemoryMonitorCapability(QObject* parent = nullptr);
    ~MemoryMonitorCapability() override;

    bool initialize(IAppContext* context);

    QList<MetricDescriptor> metricDescriptors() const override;
    int defaultIntervalMs() const override;
    void setIntervalMs(int intervalMs) override;
    void startMonitoring() override;
    void stopMonitoring() override;

    int intervalMs() const;
    const MemorySnapshot& lastSnapshot() const;

Q_SIGNALS:
    void snapshotUpdated(const Vitals::MemorySnapshot& snapshot);

private:
    void collectAndPublish();
    void publishSnapshot(const MemorySnapshot& snapshot) const;

    IAppContext* m_context = nullptr;
    std::unique_ptr<IMemoryCollector> m_collector;
    QTimer* m_timer = nullptr;
    int m_intervalMs = 2000;
    MemorySnapshot m_lastSnapshot;
};

} // namespace Vitals
