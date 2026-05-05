#include "metric/MetricCenter.h"

#include <QMetaObject>
#include <QThread>

namespace Vitals {

MetricCenter::MetricCenter(QObject* parent)
    : QObject(parent)
{
}

void MetricCenter::publishFrame(const MetricFrame& frame)
{
    if (QThread::currentThread() == thread()) {
        publishFrameOnOwnerThread(frame);
        return;
    }

    QMetaObject::invokeMethod(this, [this, frame]() {
        publishFrameOnOwnerThread(frame);
    }, Qt::QueuedConnection);
}

void MetricCenter::removePluginMetrics(const QString& pluginId)
{
    if (QThread::currentThread() == thread()) {
        removePluginMetricsOnOwnerThread(pluginId);
        return;
    }

    QMetaObject::invokeMethod(this, [this, pluginId]() {
        removePluginMetricsOnOwnerThread(pluginId);
    }, Qt::QueuedConnection);
}

bool MetricCenter::hasMetric(const QString& key) const
{
    return m_latestValues.contains(key);
}

MetricValue MetricCenter::latestValue(const QString& key) const
{
    return m_latestValues.value(key);
}

QList<MetricValue> MetricCenter::latestValues() const
{
    return m_latestValues.values();
}

void MetricCenter::publishFrameOnOwnerThread(const MetricFrame& frame)
{
    for (MetricValue value : frame.values) {
        if (!value.timestamp.isValid()) {
            value.timestamp = frame.timestamp;
        }
        m_latestValues.insert(value.key, value);
        m_metricOwners.insert(value.key, frame.pluginId);
        Q_EMIT metricUpdated(value);
    }

    Q_EMIT framePublished(frame);
}

void MetricCenter::removePluginMetricsOnOwnerThread(const QString& pluginId)
{
    QStringList keysToRemove;
    for (auto it = m_metricOwners.cbegin(); it != m_metricOwners.cend(); ++it) {
        if (it.value() == pluginId) {
            keysToRemove.append(it.key());
        }
    }

    for (const QString& key : keysToRemove) {
        m_metricOwners.remove(key);
        m_latestValues.remove(key);
        Q_EMIT metricRemoved(key);
    }
}

} // namespace Vitals
