#include "platform/taskbar/WindowsTaskbarIndicator.h"

namespace Vitals {

QString WindowsTaskbarIndicator::platformName() const
{
    return QStringLiteral("Windows taskbar");
}

QColor WindowsTaskbarIndicator::accentColor() const
{
    return QColor(QStringLiteral("#0078d4"));
}

} // namespace Vitals

