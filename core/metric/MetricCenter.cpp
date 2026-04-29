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
        Q_EMIT metricUpdated(value);
    }

    Q_EMIT framePublished(frame);
}

} // namespace Vitals

