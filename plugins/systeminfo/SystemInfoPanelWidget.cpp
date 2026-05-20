#include "SystemInfoPanelWidget.h"

#include "InfoPanelWidget.h"

#include <QScrollArea>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace Vitals {

SystemInfoPanelWidget::SystemInfoPanelWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(0, 0);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setMinimumSize(0, 0);
    scrollArea->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    m_infoPanel = new InfoPanelWidget(scrollArea);
    m_infoPanel->setMinimumSize(680, 620);
    m_infoPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_infoPanel->setPageTitle(QStringLiteral("System Information"));
    m_infoPanel->setPageSubtitle(
        QStringLiteral("A compact host summary inspired by menu-bar utilities, powered by the shared metric pipeline."));
    m_infoPanel->setDetailsTitle(QStringLiteral("Current Snapshot"));
    m_infoPanel->setHeroEyebrow(QStringLiteral("HOST"));

    scrollArea->setWidget(m_infoPanel);
    rootLayout->addWidget(scrollArea);
}

void SystemInfoPanelWidget::applySnapshot(const SystemInfoSnapshot& snapshot)
{
    const QString memoryText = formatBytes(snapshot.totalMemoryBytes);
    const QString uptimeText = formatUptime(snapshot.uptimeSeconds);

    m_infoPanel->setHeroTitle(snapshot.deviceName);
    m_infoPanel->setHeroSubtitle(
        QStringLiteral("%1  |  %2").arg(snapshot.osVersion, snapshot.cpuModel));
    m_infoPanel->setHeroMeta(snapshot.gpuModel);

    m_infoPanel->setBadges({
        {QStringLiteral("MEMORY"), memoryText},
        {QStringLiteral("UPTIME"), uptimeText}
    });

    m_infoPanel->setDetailsRows({
        {QStringLiteral("Device"), snapshot.deviceName},
        {QStringLiteral("OS"), snapshot.osVersion},
        {QStringLiteral("CPU"), snapshot.cpuModel},
        {QStringLiteral("GPU"), snapshot.gpuModel},
        {QStringLiteral("Memory"), memoryText},
        {QStringLiteral("Uptime"), uptimeText}
    });

    m_infoPanel->setTiles({
        {QStringLiteral("Machine Name"), snapshot.deviceName},
        {QStringLiteral("Operating System"), snapshot.osVersion},
        {QStringLiteral("Processor"), snapshot.cpuModel},
        {QStringLiteral("Installed Memory"), memoryText},
        {QStringLiteral("Graphics"), snapshot.gpuModel},
        {QStringLiteral("Boot Duration"), uptimeText}
    });
}

QString SystemInfoPanelWidget::formatBytes(quint64 bytes)
{
    static const char* suffixes[] = {"B", "KB", "MB", "GB", "TB"};

    double value = static_cast<double>(bytes);
    int suffixIndex = 0;
    while (value >= 1024.0 && suffixIndex < 4) {
        value /= 1024.0;
        ++suffixIndex;
    }

    return QStringLiteral("%1 %2").arg(value, 0, 'f', suffixIndex == 0 ? 0 : 1)
        .arg(QString::fromLatin1(suffixes[suffixIndex]));
}

QString SystemInfoPanelWidget::formatUptime(qint64 seconds)
{
    const qint64 days = seconds / 86400;
    const qint64 hours = (seconds % 86400) / 3600;
    const qint64 minutes = (seconds % 3600) / 60;

    if (days > 0) {
        return QStringLiteral("%1d %2h %3m").arg(days).arg(hours).arg(minutes);
    }

    return QStringLiteral("%1h %2m").arg(hours).arg(minutes);
}

} // namespace Vitals
