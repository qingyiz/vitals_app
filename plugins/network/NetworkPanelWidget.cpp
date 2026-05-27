#include "NetworkPanelWidget.h"

#include "IAppContext.h"
#include "InfoPanelWidget.h"

#include <QVBoxLayout>

namespace Vitals {

namespace {

QString formatScaledBytes(double bytes, const QString& suffix)
{
    static const char* units[] = {"B", "KB", "MB", "GB", "TB"};

    int unitIndex = 0;
    while (bytes >= 1024.0 && unitIndex < 4) {
        bytes /= 1024.0;
        ++unitIndex;
    }

    return QStringLiteral("%1 %2%3")
        .arg(bytes, 0, 'f', unitIndex == 0 ? 0 : 1)
        .arg(QString::fromLatin1(units[unitIndex]))
        .arg(suffix);
}

} // namespace

NetworkPanelWidget::NetworkPanelWidget(IAppContext* context, QWidget* parent)
    : QWidget(parent)
    , m_context(context)
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    m_infoPanel = new InfoPanelWidget(this);
    m_infoPanel->setPageTitle(text(QStringLiteral("network.title"), QStringLiteral("Network Monitor")));
    m_infoPanel->setPageSubtitle(text(QStringLiteral("network.subtitle"),
        QStringLiteral("Live interface throughput and traffic counters collected from the macOS networking stack.")));
    m_infoPanel->setDetailsTitle(text(QStringLiteral("network.currentThroughput"), QStringLiteral("Current Throughput")));
    m_infoPanel->setHeroEyebrow(text(QStringLiteral("network.networkUpper"), QStringLiteral("NETWORK")));

    rootLayout->addWidget(m_infoPanel);
}

void NetworkPanelWidget::applySnapshot(const NetworkSnapshot& snapshot)
{
    const QString downRate = formatRate(snapshot.receiveBytesPerSecond);
    const QString upRate = formatRate(snapshot.transmitBytesPerSecond);
    const QString interfaces = formatInterfaceSummary(snapshot.activeInterfaces);
    const QString primaryInterface = snapshot.primaryInterface.isEmpty()
        ? text(QStringLiteral("network.noActiveUplink"), QStringLiteral("No active uplink"))
        : snapshot.primaryInterface;

    m_infoPanel->setHeroTitle(QStringLiteral("↓ %1  |  ↑ %2").arg(downRate, upRate));
    m_infoPanel->setHeroSubtitle(primaryInterface);
    m_infoPanel->setHeroMeta(interfaces);

    m_infoPanel->setBadges({
        {text(QStringLiteral("network.downUpper"), QStringLiteral("DOWN")), downRate},
        {text(QStringLiteral("network.upUpper"), QStringLiteral("UP")), upRate},
        {text(QStringLiteral("network.linksUpper"), QStringLiteral("LINKS")), QString::number(snapshot.activeInterfaces.size())}
    });

    m_infoPanel->setDetailsRows({
        {text(QStringLiteral("network.primaryInterface"), QStringLiteral("Primary Interface")), primaryInterface},
        {text(QStringLiteral("network.activeInterfaces"), QStringLiteral("Active Interfaces")), interfaces},
        {text(QStringLiteral("network.downloadRate"), QStringLiteral("Download Rate")), downRate},
        {text(QStringLiteral("network.uploadRate"), QStringLiteral("Upload Rate")), upRate},
        {text(QStringLiteral("network.totalReceived"), QStringLiteral("Total Received")), formatBytes(snapshot.totalReceivedBytes)},
        {text(QStringLiteral("network.totalSent"), QStringLiteral("Total Sent")), formatBytes(snapshot.totalTransmittedBytes)}
    });

    m_infoPanel->setTiles({
        {text(QStringLiteral("network.primaryLink"), QStringLiteral("Primary Link")), primaryInterface},
        {text(QStringLiteral("network.interfaceCount"), QStringLiteral("Interface Count")), QString::number(snapshot.activeInterfaces.size())},
        {text(QStringLiteral("network.download"), QStringLiteral("Download")), downRate},
        {text(QStringLiteral("network.upload"), QStringLiteral("Upload")), upRate},
        {text(QStringLiteral("network.receivedTotal"), QStringLiteral("Received Total")), formatBytes(snapshot.totalReceivedBytes)},
        {text(QStringLiteral("network.sentTotal"), QStringLiteral("Sent Total")), formatBytes(snapshot.totalTransmittedBytes)}
    });
}

QString NetworkPanelWidget::formatBytes(quint64 bytes)
{
    return formatScaledBytes(static_cast<double>(bytes), QString());
}

QString NetworkPanelWidget::formatRate(double bytesPerSecond)
{
    return formatScaledBytes(qMax(0.0, bytesPerSecond), QStringLiteral("/s"));
}

QString NetworkPanelWidget::formatInterfaceSummary(const QStringList& interfaces) const
{
    if (interfaces.isEmpty()) {
        return text(QStringLiteral("network.waitingInterfaces"), QStringLiteral("Waiting for active network interfaces"));
    }

    return interfaces.join(QStringLiteral(", "));
}

QString NetworkPanelWidget::text(const QString& key, const QString& fallback) const
{
    return m_context ? m_context->translate(key, fallback) : fallback;
}

} // namespace Vitals
