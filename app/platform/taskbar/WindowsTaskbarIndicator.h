#pragma once

#include "platform/taskbar/TaskbarIndicator.h"

namespace Vitals {

class WindowsTaskbarOverlayWidget;

class WindowsTaskbarIndicator : public TaskbarIndicator
{
    Q_OBJECT

public:
    explicit WindowsTaskbarIndicator(QObject* parent = nullptr);
    ~WindowsTaskbarIndicator() override;

    void initialize(QWidget* mainWindow) override;

protected:
    QString platformName() const override;
    QColor accentColor() const override;
    int maximumVisibleLabelLength() const override;
    bool usesDynamicTrayIcon() const override;
    bool usesTrayContextMenu() const override;
    void refresh() override;

private:
    QString overlayText() const;
    QString localizedBadgeLabel(const QString& label) const;

    WindowsTaskbarOverlayWidget* m_overlayWidget = nullptr;
};

} // namespace Vitals
