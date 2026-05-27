#include "SystemInfoCollectorFactory.h"

#include "ISystemInfoCollector.h"

#if defined(Q_OS_MAC)
#include "platform/macos/MacSystemInfoCollector.h"
#elif defined(Q_OS_WIN)
#include "platform/windows/WindowsSystemInfoCollector.h"
#endif

namespace Vitals {

std::unique_ptr<ISystemInfoCollector> SystemInfoCollectorFactory::create()
{
#if defined(Q_OS_MAC)
    return std::make_unique<MacSystemInfoCollector>();
#elif defined(Q_OS_WIN)
    return std::make_unique<WindowsSystemInfoCollector>();
#else
    return nullptr;
#endif
}

} // namespace Vitals
