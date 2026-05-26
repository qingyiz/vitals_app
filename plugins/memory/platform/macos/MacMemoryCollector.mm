#include "platform/macos/MacMemoryCollector.h"

#include <mach/mach.h>
#include <sys/sysctl.h>

namespace Vitals {

namespace {

quint64 readTotalMemory()
{
    quint64 value = 0;
    size_t size = sizeof(value);
    if (sysctlbyname("hw.memsize", &value, &size, nullptr, 0) != 0) {
        return 0;
    }
    return value;
}

} // namespace

bool MacMemoryCollector::initialize()
{
    m_totalBytes = readTotalMemory();
    return m_totalBytes > 0;
}

MemorySnapshot MacMemoryCollector::collect()
{
    MemorySnapshot snapshot;
    snapshot.totalBytes = m_totalBytes;
    if (m_totalBytes == 0) {
        return snapshot;
    }

    vm_size_t pageSize = 0;
    if (host_page_size(mach_host_self(), &pageSize) != KERN_SUCCESS || pageSize == 0) {
        return snapshot;
    }

    vm_statistics64_data_t stats;
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    if (host_statistics64(mach_host_self(), HOST_VM_INFO64,
            reinterpret_cast<host_info64_t>(&stats), &count) != KERN_SUCCESS) {
        return snapshot;
    }

    const quint64 usedPages = stats.active_count + stats.wire_count + stats.compressor_page_count;
    snapshot.usedBytes = qMin(usedPages * static_cast<quint64>(pageSize), snapshot.totalBytes);
    snapshot.freeBytes = snapshot.totalBytes - snapshot.usedBytes;
    snapshot.usagePercent = snapshot.totalBytes > 0
        ? static_cast<double>(snapshot.usedBytes) * 100.0 / static_cast<double>(snapshot.totalBytes)
        : 0.0;
    return snapshot;
}

} // namespace Vitals
