#include "DashboardWidget.h"

#include "CardWidget.h"
#include "metric/MetricCenter.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QVariant>

namespace Vitals {

namespace {

QString formatBytes(double bytes)
{
    static const char* suffixes[] = {"B", "KB", "MB", "GB", "TB"};

    double value = bytes;
    int suffixIndex = 0;
    while (value >= 1024.0 && suffixIndex < 4) {
        value /= 1024.0;
        ++suffixIndex;
    }

    return QStringLiteral("%1 %2").arg(value, 0, 'f', suffixIndex == 0 ? 0 : 1)
        .arg(QString::fromLatin1(suffixes[suffixIndex]));
}

} // namespace

DashboardWidget::DashboardWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 24, 28, 28);
    root->setSpacing(18);

    auto* header = new QHBoxLayout();
    header->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("Overview"), this);
    title->setObjectName(QStringLiteral("pageTitle"));

    m_statusLabel = new QLabel(QStringLiteral("Waiting for metrics"), this);
    m_statusLabel->setObjectName(QStringLiteral("statusPill"));

    header->addWidget(title);
    header->addStretch();
    header->addWidget(m_statusLabel);
    root->addLayout(header);

    m_grid = new QGridLayout();
    m_grid->setHorizontalSpacing(14);
    m_grid->setVerticalSpacing(14);
    root->addLayout(m_grid);
    root->addStretch();

    auto* welcomeCard = ensureCard(QStringLiteral("framework.status"));
    welcomeCard->setValueText(QStringLiteral("Ready"));
    welcomeCard->setHintText(QStringLiteral("Host, plugins, taskbar indicator"));
    welcomeCard->setAccentColor(QColor(QStringLiteral("#5e5ce6")));
}

void DashboardWidget::bindMetricCenter(MetricCenter* metricCenter)
{
    connect(metricCenter, &MetricCenter::metricUpdated,
        this, &DashboardWidget::updateMetric);
    connect(metricCenter, &MetricCenter::metricRemoved,
        this, &DashboardWidget::removeMetric);
}

void DashboardWidget::updateMetric(const MetricValue& value)
{
    CardWidget* card = ensureCard(value.key);
    card->setValueText(displayValueForMetric(value));
    card->setHintText(displayHintForMetric(value));
    card->setAccentColor(accentColorForMetric(value.key));

    const int progress = progressForMetric(value);
    if (progress >= 0) {
        card->setProgressValue(progress);
    } else {
        card->clearProgress();
    }

    m_statusLabel->setText(QStringLiteral("Live"));
}

CardWidget* DashboardWidget::ensureCard(const QString& key)
{
    if (m_cards.contains(key)) {
        return m_cards.value(key);
    }

    auto* card = new CardWidget(displayTitleForMetric(key), this);
    card->setAccentColor(accentColorForMetric(key));
    const int index = m_cards.size();
    m_grid->addWidget(card, index / 3, index % 3);
    m_cards.insert(key, card);
    return card;
}

void DashboardWidget::removeMetric(const QString& key)
{
    CardWidget* card = m_cards.take(key);
    if (!card) {
        return;
    }

    m_grid->removeWidget(card);
    card->deleteLater();
    relayoutCards();

    if (m_cards.size() <= 1 && m_cards.contains(QStringLiteral("framework.status"))) {
        m_statusLabel->setText(QStringLiteral("Waiting for metrics"));
    }
}

void DashboardWidget::relayoutCards()
{
    const QList<CardWidget*> cards = m_cards.values();
    int index = 0;
    for (CardWidget* card : cards) {
        m_grid->addWidget(card, index / 3, index % 3);
        ++index;
    }
}

QString DashboardWidget::displayTitleForMetric(const QString& key) const
{
    if (key == QStringLiteral("framework.status")) return QStringLiteral("Framework");
    if (key == QStringLiteral("hello.plugin.status")) return QStringLiteral("Plugin Link");
    if (key == QStringLiteral("cpu.usage.total")) return QStringLiteral("CPU");
    if (key == QStringLiteral("memory.usage.percent")) return QStringLiteral("Memory");
    if (key == QStringLiteral("network.upload.speed")) return QStringLiteral("Upload");
    if (key == QStringLiteral("network.download.speed")) return QStringLiteral("Download");
    if (key == QStringLiteral("disk.read.speed")) return QStringLiteral("Disk Read");
    if (key == QStringLiteral("disk.write.speed")) return QStringLiteral("Disk Write");
    if (key == QStringLiteral("battery.level.percent")) return QStringLiteral("Battery");
    if (key == QStringLiteral("system.memory.total.bytes")) return QStringLiteral("Memory");
    if (key == QStringLiteral("system.gpu.model")) return QStringLiteral("GPU");
    if (key == QStringLiteral("system.uptime.seconds")) return QStringLiteral("Uptime");
    return key;
}

QString DashboardWidget::displayValueForMetric(const MetricValue& value) const
{
    const QString key = value.key;
    if (key.endsWith(QStringLiteral(".percent"))
        || key == QStringLiteral("cpu.usage.total")) {
        return QStringLiteral("%1%").arg(value.value.toDouble(), 0, 'f', 0);
    }
    if (key.endsWith(QStringLiteral(".speed"))) {
        const double bytesPerSecond = value.value.toDouble();
        return QStringLiteral("%1/s").arg(formatBytes(bytesPerSecond));
    }
    if (key.endsWith(QStringLiteral(".bytes"))) {
        return formatBytes(value.value.toDouble());
    }
    if (key == QStringLiteral("system.uptime.seconds")) {
        const qint64 seconds = value.value.toLongLong();
        return QStringLiteral("%1h %2m").arg(seconds / 3600).arg((seconds % 3600) / 60);
    }
    return value.value.toString();
}

QString DashboardWidget::displayHintForMetric(const MetricValue& value) const
{
    if (value.key == QStringLiteral("hello.plugin.status")) {
        return QStringLiteral("Dynamic plugin contract verified");
    }
    if (value.timestamp.isValid()) {
        return QStringLiteral("Updated %1").arg(value.timestamp.toString(QStringLiteral("hh:mm:ss")));
    }
    return QStringLiteral("MetricCenter");
}

QColor DashboardWidget::accentColorForMetric(const QString& key) const
{
    if (key.startsWith(QStringLiteral("cpu."))) return QColor(QStringLiteral("#ff453a"));
    if (key.startsWith(QStringLiteral("memory."))) return QColor(QStringLiteral("#32d74b"));
    if (key.startsWith(QStringLiteral("network."))) return QColor(QStringLiteral("#0a84ff"));
    if (key.startsWith(QStringLiteral("disk."))) return QColor(QStringLiteral("#bf5af2"));
    if (key.startsWith(QStringLiteral("battery."))) return QColor(QStringLiteral("#ffd60a"));
    if (key.startsWith(QStringLiteral("system."))) return QColor(QStringLiteral("#64d2ff"));
    if (key.startsWith(QStringLiteral("hello."))) return QColor(QStringLiteral("#ff9f0a"));
    return QColor(QStringLiteral("#5e5ce6"));
}

int DashboardWidget::progressForMetric(const MetricValue& value) const
{
    if (value.key.endsWith(QStringLiteral(".percent"))
        || value.key == QStringLiteral("cpu.usage.total")) {
        return qRound(value.value.toDouble());
    }
    return -1;
}

} // namespace Vitals
