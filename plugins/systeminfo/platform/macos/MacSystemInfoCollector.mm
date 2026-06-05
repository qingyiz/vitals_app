#include "platform/macos/MacSystemInfoCollector.h"

#include <QDateTime>
#include <QSysInfo>

#import <Metal/Metal.h>

#include <sys/sysctl.h>
#include <sys/time.h>

namespace Vitals {

namespace {

QString readStringSysctl(const char* name)
{
    size_t size = 0;
    if (sysctlbyname(name, nullptr, &size, nullptr, 0) != 0 || size == 0) {
        return {};
    }

    QByteArray buffer(static_cast<int>(size), Qt::Uninitialized);
    if (sysctlbyname(name, buffer.data(), &size, nullptr, 0) != 0) {
        return {};
    }

    return QString::fromUtf8(buffer.constData()).trimmed();
}

quint64 readUint64Sysctl(const char* name)
{
    quint64 value = 0;
    size_t size = sizeof(value);
    if (sysctlbyname(name, &value, &size, nullptr, 0) != 0) {
        return 0;
    }
    return value;
}

qint64 readBootTimeSeconds()
{
    struct timeval bootTime;
    size_t size = sizeof(bootTime);
    int mib[2] = {CTL_KERN, KERN_BOOTTIME};
    if (sysctl(mib, 2, &bootTime, &size, nullptr, 0) != 0) {
        return 0;
    }
    return static_cast<qint64>(bootTime.tv_sec);
}

QString readGpuModel()
{
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        return {};
    }

    NSString* name = device.name;
    return name ? QString::fromUtf8([name UTF8String]) : QString();
}

} // namespace

SystemInfoSnapshot MacSystemInfoCollector::collect()
{
    if (!m_hasStaticSnapshot) {
        m_staticSnapshot.deviceName = QSysInfo::machineHostName();
        m_staticSnapshot.osVersion = QSysInfo::prettyProductName();
        m_staticSnapshot.cpuModel = readStringSysctl("machdep.cpu.brand_string");
        m_staticSnapshot.gpuModel = readGpuModel();
        m_staticSnapshot.totalMemoryBytes = readUint64Sysctl("hw.memsize");
        m_hasStaticSnapshot = true;
    }

    SystemInfoSnapshot snapshot = m_staticSnapshot;
    const qint64 bootSeconds = readBootTimeSeconds();
    if (bootSeconds > 0) {
        snapshot.uptimeSeconds = QDateTime::currentSecsSinceEpoch() - bootSeconds;
    }

    return snapshot;
}

} // namespace Vitals
