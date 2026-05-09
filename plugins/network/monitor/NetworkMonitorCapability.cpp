#include "monitor/NetworkMonitorCapability.h"

#include "IAppContext.h"
#include "IMetricSink.h"
#include "NetworkCollectorFactory.h"

#include <QDateTime>
#include <QTimer>

namespace Vitals {

namespace {

QString interfaceSummary(const QStringList& interfaces)
{
    if (interfaces.isEmpty()) {
        return QStringLiteral("No active interfaces");
    }

    return interfaces.join(QStringLiteral(", "));
}

} // namespace

NetworkMonitorCapability::NetworkMonitorCapability(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(m_intervalMs);
    connect(m_timer, &QTimer::timeout, this, &NetworkMonitorCapability::collectAndPublish);
}

NetworkMonitorCapability::~NetworkMonitorCapability() = default;

bool NetworkMonitorCapability::initialize(IAppContext* context)
{
    m_context = context;
    m_collector = NetworkCollectorFactory::create();
    if (!m_context || !m_collector) {
        return false;
    }

    return m_collector->initialize();
}

QList<MetricDescriptor> NetworkMonitorCapability::metricDescriptors() const
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

int NetworkMonitorCapability::defaultIntervalMs() const
{
    return 2000;
}

void NetworkMonitorCapability::setIntervalMs(int intervalMs)
{
    m_intervalMs = qMax(500, intervalMs);
    m_timer->setInterval(m_intervalMs);
}

void NetworkMonitorCapability::startMonitoring()
{
    collectAndPublish();
    m_timer->start();
}

void NetworkMonitorCapability::stopMonitoring()
{
    m_timer->stop();
}

int NetworkMonitorCapability::intervalMs() const
{
    return m_intervalMs;
}

const NetworkSnapshot& NetworkMonitorCapability::lastSnapshot() const
{
    return m_lastSnapshot;
}

void NetworkMonitorCapability::collectAndPublish()
{
    if (!m_collector) {
        return;
    }

    m_lastSnapshot = m_collector->collect();
    publishSnapshot(m_lastSnapshot);
    Q_EMIT snapshotUpdated(m_lastSnapshot);
}

void NetworkMonitorCapability::publishSnapshot(const NetworkSnapshot& snapshot) const
{
    if (!m_context || !m_context->metricSink()) {
        return;
    }

    MetricFrame frame;
    frame.pluginId = QStringLiteral("com.vitals.network");
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
