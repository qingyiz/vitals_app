#include "platform/macos/MacCpuCollector.h"

#include <QByteArray>

#include <mach/mach.h>
#include <mach/processor_info.h>
#include <mach/vm_map.h>
#include <sys/sysctl.h>

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

int readIntSysctl(const char* name)
{
    int value = 0;
    size_t size = sizeof(value);
    if (sysctlbyname(name, &value, &size, nullptr, 0) != 0) {
        return 0;
    }
    return value;
}

double usagePercentForTicks(const MacCpuCollector::CpuTicks& previous, const MacCpuCollector::CpuTicks& current)
{
    const quint64 deltaUser = current.user - previous.user;
    const quint64 deltaSystem = current.system - previous.system;
    const quint64 deltaIdle = current.idle - previous.idle;
    const quint64 deltaNice = current.nice - previous.nice;
    const quint64 totalTicks = deltaUser + deltaSystem + deltaIdle + deltaNice;
    if (totalTicks == 0) {
        return 0.0;
    }

    const quint64 activeTicks = deltaUser + deltaSystem + deltaNice;
    return static_cast<double>(activeTicks) * 100.0 / static_cast<double>(totalTicks);
}

} // namespace

bool MacCpuCollector::initialize()
{
    m_cpuName = readStringSysctl("machdep.cpu.brand_string");
    m_logicalCoreCount = readIntSysctl("hw.logicalcpu");
    if (m_logicalCoreCount <= 0) {
        m_logicalCoreCount = readIntSysctl("hw.ncpu");
    }

    return sampleTicks(m_previousTicks);
}

CpuSnapshot MacCpuCollector::collect()
{
    CpuSnapshot snapshot;
    snapshot.cpuName = m_cpuName;
    snapshot.logicalCoreCount = m_logicalCoreCount;

    QList<CpuTicks> currentTicks;
    if (!sampleTicks(currentTicks)) {
        return snapshot;
    }

    if (m_previousTicks.size() != currentTicks.size()) {
        m_previousTicks = currentTicks;
        snapshot.logicalCoreCount = currentTicks.size();
        return snapshot;
    }

    snapshot.logicalCoreCount = currentTicks.size();

    double totalUsage = 0.0;
    double busiestUsage = 0.0;
    int busiestIndex = -1;

    for (int index = 0; index < currentTicks.size(); ++index) {
        const double usage = usagePercentForTicks(m_previousTicks.at(index), currentTicks.at(index));
        snapshot.perCoreUsagePercent.append(usage);
        totalUsage += usage;

        if (busiestIndex < 0 || usage > busiestUsage) {
            busiestUsage = usage;
            busiestIndex = index;
        }
    }

    if (!snapshot.perCoreUsagePercent.isEmpty()) {
        snapshot.totalUsagePercent = totalUsage / static_cast<double>(snapshot.perCoreUsagePercent.size());
        snapshot.busiestCoreIndex = busiestIndex;
        snapshot.busiestCoreUsagePercent = busiestUsage;
    }

    m_previousTicks = currentTicks;
    return snapshot;
}

int MacCpuCollector::logicalCoreCount() const
{
    return m_logicalCoreCount;
}

QString MacCpuCollector::cpuName() const
{
    return m_cpuName;
}

bool MacCpuCollector::sampleTicks(QList<CpuTicks>& ticks) const
{
    natural_t processorCount = 0;
    processor_info_array_t processorInfo;
    mach_msg_type_number_t processorInfoCount = 0;

    const kern_return_t result = host_processor_info(
        mach_host_self(),
        PROCESSOR_CPU_LOAD_INFO,
        &processorCount,
        &processorInfo,
        &processorInfoCount);
    if (result != KERN_SUCCESS) {
        return false;
    }

    ticks.clear();
    const processor_cpu_load_info_t cpuLoadInfo = reinterpret_cast<processor_cpu_load_info_t>(processorInfo);
    for (natural_t index = 0; index < processorCount; ++index) {
        CpuTicks entry;
        entry.user = cpuLoadInfo[index].cpu_ticks[CPU_STATE_USER];
        entry.system = cpuLoadInfo[index].cpu_ticks[CPU_STATE_SYSTEM];
        entry.idle = cpuLoadInfo[index].cpu_ticks[CPU_STATE_IDLE];
        entry.nice = cpuLoadInfo[index].cpu_ticks[CPU_STATE_NICE];
        ticks.append(entry);
    }

    vm_deallocate(
        mach_task_self(),
        reinterpret_cast<vm_address_t>(processorInfo),
        static_cast<vm_size_t>(processorInfoCount * sizeof(integer_t)));
    return true;
}

} // namespace Vitals
