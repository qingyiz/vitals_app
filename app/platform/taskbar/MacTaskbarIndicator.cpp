#include "platform/taskbar/MacTaskbarIndicator.h"

namespace Vitals {

QString MacTaskbarIndicator::platformName() const
{
    return QStringLiteral("macOS menu bar");
}

QString MacTaskbarIndicator::idleText() const
{
    return QStringLiteral("VT");
}

QColor MacTaskbarIndicator::accentColor() const
{
    return QColor(QStringLiteral("#1d1d1f"));
}

} // namespace Vitals

