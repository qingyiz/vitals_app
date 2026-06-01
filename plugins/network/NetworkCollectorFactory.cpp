#include "NetworkCollectorFactory.h"

#include "INetworkCollector.h"

#if defined(Q_OS_MAC)
#include "platform/macos/MacNetworkCollector.h"
#elif defined(Q_OS_WIN)
#include "platform/windows/WindowsNetworkCollector.h"
#endif

namespace Vitals {

std::unique_ptr<INetworkCollector> NetworkCollectorFactory::create()
{
#if defined(Q_OS_MAC)
    return std::make_unique<MacNetworkCollector>();
#elif defined(Q_OS_WIN)
    return std::make_unique<WindowsNetworkCollector>();
#else
    return nullptr;
#endif
}

} // namespace Vitals
