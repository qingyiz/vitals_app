#include "MemoryPanelWidget.h"

#include "InfoPanelWidget.h"

#include <QVBoxLayout>

namespace Vitals {

MemoryPanelWidget::MemoryPanelWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    m_infoPanel = new InfoPanelWidget(this);
    m_infoPanel->setPageTitle(QStringLiteral("Memory Monitor"));
    m_infoPanel->setPageSubtitle(QStringLiteral("Live physical memory usage snapshot from macOS."));
    m_infoPanel->setDetailsTitle(QStringLiteral("Current Memory"));
    m_infoPanel->setHeroEyebrow(QStringLiteral("MEMORY"));

    rootLayout->addWidget(m_infoPanel);
}

void MemoryPanelWidget::applySnapshot(const MemorySnapshot& snapshot)
{
    const QString usageText = formatPercent(snapshot.usagePercent);
    const QString usedText = formatBytes(snapshot.usedBytes);
    const QString freeText = formatBytes(snapshot.freeBytes);
    const QString totalText = formatBytes(snapshot.totalBytes);

    m_infoPanel->setHeroTitle(usageText);
    m_infoPanel->setHeroSubtitle(QStringLiteral("%1 used of %2").arg(usedText, totalText));
    m_infoPanel->setHeroMeta(QStringLiteral("%1 available").arg(freeText));

    m_infoPanel->setBadges({
        {QStringLiteral("USED"), usedText},
        {QStringLiteral("FREE"), freeText},
        {QStringLiteral("TOTAL"), totalText}
    });

    m_infoPanel->setDetailsRows({
        {QStringLiteral("Usage"), usageText},
        {QStringLiteral("Used Memory"), usedText},
        {QStringLiteral("Available Memory"), freeText},
        {QStringLiteral("Total Memory"), totalText}
    });

    m_infoPanel->setTiles({
        {QStringLiteral("Usage"), usageText},
        {QStringLiteral("Used"), usedText},
        {QStringLiteral("Available"), freeText},
        {QStringLiteral("Total"), totalText}
    });
}

QString MemoryPanelWidget::formatBytes(quint64 bytes)
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

QString MemoryPanelWidget::formatPercent(double value)
{
    const double clamped = qBound(0.0, value, 100.0);
    return QStringLiteral("%1%").arg(clamped, 0, 'f', clamped >= 10.0 ? 0 : 1);
}

} // namespace Vitals
