#pragma once

#include "platform/taskbar/TaskbarIndicator.h"

namespace Vitals {

class MacTaskbarIndicator : public TaskbarIndicator
{
    Q_OBJECT

public:
    using TaskbarIndicator::TaskbarIndicator;
    ~MacTaskbarIndicator() override;

    void initialize(QWidget* mainWindow) override;

protected:
    QString platformName() const override;
    QString idleText() const override;
    QColor accentColor() const override;
    bool prefersTextOnlyDisplay() const override;
    bool prefersSystemTintedText() const override;
    int maximumVisibleLabelLength() const override;
    void refresh() override;

private:
    class NativeBridge;
    NativeBridge* m_nativeBridge = nullptr;
};

} // namespace Vitals
