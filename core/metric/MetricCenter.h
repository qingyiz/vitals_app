#pragma once

#include "IMetricSink.h"

#include <QHash>
#include <QObject>
#include <QString>

namespace Vitals {

/**
 * \if ENGLISH
 * @brief Central in-memory metric hub used by the Vitals host
 *
 * Receives metric frames from plugins through IMetricSink, normalizes delivery
 * onto its owning thread, caches the latest value per metric key, and emits
 * UI-friendly Qt signals for dashboards and host platform integrations.
 * \endif
 *
 * \if CHINESE
 * @brief Vitals 宿主使用的内存型指标中心
 *
 * 通过 IMetricSink 接收插件上报的指标帧，将数据投递到自身所属线程，
 * 按 metric key 缓存最新值，并向 Dashboard 与宿主平台集成层发出适合 UI
 * 消费的 Qt 信号。
 * \endif
 */
class MetricCenter : public QObject, public IMetricSink
{
    Q_OBJECT

public:
    explicit MetricCenter(QObject* parent = nullptr);

    /**
     * \if ENGLISH
     * @brief Publishes a metric frame into the host metric pipeline
     *
     * This entry point may be called from plugin worker threads.
     * \endif
     *
     * \if CHINESE
     * @brief 将一帧指标数据发布到宿主指标管线中
     *
     * 该入口允许从插件工作线程中调用。
     * \endif
     */
    void publishFrame(const MetricFrame& frame) override;

    /// Returns whether the latest-value cache already contains the key.
    bool hasMetric(const QString& key) const;

    /// Returns the latest cached value for a metric key.
    MetricValue latestValue(const QString& key) const;

    /// Returns all latest cached metric values.
    QList<MetricValue> latestValues() const;

Q_SIGNALS:
    void framePublished(const Vitals::MetricFrame& frame);
    void metricUpdated(const Vitals::MetricValue& value);

private:
    void publishFrameOnOwnerThread(const MetricFrame& frame);

    QHash<QString, MetricValue> m_latestValues;
};

} // namespace Vitals
