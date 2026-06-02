#include "settings/DiskSettingsCapability.h"

#include "IAppContext.h"
#include "monitor/DiskMonitorCapability.h"

#include <QComboBox>
#include <QLabel>
#include <QSignalBlocker>
#include <QVBoxLayout>
#include <QWidget>

namespace Vitals {

DiskSettingsCapability::DiskSettingsCapability(DiskMonitorCapability* monitorCapability, IAppContext* context)
    : m_monitorCapability(monitorCapability)
    , m_context(context)
{
}

QString DiskSettingsCapability::settingsId() const
{
    return QStringLiteral("disk");
}

QString DiskSettingsCapability::settingsTitle() const
{
    return text(QStringLiteral("disk.title"), QStringLiteral("Disk Monitor"));
}

QWidget* DiskSettingsCapability::createSettingsWidget(QWidget* parent)
{
    auto* root = new QWidget(parent);
    auto* layout = new QVBoxLayout(root);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto* label = new QLabel(text(QStringLiteral("disk.selectDisk"), QStringLiteral("Disk")), root);
    label->setObjectName(QStringLiteral("panelTitle"));

    m_combo = new QComboBox(root);
    QObject::connect(m_combo, QOverload<int>::of(&QComboBox::activated), root, [this](int index) {
        if (!m_combo || !m_monitorCapability) {
            return;
        }
        const QString rootPath = m_combo->itemData(index).toString();
        if (!rootPath.isEmpty()) {
            m_monitorCapability->setSelectedRootPath(rootPath);
        }
    });

    m_hintLabel = new QLabel(text(QStringLiteral("disk.settingsHint"),
        QStringLiteral("Choose which mounted disk Vitals should publish and display. External drives appear after the OS mounts them.")), root);
    m_hintLabel->setObjectName(QStringLiteral("panelBody"));
    m_hintLabel->setWordWrap(true);

    layout->addWidget(label);
    layout->addWidget(m_combo);
    layout->addWidget(m_hintLabel);
    layout->addStretch(1);

    m_widget = root;
    rebuildCombo();
    return root;
}

void DiskSettingsCapability::updateSnapshot(const DiskSnapshot& snapshot)
{
    m_lastSnapshot = snapshot;
    rebuildCombo();
}

QString DiskSettingsCapability::text(const QString& key, const QString& fallback) const
{
    return m_context ? m_context->translate(key, fallback) : fallback;
}

QString DiskSettingsCapability::formatDiskName(const DiskInfo& disk) const
{
    QString kind = text(QStringLiteral("disk.mountedBadge"), QStringLiteral("Mounted"));
    if (disk.isExternalCandidate) {
        kind = text(QStringLiteral("disk.externalBadge"), QStringLiteral("External"));
    } else if (disk.isRoot) {
        kind = text(QStringLiteral("disk.systemBadge"), QStringLiteral("System"));
    }
    return QStringLiteral("%1 (%2) - %3").arg(disk.displayName, kind, disk.rootPath);
}

void DiskSettingsCapability::rebuildCombo()
{
    if (!m_combo) {
        return;
    }

    const QSignalBlocker blocker(m_combo);
    m_combo->clear();
    int selectedIndex = -1;
    for (const DiskInfo& disk : m_lastSnapshot.disks) {
        m_combo->addItem(formatDiskName(disk), disk.rootPath);
        if (disk.rootPath == m_lastSnapshot.selectedRootPath) {
            selectedIndex = m_combo->count() - 1;
        }
    }
    if (selectedIndex >= 0) {
        m_combo->setCurrentIndex(selectedIndex);
    }
    m_combo->setEnabled(!m_lastSnapshot.disks.isEmpty());
}

} // namespace Vitals
