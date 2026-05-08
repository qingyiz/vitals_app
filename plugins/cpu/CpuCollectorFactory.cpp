#include "CpuCollectorFactory.h"

#include "ICpuCollector.h"

#if defined(Q_OS_MAC)
#include "platform/macos/MacCpuCollector.h"
#endif

namespace Vitals {

std::unique_ptr<ICpuCollector> CpuCollectorFactory::create()
{
#if defined(Q_OS_MAC)
    return std::make_unique<MacCpuCollector>();
#else
    return nullptr;
#endif
}

} // namespace Vitals
