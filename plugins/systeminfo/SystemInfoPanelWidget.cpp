#include "SystemInfoPanelWidget.h"

#include "IAppContext.h"
#include "InfoPanelWidget.h"

#include <QFrame>
#include <QScrollArea>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace Vitals {

SystemInfoPanelWidget::SystemInfoPanelWidget(IAppContext* context, QWidget* parent)
    : QWidget(parent)
    , m_context(context)
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
    m_infoPanel->setPageTitle(text(QStringLiteral("systemInfo.title"), QStringLiteral("System Information")));
    m_infoPanel->setPageSubtitle(text(QStringLiteral("systemInfo.subtitle"),
        QStringLiteral("A compact host summary inspired by menu-bar utilities, powered by the shared metric pipeline.")));
    m_infoPanel->setDetailsTitle(text(QStringLiteral("systemInfo.currentSnapshot"), QStringLiteral("Current Snapshot")));
    m_infoPanel->setHeroEyebrow(text(QStringLiteral("systemInfo.host"), QStringLiteral("HOST")));

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
        {text(QStringLiteral("systemInfo.memoryUpper"), QStringLiteral("MEMORY")), memoryText},
        {text(QStringLiteral("systemInfo.uptimeUpper"), QStringLiteral("UPTIME")), uptimeText}
    });

    m_infoPanel->setDetailsRows({
        {text(QStringLiteral("systemInfo.device"), QStringLiteral("Device")), snapshot.deviceName},
        {text(QStringLiteral("systemInfo.os"), QStringLiteral("OS")), snapshot.osVersion},
        {text(QStringLiteral("systemInfo.cpu"), QStringLiteral("CPU")), snapshot.cpuModel},
        {text(QStringLiteral("systemInfo.gpu"), QStringLiteral("GPU")), snapshot.gpuModel},
        {text(QStringLiteral("systemInfo.memory"), QStringLiteral("Memory")), memoryText},
        {text(QStringLiteral("systemInfo.uptime"), QStringLiteral("Uptime")), uptimeText}
    });

    m_infoPanel->setTiles({
        {text(QStringLiteral("systemInfo.machineName"), QStringLiteral("Machine Name")), snapshot.deviceName},
        {text(QStringLiteral("systemInfo.operatingSystem"), QStringLiteral("Operating System")), snapshot.osVersion},
        {text(QStringLiteral("systemInfo.processor"), QStringLiteral("Processor")), snapshot.cpuModel},
        {text(QStringLiteral("systemInfo.installedMemory"), QStringLiteral("Installed Memory")), memoryText},
        {text(QStringLiteral("systemInfo.graphics"), QStringLiteral("Graphics")), snapshot.gpuModel},
        {text(QStringLiteral("systemInfo.bootDuration"), QStringLiteral("Boot Duration")), uptimeText}
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

QString SystemInfoPanelWidget::formatUptime(qint64 seconds) const
{
    const qint64 days = seconds / 86400;
    const qint64 hours = (seconds % 86400) / 3600;
    const qint64 minutes = (seconds % 3600) / 60;

    if (days > 0) {
        return text(QStringLiteral("time.daysHoursMinutes"), QStringLiteral("%1d %2h %3m"))
            .arg(days).arg(hours).arg(minutes);
    }

    return text(QStringLiteral("time.hoursMinutes"), QStringLiteral("%1h %2m")).arg(hours).arg(minutes);
}

QString SystemInfoPanelWidget::text(const QString& key, const QString& fallback) const
{
    return m_context ? m_context->translate(key, fallback) : fallback;
}

} // namespace Vitals
