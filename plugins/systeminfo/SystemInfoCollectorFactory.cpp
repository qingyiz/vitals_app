#include "SystemInfoCollectorFactory.h"

#include "ISystemInfoCollector.h"

#if defined(Q_OS_MAC)
#include "platform/macos/MacSystemInfoCollector.h"
#endif

namespace Vitals {

std::unique_ptr<ISystemInfoCollector> SystemInfoCollectorFactory::create()
{
#if defined(Q_OS_MAC)
    return std::make_unique<MacSystemInfoCollector>();
#else
    return nullptr;
#endif
}

} // namespace Vitals
