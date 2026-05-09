#pragma once

#include "IMonitorCapability.h"
#include "INetworkCollector.h"

#include <QObject>
#include <memory>

class QTimer;

namespace Vitals {

class IAppContext;

class NetworkMonitorCapability : public QObject, public IMonitorCapability
{
    Q_OBJECT

public:
    explicit NetworkMonitorCapability(QObject* parent = nullptr);
    ~NetworkMonitorCapability() override;

    bool initialize(IAppContext* context);

    QList<MetricDescriptor> metricDescriptors() const override;
    int defaultIntervalMs() const override;
    void setIntervalMs(int intervalMs) override;
    void startMonitoring() override;
    void stopMonitoring() override;

    int intervalMs() const;
    const NetworkSnapshot& lastSnapshot() const;

Q_SIGNALS:
    void snapshotUpdated(const Vitals::NetworkSnapshot& snapshot);

private:
    void collectAndPublish();
    void publishSnapshot(const NetworkSnapshot& snapshot) const;

    IAppContext* m_context = nullptr;
    std::unique_ptr<INetworkCollector> m_collector;
    QTimer* m_timer = nullptr;
    int m_intervalMs = 2000;
    NetworkSnapshot m_lastSnapshot;
};

} // namespace Vitals
