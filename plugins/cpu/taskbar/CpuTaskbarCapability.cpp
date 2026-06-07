#include "taskbar/CpuTaskbarCapability.h"

#include "IAppContext.h"
#include "monitor/CpuMonitorCapability.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QtGlobal>

namespace Vitals {

namespace {

constexpr const char* PluginId = "com.vitals.cpu";

QString formatPercentCompact(double value)
{
    const double clamped = qBound(0.0, value, 100.0);
    return QStringLiteral("%1%").arg(clamped, 0, 'f', clamped >= 10.0 ? 0 : 1);
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

CpuTaskbarCapability::CpuTaskbarCapability(const CpuMonitorCapability* monitorCapability, IAppContext* context)
    : m_monitorCapability(monitorCapability)
    , m_context(context)
{
}

QString CpuTaskbarCapability::displayText(const QHash<QString, MetricValue>& latestValues) const
{
    const MetricValue usageValue = latestValues.value(QStringLiteral("cpu.usage.total"));
    if (!usageValue.value.isValid()) {
        return QString();
    }

    return QStringLiteral("%1\n%2")
        .arg(configuredTaskbarLabel(m_context, defaultTaskbarLabel()),
            formatPercentCompact(usageValue.value.toDouble()));
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
    if (coreCount > 0) parts.append(text(QStringLiteral("cpu.logicalCoresCount"), QStringLiteral("%1 logical cores")).arg(coreCount));
    parts.append(text(QStringLiteral("cpu.totalCompact"), QStringLiteral("Total %1")).arg(formatPercentCompact(totalUsage)));
    if (busiestCoreIndex >= 0) {
        parts.append(text(QStringLiteral("cpu.peakCoreCompact"), QStringLiteral("Peak Core %1 %2"))
                         .arg(busiestCoreIndex + 1)
                         .arg(formatPercentCompact(busiestCoreUsage)));
    }
    return parts.join(QStringLiteral(" | "));
}

bool CpuTaskbarCapability::isEnabledByDefault() const
{
    return true;
}

bool CpuTaskbarCapability::supportsCustomTaskbarLabel() const
{
    return true;
}

QString CpuTaskbarCapability::defaultTaskbarLabel() const
{
    return QStringLiteral("CPU");
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
                text(QStringLiteral("cpu.core"), QStringLiteral("Core %1")).arg(index + 1),
                formatPercentCompact(coreUsage),
                QString(),
                coreUsage,
                QStringLiteral("#ff453a")
            });
        }
    }

    TaskbarDetailContent content;
    content.title = text(QStringLiteral("cpu.title"), QStringLiteral("CPU Monitor"));
    content.subtitle = model;
    content.primaryLabel = text(QStringLiteral("cpu.totalLoad"), QStringLiteral("Total Load"));
    content.primaryValue = formatPercentCompact(totalUsage);
    content.accentColor = QStringLiteral("#ff453a");
    content.badges = {
        {text(QStringLiteral("cpu.coresUpper"), QStringLiteral("CORES")), coreCount > 0 ? QString::number(coreCount) : QStringLiteral("--")},
        {text(QStringLiteral("cpu.peakCoreUpper"), QStringLiteral("PEAK CORE")), busiestCoreIndex >= 0
                ? QStringLiteral("%1  %2").arg(busiestCoreIndex + 1).arg(formatPercentCompact(busiestCoreUsage))
                : QStringLiteral("--")},
        {text(QStringLiteral("common.intervalUpper"), QStringLiteral("INTERVAL")), QStringLiteral("%1s").arg((m_monitorCapability
                    ? m_monitorCapability->intervalMs()
                    : 2000) / 1000.0, 0, 'f', 1)}
    };
    content.sections.append({text(QStringLiteral("cpu.perCoreLoad"), QStringLiteral("Per-Core Load")), coreRows});
    return content;
}

QString CpuTaskbarCapability::text(const QString& key, const QString& fallback) const
{
    return m_context ? m_context->translate(key, fallback) : fallback;
}

} // namespace Vitals
