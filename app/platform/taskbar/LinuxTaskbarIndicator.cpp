#include "platform/taskbar/LinuxTaskbarIndicator.h"

namespace Vitals {

QString LinuxTaskbarIndicator::platformName() const
{
    return QStringLiteral("Linux tray");
}

QColor LinuxTaskbarIndicator::accentColor() const
{
    return QColor(QStringLiteral("#26a269"));
}

} // namespace Vitals

