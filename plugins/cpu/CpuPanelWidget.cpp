#include "CpuPanelWidget.h"

#include "IAppContext.h"
#include "InfoPanelWidget.h"

#include <QVBoxLayout>

namespace Vitals {

CpuPanelWidget::CpuPanelWidget(IAppContext* context, QWidget* parent)
    : QWidget(parent)
    , m_context(context)
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    m_infoPanel = new InfoPanelWidget(this);
    m_infoPanel->setPageTitle(text(QStringLiteral("cpu.title"), QStringLiteral("CPU Monitor")));
    m_infoPanel->setPageSubtitle(text(QStringLiteral("cpu.subtitle"),
        QStringLiteral("Live processor load snapshot published through the shared metric pipeline.")));
    m_infoPanel->setDetailsTitle(text(QStringLiteral("cpu.currentLoad"), QStringLiteral("Current Load")));
    m_infoPanel->setHeroEyebrow(text(QStringLiteral("cpu.cpuUpper"), QStringLiteral("CPU")));

    rootLayout->addWidget(m_infoPanel);
}

void CpuPanelWidget::applySnapshot(const CpuSnapshot& snapshot)
{
    const QString totalText = formatPercent(snapshot.totalUsagePercent);
    const QString busiestText = snapshot.busiestCoreIndex >= 0
        ? text(QStringLiteral("cpu.coreUsage"), QStringLiteral("Core %1 | %2"))
              .arg(snapshot.busiestCoreIndex + 1)
              .arg(formatPercent(snapshot.busiestCoreUsagePercent))
        : text(QStringLiteral("cpu.waitingBaseline"), QStringLiteral("Waiting for baseline sample"));
    const QString coreCountText = QString::number(snapshot.logicalCoreCount);

    m_infoPanel->setHeroTitle(totalText);
    m_infoPanel->setHeroSubtitle(snapshot.cpuName);
    m_infoPanel->setHeroMeta(
        text(QStringLiteral("cpu.logicalCoresPeak"), QStringLiteral("%1 logical cores | Peak %2")).arg(coreCountText, busiestText));

    m_infoPanel->setBadges({
        {text(QStringLiteral("cpu.totalUpper"), QStringLiteral("TOTAL")), totalText},
        {text(QStringLiteral("cpu.peakUpper"), QStringLiteral("PEAK")), formatPercent(snapshot.busiestCoreUsagePercent)},
        {text(QStringLiteral("cpu.coresUpper"), QStringLiteral("CORES")), coreCountText}
    });

    m_infoPanel->setDetailsRows({
        {text(QStringLiteral("cpu.processor"), QStringLiteral("Processor")), snapshot.cpuName},
        {text(QStringLiteral("cpu.totalUsage"), QStringLiteral("Total Usage")), totalText},
        {text(QStringLiteral("cpu.logicalCores"), QStringLiteral("Logical Cores")), coreCountText},
        {text(QStringLiteral("cpu.busiestCore"), QStringLiteral("Busiest Core")), busiestText}
    });

    QList<InfoTileData> tiles;
    for (int index = 0; index < snapshot.perCoreUsagePercent.size(); ++index) {
        tiles.append({
            text(QStringLiteral("cpu.core"), QStringLiteral("Core %1")).arg(index + 1),
            formatPercent(snapshot.perCoreUsagePercent.at(index))
        });
    }
    if (tiles.isEmpty()) {
        tiles.append({
            text(QStringLiteral("cpu.sampling"), QStringLiteral("Sampling")),
            text(QStringLiteral("cpu.collectingBaseline"), QStringLiteral("Collecting baseline..."))
        });
    }
    m_infoPanel->setTiles(tiles);
}

QString CpuPanelWidget::formatPercent(double value)
{
    const double clamped = qBound(0.0, value, 100.0);
    return QStringLiteral("%1%").arg(clamped, 0, 'f', clamped >= 10.0 ? 0 : 1);
}

QString CpuPanelWidget::text(const QString& key, const QString& fallback) const
{
    return m_context ? m_context->translate(key, fallback) : fallback;
}

} // namespace Vitals
