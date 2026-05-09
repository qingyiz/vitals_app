#include "monitor/CpuMonitorCapability.h"

#include "CpuCollectorFactory.h"
#include "IAppContext.h"
#include "IMetricSink.h"

#include <QDateTime>
#include <QTimer>

namespace Vitals {

CpuMonitorCapability::CpuMonitorCapability(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(m_intervalMs);
    connect(m_timer, &QTimer::timeout, this, &CpuMonitorCapability::collectAndPublish);
}

CpuMonitorCapability::~CpuMonitorCapability() = default;

bool CpuMonitorCapability::initialize(IAppContext* context)
{
    m_context = context;
    m_collector = CpuCollectorFactory::create();
    if (!m_context || !m_collector) {
        return false;
    }

    return m_collector->initialize();
}

QList<MetricDescriptor> CpuMonitorCapability::metricDescriptors() const
{
    QList<MetricDescriptor> descriptors = {
        {QStringLiteral("cpu.model"), QStringLiteral("CPU Model"),
            QStringLiteral("Human-readable processor model string."), QString(), MetricValueType::String},
        {QStringLiteral("cpu.logical.cores"), QStringLiteral("Logical Cores"),
            QStringLiteral("Logical processor count reported by the current platform."), QString(), MetricValueType::Integer},
        {QStringLiteral("cpu.usage.total"), QStringLiteral("Total CPU Usage"),
            QStringLiteral("Average CPU utilization across logical cores."), QStringLiteral("%"), MetricValueType::Percentage}
    };

    const int logicalCoreCount = m_collector ? m_collector->logicalCoreCount() : m_lastSnapshot.logicalCoreCount;
    for (int index = 0; index < logicalCoreCount; ++index) {
        descriptors.append({
            QStringLiteral("cpu.usage.core%1").arg(index),
            QStringLiteral("CPU Core %1 Usage").arg(index + 1),
            QStringLiteral("Utilization for logical CPU core %1.").arg(index + 1),
            QStringLiteral("%"),
            MetricValueType::Percentage
        });
    }

    return descriptors;
}

int CpuMonitorCapability::defaultIntervalMs() const
{
    return 2000;
}

void CpuMonitorCapability::setIntervalMs(int intervalMs)
{
    m_intervalMs = qMax(500, intervalMs);
    m_timer->setInterval(m_intervalMs);
}

void CpuMonitorCapability::startMonitoring()
{
    collectAndPublish();
    m_timer->start();
}

void CpuMonitorCapability::stopMonitoring()
{
    m_timer->stop();
}

int CpuMonitorCapability::intervalMs() const
{
    return m_intervalMs;
}

const CpuSnapshot& CpuMonitorCapability::lastSnapshot() const
{
    return m_lastSnapshot;
}

void CpuMonitorCapability::collectAndPublish()
{
    if (!m_collector) {
        return;
    }

    m_lastSnapshot = m_collector->collect();
    publishSnapshot(m_lastSnapshot);
    Q_EMIT snapshotUpdated(m_lastSnapshot);
}

void CpuMonitorCapability::publishSnapshot(const CpuSnapshot& snapshot) const
{
    if (!m_context || !m_context->metricSink()) {
        return;
    }

    MetricFrame frame;
    frame.pluginId = QStringLiteral("com.vitals.cpu");
    frame.timestamp = QDateTime::currentDateTime();
    frame.values = {
        {QStringLiteral("cpu.model"), snapshot.cpuName, frame.timestamp, {}},
        {QStringLiteral("cpu.logical.cores"), snapshot.logicalCoreCount, frame.timestamp, {}},
        {QStringLiteral("cpu.usage.total"), snapshot.totalUsagePercent, frame.timestamp, {}}
    };

    for (int index = 0; index < snapshot.perCoreUsagePercent.size(); ++index) {
        frame.values.append({
            QStringLiteral("cpu.usage.core%1").arg(index),
            snapshot.perCoreUsagePercent.at(index),
            frame.timestamp,
            {}
        });
    }

    m_context->metricSink()->publishFrame(frame);
}

} // namespace Vitals
