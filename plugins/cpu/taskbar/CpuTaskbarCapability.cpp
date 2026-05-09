#include "taskbar/CpuTaskbarCapability.h"

#include "monitor/CpuMonitorCapability.h"

#include <QtGlobal>

namespace Vitals {

namespace {

QString formatPercentCompact(double value)
{
    const double clamped = qBound(0.0, value, 100.0);
    return QStringLiteral("%1%").arg(clamped, 0, 'f', clamped >= 10.0 ? 0 : 1);
}

} // namespace

CpuTaskbarCapability::CpuTaskbarCapability(const CpuMonitorCapability* monitorCapability)
    : m_monitorCapability(monitorCapability)
{
}

QString CpuTaskbarCapability::displayText(const QHash<QString, MetricValue>& latestValues) const
{
    const MetricValue usageValue = latestValues.value(QStringLiteral("cpu.usage.total"));
    if (!usageValue.value.isValid()) {
        return QString();
    }

    return QStringLiteral("CPU:\n%1").arg(formatPercentCompact(usageValue.value.toDouble()));
}

QString CpuTaskbarCapability::tooltip(const QHash<QString, MetricValue>& latestValues) const
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

bool CpuTaskbarCapability::isEnabledByDefault() const
{
    return true;
}

TaskbarDetailContent CpuTaskbarCapability::detailContent(const QHash<QString, MetricValue>& latestValues) const
{
    const MetricValue usageValue = latestValues.value(QStringLiteral("cpu.usage.total"));
    if (!usageValue.value.isValid()) {
        return {};
    }

    const QString model = latestValues.value(QStringLiteral("cpu.model")).value.toString();
    const int coreCount = latestValues.value(QStringLiteral("cpu.logical.cores")).value.toInt();
    const double totalUsage = usageValue.value.toDouble();

    int busiestCoreIndex = -1;
    double busiestCoreUsage = 0.0;
    QList<TaskbarDetailRow> coreRows;
    for (int index = 0; index < coreCount; ++index) {
        const double coreUsage = latestValues.value(QStringLiteral("cpu.usage.core%1").arg(index)).value.toDouble();
        if (index == 0 || coreUsage > busiestCoreUsage) {
            busiestCoreUsage = coreUsage;
            busiestCoreIndex = index;
        }

        if (index < 8) {
            coreRows.append({
                QStringLiteral("Core %1").arg(index + 1),
                formatPercentCompact(coreUsage),
                QString(),
                coreUsage,
                QStringLiteral("#ff453a")
            });
        }
    }

    TaskbarDetailContent content;
    content.title = QStringLiteral("CPU Monitor");
    content.subtitle = model;
    content.primaryLabel = QStringLiteral("Total Load");
    content.primaryValue = formatPercentCompact(totalUsage);
    content.accentColor = QStringLiteral("#ff453a");
    content.badges = {
        {QStringLiteral("CORES"), coreCount > 0 ? QString::number(coreCount) : QStringLiteral("--")},
        {QStringLiteral("PEAK CORE"), busiestCoreIndex >= 0
                ? QStringLiteral("%1  %2").arg(busiestCoreIndex + 1).arg(formatPercentCompact(busiestCoreUsage))
                : QStringLiteral("--")},
        {QStringLiteral("INTERVAL"), QStringLiteral("%1s").arg((m_monitorCapability
                    ? m_monitorCapability->intervalMs()
                    : 2000) / 1000.0, 0, 'f', 1)}
    };
    content.sections.append({QStringLiteral("Per-Core Load"), coreRows});
    return content;
}

} // namespace Vitals
