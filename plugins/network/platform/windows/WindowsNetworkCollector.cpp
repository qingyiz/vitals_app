#include "platform/windows/WindowsNetworkCollector.h"

#include <QDateTime>
#include <QChar>
#include <QString>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2def.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>

namespace Vitals {

namespace {

bool containsCjk(const QString& text)
{
    for (const QChar character : text) {
        const ushort code = character.unicode();
        if ((code >= 0x3400 && code <= 0x9fff)
            || (code >= 0xf900 && code <= 0xfaff)) {
            return true;
        }
    }
    return false;
}

QString adapterDisplayName(const MIB_IF_ROW2& row)
{
    switch (row.Type) {
    case IF_TYPE_IEEE80211:
        return QStringLiteral("Wi-Fi");
    case IF_TYPE_ETHERNET_CSMACD:
        return QStringLiteral("Ethernet");
    case IF_TYPE_PPP:
        return QStringLiteral("PPP");
    case IF_TYPE_TUNNEL:
        return QStringLiteral("Tunnel");
    default:
        break;
    }

    QString name = QString::fromWCharArray(row.Description).trimmed();
    if (containsCjk(name)) {
        name.clear();
    }

    if (name.isEmpty()) {
        name = QStringLiteral("Interface %1").arg(row.InterfaceIndex);
    }
    return name;
}

bool isUsableAdapter(const MIB_IF_ROW2& row)
{
    if (row.OperStatus != IfOperStatusUp) {
        return false;
    }

    if (row.Type == IF_TYPE_SOFTWARE_LOOPBACK) {
        return false;
    }

    if (!row.InterfaceAndOperStatusFlags.HardwareInterface) {
        return false;
    }

    if (row.InterfaceAndOperStatusFlags.FilterInterface
        || row.InterfaceAndOperStatusFlags.EndPointInterface
        || row.InterfaceAndOperStatusFlags.NotMediaConnected) {
        return false;
    }

    return true;
}

} // namespace

bool WindowsNetworkCollector::initialize()
{
    m_previousSampleMs = QDateTime::currentMSecsSinceEpoch();
    return sampleTotals(m_previousTotals);
}

NetworkSnapshot WindowsNetworkCollector::collect()
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

bool WindowsNetworkCollector::sampleTotals(Totals& totals) const
{
    MIB_IF_TABLE2* table = nullptr;
    const NETIO_STATUS status = GetIfTable2(&table);
    if (status != NO_ERROR || !table) {
        return false;
    }

    quint64 busiestTraffic = 0;
    for (ULONG index = 0; index < table->NumEntries; ++index) {
        const MIB_IF_ROW2& row = table->Table[index];
        if (!isUsableAdapter(row)) {
            continue;
        }

        const QString interfaceName = adapterDisplayName(row);
        if (!totals.activeInterfaces.contains(interfaceName)) {
            totals.activeInterfaces.append(interfaceName);
        }

        const quint64 receivedBytes = static_cast<quint64>(row.InOctets);
        const quint64 transmittedBytes = static_cast<quint64>(row.OutOctets);
        totals.receivedBytes += receivedBytes;
        totals.transmittedBytes += transmittedBytes;

        const quint64 combinedTraffic = receivedBytes + transmittedBytes;
        if (combinedTraffic >= busiestTraffic) {
            busiestTraffic = combinedTraffic;
            totals.primaryInterface = interfaceName;
        }
    }

    FreeMibTable(table);
    return true;
}

} // namespace Vitals
