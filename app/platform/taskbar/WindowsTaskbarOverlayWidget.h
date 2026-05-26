#pragma once

#include <QWidget>

class QLabel;
class QMouseEvent;
class QTimer;

namespace Vitals {

class WindowsTaskbarOverlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WindowsTaskbarOverlayWidget(QWidget* parent = nullptr);

    void setDisplayText(const QString& text);
    void setDisplayTooltip(const QString& tooltip);
    void showInTaskbar();
    void hideFromTaskbar();
    void updatePlacement();

Q_SIGNALS:
    void detailRequested(const QRect& globalAnchorRect);

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QRect taskbarAnchorRect() const;
    QRect trayNotifyRect() const;
    void applyWindowStyle();

    QLabel* m_label = nullptr;
    QTimer* m_placementTimer = nullptr;
    bool m_shouldDisplay = false;
};

} // namespace Vitals
