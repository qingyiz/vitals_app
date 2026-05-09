#include "taskbar/NetworkTaskbarCapability.h"

#include "monitor/NetworkMonitorCapability.h"

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

NetworkTaskbarCapability::NetworkTaskbarCapability(const NetworkMonitorCapability* monitorCapability)
    : m_monitorCapability(monitorCapability)
{
}

QString NetworkTaskbarCapability::displayText(const QHash<QString, MetricValue>& latestValues) const
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

QString NetworkTaskbarCapability::tooltip(const QHash<QString, MetricValue>& latestValues) const
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
        {QStringLiteral("INTERVAL"), QStringLiteral("%1s").arg((m_monitorCapability
                    ? m_monitorCapability->intervalMs()
                    : 2000) / 1000.0, 0, 'f', 1)}
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

} // namespace Vitals
