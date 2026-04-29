#pragma once

#include "platform/taskbar/TaskbarIndicator.h"

namespace Vitals {

class WindowsTaskbarIndicator : public TaskbarIndicator
{
    Q_OBJECT

public:
    using TaskbarIndicator::TaskbarIndicator;

protected:
    QString platformName() const override;
    QColor accentColor() const override;
};

} // namespace Vitals

