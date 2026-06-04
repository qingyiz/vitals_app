#include "platform/windows/WindowsSystemInfoCollector.h"

#include <QSettings>
#include <QString>
#include <QSysInfo>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Vitals {

namespace {

QString readCpuModel()
{
    QSettings processor(
        QStringLiteral("HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"),
        QSettings::NativeFormat);

    const QString processorName = processor.value(QStringLiteral("ProcessorNameString")).toString().trimmed();
    if (!processorName.isEmpty()) {
        return processorName;
    }

    return QSysInfo::currentCpuArchitecture();
}

QString displayDeviceName(const DISPLAY_DEVICEW& device)
{
    return QString::fromWCharArray(device.DeviceString).trimmed();
}

QString readGpuModel()
{
    QString firstActiveDevice;

    for (DWORD index = 0;; ++index) {
        DISPLAY_DEVICEW device = {};
        device.cb = sizeof(device);

        if (!EnumDisplayDevicesW(nullptr, index, &device, 0)) {
            break;
        }

        if (device.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) {
            continue;
        }

        const QString name = displayDeviceName(device);
        if (name.isEmpty()) {
            continue;
        }

        if (firstActiveDevice.isEmpty() && (device.StateFlags & DISPLAY_DEVICE_ACTIVE)) {
            firstActiveDevice = name;
        }

        if (device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
            return name;
        }
    }

    return firstActiveDevice;
}

quint64 readTotalMemoryBytes()
{
    MEMORYSTATUSEX status = {};
    status.dwLength = sizeof(status);
    if (!GlobalMemoryStatusEx(&status)) {
        return 0;
    }

    return static_cast<quint64>(status.ullTotalPhys);
}

qint64 readUptimeSeconds()
{
    return static_cast<qint64>(GetTickCount64() / 1000ULL);
}

} // namespace

SystemInfoSnapshot WindowsSystemInfoCollector::collect()
{
    if (!m_hasStaticSnapshot) {
        m_staticSnapshot.deviceName = QSysInfo::machineHostName();
        m_staticSnapshot.osVersion = QSysInfo::prettyProductName();
        m_staticSnapshot.cpuModel = readCpuModel();
        m_staticSnapshot.gpuModel = readGpuModel();
        m_staticSnapshot.totalMemoryBytes = readTotalMemoryBytes();
        m_hasStaticSnapshot = true;
    }

    SystemInfoSnapshot snapshot = m_staticSnapshot;
    snapshot.uptimeSeconds = readUptimeSeconds();

    return snapshot;
}

} // namespace Vitals
