#include "platform/taskbar/WindowsTaskbarIndicator.h"

#include "platform/taskbar/WindowsTaskbarOverlayWidget.h"

namespace Vitals {

WindowsTaskbarIndicator::WindowsTaskbarIndicator(QObject* parent)
    : TaskbarIndicator(parent)
{
}

WindowsTaskbarIndicator::~WindowsTaskbarIndicator()
{
    qDeleteAll(m_overlayWidgets);
    m_overlayWidgets.clear();
}

void WindowsTaskbarIndicator::initialize(QWidget* mainWindow)
{
    TaskbarIndicator::initialize(mainWindow);
    refresh();
}

QString WindowsTaskbarIndicator::platformName() const
{
    return QStringLiteral("Windows taskbar");
}

QColor WindowsTaskbarIndicator::accentColor() const
{
    return QColor(QStringLiteral("#0078d4"));
}

int WindowsTaskbarIndicator::maximumVisibleLabelLength() const
{
    return 2;
}

bool WindowsTaskbarIndicator::usesDynamicTrayIcon() const
{
    return false;
}

bool WindowsTaskbarIndicator::usesTrayContextMenu() const
{
    return false;
}

void WindowsTaskbarIndicator::refresh()
{
    TaskbarIndicator::refresh();

    if (!hasPluginDisplays()) {
        for (WindowsTaskbarOverlayWidget* widget : m_overlayWidgets) {
            widget->hideFromTaskbar();
        }
        return;
    }

    rebuildOverlayWidgets();

    int anchorOffsetPx = 2;
    const QList<TaskbarPluginDisplay> displays = pluginDisplays();
    const int count = qMin(displays.size(), m_overlayWidgets.size());
    for (int index = 0; index < count; ++index) {
        WindowsTaskbarOverlayWidget* widget = m_overlayWidgets.at(index);
        const TaskbarPluginDisplay& display = displays.at(index);
        const QString text = overlayTextForDisplay(display);
        if (text.isEmpty()) {
            widget->hideFromTaskbar();
            continue;
        }

        widget->setDisplayText(text);
        widget->setDisplayTooltip(tooltipForDisplay(display, latestValues()));
        widget->setAnchorOffsetPx(anchorOffsetPx);
        widget->showInTaskbar();
        anchorOffsetPx += widget->width() + 2;
    }
}

QString WindowsTaskbarIndicator::overlayTextForDisplay(const TaskbarPluginDisplay& display) const
{
    const QString label = labelForDisplay(display, latestValues()).trimmed();
    if (label.contains(QLatin1Char('\n'))) {
        return label;
    }

    const QString simplified = label.simplified();
    if (simplified.isEmpty()) {
        return {};
    }

    const QStringList parts = simplified.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (parts.size() >= 2) {
        return QStringLiteral("%1\n%2").arg(parts.at(0), parts.mid(1).join(QStringLiteral(" ")));
    }
    return simplified;
}

void WindowsTaskbarIndicator::rebuildOverlayWidgets()
{
    const int displayCount = pluginDisplays().size();
    while (m_overlayWidgets.size() > displayCount) {
        WindowsTaskbarOverlayWidget* widget = m_overlayWidgets.takeLast();
        widget->hideFromTaskbar();
        widget->deleteLater();
    }

    while (m_overlayWidgets.size() < displayCount) {
        auto* widget = new WindowsTaskbarOverlayWidget();
        connect(widget, &WindowsTaskbarOverlayWidget::detailRequested, this,
            [this, widget](const QRect& anchorRect) {
                showDetailForOverlay(widget, anchorRect);
            });
        m_overlayWidgets.append(widget);
    }
}

void WindowsTaskbarIndicator::showDetailForOverlay(
    WindowsTaskbarOverlayWidget* widget,
    const QRect& anchorRect)
{
    const int index = m_overlayWidgets.indexOf(widget);
    if (index < 0 || index >= pluginDisplays().size()) {
        return;
    }

    const TaskbarDetailContent content = detailContentForDisplay(
        pluginDisplays().at(index),
        latestValues());
    if (content.isEmpty()) {
        return;
    }

    showDetailMenuNear(QList<TaskbarDetailContent>{content}, anchorRect);
}

} // namespace Vitals
