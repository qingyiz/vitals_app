#include "MemoryCollectorFactory.h"

#include "IMemoryCollector.h"

#if defined(Q_OS_MAC)
#include "platform/macos/MacMemoryCollector.h"
#elif defined(Q_OS_WIN)
#include "platform/windows/WindowsMemoryCollector.h"
#elif defined(Q_OS_LINUX)
#include "platform/linux/LinuxMemoryCollector.h"
#endif

namespace Vitals {

std::unique_ptr<IMemoryCollector> MemoryCollectorFactory::create()
{
#if defined(Q_OS_MAC)
    return std::make_unique<MacMemoryCollector>();
#elif defined(Q_OS_WIN)
    return std::make_unique<WindowsMemoryCollector>();
#elif defined(Q_OS_LINUX)
    return std::make_unique<LinuxMemoryCollector>();
#else
    return nullptr;
#endif
}

} // namespace Vitals
