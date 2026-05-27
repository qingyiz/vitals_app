#include "MemoryPanelWidget.h"

#include "IAppContext.h"
#include "InfoPanelWidget.h"

#include <QVBoxLayout>

namespace Vitals {

MemoryPanelWidget::MemoryPanelWidget(IAppContext* context, QWidget* parent)
    : QWidget(parent)
    , m_context(context)
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    m_infoPanel = new InfoPanelWidget(this);
    m_infoPanel->setPageTitle(text(QStringLiteral("memory.title"), QStringLiteral("Memory Monitor")));
    m_infoPanel->setPageSubtitle(text(QStringLiteral("memory.subtitle"),
        QStringLiteral("Live physical memory usage snapshot from the active platform.")));
    m_infoPanel->setDetailsTitle(text(QStringLiteral("memory.currentMemory"), QStringLiteral("Current Memory")));
    m_infoPanel->setHeroEyebrow(text(QStringLiteral("memory.memoryUpper"), QStringLiteral("MEMORY")));

    rootLayout->addWidget(m_infoPanel);
}

void MemoryPanelWidget::applySnapshot(const MemorySnapshot& snapshot)
{
    const QString usageText = formatPercent(snapshot.usagePercent);
    const QString usedText = formatBytes(snapshot.usedBytes);
    const QString freeText = formatBytes(snapshot.freeBytes);
    const QString totalText = formatBytes(snapshot.totalBytes);

    m_infoPanel->setHeroTitle(usageText);
    m_infoPanel->setHeroSubtitle(text(QStringLiteral("memory.usedOfTotal"), QStringLiteral("%1 used of %2")).arg(usedText, totalText));
    m_infoPanel->setHeroMeta(text(QStringLiteral("memory.available"), QStringLiteral("%1 available")).arg(freeText));

    m_infoPanel->setBadges({
        {text(QStringLiteral("memory.usedUpper"), QStringLiteral("USED")), usedText},
        {text(QStringLiteral("memory.freeUpper"), QStringLiteral("FREE")), freeText},
        {text(QStringLiteral("memory.totalUpper"), QStringLiteral("TOTAL")), totalText}
    });

    m_infoPanel->setDetailsRows({
        {text(QStringLiteral("memory.usage"), QStringLiteral("Usage")), usageText},
        {text(QStringLiteral("memory.usedMemory"), QStringLiteral("Used Memory")), usedText},
        {text(QStringLiteral("memory.availableMemory"), QStringLiteral("Available Memory")), freeText},
        {text(QStringLiteral("memory.totalMemory"), QStringLiteral("Total Memory")), totalText}
    });

    m_infoPanel->setTiles({
        {text(QStringLiteral("memory.usage"), QStringLiteral("Usage")), usageText},
        {text(QStringLiteral("memory.used"), QStringLiteral("Used")), usedText},
        {text(QStringLiteral("memory.availableTile"), QStringLiteral("Available")), freeText},
        {text(QStringLiteral("memory.total"), QStringLiteral("Total")), totalText}
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

QString MemoryPanelWidget::text(const QString& key, const QString& fallback) const
{
    return m_context ? m_context->translate(key, fallback) : fallback;
}

} // namespace Vitals
