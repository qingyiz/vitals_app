#include "DiskPanelWidget.h"

#include "IAppContext.h"
#include "InfoPanelWidget.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>
#include <QVBoxLayout>
#include <QtGlobal>

namespace Vitals {

DiskPanelWidget::DiskPanelWidget(IAppContext* context, QWidget* parent)
    : QWidget(parent)
    , m_context(context)
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto* selectorBar = new QWidget(this);
    selectorBar->setObjectName(QStringLiteral("diskSelectorBar"));
    auto* selectorLayout = new QHBoxLayout(selectorBar);
    selectorLayout->setContentsMargins(22, 14, 22, 0);
    selectorLayout->setSpacing(10);

    auto* selectorLabel = new QLabel(text(QStringLiteral("disk.selectDisk"), QStringLiteral("Disk")), selectorBar);
    selectorLabel->setObjectName(QStringLiteral("languageSelectorLabel"));

    m_diskCombo = new QComboBox(selectorBar);
    m_diskCombo->setMinimumWidth(280);
    connect(m_diskCombo, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        if (m_updatingCombo) {
            return;
        }
        const QString rootPath = m_diskCombo->itemData(index).toString();
        if (!rootPath.isEmpty()) {
            Q_EMIT diskSelected(rootPath);
        }
    });

    selectorLayout->addWidget(selectorLabel);
    selectorLayout->addWidget(m_diskCombo);
    selectorLayout->addStretch(1);

    m_infoPanel = new InfoPanelWidget(this);
    m_infoPanel->setPageTitle(text(QStringLiteral("disk.title"), QStringLiteral("Disk Monitor")));
    m_infoPanel->setPageSubtitle(text(QStringLiteral("disk.subtitle"),
        QStringLiteral("Mounted-volume capacity monitor with selectable internal and external disks.")));
    m_infoPanel->setDetailsTitle(text(QStringLiteral("disk.currentSnapshot"), QStringLiteral("Current Disk")));
    m_infoPanel->setHeroEyebrow(text(QStringLiteral("disk.diskUpper"), QStringLiteral("DISK")));

    rootLayout->addWidget(selectorBar);
    rootLayout->addWidget(m_infoPanel, 1);
}

void DiskPanelWidget::applySnapshot(const DiskSnapshot& snapshot)
{
    rebuildDiskSelector(snapshot);

    const DiskInfo& disk = snapshot.selectedDisk;
    if (disk.rootPath.isEmpty()) {
        const QString waiting = text(QStringLiteral("disk.noMountedDisk"), QStringLiteral("No ready mounted disk"));
        m_infoPanel->setHeroTitle(waiting);
        m_infoPanel->setHeroSubtitle(text(QStringLiteral("disk.waitingMount"), QStringLiteral("Waiting for a mounted volume")));
        m_infoPanel->setHeroMeta(QString());
        m_infoPanel->setBadges({});
        m_infoPanel->setDetailsRows({});
        m_infoPanel->setTiles({});
        return;
    }

    const double percent = usagePercent(disk);
    const bool hasActivity = disk.activityPercent >= 0.0;
    const QString activityText = hasActivity ? formatPercent(disk.activityPercent) : QStringLiteral("--");
    const QString percentText = formatPercent(percent);
    const QString availableText = formatBytes(disk.bytesAvailable);
    const QString totalText = formatBytes(disk.bytesTotal);
    const QString usedText = formatBytes(usedBytes(disk));
    const QString kindText = diskKindLabel(disk);

    m_infoPanel->setHeroTitle(hasActivity ? activityText : percentText);
    m_infoPanel->setHeroSubtitle(formatDiskName(disk));
    m_infoPanel->setHeroMeta(QStringLiteral("%1 | %2").arg(disk.rootPath, kindText));

    m_infoPanel->setBadges({
        {text(QStringLiteral("disk.activityUpper"), QStringLiteral("ACTIVE")), activityText},
        {text(QStringLiteral("disk.usedUpper"), QStringLiteral("USED")), usedText},
        {text(QStringLiteral("disk.freeUpper"), QStringLiteral("FREE")), availableText},
    });

    m_infoPanel->setDetailsRows({
        {text(QStringLiteral("disk.name"), QStringLiteral("Name")), disk.displayName},
        {text(QStringLiteral("disk.mountPath"), QStringLiteral("Mount Path")), disk.rootPath},
        {text(QStringLiteral("disk.kind"), QStringLiteral("Kind")), kindText},
        {text(QStringLiteral("disk.fileSystem"), QStringLiteral("File System")), disk.fileSystemType.isEmpty() ? QStringLiteral("--") : disk.fileSystemType},
        {text(QStringLiteral("disk.device"), QStringLiteral("Device")), disk.device.isEmpty() ? QStringLiteral("--") : disk.device},
        {text(QStringLiteral("disk.readOnly"), QStringLiteral("Read Only")), disk.isReadOnly
                ? text(QStringLiteral("common.yes"), QStringLiteral("Yes"))
                : text(QStringLiteral("common.no"), QStringLiteral("No"))}
    });

    m_infoPanel->setTiles({
        {text(QStringLiteral("disk.activity"), QStringLiteral("Disk Activity")), activityText},
        {text(QStringLiteral("disk.usage"), QStringLiteral("Capacity Usage")), percentText},
        {text(QStringLiteral("disk.totalCapacity"), QStringLiteral("Total Capacity")), totalText},
        {text(QStringLiteral("disk.usedCapacity"), QStringLiteral("Used Capacity")), usedText},
        {text(QStringLiteral("disk.availableCapacity"), QStringLiteral("Available Capacity")), availableText},
        {text(QStringLiteral("disk.mountedDisks"), QStringLiteral("Mounted Disks")), QString::number(snapshot.diskCount)},
        {text(QStringLiteral("disk.selectedVolume"), QStringLiteral("Selected Volume")), formatDiskName(disk)}
    });
}

