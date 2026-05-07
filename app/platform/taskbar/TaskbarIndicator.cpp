#include "platform/taskbar/TaskbarIndicator.h"

#include "metric/MetricCenter.h"

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QFontMetrics>
#include <QMenu>
#include <QPalette>
#include <QPainter>
#include <QPixmap>
#include <QRectF>
#include <QSystemTrayIcon>
#include <QtMath>

namespace Vitals {

namespace {

QColor preferredTextColor()
{
    QColor color = QApplication::palette().color(QPalette::WindowText);
    if (!color.isValid()) {
        color = QColor(QStringLiteral("#111111"));
    }
    color.setAlpha(245);
    return color;
}

QColor preferredTextHaloColor(const QColor& textColor)
{
    if (textColor.lightness() < 128) {
        return QColor(255, 255, 255, 72);
    }
    return QColor(0, 0, 0, 92);
}

struct TextLayout
{
    QStringList lines;
    int fontPixelSize = 8;
    int width = 0;
    int height = 0;
};

QString normalizedLabel(const QString& label)
{
    return label.simplified();
}

QStringList tokenizedLabel(const QString& label)
{
    return normalizedLabel(label).split(QLatin1Char(' '), Qt::SkipEmptyParts);
}

QList<QStringList> candidateLayouts(const QString& label)
{
    QList<QStringList> candidates;
    const QString normalized = normalizedLabel(label);
    if (normalized.isEmpty()) {
        candidates.append({QString()});
        return candidates;
    }

    candidates.append({normalized});

    const QStringList explicitLines = label.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    if (explicitLines.size() >= 2) {
        candidates.prepend({explicitLines.at(0).simplified(), explicitLines.at(1).simplified()});
    }

    const QStringList tokens = tokenizedLabel(label);
    for (int splitIndex = 1; splitIndex < tokens.size(); ++splitIndex) {
        candidates.append({
            tokens.mid(0, splitIndex).join(QStringLiteral(" ")),
            tokens.mid(splitIndex).join(QStringLiteral(" "))
        });
    }

    if (!normalized.contains(QLatin1Char(' ')) && normalized.size() >= 4) {
        const int midpoint = normalized.size() / 2;
        candidates.append({
            normalized.left(midpoint),
            normalized.mid(midpoint)
        });
    }

    return candidates;
}

TextLayout bestTextLayout(const QString& label)
{
    constexpr int kMaxWidth = 40;
    constexpr int kMinWidth = 22;
    constexpr int kMaxHeight = 22;
    constexpr int kHorizontalPadding = 4;

    TextLayout best;
    best.lines = QStringList{normalizedLabel(label)};

    const QList<QStringList> candidates = candidateLayouts(label);
    for (const QStringList& candidateLines : candidates) {
        QStringList lines;
        for (const QString& line : candidateLines) {
            if (!line.trimmed().isEmpty()) {
                lines.append(line.trimmed());
            }
        }
        if (lines.isEmpty() || lines.size() > 2) {
            continue;
        }

        for (int fontPixelSize = 18; fontPixelSize >= 8; --fontPixelSize) {
            QFont font = QApplication::font();
            font.setWeight(QFont::DemiBold);
            font.setPixelSize(fontPixelSize);
            QFontMetrics metrics(font);

            int widestLine = 0;
            for (const QString& line : lines) {
                widestLine = qMax(widestLine, metrics.horizontalAdvance(line));
            }

            const int totalHeight = lines.size() * metrics.height();
            const int totalWidth = widestLine + kHorizontalPadding * 2;
            if (totalHeight > kMaxHeight || totalWidth > kMaxWidth) {
                continue;
            }

            const bool isBetter = fontPixelSize > best.fontPixelSize
                || (fontPixelSize == best.fontPixelSize && totalWidth < best.width);
            if (isBetter) {
                best.lines = lines;
                best.fontPixelSize = fontPixelSize;
                best.width = qMax(kMinWidth, totalWidth);
                best.height = kMaxHeight;
            }
            break;
        }
    }

    if (best.width <= 0) {
        QFont font = QApplication::font();
        font.setWeight(QFont::DemiBold);
        font.setPixelSize(best.fontPixelSize);
        QFontMetrics metrics(font);

        int widestLine = 0;
        for (const QString& line : best.lines) {
            widestLine = qMax(widestLine, metrics.horizontalAdvance(line));
        }

        best.width = qMin(kMaxWidth, qMax(kMinWidth, widestLine + kHorizontalPadding * 2));
        best.height = kMaxHeight;
    }

    return best;
}

} // namespace

TaskbarIndicator::TaskbarIndicator(QObject* parent)
    : QObject(parent)
{
}

TaskbarIndicator::~TaskbarIndicator() = default;

void TaskbarIndicator::initialize(QWidget* mainWindow)
{
    setMainWindow(mainWindow);

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning() << "System tray is not available on this platform/session";
        return;
    }

