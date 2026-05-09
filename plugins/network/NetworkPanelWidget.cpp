#include "NetworkPanelWidget.h"

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

NetworkPanelWidget::NetworkPanelWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    m_infoPanel = new InfoPanelWidget(this);
    m_infoPanel->setPageTitle(QStringLiteral("Network Monitor"));
    m_infoPanel->setPageSubtitle(
        QStringLiteral("Live interface throughput and traffic counters collected from the macOS networking stack."));
    m_infoPanel->setDetailsTitle(QStringLiteral("Current Throughput"));
    m_infoPanel->setHeroEyebrow(QStringLiteral("NETWORK"));

    rootLayout->addWidget(m_infoPanel);
}

void NetworkPanelWidget::applySnapshot(const NetworkSnapshot& snapshot)
{
    const QString downRate = formatRate(snapshot.receiveBytesPerSecond);
    const QString upRate = formatRate(snapshot.transmitBytesPerSecond);
    const QString interfaces = formatInterfaceSummary(snapshot.activeInterfaces);
    const QString primaryInterface = snapshot.primaryInterface.isEmpty()
        ? QStringLiteral("No active uplink")
        : snapshot.primaryInterface;

    m_infoPanel->setHeroTitle(QStringLiteral("↓ %1  |  ↑ %2").arg(downRate, upRate));
    m_infoPanel->setHeroSubtitle(primaryInterface);
    m_infoPanel->setHeroMeta(interfaces);

    m_infoPanel->setBadges({
        {QStringLiteral("DOWN"), downRate},
        {QStringLiteral("UP"), upRate},
        {QStringLiteral("LINKS"), QString::number(snapshot.activeInterfaces.size())}
    });

    m_infoPanel->setDetailsRows({
        {QStringLiteral("Primary Interface"), primaryInterface},
        {QStringLiteral("Active Interfaces"), interfaces},
        {QStringLiteral("Download Rate"), downRate},
        {QStringLiteral("Upload Rate"), upRate},
        {QStringLiteral("Total Received"), formatBytes(snapshot.totalReceivedBytes)},
        {QStringLiteral("Total Sent"), formatBytes(snapshot.totalTransmittedBytes)}
    });

    m_infoPanel->setTiles({
        {QStringLiteral("Primary Link"), primaryInterface},
        {QStringLiteral("Interface Count"), QString::number(snapshot.activeInterfaces.size())},
        {QStringLiteral("Download"), downRate},
        {QStringLiteral("Upload"), upRate},
        {QStringLiteral("Received Total"), formatBytes(snapshot.totalReceivedBytes)},
        {QStringLiteral("Sent Total"), formatBytes(snapshot.totalTransmittedBytes)}
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

QString NetworkPanelWidget::formatInterfaceSummary(const QStringList& interfaces)
{
    if (interfaces.isEmpty()) {
        return QStringLiteral("Waiting for active network interfaces");
    }

    return interfaces.join(QStringLiteral(", "));
}

} // namespace Vitals
