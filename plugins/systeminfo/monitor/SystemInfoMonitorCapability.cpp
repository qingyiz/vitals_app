#include "monitor/SystemInfoMonitorCapability.h"

#include "IAppContext.h"
#include "IMetricSink.h"
#include "SystemInfoCollectorFactory.h"

#include <QDateTime>
#include <QTimer>

namespace Vitals {

SystemInfoMonitorCapability::SystemInfoMonitorCapability(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(m_intervalMs);
    connect(m_timer, &QTimer::timeout, this, &SystemInfoMonitorCapability::collectAndPublish);
}

SystemInfoMonitorCapability::~SystemInfoMonitorCapability() = default;

bool SystemInfoMonitorCapability::initialize(IAppContext* context)
{
    m_context = context;
    m_collector = SystemInfoCollectorFactory::create();
    return m_context != nullptr && m_collector != nullptr;
}

QList<MetricDescriptor> SystemInfoMonitorCapability::metricDescriptors() const
{
    return {
        {QStringLiteral("system.device.name"), QStringLiteral("Device Name"),
            QStringLiteral("Host machine name reported by macOS."), QString(), MetricValueType::String},
        {QStringLiteral("system.os.version"), QStringLiteral("Operating System"),
            QStringLiteral("Human-readable macOS version."), QString(), MetricValueType::String},
        {QStringLiteral("system.cpu.model"), QStringLiteral("CPU Model"),
            QStringLiteral("Processor brand string."), QString(), MetricValueType::String},
        {QStringLiteral("system.gpu.model"), QStringLiteral("GPU Model"),
            QStringLiteral("Primary graphics device model."), QString(), MetricValueType::String},
        {QStringLiteral("system.memory.total.bytes"), QStringLiteral("Total Memory"),
            QStringLiteral("Installed physical memory in bytes."), QStringLiteral("B"), MetricValueType::Bytes},
        {QStringLiteral("system.uptime.seconds"), QStringLiteral("System Uptime"),
            QStringLiteral("Elapsed time since the last boot."), QStringLiteral("s"), MetricValueType::Integer}
    };
}

int SystemInfoMonitorCapability::defaultIntervalMs() const
{
    return 5000;
}

void SystemInfoMonitorCapability::setIntervalMs(int intervalMs)
{
    m_intervalMs = qMax(1000, intervalMs);
    m_timer->setInterval(m_intervalMs);
}

void SystemInfoMonitorCapability::startMonitoring()
{
    collectAndPublish();
    m_timer->start();
}

void SystemInfoMonitorCapability::stopMonitoring()
{
    m_timer->stop();
}

int SystemInfoMonitorCapability::intervalMs() const
{
    return m_intervalMs;
}

const SystemInfoSnapshot& SystemInfoMonitorCapability::lastSnapshot() const
{
    return m_lastSnapshot;
}

void SystemInfoMonitorCapability::collectAndPublish()
{
    if (!m_collector) {
        return;
    }

    m_lastSnapshot = m_collector->collect();
    publishSnapshot(m_lastSnapshot);
    Q_EMIT snapshotUpdated(m_lastSnapshot);
}

void SystemInfoMonitorCapability::publishSnapshot(const SystemInfoSnapshot& snapshot) const
{
    if (!m_context || !m_context->metricSink()) {
        return;
    }

    MetricFrame frame;
    frame.pluginId = QStringLiteral("com.vitals.systeminfo");
    frame.timestamp = QDateTime::currentDateTime();
    frame.values = {
        {QStringLiteral("system.device.name"), snapshot.deviceName, frame.timestamp, {}},
        {QStringLiteral("system.os.version"), snapshot.osVersion, frame.timestamp, {}},
        {QStringLiteral("system.cpu.model"), snapshot.cpuModel, frame.timestamp, {}},
        {QStringLiteral("system.gpu.model"), snapshot.gpuModel, frame.timestamp, {}},
        {QStringLiteral("system.memory.total.bytes"), QVariant::fromValue(snapshot.totalMemoryBytes), frame.timestamp, {}},
        {QStringLiteral("system.uptime.seconds"), QVariant::fromValue(snapshot.uptimeSeconds), frame.timestamp, {}}
    };
    m_context->metricSink()->publishFrame(frame);
}

} // namespace Vitals
