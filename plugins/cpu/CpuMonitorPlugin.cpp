#include "CpuMonitorPlugin.h"

#include "CpuCollectorFactory.h"
#include "CpuPanelWidget.h"
#include "IAppContext.h"
#include "IMetricSink.h"

#include <QDateTime>
#include <QTimer>

namespace Vitals {

namespace {

QString formatPercentCompact(double value)
{
    const double clamped = qBound(0.0, value, 100.0);
    return QStringLiteral("%1%").arg(clamped, 0, 'f', clamped >= 10.0 ? 0 : 1);
}

} // namespace

CpuMonitorPlugin::CpuMonitorPlugin(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(m_intervalMs);
    connect(m_timer, &QTimer::timeout, this, &CpuMonitorPlugin::collectAndPublish);
}

CpuMonitorPlugin::~CpuMonitorPlugin() = default;

PluginMetaInfo CpuMonitorPlugin::metaInfo() const
{
    return {
        QStringLiteral("com.vitals.cpu"),
        QStringLiteral("CPU Monitor"),
        QStringLiteral("Live CPU usage monitor plugin for macOS."),
        QStringLiteral("0.1.0"),
        QStringLiteral("Vitals"),
        QStringLiteral("monitor"),
        {QStringLiteral("macos")},
        QStringLiteral("0.1.0"),
        true
    };
}

bool CpuMonitorPlugin::initialize(IAppContext* context)
{
    m_context = context;
    m_collector = CpuCollectorFactory::create();
    if (!m_context || !m_collector) {
        return false;
    }

    return m_collector->initialize();
}

void CpuMonitorPlugin::start()
{
    collectAndPublish();
    m_timer->start();
}

void CpuMonitorPlugin::stop()
{
    m_timer->stop();
}

void CpuMonitorPlugin::shutdown()
{
    m_timer->stop();
    m_collector.reset();
    m_panel = nullptr;
    m_context = nullptr;
}

QList<MetricDescriptor> CpuMonitorPlugin::metricDescriptors() const
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

int CpuMonitorPlugin::defaultIntervalMs() const
{
    return 2000;
}

void CpuMonitorPlugin::setIntervalMs(int intervalMs)
{
    m_intervalMs = qMax(500, intervalMs);
    m_timer->setInterval(m_intervalMs);
}

QString CpuMonitorPlugin::panelId() const
{
    return QStringLiteral("cpu");
}

QString CpuMonitorPlugin::panelName() const
{
    return QStringLiteral("CPU Monitor");
}

QString CpuMonitorPlugin::panelIconKey() const
{
    return QStringLiteral("cpu");
}

QWidget* CpuMonitorPlugin::createPanel(QWidget* parent)
{
    auto* panel = new CpuPanelWidget(parent);
    panel->applySnapshot(m_lastSnapshot);
    m_panel = panel;
    return panel;
}

QString CpuMonitorPlugin::taskbarDisplayText(const QHash<QString, MetricValue>& latestValues) const
{
    const MetricValue usageValue = latestValues.value(QStringLiteral("cpu.usage.total"));
    if (!usageValue.value.isValid()) {
        return QString();
    }

    return QStringLiteral("CPU\n%1").arg(formatPercentCompact(usageValue.value.toDouble()));
}

QString CpuMonitorPlugin::taskbarDisplayTooltip(const QHash<QString, MetricValue>& latestValues) const
{
    const QString model = latestValues.value(QStringLiteral("cpu.model")).value.toString();
    const int coreCount = latestValues.value(QStringLiteral("cpu.logical.cores")).value.toInt();
    const double totalUsage = latestValues.value(QStringLiteral("cpu.usage.total")).value.toDouble();

    int busiestCoreIndex = -1;
    double busiestCoreUsage = 0.0;
    for (int index = 0; index < coreCount; ++index) {
        const double coreUsage = latestValues.value(QStringLiteral("cpu.usage.core%1").arg(index)).value.toDouble();
        if (index == 0 || coreUsage > busiestCoreUsage) {
            busiestCoreUsage = coreUsage;
            busiestCoreIndex = index;
        }
    }

    QStringList parts;
    if (!model.isEmpty()) parts.append(model);
    if (coreCount > 0) parts.append(QStringLiteral("%1 logical cores").arg(coreCount));
    parts.append(QStringLiteral("Total %1").arg(formatPercentCompact(totalUsage)));
    if (busiestCoreIndex >= 0) {
        parts.append(QStringLiteral("Peak Core %1 %2")
                         .arg(busiestCoreIndex + 1)
                         .arg(formatPercentCompact(busiestCoreUsage)));
    }
    return parts.join(QStringLiteral(" | "));
}

bool CpuMonitorPlugin::isTaskbarDisplayEnabledByDefault() const
{
    return true;
}

void CpuMonitorPlugin::collectAndPublish()
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

void CpuMonitorPlugin::publishSnapshot(const CpuSnapshot& snapshot) const
{
    if (!m_context || !m_context->metricSink()) {
        return;
    }

    MetricFrame frame;
    frame.pluginId = metaInfo().id;
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