    m_menu = new QMenu(m_mainWindow);
    m_summaryAction = m_menu->addAction(QStringLiteral("Vitals: %1").arg(idleText()));
    m_summaryAction->setEnabled(false);
    m_menu->addSeparator();
    m_showAction = m_menu->addAction(QStringLiteral("Show Vitals"));
    m_quitAction = m_menu->addAction(QStringLiteral("Quit"));

    connect(m_showAction, &QAction::triggered, this, &TaskbarIndicator::showRequested);
    connect(m_quitAction, &QAction::triggered, this, &TaskbarIndicator::quitRequested);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu(m_menu);
    m_trayIcon->setToolTip(QStringLiteral("Vitals %1").arg(platformName()));
    m_trayIcon->setIcon(buildIcon(idleText()));
    connect(m_trayIcon, &QSystemTrayIcon::activated, this,
        [this](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger
                || reason == QSystemTrayIcon::DoubleClick) {
                Q_EMIT showRequested();
            }
        });
    refresh();
}

void TaskbarIndicator::bindMetricCenter(MetricCenter* metricCenter)
{
    connect(metricCenter, &MetricCenter::metricUpdated,
        this, &TaskbarIndicator::handleMetricUpdated);
    connect(metricCenter, &MetricCenter::metricRemoved,
        this, &TaskbarIndicator::handleMetricRemoved);
}

void TaskbarIndicator::setPluginDisplays(const QList<TaskbarPluginDisplay>& pluginDisplays)
{
    m_pluginDisplays = pluginDisplays;
    refresh();
}

QString TaskbarIndicator::idleText() const
{
    return QStringLiteral("OK");
}

QString TaskbarIndicator::iconLabel(const QHash<QString, MetricValue>& latestValues) const
{
    QStringList segments;
    for (const TaskbarPluginDisplay& display : m_pluginDisplays) {
        if (!display.provider) {
            continue;
        }

        const QString text = display.provider->taskbarDisplayText(latestValues).trimmed();
        if (!text.isEmpty()) {
            segments.append(text);
        }
    }

    if (!segments.isEmpty()) {
        return segments.join(QStringLiteral("  "));
    }

    const QStringList priorityKeys = {
        QStringLiteral("cpu.usage.total"),
        QStringLiteral("memory.usage.percent"),
        QStringLiteral("hello.plugin.status")
    };

    for (const QString& key : priorityKeys) {
        if (latestValues.contains(key)) {
            const MetricValue value = latestValues.value(key);
            if (key == QStringLiteral("cpu.usage.total")) {
                return QStringLiteral("C%1").arg(value.value.toInt());
            }
            if (key == QStringLiteral("memory.usage.percent")) {
                return QStringLiteral("M%1").arg(value.value.toInt());
            }
            return QStringLiteral("OK");
        }
    }

    return idleText();
}

