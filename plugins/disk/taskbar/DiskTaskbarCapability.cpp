#include "taskbar/DiskTaskbarCapability.h"

#include "IAppContext.h"
#include "monitor/DiskMonitorCapability.h"

#include <QtGlobal>

namespace Vitals {

namespace {

QString formatBytes(qint64 bytes)
{
    static const char* units[] = {"B", "KB", "MB", "GB", "TB", "PB"};

    double value = static_cast<double>(qMax<qint64>(0, bytes));
    int unitIndex = 0;
    while (value >= 1024.0 && unitIndex < 5) {
        value /= 1024.0;
        ++unitIndex;
    }

    const int precision = (unitIndex == 0 || value >= 10.0) ? 0 : 1;
    return QStringLiteral("%1 %2")
        .arg(value, 0, 'f', precision)
        .arg(QString::fromLatin1(units[unitIndex]));
}

QString formatPercent(double value)
{
    return QStringLiteral("%1%").arg(qBound(0.0, value, 100.0), 0, 'f', 0);
}

} // namespace

DiskTaskbarCapability::DiskTaskbarCapability(const DiskMonitorCapability* monitorCapability, IAppContext* context)
    : m_monitorCapability(monitorCapability)
    , m_context(context)
{
}

QString DiskTaskbarCapability::displayText(const QHash<QString, MetricValue>& latestValues) const
{
    const MetricValue usageValue = latestValues.value(QStringLiteral("disk.usage.percent"));
    if (!usageValue.value.isValid()) {
        return QString();
    }

    return QStringLiteral("Disk %1").arg(formatPercent(usageValue.value.toDouble()));
}

QString DiskTaskbarCapability::tooltip(const QHash<QString, MetricValue>& latestValues) const
{
    const QString name = latestValues.value(QStringLiteral("disk.selected.name")).value.toString();
    const double usage = latestValues.value(QStringLiteral("disk.usage.percent")).value.toDouble();
    const qint64 available = latestValues.value(QStringLiteral("disk.bytes.available")).value.toLongLong();
    if (name.isEmpty()) {
        return QString();
    }

    return text(QStringLiteral("disk.tooltip"), QStringLiteral("%1: %2 used, %3 available"))
        .arg(name, formatPercent(usage), formatBytes(available));
}

bool DiskTaskbarCapability::isEnabledByDefault() const
{
    return false;
}

TaskbarDetailContent DiskTaskbarCapability::detailContent(const QHash<QString, MetricValue>& latestValues) const
{
    const QString name = latestValues.value(QStringLiteral("disk.selected.name")).value.toString();
    const QString root = latestValues.value(QStringLiteral("disk.selected.root")).value.toString();
    const QString fileSystem = latestValues.value(QStringLiteral("disk.selected.filesystem")).value.toString();
    const QString kind = latestValues.value(QStringLiteral("disk.selected.kind")).value.toString();
    const double usage = latestValues.value(QStringLiteral("disk.usage.percent")).value.toDouble();
    const qint64 total = latestValues.value(QStringLiteral("disk.bytes.total")).value.toLongLong();
    const qint64 used = latestValues.value(QStringLiteral("disk.bytes.used")).value.toLongLong();
    const qint64 available = latestValues.value(QStringLiteral("disk.bytes.available")).value.toLongLong();

    if (name.isEmpty()) {
        return {};
    }

    TaskbarDetailContent content;
    content.title = text(QStringLiteral("disk.title"), QStringLiteral("Disk Monitor"));
    content.subtitle = root;
    content.primaryLabel = name;
    content.primaryValue = formatPercent(usage);
    content.accentColor = QStringLiteral("#bf5af2");
    content.badges = {
        {text(QStringLiteral("disk.freeUpper"), QStringLiteral("FREE")), formatBytes(available)},
        {text(QStringLiteral("disk.totalUpper"), QStringLiteral("TOTAL")), formatBytes(total)},
        {text(QStringLiteral("common.intervalUpper"), QStringLiteral("INTERVAL")), QStringLiteral("%1s").arg((m_monitorCapability
                    ? m_monitorCapability->intervalMs()
                    : 2000) / 1000.0, 0, 'f', 1)}
    };
    content.sections.append({
        text(QStringLiteral("disk.capacity"), QStringLiteral("Capacity")),
        {
            {text(QStringLiteral("disk.usedCapacity"), QStringLiteral("Used Capacity")), formatBytes(used), QString(), usage / 100.0, QStringLiteral("#bf5af2")},
            {text(QStringLiteral("disk.availableCapacity"), QStringLiteral("Available Capacity")), formatBytes(available), QString(), -1.0, QString()}
        }
    });
    content.sections.append({
        text(QStringLiteral("disk.volumeDetails"), QStringLiteral("Volume Details")),
        {
            {text(QStringLiteral("disk.kind"), QStringLiteral("Kind")), kind, QString(), -1.0, QString()},
            {text(QStringLiteral("disk.fileSystem"), QStringLiteral("File System")), fileSystem.isEmpty() ? QStringLiteral("--") : fileSystem, QString(), -1.0, QString()},
            {text(QStringLiteral("disk.mountPath"), QStringLiteral("Mount Path")), root, QString(), -1.0, QString()}
        }
    });
    return content;
}

QString DiskTaskbarCapability::text(const QString& key, const QString& fallback) const
{
    return m_context ? m_context->translate(key, fallback) : fallback;
}

} // namespace Vitals
