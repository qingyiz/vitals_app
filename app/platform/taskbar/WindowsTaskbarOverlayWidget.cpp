#include "platform/taskbar/WindowsTaskbarOverlayWidget.h"

#include <QApplication>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QRect>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

namespace Vitals {

namespace {

QRect rectFromWindow(HWND window)
{
    RECT rect = {};
    if (!window || !GetWindowRect(window, &rect)) {
        return {};
    }
    return QRect(
        QPoint(rect.left, rect.top),
        QPoint(rect.right - 1, rect.bottom - 1));
}

HWND findTrayNotifyWindow()
{
    HWND taskbar = FindWindowW(L"Shell_TrayWnd", nullptr);
    if (!taskbar) {
        return nullptr;
    }

    HWND tray = FindWindowExW(taskbar, nullptr, L"TrayNotifyWnd", nullptr);
    if (tray) {
        return tray;
    }

    HWND rebar = FindWindowExW(taskbar, nullptr, L"ReBarWindow32", nullptr);
    return rebar ? FindWindowExW(rebar, nullptr, L"TrayNotifyWnd", nullptr) : nullptr;
}

HWND findTaskbarWindow()
{
    return FindWindowW(L"Shell_TrayWnd", nullptr);
}

QRect appBarTaskbarRect()
{
    APPBARDATA data = {};
    data.cbSize = sizeof(data);
    if (!SHAppBarMessage(ABM_GETTASKBARPOS, &data)) {
        return {};
    }

    return QRect(
        QPoint(data.rc.left, data.rc.top),
        QPoint(data.rc.right - 1, data.rc.bottom - 1));
}

bool isHorizontalTaskbar(const QRect& taskbarRect)
{
    return taskbarRect.width() >= taskbarRect.height();
}

} // namespace

WindowsTaskbarOverlayWidget::WindowsTaskbarOverlayWidget(QWidget* parent)
    : QWidget(parent)
    , m_label(new QLabel(this))
    , m_placementTimer(new QTimer(this))
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setAttribute(Qt::WA_TranslucentBackground, false);
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::PointingHandCursor);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(3, 2, 3, 2);
    layout->setSpacing(0);

    QFont font(QStringLiteral("Microsoft YaHei UI"));
    if (!font.exactMatch()) {
        font = QApplication::font();
        font.setFamily(QStringLiteral("Segoe UI"));
    }
    font.setPixelSize(11);
    font.setWeight(QFont::Normal);
    m_label->setFont(font);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setMinimumSize(0, 26);
    m_label->setStyleSheet(QStringLiteral("color: white; background: transparent;"));
    m_label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    layout->addWidget(m_label);

    setFixedSize(34, 30);
    setStyleSheet(QStringLiteral("background: transparent;"));

    connect(m_placementTimer, &QTimer::timeout, this, &WindowsTaskbarOverlayWidget::updatePlacement);
    m_placementTimer->start(1500);
}

void WindowsTaskbarOverlayWidget::setDisplayText(const QString& text)
{
    m_label->setText(text);

    QFontMetrics metrics(m_label->font());
    int widestLine = 0;
    const QStringList lines = text.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        widestLine = qMax(widestLine, metrics.horizontalAdvance(line));
    }

    const int horizontalPadding = 8;
    const int compactWidth = qBound(32, widestLine + horizontalPadding, 72);
    if (width() != compactWidth) {
        setFixedWidth(compactWidth);
    }
}

void WindowsTaskbarOverlayWidget::setDisplayTooltip(const QString& tooltip)
{
    setToolTip(tooltip);
    m_label->setToolTip(tooltip);
}

void WindowsTaskbarOverlayWidget::setAnchorOffsetPx(int offsetPx)
{
    m_anchorOffsetPx = qMax(2, offsetPx);
    updatePlacement();
}

void WindowsTaskbarOverlayWidget::showInTaskbar()
{
    m_shouldDisplay = true;
    winId();
    updatePlacement();
    HWND hwnd = reinterpret_cast<HWND>(winId());
    if (hwnd) {
        ShowWindow(hwnd, SW_SHOWNOACTIVATE);
    }
    setVisible(true);
    updatePlacement();
}