QString DiskPanelWidget::text(const QString& key, const QString& fallback) const
{
    return m_context ? m_context->translate(key, fallback) : fallback;
}

QString DiskPanelWidget::formatBytes(qint64 bytes) const
{
    static const char* units[] = {"B", "KB", "MB", "GB", "TB", "PB"};

    double value = static_cast<double>(qMax<qint64>(0, bytes));
    int unitIndex = 0;
    while (value >= 1024.0 && unitIndex < 5) {
        value /= 1024.0;
        ++unitIndex;
    }

    return QStringLiteral("%1 %2")
        .arg(value, 0, 'f', unitIndex == 0 ? 0 : 1)
        .arg(QString::fromLatin1(units[unitIndex]));
}

QString DiskPanelWidget::formatPercent(double value) const
{
    return QStringLiteral("%1%").arg(qBound(0.0, value, 100.0), 0, 'f', 1);
}

QString DiskPanelWidget::formatDiskName(const DiskInfo& disk) const
{
    const QString suffix = disk.isExternalCandidate
        ? text(QStringLiteral("disk.externalBadge"), QStringLiteral("External"))
        : (disk.isRoot ? text(QStringLiteral("disk.systemBadge"), QStringLiteral("System")) : text(QStringLiteral("disk.mountedBadge"), QStringLiteral("Mounted")));
    return QStringLiteral("%1 (%2)").arg(disk.displayName, suffix);
}

QString DiskPanelWidget::diskKindLabel(const DiskInfo& disk) const
{
    if (disk.isExternalCandidate) {
        return text(QStringLiteral("disk.externalDisk"), QStringLiteral("External disk"));
    }
    if (disk.isRoot) {
        return text(QStringLiteral("disk.systemDisk"), QStringLiteral("System disk"));
    }
    return text(QStringLiteral("disk.mountedVolume"), QStringLiteral("Mounted volume"));
}

double DiskPanelWidget::usagePercent(const DiskInfo& disk) const
{
    if (disk.bytesTotal <= 0) {
        return 0.0;
    }
    return static_cast<double>(usedBytes(disk)) * 100.0 / static_cast<double>(disk.bytesTotal);
}

qint64 DiskPanelWidget::usedBytes(const DiskInfo& disk) const
{
    return qMax<qint64>(0, disk.bytesTotal - disk.bytesAvailable);
}

void DiskPanelWidget::rebuildDiskSelector(const DiskSnapshot& snapshot)
{
    if (!m_diskCombo) {
        return;
    }

    m_updatingCombo = true;
    const QSignalBlocker blocker(m_diskCombo);
    m_diskCombo->clear();
    int selectedIndex = -1;
    for (const DiskInfo& disk : snapshot.disks) {
        m_diskCombo->addItem(formatDiskName(disk), disk.rootPath);
        if (disk.rootPath == snapshot.selectedRootPath) {
            selectedIndex = m_diskCombo->count() - 1;
        }
    }
    if (selectedIndex >= 0) {
        m_diskCombo->setCurrentIndex(selectedIndex);
    }
    m_diskCombo->setEnabled(!snapshot.disks.isEmpty());
    m_updatingCombo = false;
}

} // namespace Vitals
