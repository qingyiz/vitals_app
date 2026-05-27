#include "platform/windows/WindowsCpuCollector.h"

#include <QSettings>
#include <QSysInfo>
#include <QtGlobal>
#include <QVector>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Vitals {

namespace {

using NtQuerySystemInformationFunction = LONG(WINAPI*)(ULONG, PVOID, ULONG, PULONG);

constexpr ULONG SystemProcessorPerformanceInformation = 8;
constexpr quint64 AllProcessorGroups = 0xffff;

struct SystemProcessorPerformanceInformationEntry
{
    LARGE_INTEGER idleTime;
    LARGE_INTEGER kernelTime;
    LARGE_INTEGER userTime;
    LARGE_INTEGER dpcTime;
    LARGE_INTEGER interruptTime;
    ULONG interruptCount;
};

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

int readLogicalCoreCount()
{
    const DWORD count = GetActiveProcessorCount(static_cast<WORD>(AllProcessorGroups));
    if (count != 0) {
        return static_cast<int>(count);
    }

    SYSTEM_INFO info = {};
    GetNativeSystemInfo(&info);
    return static_cast<int>(info.dwNumberOfProcessors);
}

NtQuerySystemInformationFunction resolveNtQuerySystemInformation()
{
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll) {
        return nullptr;
    }

    return reinterpret_cast<NtQuerySystemInformationFunction>(
        GetProcAddress(ntdll, "NtQuerySystemInformation"));
}

quint64 toUnsignedTicks(const LARGE_INTEGER& value)
{
    return value.QuadPart > 0 ? static_cast<quint64>(value.QuadPart) : 0ULL;
}

double usagePercentForTicks(const WindowsCpuCollector::CpuTicks& previous,
    const WindowsCpuCollector::CpuTicks& current)
{
    const quint64 deltaIdle = current.idle - previous.idle;
    const quint64 deltaKernel = current.kernel - previous.kernel;
    const quint64 deltaUser = current.user - previous.user;
    const quint64 totalTicks = deltaKernel + deltaUser;
    if (totalTicks == 0 || totalTicks < deltaIdle) {
        return 0.0;
    }

    return static_cast<double>(totalTicks - deltaIdle) * 100.0 / static_cast<double>(totalTicks);
}

} // namespace

bool WindowsCpuCollector::initialize()
{
    m_cpuName = readCpuModel();
    m_logicalCoreCount = readLogicalCoreCount();

    return sampleTicks(m_previousTicks);
}

CpuSnapshot WindowsCpuCollector::collect()
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
        m_logicalCoreCount = snapshot.logicalCoreCount;
        return snapshot;
    }

    snapshot.logicalCoreCount = currentTicks.size();
    m_logicalCoreCount = snapshot.logicalCoreCount;

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

int WindowsCpuCollector::logicalCoreCount() const
{
    return m_logicalCoreCount;
}

QString WindowsCpuCollector::cpuName() const
{
    return m_cpuName;
}

bool WindowsCpuCollector::sampleTicks(QList<CpuTicks>& ticks) const
{
    const NtQuerySystemInformationFunction querySystemInformation = resolveNtQuerySystemInformation();
    if (!querySystemInformation || m_logicalCoreCount <= 0) {
        return false;
    }

    QVector<SystemProcessorPerformanceInformationEntry> entries;
    entries.resize(m_logicalCoreCount);

    const ULONG bufferSize = static_cast<ULONG>(
        entries.size() * sizeof(SystemProcessorPerformanceInformationEntry));
    ULONG returnedSize = 0;
    const LONG status = querySystemInformation(
        SystemProcessorPerformanceInformation,
        entries.data(),
        bufferSize,
        &returnedSize);
    if (status < 0) {
        return false;
    }

    const int returnedCount = returnedSize > 0
        ? static_cast<int>(returnedSize / sizeof(SystemProcessorPerformanceInformationEntry))
        : entries.size();

    ticks.clear();
    for (int index = 0; index < qMin(entries.size(), returnedCount); ++index) {
        const auto& entry = entries.at(index);
        ticks.append({
            toUnsignedTicks(entry.idleTime),
            toUnsignedTicks(entry.kernelTime),
            toUnsignedTicks(entry.userTime)
        });
    }

    return !ticks.isEmpty();
}

} // namespace Vitals