void WindowsTaskbarOverlayWidget::hideFromTaskbar()
{
    m_shouldDisplay = false;
    HWND hwnd = reinterpret_cast<HWND>(winId());
    if (hwnd && IsWindow(hwnd)) {
        ShowWindow(hwnd, SW_HIDE);
    }
    setVisible(false);
    destroy(true, false);
}

void WindowsTaskbarOverlayWidget::updatePlacement()
{
    if (!m_shouldDisplay) {
        return;
    }

    HWND taskbarWindow = findTaskbarWindow();
    if (!taskbarWindow) {
        setVisible(false);
        return;
    }

    const QRect taskbarRect = taskbarAnchorRect();
    if (!taskbarRect.isValid()) {
        return;
    }

    const QRect trayRect = trayNotifyRect();
    const bool horizontal = isHorizontalTaskbar(taskbarRect);

    QPoint topLeft;
    if (horizontal) {
        const int anchorLeft = trayRect.isValid() ? trayRect.left() : taskbarRect.right();
        topLeft.setX(anchorLeft - width() - m_anchorOffsetPx);
        topLeft.setY(taskbarRect.top() + (taskbarRect.height() - height()) / 2);
    } else {
        const int anchorTop = trayRect.isValid() ? trayRect.top() : taskbarRect.bottom();
        topLeft.setX(taskbarRect.left() + (taskbarRect.width() - width()) / 2);
        topLeft.setY(anchorTop - height() - m_anchorOffsetPx);
    }

    QRect screenBounds;
    if (QScreen* screen = QGuiApplication::screenAt(taskbarRect.center())) {
        screenBounds = screen->geometry();
    }
    if (screenBounds.isValid()) {
        topLeft.setX(qBound(screenBounds.left(), topLeft.x(), screenBounds.right() - width() + 1));
        topLeft.setY(qBound(screenBounds.top(), topLeft.y(), screenBounds.bottom() - height() + 1));
    }

    applyWindowStyle();
    setVisible(true);

    const QPoint localTopLeft = topLeft - taskbarRect.topLeft();
    const int boundedX = qBound(0, localTopLeft.x(), qMax(0, taskbarRect.width() - width()));
    const int boundedY = qBound(0, localTopLeft.y(), qMax(0, taskbarRect.height() - height()));
    SetWindowPos(reinterpret_cast<HWND>(winId()), HWND_TOP,
        boundedX, boundedY, width(), height(),
        SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void WindowsTaskbarOverlayWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        Q_EMIT detailRequested(QRect(mapToGlobal(rect().topLeft()), size()));
        event->accept();
        return;
    }

    QWidget::mouseReleaseEvent(event);
}

QRect WindowsTaskbarOverlayWidget::taskbarAnchorRect() const
{
    const QRect appBarRect = appBarTaskbarRect();
    if (appBarRect.isValid()) {
        return appBarRect;
    }

    return rectFromWindow(FindWindowW(L"Shell_TrayWnd", nullptr));
}

QRect WindowsTaskbarOverlayWidget::trayNotifyRect() const
{
    return rectFromWindow(findTrayNotifyWindow());
}

void WindowsTaskbarOverlayWidget::applyWindowStyle()
{
    HWND hwnd = reinterpret_cast<HWND>(winId());
    if (!hwnd) {
        return;
    }

    HWND taskbarWindow = findTaskbarWindow();
    if (!taskbarWindow) {
        return;
    }

    if (GetParent(hwnd) != taskbarWindow) {
        SetParent(hwnd, taskbarWindow);
    }

    LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    style |= WS_CHILD | WS_VISIBLE;
    style &= ~(WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
    SetWindowLongPtrW(hwnd, GWL_STYLE, style);

    LONG_PTR exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    exStyle |= WS_EX_NOACTIVATE;
    exStyle &= ~(WS_EX_APPWINDOW | WS_EX_TOPMOST | WS_EX_TOOLWINDOW);
    SetWindowLongPtrW(hwnd, GWL_EXSTYLE, exStyle);
}

} // namespace Vitals
