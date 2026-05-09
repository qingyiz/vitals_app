#include "NetworkMonitorPlugin.h"

#include "IAppContext.h"
#include "IMetricSink.h"
#include "NetworkCollectorFactory.h"
#include "NetworkPanelWidget.h"

#include <QDateTime>
#include <QTimer>

namespace Vitals {

namespace {

QString formatCompactRate(double bytesPerSecond)
{
    static const char* units[] = {"B/s", "KB/s", "MB/s", "GB/s"};

    double value = qMax(0.0, bytesPerSecond);
    int unitIndex = 0;
    while (value >= 1024.0 && unitIndex < 3) {
        value /= 1024.0;
        ++unitIndex;
    }

    return QStringLiteral("%1 %2")
        .arg(value, 0, 'f', unitIndex == 0 ? 0 : 1)
        .arg(QString::fromLatin1(units[unitIndex]));
}

QString formatCompactBytes(quint64 bytes)
{
    static const char* units[] = {"B", "KB", "MB", "GB", "TB"};

    double value = static_cast<double>(bytes);
    int unitIndex = 0;
    while (value >= 1024.0 && unitIndex < 4) {
        value /= 1024.0;
        ++unitIndex;
    }

    return QStringLiteral("%1 %2")
        .arg(value, 0, 'f', unitIndex == 0 ? 0 : 1)
        .arg(QString::fromLatin1(units[unitIndex]));
}

QString interfaceSummary(const QStringList& interfaces)
{
    if (interfaces.isEmpty()) {
        return QStringLiteral("No active interfaces");
    }

    return interfaces.join(QStringLiteral(", "));
}

} // namespace

NetworkMonitorPlugin::NetworkMonitorPlugin(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(m_intervalMs);
    connect(m_timer, &QTimer::timeout, this, &NetworkMonitorPlugin::collectAndPublish);
}

NetworkMonitorPlugin::~NetworkMonitorPlugin() = default;

PluginMetaInfo NetworkMonitorPlugin::metaInfo() const
{
    return {
        QStringLiteral("com.vitals.network"),
        QStringLiteral("Network Monitor"),
        QStringLiteral("Live network throughput and traffic counters for macOS."),
        QStringLiteral("0.1.0"),
        QStringLiteral("Vitals"),
        QStringLiteral("monitor"),
        {QStringLiteral("macos")},
        QStringLiteral("0.1.0"),
        true
    };
}

bool NetworkMonitorPlugin::initialize(IAppContext* context)
{
    m_context = context;
    m_collector = NetworkCollectorFactory::create();
    if (!m_context || !m_collector) {
        return false;
    }

    return m_collector->initialize();
}

void NetworkMonitorPlugin::start()
{
    collectAndPublish();
    m_timer->start();
}

void NetworkMonitorPlugin::stop()
{
    m_timer->stop();
}

void NetworkMonitorPlugin::shutdown()
{
    m_timer->stop();
    m_collector.reset();
    m_panel = nullptr;
    m_context = nullptr;
}

QList<MetricDescriptor> NetworkMonitorPlugin::metricDescriptors() const
{
    return {
        {QStringLiteral("network.interface.primary"), QStringLiteral("Primary Interface"),
            QStringLiteral("Most active non-loopback network interface detected in the latest sample."), QString(), MetricValueType::String},
        {QStringLiteral("network.interfaces.active"), QStringLiteral("Active Interfaces"),
            QStringLiteral("Comma-separated list of currently active non-loopback interfaces."), QString(), MetricValueType::String},
        {QStringLiteral("network.download.rate"), QStringLiteral("Download Rate"),
            QStringLiteral("Aggregate inbound throughput across active network interfaces."), QStringLiteral("B/s"), MetricValueType::BytesPerSecond},
        {QStringLiteral("network.upload.rate"), QStringLiteral("Upload Rate"),
            QStringLiteral("Aggregate outbound throughput across active network interfaces."), QStringLiteral("B/s"), MetricValueType::BytesPerSecond},
        {QStringLiteral("network.download.total.bytes"), QStringLiteral("Total Received"),
            QStringLiteral("Cumulative inbound byte counter across active network interfaces."), QStringLiteral("B"), MetricValueType::Bytes},
        {QStringLiteral("network.upload.total.bytes"), QStringLiteral("Total Sent"),
            QStringLiteral("Cumulative outbound byte counter across active network interfaces."), QStringLiteral("B"), MetricValueType::Bytes}
    };
}

int NetworkMonitorPlugin::defaultIntervalMs() const
{
    return 2000;
}

void NetworkMonitorPlugin::setIntervalMs(int intervalMs)
{
    m_intervalMs = qMax(500, intervalMs);
    m_timer->setInterval(m_intervalMs);
}

QString NetworkMonitorPlugin::panelId() const
{
    return QStringLiteral("network");
}

QString NetworkMonitorPlugin::panelName() const
{
    return QStringLiteral("Network Monitor");
}

QString NetworkMonitorPlugin::panelIconKey() const
{
    return QStringLiteral("network");
}

QWidget* NetworkMonitorPlugin::createPanel(QWidget* parent)
{
    auto* panel = new NetworkPanelWidget(parent);
    panel->applySnapshot(m_lastSnapshot);
    m_panel = panel;
    return panel;
}

QString NetworkMonitorPlugin::taskbarDisplayText(const QHash<QString, MetricValue>& latestValues) const
{
    const MetricValue downloadValue = latestValues.value(QStringLiteral("network.download.rate"));
    const MetricValue uploadValue = latestValues.value(QStringLiteral("network.upload.rate"));
    if (!downloadValue.value.isValid() || !uploadValue.value.isValid()) {
        return QString();
    }

    return QStringLiteral("↑：%1\n↓：%2")
        .arg(formatCompactRate(uploadValue.value.toDouble()))
        .arg(formatCompactRate(downloadValue.value.toDouble()));
}

QString NetworkMonitorPlugin::taskbarDisplayTooltip(const QHash<QString, MetricValue>& latestValues) const
{
    const QString primaryInterface = latestValues.value(QStringLiteral("network.interface.primary")).value.toString();
    const QString activeInterfaces = latestValues.value(QStringLiteral("network.interfaces.active")).value.toString();
    const quint64 totalReceived = latestValues.value(QStringLiteral("network.download.total.bytes")).value.toULongLong();
    const quint64 totalSent = latestValues.value(QStringLiteral("network.upload.total.bytes")).value.toULongLong();

    QStringList parts;
    if (!primaryInterface.isEmpty()) parts.append(QStringLiteral("Primary %1").arg(primaryInterface));
    if (!activeInterfaces.isEmpty()) parts.append(activeInterfaces);
    if (totalReceived > 0) parts.append(QStringLiteral("Down %1").arg(formatCompactBytes(totalReceived)));
    if (totalSent > 0) parts.append(QStringLiteral("Up %1").arg(formatCompactBytes(totalSent)));
    return parts.join(QStringLiteral(" | "));
}

bool NetworkMonitorPlugin::isTaskbarDisplayEnabledByDefault() const
{
    return true;
}

TaskbarDetailContent NetworkMonitorPlugin::taskbarDetailContent(const QHash<QString, MetricValue>& latestValues) const
{
    const QString primaryInterface = latestValues.value(QStringLiteral("network.interface.primary")).value.toString();
    const QString activeInterfaces = latestValues.value(QStringLiteral("network.interfaces.active")).value.toString();
    const double downloadRate = latestValues.value(QStringLiteral("network.download.rate")).value.toDouble();
    const double uploadRate = latestValues.value(QStringLiteral("network.upload.rate")).value.toDouble();
    const quint64 totalReceived = latestValues.value(QStringLiteral("network.download.total.bytes")).value.toULongLong();
    const quint64 totalSent = latestValues.value(QStringLiteral("network.upload.total.bytes")).value.toULongLong();

    if (primaryInterface.isEmpty() && activeInterfaces.isEmpty() && downloadRate <= 0.0
        && uploadRate <= 0.0 && totalReceived == 0 && totalSent == 0) {
        return {};
    }

    TaskbarDetailContent content;
    content.title = QStringLiteral("Network Monitor");
    content.subtitle = primaryInterface.isEmpty()
        ? QStringLiteral("No active uplink")
        : primaryInterface;
    content.primaryLabel = QStringLiteral("Download");
    content.primaryValue = formatCompactRate(downloadRate);
    content.accentColor = QStringLiteral("#30d158");
    content.badges = {
        {QStringLiteral("UP"), formatCompactRate(uploadRate)},
        {QStringLiteral("LINKS"), activeInterfaces.isEmpty() ? QStringLiteral("0")
                                                             : QString::number(activeInterfaces.split(QStringLiteral(", ")).size())},
        {QStringLiteral("INTERVAL"), QStringLiteral("%1s").arg(m_intervalMs / 1000.0, 0, 'f', 1)}
    };
    content.sections.append({
        QStringLiteral("Live Throughput"),
        {
            {QStringLiteral("Download"), formatCompactRate(downloadRate), QString(), qMin(downloadRate / (1024.0 * 1024.0 * 10.0), 1.0), QStringLiteral("#30d158")},
            {QStringLiteral("Upload"), formatCompactRate(uploadRate), QString(), qMin(uploadRate / (1024.0 * 1024.0 * 10.0), 1.0), QStringLiteral("#0a84ff")}
        }
    });
    content.sections.append({
        QStringLiteral("Traffic Counters"),
        {
            {QStringLiteral("Primary Interface"), primaryInterface.isEmpty() ? QStringLiteral("--") : primaryInterface, QString(), -1.0, QString()},
            {QStringLiteral("Active Interfaces"), activeInterfaces.isEmpty() ? QStringLiteral("--") : activeInterfaces, QString(), -1.0, QString()},
            {QStringLiteral("Total Received"), formatCompactBytes(totalReceived), QString(), -1.0, QString()},
            {QStringLiteral("Total Sent"), formatCompactBytes(totalSent), QString(), -1.0, QString()}
        }
    });
    return content;
}

void NetworkMonitorPlugin::collectAndPublish()
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

void NetworkMonitorPlugin::publishSnapshot(const NetworkSnapshot& snapshot) const
{
    if (!m_context || !m_context->metricSink()) {
        return;
    }

    MetricFrame frame;
    frame.pluginId = metaInfo().id;
    frame.timestamp = QDateTime::currentDateTime();
    frame.values = {
        {QStringLiteral("network.interface.primary"), snapshot.primaryInterface, frame.timestamp, {}},
        {QStringLiteral("network.interfaces.active"), interfaceSummary(snapshot.activeInterfaces), frame.timestamp, {}},
        {QStringLiteral("network.download.rate"), snapshot.receiveBytesPerSecond, frame.timestamp, {}},
        {QStringLiteral("network.upload.rate"), snapshot.transmitBytesPerSecond, frame.timestamp, {}},
        {QStringLiteral("network.download.total.bytes"), QVariant::fromValue<qulonglong>(snapshot.totalReceivedBytes), frame.timestamp, {}},
        {QStringLiteral("network.upload.total.bytes"), QVariant::fromValue<qulonglong>(snapshot.totalTransmittedBytes), frame.timestamp, {}}
    };
    m_context->metricSink()->publishFrame(frame);
}

} // namespace Vitals
