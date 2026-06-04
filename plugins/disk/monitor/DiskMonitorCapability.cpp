#include "monitor/DiskMonitorCapability.h"

#include "DiskCollectorFactory.h"
#include "IAppContext.h"
#include "IMetricSink.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QtGlobal>

namespace Vitals {

namespace {

constexpr const char* PluginId = "com.vitals.disk";

double usagePercent(const DiskInfo& disk)
{
    if (disk.bytesTotal <= 0) {
        return 0.0;
    }

    const qint64 used = qMax<qint64>(0, disk.bytesTotal - disk.bytesAvailable);
    return qBound(0.0, static_cast<double>(used) * 100.0 / static_cast<double>(disk.bytesTotal), 100.0);
}

qint64 usedBytes(const DiskInfo& disk)
{
    return qMax<qint64>(0, disk.bytesTotal - disk.bytesAvailable);
}

QString diskKind(const DiskInfo& disk)
{
    if (disk.isExternalCandidate) {
        return QStringLiteral("external");
    }
    if (disk.isRoot) {
        return QStringLiteral("system");
    }
    return QStringLiteral("mounted");
}

} // namespace

DiskMonitorCapability::DiskMonitorCapability(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(m_intervalMs);
    connect(m_timer, &QTimer::timeout, this, &DiskMonitorCapability::collectAndPublish);
}

DiskMonitorCapability::~DiskMonitorCapability() = default;

bool DiskMonitorCapability::initialize(IAppContext* context)
{
    m_context = context;
    m_collector = DiskCollectorFactory::create();
    if (!m_context || !m_collector) {
        return false;
    }

    loadConfig();
    return m_collector->initialize();
}

QList<MetricDescriptor> DiskMonitorCapability::metricDescriptors() const
{
    return {
        {QStringLiteral("disk.selected.name"), QStringLiteral("Selected Disk"),
            QStringLiteral("Display name of the currently selected mounted disk."), QString(), MetricValueType::String},
        {QStringLiteral("disk.selected.root"), QStringLiteral("Mount Path"),
            QStringLiteral("Mounted root path of the selected disk."), QString(), MetricValueType::String},
        {QStringLiteral("disk.selected.device"), QStringLiteral("Device"),
            QStringLiteral("Device identifier reported by the operating system."), QString(), MetricValueType::String},
        {QStringLiteral("disk.selected.filesystem"), QStringLiteral("File System"),
            QStringLiteral("File system type of the selected disk."), QString(), MetricValueType::String},
        {QStringLiteral("disk.selected.kind"), QStringLiteral("Disk Kind"),
            QStringLiteral("Best-effort classification of the selected disk."), QString(), MetricValueType::String},
        {QStringLiteral("disk.usage.percent"), QStringLiteral("Disk Usage"),
            QStringLiteral("Used capacity percentage of the selected disk."), QStringLiteral("%"), MetricValueType::Percentage},
        {QStringLiteral("disk.bytes.total"), QStringLiteral("Total Capacity"),
            QStringLiteral("Total byte capacity of the selected disk."), QStringLiteral("B"), MetricValueType::Bytes},
        {QStringLiteral("disk.bytes.used"), QStringLiteral("Used Capacity"),
            QStringLiteral("Used byte capacity of the selected disk."), QStringLiteral("B"), MetricValueType::Bytes},
        {QStringLiteral("disk.bytes.available"), QStringLiteral("Available Capacity"),
            QStringLiteral("Available byte capacity of the selected disk."), QStringLiteral("B"), MetricValueType::Bytes},
        {QStringLiteral("disk.count"), QStringLiteral("Mounted Disk Count"),
            QStringLiteral("Number of ready mounted disks currently visible to the host."), QString(), MetricValueType::Integer}
    };
}

int DiskMonitorCapability::defaultIntervalMs() const
{
    return 5000;
}

void DiskMonitorCapability::setIntervalMs(int intervalMs)
{
    m_intervalMs = qMax(3000, intervalMs);
    m_timer->setInterval(m_intervalMs);
}

void DiskMonitorCapability::startMonitoring()
{
    collectAndPublish();
    m_timer->start();
}

void DiskMonitorCapability::stopMonitoring()
{
    m_timer->stop();
}

int DiskMonitorCapability::intervalMs() const
{
    return m_intervalMs;
}

const DiskSnapshot& DiskMonitorCapability::lastSnapshot() const
{
    return m_lastSnapshot;
}

QString DiskMonitorCapability::selectedRootPath() const
{
    return m_selectedRootPath;
}

void DiskMonitorCapability::setSelectedRootPath(const QString& rootPath)
{
    if (m_selectedRootPath == rootPath) {
        refreshNow();
        return;
    }

    m_selectedRootPath = rootPath;
    saveConfig();
    refreshNow();
}

void DiskMonitorCapability::refreshNow()
{
    collectAndPublish();
}

void DiskMonitorCapability::collectAndPublish()
{
    if (!m_collector) {
        return;
    }

    m_lastSnapshot = m_collector->collect(m_selectedRootPath);
    if (!m_lastSnapshot.selectedRootPath.isEmpty() && m_lastSnapshot.selectedRootPath != m_selectedRootPath) {
        m_selectedRootPath = m_lastSnapshot.selectedRootPath;
        saveConfig();
    }
    publishSnapshot(m_lastSnapshot);
    Q_EMIT snapshotUpdated(m_lastSnapshot);
}

void DiskMonitorCapability::publishSnapshot(const DiskSnapshot& snapshot) const
{
    if (!m_context || !m_context->metricSink() || snapshot.selectedDisk.rootPath.isEmpty()) {
        return;
    }

    const DiskInfo& disk = snapshot.selectedDisk;
    const QDateTime timestamp = QDateTime::currentDateTime();
    MetricFrame frame;
    frame.pluginId = QString::fromLatin1(PluginId);
    frame.timestamp = timestamp;
    frame.values = {
        {QStringLiteral("disk.selected.name"), disk.displayName, timestamp, {}},
        {QStringLiteral("disk.selected.root"), disk.rootPath, timestamp, {}},
        {QStringLiteral("disk.selected.device"), disk.device, timestamp, {}},
        {QStringLiteral("disk.selected.filesystem"), disk.fileSystemType, timestamp, {}},
        {QStringLiteral("disk.selected.kind"), diskKind(disk), timestamp, {}},
        {QStringLiteral("disk.usage.percent"), usagePercent(disk), timestamp, {}},
        {QStringLiteral("disk.bytes.total"), QVariant::fromValue<qlonglong>(disk.bytesTotal), timestamp, {}},
        {QStringLiteral("disk.bytes.used"), QVariant::fromValue<qlonglong>(usedBytes(disk)), timestamp, {}},
        {QStringLiteral("disk.bytes.available"), QVariant::fromValue<qlonglong>(disk.bytesAvailable), timestamp, {}},
        {QStringLiteral("disk.count"), snapshot.diskCount, timestamp, {}}
    };
    m_context->metricSink()->publishFrame(frame);
}

void DiskMonitorCapability::loadConfig()
{
    if (!m_context) {
        return;
    }

    QFile file(m_context->configPathForPlugin(QString::fromLatin1(PluginId)));
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return;
    }

    const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
    m_selectedRootPath = root.value(QStringLiteral("selectedRootPath")).toString();
    setIntervalMs(root.value(QStringLiteral("intervalMs")).toInt(m_intervalMs));
}

void DiskMonitorCapability::saveConfig() const
{
    if (!m_context) {
        return;
    }

    const QString configPath = m_context->configPathForPlugin(QString::fromLatin1(PluginId));
    QDir dir = QFileInfo(configPath).dir();
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        return;
    }

    QJsonObject root;
    root.insert(QStringLiteral("selectedRootPath"), m_selectedRootPath);
    root.insert(QStringLiteral("intervalMs"), m_intervalMs);

    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

} // namespace Vitals
