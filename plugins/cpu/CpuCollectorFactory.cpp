#include "CpuCollectorFactory.h"

#include "ICpuCollector.h"

#if defined(Q_OS_MAC)
#include "platform/macos/MacCpuCollector.h"
#elif defined(Q_OS_WIN)
#include "platform/windows/WindowsCpuCollector.h"
#endif

namespace Vitals {

std::unique_ptr<ICpuCollector> CpuCollectorFactory::create()
{
#if defined(Q_OS_MAC)
    return std::make_unique<MacCpuCollector>();
#elif defined(Q_OS_WIN)
    return std::make_unique<WindowsCpuCollector>();
#else
    return nullptr;
#endif
}

} // namespace Vitals
