#include "NetworkCollectorFactory.h"

#include "INetworkCollector.h"

#if defined(Q_OS_MAC)
#include "platform/macos/MacNetworkCollector.h"
#endif

namespace Vitals {

std::unique_ptr<INetworkCollector> NetworkCollectorFactory::create()
{
#if defined(Q_OS_MAC)
    return std::make_unique<MacNetworkCollector>();
#else
    return nullptr;
#endif
}

} // namespace Vitals
