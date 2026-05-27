#include "platform/windows/WindowsMemoryCollector.h"

#include <QtGlobal>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Vitals {

namespace {

MemorySnapshot queryMemoryStatus()
{
    MEMORYSTATUSEX status = {};
    status.dwLength = sizeof(status);
    if (!GlobalMemoryStatusEx(&status)) {
        return {};
    }

    MemorySnapshot snapshot;
    snapshot.totalBytes = static_cast<quint64>(status.ullTotalPhys);
    snapshot.freeBytes = static_cast<quint64>(status.ullAvailPhys);
    if (snapshot.totalBytes >= snapshot.freeBytes) {
        snapshot.usedBytes = snapshot.totalBytes - snapshot.freeBytes;
    }
    snapshot.usagePercent = static_cast<double>(status.dwMemoryLoad);
    return snapshot;
}

} // namespace

bool WindowsMemoryCollector::initialize()
{
    return queryMemoryStatus().totalBytes > 0;
}

MemorySnapshot WindowsMemoryCollector::collect()
{
    return queryMemoryStatus();
}

} // namespace Vitals
