#include "ToggleSwitch.h"

#include <QPainter>
#include <QPaintEvent>

namespace Vitals {

ToggleSwitch::ToggleSwitch(QWidget* parent)
    : QAbstractButton(parent)
{
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

QSize ToggleSwitch::sizeHint() const
{
    return {42, 24};
}

void ToggleSwitch::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF trackRect(1.0, 1.0, width() - 2.0, height() - 2.0);
    const QColor trackColor = isChecked()
        ? QColor(QStringLiteral("#34c759"))
        : QColor(QStringLiteral("#c7ccd4"));

    painter.setPen(Qt::NoPen);
    painter.setBrush(trackColor);
    painter.drawRoundedRect(trackRect, trackRect.height() / 2.0, trackRect.height() / 2.0);

    const qreal knobSize = height() - 6.0;
    const qreal knobX = isChecked()
        ? width() - knobSize - 3.0
        : 3.0;

    painter.setBrush(Qt::white);
    painter.drawEllipse(QRectF(knobX, 3.0, knobSize, knobSize));
}

} // namespace Vitals
