#include "MemoryCollectorFactory.h"

#include "IMemoryCollector.h"

#if defined(Q_OS_MAC)
#include "platform/macos/MacMemoryCollector.h"
#endif

namespace Vitals {

std::unique_ptr<IMemoryCollector> MemoryCollectorFactory::create()
{
#if defined(Q_OS_MAC)
    return std::make_unique<MacMemoryCollector>();
#else
    return nullptr;
#endif
}

} // namespace Vitals