QString TaskbarIndicator::tooltipText(const QHash<QString, MetricValue>& latestValues) const
{
    if (latestValues.isEmpty()) {
        return QStringLiteral("Vitals %1\nWaiting for plugin metrics").arg(platformName());
    }

    QStringList lines;
    lines.append(QStringLiteral("Vitals %1").arg(platformName()));
    for (const TaskbarPluginDisplay& display : m_pluginDisplays) {
        if (!display.provider) {
            continue;
        }

        const QString label = display.provider->taskbarDisplayText(latestValues).trimmed();
        if (label.isEmpty()) {
            continue;
        }

        QString tooltip = display.provider->taskbarDisplayTooltip(latestValues).trimmed();
        if (tooltip.isEmpty()) {
            tooltip = label;
        }

        lines.append(QStringLiteral("%1: %2").arg(display.pluginName, tooltip));
    }

    if (lines.size() > 1) {
        return lines.join(QStringLiteral("\n"));
    }

    for (const QString& key : orderedMetricKeys()) {
        const MetricValue value = latestValues.value(key);
        lines.append(QStringLiteral("%1: %2").arg(humanizedMetricName(key), formatMetricValue(value)));
        if (lines.size() >= 6) {
            break;
        }
    }
    return lines.join(QStringLiteral("\n"));
}

QColor TaskbarIndicator::accentColor() const
{
    return QColor(QStringLiteral("#2f7cf6"));
}

bool TaskbarIndicator::prefersTextOnlyDisplay() const
{
    return false;
}

bool TaskbarIndicator::prefersSystemTintedText() const
{
    return false;
}

int TaskbarIndicator::maximumVisibleLabelLength() const
{
    return 3;
}

QWidget* TaskbarIndicator::mainWindow() const
{
    return m_mainWindow;
}

void TaskbarIndicator::setMainWindow(QWidget* mainWindow)
{
    m_mainWindow = mainWindow;
}

bool TaskbarIndicator::hasPluginDisplays() const
{
    return !m_pluginDisplays.isEmpty();
}

QString TaskbarIndicator::currentLabel() const
{
    return iconLabel(m_latestValues);
}

QString TaskbarIndicator::currentTooltip() const
{
    return tooltipText(m_latestValues);
}

void TaskbarIndicator::emitShowRequested()
{
    Q_EMIT showRequested();
}

void TaskbarIndicator::emitQuitRequested()
{
    Q_EMIT quitRequested();
}

QString TaskbarIndicator::formatMetricValue(const MetricValue& value) const
{
    if (value.value.type() == QVariant::Double) {
        return QString::number(value.value.toDouble(), 'f', 1);
    }
    return value.value.toString();
}

QString TaskbarIndicator::humanizedMetricName(const QString& key) const
{
    if (key == QStringLiteral("cpu.usage.total")) return QStringLiteral("CPU");
    if (key == QStringLiteral("memory.usage.percent")) return QStringLiteral("Memory");
    if (key == QStringLiteral("network.upload.speed")) return QStringLiteral("Upload");
    if (key == QStringLiteral("network.download.speed")) return QStringLiteral("Download");
    if (key == QStringLiteral("disk.read.speed")) return QStringLiteral("Disk Read");
    if (key == QStringLiteral("disk.write.speed")) return QStringLiteral("Disk Write");
    if (key == QStringLiteral("battery.level.percent")) return QStringLiteral("Battery");
    if (key == QStringLiteral("hello.plugin.status")) return QStringLiteral("Hello");
    if (key == QStringLiteral("system.device.name")) return QStringLiteral("Device");
    if (key == QStringLiteral("system.os.version")) return QStringLiteral("OS");
    if (key == QStringLiteral("system.cpu.model")) return QStringLiteral("CPU");
    if (key == QStringLiteral("system.gpu.model")) return QStringLiteral("GPU");
    if (key == QStringLiteral("system.memory.total.bytes")) return QStringLiteral("Memory");
    if (key == QStringLiteral("system.uptime.seconds")) return QStringLiteral("Uptime");
    return key;
}

