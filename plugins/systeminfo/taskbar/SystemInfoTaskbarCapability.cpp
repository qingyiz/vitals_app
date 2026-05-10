#include "taskbar/SystemInfoTaskbarCapability.h"

#include "monitor/SystemInfoMonitorCapability.h"

#include <QStringList>

namespace Vitals {

namespace {

QString formatMemoryCompact(qint64 bytes)
{
    const double gib = static_cast<double>(bytes) / 1024.0 / 1024.0 / 1024.0;
    return QStringLiteral("%1G").arg(gib, 0, 'f', gib >= 10.0 ? 0 : 1);
}

QString formatUptimeCompact(qint64 seconds)
{
    const qint64 days = seconds / 86400;
    if (days > 0) {
        return QStringLiteral("%1d").arg(days);
    }

    const qint64 hours = seconds / 3600;
    if (hours > 0) {
        return QStringLiteral("%1h").arg(hours);
    }

    return QStringLiteral("%1m").arg(seconds / 60);
}

QString formatUptimeVerbose(qint64 seconds)
{
    const qint64 days = seconds / 86400;
    const qint64 hours = (seconds % 86400) / 3600;
    const qint64 minutes = (seconds % 3600) / 60;
    return QStringLiteral("%1d %2h %3m").arg(days).arg(hours).arg(minutes);
}

} // namespace

SystemInfoTaskbarCapability::SystemInfoTaskbarCapability(const SystemInfoMonitorCapability* monitorCapability)
    : m_monitorCapability(monitorCapability)
{
}

QString SystemInfoTaskbarCapability::displayText(const QHash<QString, MetricValue>& latestValues) const
{
    const MetricValue memoryValue = latestValues.value(QStringLiteral("system.memory.total.bytes"));
    const MetricValue uptimeValue = latestValues.value(QStringLiteral("system.uptime.seconds"));
    if (!memoryValue.value.isValid() || !uptimeValue.value.isValid()) {
        return QString();
    }

    return QStringLiteral("%1 %2")
        .arg(formatMemoryCompact(memoryValue.value.toLongLong()))
        .arg(formatUptimeCompact(uptimeValue.value.toLongLong()));
}

QString SystemInfoTaskbarCapability::tooltip(const QHash<QString, MetricValue>& latestValues) const
{
    const QString device = latestValues.value(QStringLiteral("system.device.name")).value.toString();
    const QString os = latestValues.value(QStringLiteral("system.os.version")).value.toString();
    const QString cpu = latestValues.value(QStringLiteral("system.cpu.model")).value.toString();
    const QString gpu = latestValues.value(QStringLiteral("system.gpu.model")).value.toString();
    const qint64 memory = latestValues.value(QStringLiteral("system.memory.total.bytes")).value.toLongLong();
    const qint64 uptime = latestValues.value(QStringLiteral("system.uptime.seconds")).value.toLongLong();

    QStringList parts;
    if (!device.isEmpty()) parts.append(device);
    if (!os.isEmpty()) parts.append(os);
    if (!cpu.isEmpty()) parts.append(cpu);
    if (!gpu.isEmpty()) parts.append(gpu);
    if (memory > 0) parts.append(formatMemoryCompact(memory));
    if (uptime > 0) parts.append(formatUptimeVerbose(uptime));
    return parts.join(QStringLiteral(" | "));
}

bool SystemInfoTaskbarCapability::isEnabledByDefault() const
{
    return true;
}

TaskbarDetailContent SystemInfoTaskbarCapability::detailContent(const QHash<QString, MetricValue>& latestValues) const
{
    const QString device = latestValues.value(QStringLiteral("system.device.name")).value.toString();
    const QString os = latestValues.value(QStringLiteral("system.os.version")).value.toString();
    const QString cpu = latestValues.value(QStringLiteral("system.cpu.model")).value.toString();
    const QString gpu = latestValues.value(QStringLiteral("system.gpu.model")).value.toString();
    const qint64 memory = latestValues.value(QStringLiteral("system.memory.total.bytes")).value.toLongLong();
    const qint64 uptime = latestValues.value(QStringLiteral("system.uptime.seconds")).value.toLongLong();

    if (device.isEmpty() && os.isEmpty() && cpu.isEmpty() && gpu.isEmpty() && memory <= 0 && uptime <= 0) {
        return {};
    }

    TaskbarDetailContent content;
    content.title = QStringLiteral("System Information");
    content.subtitle = os;
    content.primaryLabel = QStringLiteral("Device");
    content.primaryValue = device.isEmpty() ? QStringLiteral("Vitals") : device;
    content.accentColor = QStringLiteral("#64d2ff");

    content.badges = {
        {QStringLiteral("MEMORY"), memory > 0 ? formatMemoryCompact(memory) : QStringLiteral("--")},
        {QStringLiteral("UPTIME"), uptime > 0 ? formatUptimeCompact(uptime) : QStringLiteral("--")},
        {QStringLiteral("INTERVAL"), QStringLiteral("%1s").arg((m_monitorCapability
                    ? m_monitorCapability->intervalMs()
                    : 5000) / 1000.0, 0, 'f', 1)}
    };

    content.sections.append({
        QStringLiteral("Hardware"),
        {
            {QStringLiteral("CPU"), cpu.isEmpty() ? QStringLiteral("--") : cpu, QString(), -1.0, QString()},
            {QStringLiteral("GPU"), gpu.isEmpty() ? QStringLiteral("--") : gpu, QString(), -1.0, QString()},
            {QStringLiteral("Memory"), memory > 0 ? formatMemoryCompact(memory) : QStringLiteral("--"), QString(), -1.0, QString()}
        }
    });
    content.sections.append({
        QStringLiteral("System"),
        {
            {QStringLiteral("Operating System"), os.isEmpty() ? QStringLiteral("--") : os, QString(), -1.0, QString()},
            {QStringLiteral("Uptime"), uptime > 0 ? formatUptimeVerbose(uptime) : QStringLiteral("--"), QString(), -1.0, QString()}
        }
    });
    return content;
}

} // namespace Vitals
