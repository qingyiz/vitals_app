#include "SystemInfoPlugin.h"

#include "IAppContext.h"
#include "IMetricSink.h"
#include "SystemInfoCollectorFactory.h"
#include "SystemInfoPanelWidget.h"

#include <QDateTime>
#include <QTimer>

namespace Vitals {

namespace {

QString formatMemoryCompact(qint64 bytes)
{
    const double gib = static_cast<double>(bytes) / 1024.0 / 1024.0 / 1024.0;
    return QStringLiteral("%1G").arg(gib, 0, 'f', gib >= 10.0 ? 0 : 1);
}

QString formatUptimeCompact(qint64 seconds)
{
    const qint64 days = seconds / 86400;
    if (days > 0) {
        return QStringLiteral("%1d").arg(days);
    }

    const qint64 hours = seconds / 3600;
    if (hours > 0) {
        return QStringLiteral("%1h").arg(hours);
    }

    return QStringLiteral("%1m").arg(seconds / 60);
}

QString formatUptimeVerbose(qint64 seconds)
{
    const qint64 days = seconds / 86400;
    const qint64 hours = (seconds % 86400) / 3600;
    const qint64 minutes = (seconds % 3600) / 60;
    return QStringLiteral("%1d %2h %3m").arg(days).arg(hours).arg(minutes);
}

} // namespace

SystemInfoPlugin::SystemInfoPlugin(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(m_intervalMs);
    connect(m_timer, &QTimer::timeout, this, &SystemInfoPlugin::collectAndPublish);
}

SystemInfoPlugin::~SystemInfoPlugin() = default;

PluginMetaInfo SystemInfoPlugin::metaInfo() const
{
    return {
        QStringLiteral("com.vitals.systeminfo"),
        QStringLiteral("System Information"),
        QStringLiteral("macOS system identity and host information plugin."),
        QStringLiteral("0.1.0"),
        QStringLiteral("Vitals"),
        QStringLiteral("monitor"),
        {QStringLiteral("macos")},
        QStringLiteral("0.1.0"),
        true
    };
}

bool SystemInfoPlugin::initialize(IAppContext* context)
{
    m_context = context;
    m_collector = SystemInfoCollectorFactory::create();
    return m_context != nullptr && m_collector != nullptr;
}

void SystemInfoPlugin::start()
{
    collectAndPublish();
    m_timer->start();
}

void SystemInfoPlugin::stop()
{
    m_timer->stop();
}

void SystemInfoPlugin::shutdown()
{
    m_timer->stop();
    m_collector.reset();
    m_panel = nullptr;
    m_context = nullptr;
}

QList<MetricDescriptor> SystemInfoPlugin::metricDescriptors() const
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

int SystemInfoPlugin::defaultIntervalMs() const
{
    return 5000;
}

void SystemInfoPlugin::setIntervalMs(int intervalMs)
{
    m_intervalMs = qMax(1000, intervalMs);
    m_timer->setInterval(m_intervalMs);
}

QString SystemInfoPlugin::panelId() const
{
    return QStringLiteral("systeminfo");
}

QString SystemInfoPlugin::panelName() const
{
    return QStringLiteral("System Info");
}

QString SystemInfoPlugin::panelIconKey() const
{
    return QStringLiteral("system");
}

QWidget* SystemInfoPlugin::createPanel(QWidget* parent)
{
    auto* panel = new SystemInfoPanelWidget(parent);
    panel->applySnapshot(m_lastSnapshot);
    m_panel = panel;
    return panel;
}

QString SystemInfoPlugin::taskbarDisplayText(const QHash<QString, MetricValue>& latestValues) const
{
    const MetricValue memoryValue = latestValues.value(QStringLiteral("system.memory.total.bytes"));
    const MetricValue uptimeValue = latestValues.value(QStringLiteral("system.uptime.seconds"));
    if (!memoryValue.value.isValid() || !uptimeValue.value.isValid()) {
        return QString();
    }

    return QStringLiteral("%1 %2")
        .arg(formatMemoryCompact(memoryValue.value.toLongLong()))
        .arg(formatUptimeCompact(uptimeValue.value.toLongLong()));
}

QString SystemInfoPlugin::taskbarDisplayTooltip(const QHash<QString, MetricValue>& latestValues) const
{
    const QString device = latestValues.value(QStringLiteral("system.device.name")).value.toString();
    const QString os = latestValues.value(QStringLiteral("system.os.version")).value.toString();
    const QString cpu = latestValues.value(QStringLiteral("system.cpu.model")).value.toString();
    const QString gpu = latestValues.value(QStringLiteral("system.gpu.model")).value.toString();
    const qint64 memory = latestValues.value(QStringLiteral("system.memory.total.bytes")).value.toLongLong();
    const qint64 uptime = latestValues.value(QStringLiteral("system.uptime.seconds")).value.toLongLong();

    QStringList parts;
    if (!device.isEmpty()) parts.append(device);
    if (!os.isEmpty()) parts.append(os);
    if (!cpu.isEmpty()) parts.append(cpu);
    if (!gpu.isEmpty()) parts.append(gpu);
    if (memory > 0) parts.append(formatMemoryCompact(memory));
    if (uptime > 0) parts.append(formatUptimeVerbose(uptime));
    return parts.join(QStringLiteral(" | "));
}

bool SystemInfoPlugin::isTaskbarDisplayEnabledByDefault() const
{
    return true;
}

void SystemInfoPlugin::collectAndPublish()
{
    if (!m_collector) {
        return;
    }

    m_lastSnapshot = m_collector->collect();
    publishSnapshot(m_lastSnapshot);

    if (m_panel) {
        m_panel->applySnapshot(m_lastSnapshot);
    }
}

void SystemInfoPlugin::publishSnapshot(const SystemInfoSnapshot& snapshot) const
{
    if (!m_context || !m_context->metricSink()) {
        return;
    }

    MetricFrame frame;
    frame.pluginId = metaInfo().id;
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
