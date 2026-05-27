#include "taskbar/MemoryTaskbarCapability.h"

#include "IAppContext.h"
#include "monitor/MemoryMonitorCapability.h"

#include <QtGlobal>

namespace Vitals {

namespace {

QString formatBytes(quint64 bytes)
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

QString formatPercent(double value)
{
    const double clamped = qBound(0.0, value, 100.0);
    return QStringLiteral("%1%").arg(clamped, 0, 'f', clamped >= 10.0 ? 0 : 1);
}

} // namespace

MemoryTaskbarCapability::MemoryTaskbarCapability(const MemoryMonitorCapability* monitorCapability, IAppContext* context)
    : m_monitorCapability(monitorCapability)
    , m_context(context)
{
}

QString MemoryTaskbarCapability::displayText(const QHash<QString, MetricValue>& latestValues) const
{
    const MetricValue usageValue = latestValues.value(QStringLiteral("memory.usage.percent"));
    if (!usageValue.value.isValid()) {
        return QString();
    }

    return QStringLiteral("MEM:\n%1").arg(formatPercent(usageValue.value.toDouble()));
}

QString MemoryTaskbarCapability::tooltip(const QHash<QString, MetricValue>& latestValues) const
{
    const double usage = latestValues.value(QStringLiteral("memory.usage.percent")).value.toDouble();
    const quint64 used = latestValues.value(QStringLiteral("memory.used.bytes")).value.toULongLong();
    const quint64 total = latestValues.value(QStringLiteral("memory.total.bytes")).value.toULongLong();

    return text(QStringLiteral("memory.tooltip"), QStringLiteral("Memory %1 | %2 of %3 used"))
        .arg(formatPercent(usage), formatBytes(used), formatBytes(total));
}

bool MemoryTaskbarCapability::isEnabledByDefault() const
{
    return true;
}

TaskbarDetailContent MemoryTaskbarCapability::detailContent(const QHash<QString, MetricValue>& latestValues) const
{
    const MetricValue usageValue = latestValues.value(QStringLiteral("memory.usage.percent"));
    if (!usageValue.value.isValid()) {
        return {};
    }

    const double usage = usageValue.value.toDouble();
    const quint64 used = latestValues.value(QStringLiteral("memory.used.bytes")).value.toULongLong();
    const quint64 free = latestValues.value(QStringLiteral("memory.free.bytes")).value.toULongLong();
    const quint64 total = latestValues.value(QStringLiteral("memory.total.bytes")).value.toULongLong();

    TaskbarDetailContent content;
    content.title = text(QStringLiteral("memory.title"), QStringLiteral("Memory Monitor"));
    content.primaryLabel = text(QStringLiteral("memory.memoryUsage"), QStringLiteral("Memory Usage"));
    content.primaryValue = formatPercent(usage);
    content.accentColor = QStringLiteral("#32d74b");
    content.badges = {
        {text(QStringLiteral("memory.usedUpper"), QStringLiteral("USED")), formatBytes(used)},
        {text(QStringLiteral("memory.freeUpper"), QStringLiteral("FREE")), formatBytes(free)},
        {text(QStringLiteral("common.intervalUpper"), QStringLiteral("INTERVAL")), QStringLiteral("%1s").arg((m_monitorCapability
                    ? m_monitorCapability->intervalMs()
                    : 2000) / 1000.0, 0, 'f', 1)}
    };
    content.sections.append({
        text(QStringLiteral("memory.memory"), QStringLiteral("Memory")),
        {
            {text(QStringLiteral("memory.used"), QStringLiteral("Used")), formatBytes(used), QString(), usage, QStringLiteral("#32d74b")},
            {text(QStringLiteral("memory.availableTile"), QStringLiteral("Available")), formatBytes(free), QString(), -1.0, QString()},
            {text(QStringLiteral("memory.total"), QStringLiteral("Total")), formatBytes(total), QString(), -1.0, QString()}
        }
    });
    return content;
}

QString MemoryTaskbarCapability::text(const QString& key, const QString& fallback) const
{
    return m_context ? m_context->translate(key, fallback) : fallback;
}

} // namespace Vitals
