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
    QString overlayTextForDisplay(const TaskbarPluginDisplay& display) const;
    void rebuildOverlayWidgets();
    void showDetailForOverlay(WindowsTaskbarOverlayWidget* widget, const QRect& anchorRect);

    QList<WindowsTaskbarOverlayWidget*> m_overlayWidgets;
};

} // namespace Vitals
