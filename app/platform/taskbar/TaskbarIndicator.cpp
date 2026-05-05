#include "platform/taskbar/TaskbarIndicator.h"

#include "metric/MetricCenter.h"

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QDebug>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QSystemTrayIcon>

namespace Vitals {

TaskbarIndicator::TaskbarIndicator(QObject* parent)
    : QObject(parent)
{
}

TaskbarIndicator::~TaskbarIndicator() = default;

void TaskbarIndicator::initialize(QWidget* mainWindow)
{
    m_mainWindow = mainWindow;

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

    m_trayIcon->show();
}

void TaskbarIndicator::bindMetricCenter(MetricCenter* metricCenter)
{
    connect(metricCenter, &MetricCenter::metricUpdated,
        this, &TaskbarIndicator::handleMetricUpdated);
    connect(metricCenter, &MetricCenter::metricRemoved,
        this, &TaskbarIndicator::handleMetricRemoved);
}

QString TaskbarIndicator::idleText() const
{
    return QStringLiteral("OK");
}

QString TaskbarIndicator::iconLabel(const QHash<QString, MetricValue>& latestValues) const
{
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
    for (const QString& key : orderedMetricKeys()) {
        const MetricValue value = latestValues.value(key);
        lines.append(QStringLiteral("%1: %2").arg(key, formatMetricValue(value)));
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

QString TaskbarIndicator::formatMetricValue(const MetricValue& value) const
{
    if (value.value.type() == QVariant::Double) {
        return QString::number(value.value.toDouble(), 'f', 1);
    }
    return value.value.toString();
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

    const QString label = iconLabel(m_latestValues);
    const QString tooltip = tooltipText(m_latestValues);
    m_trayIcon->setIcon(buildIcon(label));
    m_trayIcon->setToolTip(tooltip);

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
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(accentColor());
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(QRectF(4, 4, 56, 56), 12, 12);

    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(label.size() > 2 ? 18 : 22);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, label.left(3));

    return QIcon(pixmap);
}

} // namespace Vitals
