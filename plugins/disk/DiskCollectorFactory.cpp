#include "DiskCollectorFactory.h"

#include "IDiskCollector.h"

#if defined(Q_OS_MAC)
#include "platform/macos/MacDiskCollector.h"
#endif

namespace Vitals {

std::unique_ptr<IDiskCollector> DiskCollectorFactory::create()
{
#if defined(Q_OS_MAC)
    return std::make_unique<MacDiskCollector>();
#else
    return nullptr;
#endif
}

} // namespace Vitals
