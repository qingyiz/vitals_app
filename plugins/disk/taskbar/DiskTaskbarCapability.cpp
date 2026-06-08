#include "taskbar/DiskTaskbarCapability.h"

#include "IAppContext.h"
#include "monitor/DiskMonitorCapability.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtGlobal>

namespace Vitals {

namespace {

constexpr const char* PluginId = "com.vitals.disk";

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

QString configuredTaskbarLabel(IAppContext* context, const QString& fallback)
{
    if (!context) {
        return fallback;
    }

    QFile file(context->configPathForPlugin(QString::fromLatin1(PluginId)));
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return fallback;
    }

    const QString label = QJsonDocument::fromJson(file.readAll())
        .object()
        .value(QStringLiteral("taskbarLabel"))
        .toString()
        .trimmed();
    return label.isEmpty() ? fallback : label;
}

} // namespace

DiskTaskbarCapability::DiskTaskbarCapability(const DiskMonitorCapability* monitorCapability, IAppContext* context)
    : m_monitorCapability(monitorCapability)
    , m_context(context)
{
}

QString DiskTaskbarCapability::displayText(const QHash<QString, MetricValue>& latestValues) const
{
    MetricValue usageValue = latestValues.value(QStringLiteral("disk.activity.percent"));
    if (!usageValue.value.isValid()) {
        usageValue = latestValues.value(QStringLiteral("disk.usage.percent"));
    }
    if (!usageValue.value.isValid()) {
        return QString();
    }

    return QStringLiteral("%1 %2")
        .arg(configuredTaskbarLabel(m_context, defaultTaskbarLabel()),
            formatPercent(usageValue.value.toDouble()));
}

QString DiskTaskbarCapability::tooltip(const QHash<QString, MetricValue>& latestValues) const
{
    const QString name = latestValues.value(QStringLiteral("disk.selected.name")).value.toString();
    const MetricValue activityValue = latestValues.value(QStringLiteral("disk.activity.percent"));
    const double activity = activityValue.value.isValid() ? activityValue.value.toDouble() : -1.0;
    const double usage = latestValues.value(QStringLiteral("disk.usage.percent")).value.toDouble();
    const qint64 available = latestValues.value(QStringLiteral("disk.bytes.available")).value.toLongLong();
    if (name.isEmpty()) {
        return QString();
    }

    if (activity >= 0.0) {
        return text(QStringLiteral("disk.tooltipWithActivity"), QStringLiteral("%1: %2 active, %3 capacity used, %4 available"))
            .arg(name, formatPercent(activity), formatPercent(usage), formatBytes(available));
    }
    return text(QStringLiteral("disk.tooltip"), QStringLiteral("%1: %2 capacity used, %3 available"))
        .arg(name, formatPercent(usage), formatBytes(available));
}

bool DiskTaskbarCapability::isEnabledByDefault() const
{
    return false;
}

bool DiskTaskbarCapability::supportsCustomTaskbarLabel() const
{
    return true;
}

QString DiskTaskbarCapability::defaultTaskbarLabel() const
{
    return QStringLiteral("Disk");
}

TaskbarDetailContent DiskTaskbarCapability::detailContent(const QHash<QString, MetricValue>& latestValues) const
{
    const QString name = latestValues.value(QStringLiteral("disk.selected.name")).value.toString();
    const QString root = latestValues.value(QStringLiteral("disk.selected.root")).value.toString();
    const QString fileSystem = latestValues.value(QStringLiteral("disk.selected.filesystem")).value.toString();
    const QString kind = latestValues.value(QStringLiteral("disk.selected.kind")).value.toString();
    const MetricValue activityValue = latestValues.value(QStringLiteral("disk.activity.percent"));
    const double activity = activityValue.value.isValid() ? activityValue.value.toDouble() : -1.0;
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
    content.primaryValue = activity >= 0.0 ? formatPercent(activity) : formatPercent(usage);
    content.accentColor = QStringLiteral("#bf5af2");
    content.badges = {
        {text(QStringLiteral("disk.activityUpper"), QStringLiteral("ACTIVE")), activity >= 0.0 ? formatPercent(activity) : QStringLiteral("--")},
        {text(QStringLiteral("disk.freeUpper"), QStringLiteral("FREE")), formatBytes(available)},
        {text(QStringLiteral("disk.totalUpper"), QStringLiteral("TOTAL")), formatBytes(total)}
    };
    content.sections.append({
        text(QStringLiteral("disk.activity"), QStringLiteral("Disk Activity")),
        {
            {text(QStringLiteral("disk.activeTime"), QStringLiteral("Active Time")), activity >= 0.0 ? formatPercent(activity) : QStringLiteral("--"), QString(), activity >= 0.0 ? activity / 100.0 : -1.0, QStringLiteral("#bf5af2")},
            {text(QStringLiteral("common.intervalUpper"), QStringLiteral("INTERVAL")), QStringLiteral("%1s").arg((m_monitorCapability
                    ? m_monitorCapability->intervalMs()
                    : 2000) / 1000.0, 0, 'f', 1), QString(), -1.0, QString()}
        }
    });
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
