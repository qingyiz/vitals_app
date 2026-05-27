#include "platform/taskbar/WindowsTaskbarIndicator.h"

#include "platform/taskbar/WindowsTaskbarOverlayWidget.h"

namespace Vitals {

WindowsTaskbarIndicator::WindowsTaskbarIndicator(QObject* parent)
    : TaskbarIndicator(parent)
{
}

WindowsTaskbarIndicator::~WindowsTaskbarIndicator()
{
    delete m_overlayWidget;
}

void WindowsTaskbarIndicator::initialize(QWidget* mainWindow)
{
    TaskbarIndicator::initialize(mainWindow);

    if (!m_overlayWidget) {
        m_overlayWidget = new WindowsTaskbarOverlayWidget();
        connect(m_overlayWidget, &WindowsTaskbarOverlayWidget::detailRequested,
            this, &WindowsTaskbarIndicator::showDetailMenuNear);
    }
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

    if (!m_overlayWidget) {
        return;
    }

    const QString text = overlayText();
    if (!hasPluginDisplays() || text.isEmpty()) {
        m_overlayWidget->hideFromTaskbar();
        return;
    }

    m_overlayWidget->setDisplayText(text);
    m_overlayWidget->setDisplayTooltip(currentTooltip());
    m_overlayWidget->showInTaskbar();
}

QString WindowsTaskbarIndicator::overlayText() const
{
    const QList<TaskbarDetailContent> contents = currentDetailContents();
    for (const TaskbarDetailContent& content : contents) {
        QStringList lines;
        for (const TaskbarDetailBadge& badge : content.badges) {
            if (badge.label == QStringLiteral("INTERVAL")) {
                continue;
            }
            lines.append(QStringLiteral("%1: %2")
                .arg(localizedBadgeLabel(badge.label), badge.value));
            if (lines.size() == 2) {
                return lines.join(QStringLiteral("\n"));
            }
        }
    }

    QString label = currentLabel().simplified();
    if (label.isEmpty()) {
        return {};
    }

    const QStringList parts = label.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (parts.size() >= 2) {
        return QStringLiteral("%1\n%2").arg(parts.at(0), parts.mid(1).join(QStringLiteral(" ")));
    }
    return label;
}

QString WindowsTaskbarIndicator::localizedBadgeLabel(const QString& label) const
{
    if (label == QStringLiteral("MEMORY")) {
        return QString::fromUtf8("\xE5\x86\x85\xE5\xAD\x98");
    }
    if (label == QStringLiteral("UPTIME")) {
        return QString::fromUtf8("\xE8\xBF\x90\xE8\xA1\x8C");
    }
    return label;
}

} // namespace Vitals
