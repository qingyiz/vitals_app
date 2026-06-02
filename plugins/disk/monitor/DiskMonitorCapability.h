#pragma once

#include "IDiskCollector.h"
#include "IMonitorCapability.h"

#include <QObject>
#include <memory>

class QTimer;

namespace Vitals {

class IAppContext;

/**
 * \if ENGLISH
 * @brief Monitoring capability for mounted disk capacity and selection state
 *
 * Owns the disk collector, periodically samples mounted volumes, publishes
 * normalized disk metrics, and persists the user's selected mounted root path.
 * \endif
 *
 * \if CHINESE
 * @brief 负责已挂载磁盘容量与选择状态的监控能力
 *
 * 持有磁盘采集器，周期性采样已挂载卷，发布统一磁盘指标，并持久化用户
 * 当前选择的挂载根路径。
 * \endif
 */
class DiskMonitorCapability : public QObject, public IMonitorCapability
{
    Q_OBJECT

public:
    explicit DiskMonitorCapability(QObject* parent = nullptr);
    ~DiskMonitorCapability() override;

    bool initialize(IAppContext* context);

    QList<MetricDescriptor> metricDescriptors() const override;
    int defaultIntervalMs() const override;
    void setIntervalMs(int intervalMs) override;
    void startMonitoring() override;
    void stopMonitoring() override;

    int intervalMs() const;
    const DiskSnapshot& lastSnapshot() const;
    QString selectedRootPath() const;

public Q_SLOTS:
    void setSelectedRootPath(const QString& rootPath);
    void refreshNow();

Q_SIGNALS:
    void snapshotUpdated(const DiskSnapshot& snapshot);

private:
    void collectAndPublish();
    void publishSnapshot(const DiskSnapshot& snapshot) const;
    void loadConfig();
    void saveConfig() const;

    IAppContext* m_context = nullptr;
    std::unique_ptr<IDiskCollector> m_collector;
    QTimer* m_timer = nullptr;
    int m_intervalMs = 2000;
    QString m_selectedRootPath;
    DiskSnapshot m_lastSnapshot;
};

} // namespace Vitals
