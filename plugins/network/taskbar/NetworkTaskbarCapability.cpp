#include "taskbar/NetworkTaskbarCapability.h"

#include "IAppContext.h"
#include "monitor/NetworkMonitorCapability.h"

#include <QStringList>
#include <QtGlobal>

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

QString formatMenuBarRate(double bytesPerSecond)
{
    static const char* units[] = {"B/s", "KB/s", "MB/s", "GB/s"};

    double value = qMax(0.0, bytesPerSecond);
    int unitIndex = 0;
    while (value >= 1024.0 && unitIndex < 3) {
        value /= 1024.0;
        ++unitIndex;
    }

    const int precision = (unitIndex == 0 || value >= 10.0) ? 0 : 1;
    return QStringLiteral("%1 %2")
        .arg(value, 0, 'f', precision)
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

} // namespace

NetworkTaskbarCapability::NetworkTaskbarCapability(const NetworkMonitorCapability* monitorCapability, IAppContext* context)
    : m_monitorCapability(monitorCapability)
    , m_context(context)
{
}

QString NetworkTaskbarCapability::displayText(const QHash<QString, MetricValue>& latestValues) const
{
    const MetricValue downloadValue = latestValues.value(QStringLiteral("network.download.rate"));
    const MetricValue uploadValue = latestValues.value(QStringLiteral("network.upload.rate"));
    if (!downloadValue.value.isValid() || !uploadValue.value.isValid()) {
        return QString();
    }

    return QStringLiteral("%1\n%2")
        .arg(formatMenuBarRate(uploadValue.value.toDouble()))
        .arg(formatMenuBarRate(downloadValue.value.toDouble()));
}

QString NetworkTaskbarCapability::tooltip(const QHash<QString, MetricValue>& latestValues) const
{
    const QString primaryInterface = latestValues.value(QStringLiteral("network.interface.primary")).value.toString();
    const QString activeInterfaces = latestValues.value(QStringLiteral("network.interfaces.active")).value.toString();
    const quint64 totalReceived = latestValues.value(QStringLiteral("network.download.total.bytes")).value.toULongLong();
    const quint64 totalSent = latestValues.value(QStringLiteral("network.upload.total.bytes")).value.toULongLong();

    QStringList parts;
    if (!primaryInterface.isEmpty()) parts.append(text(QStringLiteral("network.primaryCompact"), QStringLiteral("Primary %1")).arg(primaryInterface));
    if (!activeInterfaces.isEmpty()) parts.append(activeInterfaces);
    if (totalReceived > 0) parts.append(text(QStringLiteral("network.downCompact"), QStringLiteral("Down %1")).arg(formatCompactBytes(totalReceived)));
    if (totalSent > 0) parts.append(text(QStringLiteral("network.upCompact"), QStringLiteral("Up %1")).arg(formatCompactBytes(totalSent)));
    return parts.join(QStringLiteral(" | "));
}

bool NetworkTaskbarCapability::isEnabledByDefault() const
{
    return true;
}

TaskbarDetailContent NetworkTaskbarCapability::detailContent(const QHash<QString, MetricValue>& latestValues) const
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
    content.title = text(QStringLiteral("network.title"), QStringLiteral("Network Monitor"));
    content.subtitle = primaryInterface.isEmpty()
        ? text(QStringLiteral("network.noActiveUplink"), QStringLiteral("No active uplink"))
        : primaryInterface;
    content.primaryLabel = text(QStringLiteral("network.download"), QStringLiteral("Download"));
    content.primaryValue = formatCompactRate(downloadRate);
    content.accentColor = QStringLiteral("#30d158");
    content.badges = {
        {text(QStringLiteral("network.upUpper"), QStringLiteral("UP")), formatCompactRate(uploadRate)},
        {text(QStringLiteral("network.linksUpper"), QStringLiteral("LINKS")), activeInterfaces.isEmpty() ? QStringLiteral("0")
                                                             : QString::number(activeInterfaces.split(QStringLiteral(", ")).size())},
        {text(QStringLiteral("common.intervalUpper"), QStringLiteral("INTERVAL")), QStringLiteral("%1s").arg((m_monitorCapability
                    ? m_monitorCapability->intervalMs()
                    : 2000) / 1000.0, 0, 'f', 1)}
    };
    content.sections.append({
        text(QStringLiteral("network.liveThroughput"), QStringLiteral("Live Throughput")),
        {
            {text(QStringLiteral("network.download"), QStringLiteral("Download")), formatCompactRate(downloadRate), QString(), qMin(downloadRate / (1024.0 * 1024.0 * 10.0), 1.0), QStringLiteral("#30d158")},
            {text(QStringLiteral("network.upload"), QStringLiteral("Upload")), formatCompactRate(uploadRate), QString(), qMin(uploadRate / (1024.0 * 1024.0 * 10.0), 1.0), QStringLiteral("#0a84ff")}
        }
    });
    content.sections.append({
        text(QStringLiteral("network.trafficCounters"), QStringLiteral("Traffic Counters")),
        {
            {text(QStringLiteral("network.primaryInterface"), QStringLiteral("Primary Interface")), primaryInterface.isEmpty() ? QStringLiteral("--") : primaryInterface, QString(), -1.0, QString()},
            {text(QStringLiteral("network.activeInterfaces"), QStringLiteral("Active Interfaces")), activeInterfaces.isEmpty() ? QStringLiteral("--") : activeInterfaces, QString(), -1.0, QString()},
            {text(QStringLiteral("network.totalReceived"), QStringLiteral("Total Received")), formatCompactBytes(totalReceived), QString(), -1.0, QString()},
            {text(QStringLiteral("network.totalSent"), QStringLiteral("Total Sent")), formatCompactBytes(totalSent), QString(), -1.0, QString()}
        }
    });
    return content;
}

QString NetworkTaskbarCapability::text(const QString& key, const QString& fallback) const
{
    return m_context ? m_context->translate(key, fallback) : fallback;
}

} // namespace Vitals
