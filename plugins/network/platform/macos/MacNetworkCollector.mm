#include "platform/macos/MacNetworkCollector.h"

#include <QDateTime>
#include <QStringList>

#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>

namespace Vitals {

bool MacNetworkCollector::initialize()
{
    m_previousSampleMs = QDateTime::currentMSecsSinceEpoch();
    return sampleTotals(m_previousTotals);
}

NetworkSnapshot MacNetworkCollector::collect()
{
    NetworkSnapshot snapshot;

    Totals currentTotals;
    if (!sampleTotals(currentTotals)) {
        return snapshot;
    }

    snapshot.primaryInterface = currentTotals.primaryInterface;
    snapshot.activeInterfaces = currentTotals.activeInterfaces;
    snapshot.totalReceivedBytes = currentTotals.receivedBytes;
    snapshot.totalTransmittedBytes = currentTotals.transmittedBytes;

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    const qint64 elapsedMs = nowMs - m_previousSampleMs;
    if (m_previousSampleMs > 0 && elapsedMs > 0) {
        const double elapsedSeconds = static_cast<double>(elapsedMs) / 1000.0;
        const quint64 receivedDelta = currentTotals.receivedBytes >= m_previousTotals.receivedBytes
            ? currentTotals.receivedBytes - m_previousTotals.receivedBytes
            : 0;
        const quint64 transmittedDelta = currentTotals.transmittedBytes >= m_previousTotals.transmittedBytes
            ? currentTotals.transmittedBytes - m_previousTotals.transmittedBytes
            : 0;

        snapshot.receiveBytesPerSecond = static_cast<double>(receivedDelta) / elapsedSeconds;
        snapshot.transmitBytesPerSecond = static_cast<double>(transmittedDelta) / elapsedSeconds;
    }

    m_previousTotals = currentTotals;
    m_previousSampleMs = nowMs;
    return snapshot;
}

bool MacNetworkCollector::sampleTotals(Totals& totals) const
{
    ifaddrs* interfaces = nullptr;
    if (getifaddrs(&interfaces) != 0 || !interfaces) {
        return false;
    }

    quint64 busiestTraffic = 0;
    for (ifaddrs* entry = interfaces; entry != nullptr; entry = entry->ifa_next) {
        if (!entry->ifa_addr || !entry->ifa_data) {
            continue;
        }

        if (entry->ifa_addr->sa_family != AF_LINK) {
            continue;
        }

        if ((entry->ifa_flags & IFF_UP) == 0 || (entry->ifa_flags & IFF_LOOPBACK) != 0) {
            continue;
        }

        const auto* stats = reinterpret_cast<if_data*>(entry->ifa_data);
        if (!stats) {
            continue;
        }

        const QString interfaceName = QString::fromUtf8(entry->ifa_name);
        if (!totals.activeInterfaces.contains(interfaceName)) {
            totals.activeInterfaces.append(interfaceName);
        }

        totals.receivedBytes += stats->ifi_ibytes;
        totals.transmittedBytes += stats->ifi_obytes;

        const quint64 combinedTraffic = stats->ifi_ibytes + stats->ifi_obytes;
        if (combinedTraffic >= busiestTraffic) {
            busiestTraffic = combinedTraffic;
            totals.primaryInterface = interfaceName;
        }
    }

    freeifaddrs(interfaces);
    return true;
}

} // namespace Vitals
