#include "platform/taskbar/TaskbarIndicatorFactory.h"

#include <QtGlobal>

#if defined(Q_OS_WIN)
#include "platform/taskbar/WindowsTaskbarIndicator.h"
#elif defined(Q_OS_MAC)
#include "platform/taskbar/MacTaskbarIndicator.h"
#else
#include "platform/taskbar/LinuxTaskbarIndicator.h"
#endif

namespace Vitals {

TaskbarIndicator* createTaskbarIndicator(QObject* parent)
{
#if defined(Q_OS_WIN)
    return new WindowsTaskbarIndicator(parent);
#elif defined(Q_OS_MAC)
    return new MacTaskbarIndicator(parent);
#else
    return new LinuxTaskbarIndicator(parent);
#endif
}

} // namespace Vitals