void TaskbarIndicator::handleMetricUpdated(const MetricValue& value)
{
    m_latestValues.insert(value.key, value);
    refresh();
}

void TaskbarIndicator::handleMetricRemoved(const QString& key)
{
    m_latestValues.remove(key);
    refresh();
}

void TaskbarIndicator::refresh()
{
    if (!m_trayIcon) {
        return;
    }

    if (!hasPluginDisplays()) {
        m_trayIcon->hide();
        if (m_summaryAction) {
            m_summaryAction->setText(QStringLiteral("Vitals: taskbar display disabled"));
        }
        return;
    }

    const QString label = currentLabel();
    const QString tooltip = currentTooltip();
    m_trayIcon->setIcon(buildIcon(label));
    m_trayIcon->setToolTip(tooltip);
    if (!m_trayIcon->isVisible()) {
        m_trayIcon->show();
    }

    if (m_summaryAction) {
        m_summaryAction->setText(tooltip.split(QStringLiteral("\n")).join(QStringLiteral("  |  ")));
    }
}

QStringList TaskbarIndicator::orderedMetricKeys() const
{
    QStringList priority = {
        QStringLiteral("cpu.usage.total"),
        QStringLiteral("memory.usage.percent"),
        QStringLiteral("network.upload.speed"),
        QStringLiteral("network.download.speed"),
        QStringLiteral("disk.read.speed"),
        QStringLiteral("disk.write.speed"),
        QStringLiteral("battery.level.percent"),
        QStringLiteral("hello.plugin.status")
    };

    QStringList result;
    for (const QString& key : priority) {
        if (m_latestValues.contains(key)) {
            result.append(key);
        }
    }

    const QStringList keys = m_latestValues.keys();
    for (const QString& key : keys) {
        if (!result.contains(key)) {
            result.append(key);
        }
    }
    return result;
}

QIcon TaskbarIndicator::buildIcon(const QString& label) const
{
    return QIcon(buildPixmap(label));
}

QPixmap TaskbarIndicator::buildPixmap(const QString& label) const
{
    const QString visibleLabel = label.left(maximumVisibleLabelLength());

    if (prefersTextOnlyDisplay()) {
        const qreal devicePixelRatio = m_mainWindow ? m_mainWindow->devicePixelRatioF() : qApp->devicePixelRatio();
        const TextLayout layout = bestTextLayout(visibleLabel);
        const int width = layout.width;
        const int height = layout.height;

        QFont font = QApplication::font();
        font.setWeight(QFont::DemiBold);
        font.setPixelSize(layout.fontPixelSize);
        QFontMetrics metrics(font);

        QPixmap pixmap(qCeil(width * devicePixelRatio), qCeil(height * devicePixelRatio));
        pixmap.setDevicePixelRatio(devicePixelRatio);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);
        painter.setFont(font);

        const bool useSystemTint = prefersSystemTintedText();
        const QColor textColor = useSystemTint ? QColor(Qt::white) : preferredTextColor();
        const int lineHeight = metrics.height();
        const int totalTextHeight = layout.lines.size() * lineHeight;
        const qreal startY = (height - totalTextHeight) / 2.0;

        for (int index = 0; index < layout.lines.size(); ++index) {
            const QRectF lineRect(0.0, startY + index * lineHeight, width, lineHeight);
            if (!useSystemTint) {
                painter.setPen(preferredTextHaloColor(textColor));
                painter.drawText(lineRect.translated(0.0, 0.5), Qt::AlignCenter, layout.lines.at(index));
            }
            painter.setPen(textColor);
            painter.drawText(lineRect, Qt::AlignCenter, layout.lines.at(index));
        }
        return pixmap;
    }

    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(accentColor());
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(QRectF(4, 4, 56, 56), 12, 12);

    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(visibleLabel.size() > 2 ? 18 : 22);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, visibleLabel);

    return pixmap;
}

} // namespace Vitals
