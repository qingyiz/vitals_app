#include "monitor/MemoryMonitorCapability.h"

#include "IAppContext.h"
#include "IMetricSink.h"
#include "MemoryCollectorFactory.h"

#include <QDateTime>
#include <QTimer>

namespace Vitals {

MemoryMonitorCapability::MemoryMonitorCapability(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(m_intervalMs);
    connect(m_timer, &QTimer::timeout, this, &MemoryMonitorCapability::collectAndPublish);
}

MemoryMonitorCapability::~MemoryMonitorCapability() = default;

bool MemoryMonitorCapability::initialize(IAppContext* context)
{
    m_context = context;
    m_collector = MemoryCollectorFactory::create();
    if (!m_context || !m_collector) {
        return false;
    }

    return m_collector->initialize();
}

QList<MetricDescriptor> MemoryMonitorCapability::metricDescriptors() const
{
    return {
        {QStringLiteral("memory.total.bytes"), QStringLiteral("Total Memory"),
            QStringLiteral("Installed physical memory in bytes."), QStringLiteral("B"), MetricValueType::Bytes},
        {QStringLiteral("memory.used.bytes"), QStringLiteral("Used Memory"),
            QStringLiteral("Physical memory currently in use."), QStringLiteral("B"), MetricValueType::Bytes},
        {QStringLiteral("memory.free.bytes"), QStringLiteral("Available Memory"),
            QStringLiteral("Physical memory currently available."), QStringLiteral("B"), MetricValueType::Bytes},
        {QStringLiteral("memory.usage.percent"), QStringLiteral("Memory Usage"),
            QStringLiteral("Percentage of physical memory currently in use."), QStringLiteral("%"), MetricValueType::Percentage}
    };
}

int MemoryMonitorCapability::defaultIntervalMs() const
{
    return 2000;
}

void MemoryMonitorCapability::setIntervalMs(int intervalMs)
{
    m_intervalMs = qMax(500, intervalMs);
    m_timer->setInterval(m_intervalMs);
}

void MemoryMonitorCapability::startMonitoring()
{
    collectAndPublish();
    m_timer->start();
}

void MemoryMonitorCapability::stopMonitoring()
{
    m_timer->stop();
}

int MemoryMonitorCapability::intervalMs() const
{
    return m_intervalMs;
}

const MemorySnapshot& MemoryMonitorCapability::lastSnapshot() const
{
    return m_lastSnapshot;
}

void MemoryMonitorCapability::collectAndPublish()
{
    if (!m_collector) {
        return;
    }

    m_lastSnapshot = m_collector->collect();
    publishSnapshot(m_lastSnapshot);
    Q_EMIT snapshotUpdated(m_lastSnapshot);
}

void MemoryMonitorCapability::publishSnapshot(const MemorySnapshot& snapshot) const
{
    if (!m_context || !m_context->metricSink()) {
        return;
    }

    MetricFrame frame;
    frame.pluginId = QStringLiteral("com.vitals.memory");
    frame.timestamp = QDateTime::currentDateTime();
    frame.values = {
        {QStringLiteral("memory.total.bytes"), QVariant::fromValue<qulonglong>(snapshot.totalBytes), frame.timestamp, {}},
        {QStringLiteral("memory.used.bytes"), QVariant::fromValue<qulonglong>(snapshot.usedBytes), frame.timestamp, {}},
        {QStringLiteral("memory.free.bytes"), QVariant::fromValue<qulonglong>(snapshot.freeBytes), frame.timestamp, {}},
        {QStringLiteral("memory.usage.percent"), snapshot.usagePercent, frame.timestamp, {}}
    };
    m_context->metricSink()->publishFrame(frame);
}

} // namespace Vitals
