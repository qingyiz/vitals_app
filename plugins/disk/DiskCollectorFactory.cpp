#include "DiskCollectorFactory.h"

#include "IDiskCollector.h"

#if defined(Q_OS_MAC)
#include "platform/macos/MacDiskCollector.h"
#endif

#if defined(Q_OS_WIN)
#include "platform/windows/WindowsDiskCollector.h"
#endif

namespace Vitals {

std::unique_ptr<IDiskCollector> DiskCollectorFactory::create()
{
#if defined(Q_OS_MAC)
    return std::make_unique<MacDiskCollector>();
#elif defined(Q_OS_WIN)
    return std::make_unique<WindowsDiskCollector>();
#else
    return nullptr;
#endif
}

} // namespace Vitals
