#include "SystemInfoPlugin.h"

#include "IAppContext.h"
#include "IMetricSink.h"
#include "SystemInfoCollectorFactory.h"
#include "SystemInfoPanelWidget.h"

#include <QDateTime>
#include <QTimer>

namespace Vitals {

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
        QStringLiteral("0.1.0")
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
