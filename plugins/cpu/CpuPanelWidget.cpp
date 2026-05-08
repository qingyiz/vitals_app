#include "CpuPanelWidget.h"

#include "InfoPanelWidget.h"

#include <QVBoxLayout>

namespace Vitals {

CpuPanelWidget::CpuPanelWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    m_infoPanel = new InfoPanelWidget(this);
    m_infoPanel->setPageTitle(QStringLiteral("CPU Monitor"));
    m_infoPanel->setPageSubtitle(
        QStringLiteral("Live processor load snapshot published through the shared metric pipeline."));
    m_infoPanel->setDetailsTitle(QStringLiteral("Current Load"));
    m_infoPanel->setHeroEyebrow(QStringLiteral("CPU"));

    rootLayout->addWidget(m_infoPanel);
}

void CpuPanelWidget::applySnapshot(const CpuSnapshot& snapshot)
{
    const QString totalText = formatPercent(snapshot.totalUsagePercent);
    const QString busiestText = snapshot.busiestCoreIndex >= 0
        ? QStringLiteral("Core %1  |  %2")
              .arg(snapshot.busiestCoreIndex + 1)
              .arg(formatPercent(snapshot.busiestCoreUsagePercent))
        : QStringLiteral("Waiting for baseline sample");
    const QString coreCountText = QString::number(snapshot.logicalCoreCount);

    m_infoPanel->setHeroTitle(totalText);
    m_infoPanel->setHeroSubtitle(snapshot.cpuName);
    m_infoPanel->setHeroMeta(
        QStringLiteral("%1 logical cores  |  Peak %2").arg(coreCountText, busiestText));

    m_infoPanel->setBadges({
        {QStringLiteral("TOTAL"), totalText},
        {QStringLiteral("PEAK"), formatPercent(snapshot.busiestCoreUsagePercent)},
        {QStringLiteral("CORES"), coreCountText}
    });

    m_infoPanel->setDetailsRows({
        {QStringLiteral("Processor"), snapshot.cpuName},
        {QStringLiteral("Total Usage"), totalText},
        {QStringLiteral("Logical Cores"), coreCountText},
        {QStringLiteral("Busiest Core"), busiestText}
    });

    QList<InfoTileData> tiles;
    for (int index = 0; index < snapshot.perCoreUsagePercent.size(); ++index) {
        tiles.append({
            QStringLiteral("Core %1").arg(index + 1),
            formatPercent(snapshot.perCoreUsagePercent.at(index))
        });
    }
    if (tiles.isEmpty()) {
        tiles.append({QStringLiteral("Sampling"), QStringLiteral("Collecting baseline...")});
    }
    m_infoPanel->setTiles(tiles);
}

QString CpuPanelWidget::formatPercent(double value)
{
    const double clamped = qBound(0.0, value, 100.0);
    return QStringLiteral("%1%").arg(clamped, 0, 'f', clamped >= 10.0 ? 0 : 1);
}

} // namespace Vitals
