#include "DashboardWidget.h"

#include "CardWidget.h"
#include "language/LanguageManager.h"
#include "metric/MetricCenter.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSizePolicy>
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

DashboardWidget::DashboardWidget(LanguageManager* languageManager, QWidget* parent)
    : QWidget(parent)
    , m_languageManager(languageManager)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(22, 18, 22, 20);
    root->setSpacing(12);
    setMinimumSize(0, 0);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    auto* header = new QHBoxLayout();
    header->setSpacing(12);

    m_titleLabel = new QLabel(text(QStringLiteral("dashboard.title"), QStringLiteral("Overview")), this);
    m_titleLabel->setObjectName(QStringLiteral("pageTitle"));
    m_titleLabel->setMinimumWidth(0);
    m_titleLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    m_statusLabel = new QLabel(text(QStringLiteral("dashboard.waitingMetrics"), QStringLiteral("Waiting for metrics")), this);
    m_statusLabel->setObjectName(QStringLiteral("statusPill"));

    header->addWidget(m_titleLabel);
    header->addStretch();
    header->addWidget(m_statusLabel);
    root->addLayout(header);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setMinimumSize(0, 0);
    scrollArea->viewport()->setMinimumSize(0, 0);
    scrollArea->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    auto* content = new QWidget(scrollArea);
    content->setMinimumSize(0, 0);
    content->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_groupLayout = new QVBoxLayout(content);
    m_groupLayout->setContentsMargins(0, 0, 0, 0);
    m_groupLayout->setSpacing(10);
    m_groupLayout->setAlignment(Qt::AlignTop);

    m_emptyLabel = new QLabel(text(QStringLiteral("dashboard.empty"), QStringLiteral("Waiting for plugin metrics")), content);
    m_emptyLabel->setObjectName(QStringLiteral("panelBody"));
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_groupLayout->addWidget(m_emptyLabel);

    scrollArea->setWidget(content);
    root->addWidget(scrollArea, 1);
}

void DashboardWidget::bindMetricCenter(MetricCenter* metricCenter)
{
    connect(metricCenter, &MetricCenter::framePublished,
        this, &DashboardWidget::updateFrame);
    connect(metricCenter, &MetricCenter::metricRemoved,
        this, &DashboardWidget::removeMetric);
}

void DashboardWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    for (auto it = m_groups.begin(); it != m_groups.end(); ++it) {
        relayoutMetricCards(it.value());
    }
}

void DashboardWidget::updateFrame(const MetricFrame& frame)
{
    if (frame.pluginId.isEmpty()) {
        return;
    }

    QHash<QString, MetricValue>& pluginValues = m_pluginMetrics[frame.pluginId];
    for (const MetricValue& value : frame.values) {
        pluginValues.insert(value.key, value);
        m_metricOwners.insert(value.key, frame.pluginId);
    }

    updatePluginSummary(frame.pluginId);
    m_statusLabel->setText(text(QStringLiteral("dashboard.live"), QStringLiteral("Live")));
    if (m_emptyLabel) {
        m_emptyLabel->hide();
    }
}

void DashboardWidget::removeMetric(const QString& key)
{
    const QString pluginId = m_metricOwners.take(key);
    if (pluginId.isEmpty()) {
        return;
    }

    QHash<QString, MetricValue>& pluginValues = m_pluginMetrics[pluginId];
    pluginValues.remove(key);
    if (pluginValues.isEmpty()) {
        m_pluginMetrics.remove(pluginId);
        removePluginGroup(pluginId);
    } else {
        PluginGroup& group = ensurePluginGroup(pluginId);
        CardWidget* metricCard = group.metricCards.take(key);
        if (metricCard) {
            group.detailsGrid->removeWidget(metricCard);
            metricCard->deleteLater();
            relayoutMetricCards(group);
        }
        updatePluginSummary(pluginId);
    }

    if (m_groups.isEmpty()) {
        m_statusLabel->setText(text(QStringLiteral("dashboard.waitingMetrics"), QStringLiteral("Waiting for metrics")));
        if (m_emptyLabel) {
            m_emptyLabel->show();
        }
    }
}

DashboardWidget::PluginGroup& DashboardWidget::ensurePluginGroup(const QString& pluginId)
{
    PluginGroup& group = m_groups[pluginId];
    if (group.container) {
        return group;
    }

    group.container = new QFrame(this);
    group.container->setObjectName(QStringLiteral("dashboardPluginGroup"));
    group.container->setMinimumWidth(300);
    group.container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    group.container->setStyleSheet(QStringLiteral(R"(
        QFrame#dashboardPluginGroup {
            background: rgba(255, 255, 255, 0.70);
            border: 1px solid #dedee3;
            border-radius: 8px;
        }
        QPushButton#dashboardGroupToggle {
            background: transparent;
            border: none;
            color: #1d252d;
            font-size: 14px;
            font-weight: 700;
            text-align: left;
            padding: 0;
        }
    )"));

    auto* groupLayout = new QVBoxLayout(group.container);
    groupLayout->setContentsMargins(12, 10, 12, 12);
    groupLayout->setSpacing(9);

    auto* header = new QHBoxLayout();
    header->setSpacing(10);

    group.toggleButton = new QPushButton(group.container);
    group.toggleButton->setObjectName(QStringLiteral("dashboardGroupToggle"));
    group.toggleButton->setCheckable(true);
    group.toggleButton->setChecked(false);
    group.toggleButton->setCursor(Qt::PointingHandCursor);
    group.toggleButton->setMinimumWidth(0);
    group.toggleButton->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    group.subtitleLabel = new QLabel(group.container);
    group.subtitleLabel->setObjectName(QStringLiteral("panelBody"));
    group.subtitleLabel->setMinimumWidth(0);
    group.subtitleLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    header->addWidget(group.toggleButton, 1);
    header->addWidget(group.subtitleLabel);
    groupLayout->addLayout(header);

    group.summaryCard = new CardWidget(pluginTitle(pluginId), group.container);
    group.summaryCard->setMinimumWidth(260);
    groupLayout->addWidget(group.summaryCard);

    group.detailsContainer = new QWidget(group.container);
    group.detailsContainer->setMinimumWidth(260);
    group.detailsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    group.detailsGrid = new QGridLayout(group.detailsContainer);
    group.detailsGrid->setContentsMargins(0, 0, 0, 0);
    group.detailsGrid->setHorizontalSpacing(10);
    group.detailsGrid->setVerticalSpacing(10);
    group.detailsContainer->hide();
    groupLayout->addWidget(group.detailsContainer);

    connect(group.toggleButton, &QPushButton::toggled, this, [this, pluginId](bool checked) {
        PluginGroup& currentGroup = m_groups[pluginId];
        currentGroup.expanded = checked;
        if (currentGroup.detailsContainer) {
            currentGroup.detailsContainer->setVisible(checked);
        }
        updateGroupHeader(pluginId);
    });

    m_groupLayout->addWidget(group.container);
    updateGroupHeader(pluginId);
    return group;
}

CardWidget* DashboardWidget::ensureMetricCard(const QString& pluginId, const QString& metricKey)
{
    PluginGroup& group = ensurePluginGroup(pluginId);
    if (group.metricCards.contains(metricKey)) {
        return group.metricCards.value(metricKey);
    }

    auto* card = new CardWidget(displayTitleForMetric(metricKey), group.detailsContainer);
    card->setAccentColor(accentColorForMetric(metricKey));
    group.metricCards.insert(metricKey, card);
    relayoutMetricCards(group);
    return card;
}

void DashboardWidget::removePluginGroup(const QString& pluginId)
{
    PluginGroup group = m_groups.take(pluginId);
    if (!group.container) {
        return;
    }

    m_groupLayout->removeWidget(group.container);
    group.container->deleteLater();
}

void DashboardWidget::relayoutMetricCards(PluginGroup& group)
{
    int index = 0;
    QStringList keys = group.metricCards.keys();
    keys.sort();
    const int columns = detailColumnCount(group);
    for (int column = 0; column < 3; ++column) {
        group.detailsGrid->setColumnStretch(column, column < columns ? 1 : 0);
        group.detailsGrid->setColumnMinimumWidth(column, column < columns ? 260 : 0);
    }
    for (const QString& key : keys) {
        CardWidget* card = group.metricCards.value(key);
        group.detailsGrid->addWidget(card, index / columns, index % columns);
        ++index;
    }
}

int DashboardWidget::detailColumnCount(const PluginGroup& group) const
{
    const int availableWidth = group.detailsContainer ? group.detailsContainer->width() : this->width();
    if (availableWidth < 460) {
        return 1;
    }
    if (availableWidth < 760) {
        return 2;
    }
    return 3;
}

void DashboardWidget::updatePluginSummary(const QString& pluginId)
{
    const QHash<QString, MetricValue> pluginValues = m_pluginMetrics.value(pluginId);
    if (pluginValues.isEmpty()) {
        return;
    }

    PluginGroup& group = ensurePluginGroup(pluginId);
    CardWidget* summaryCard = group.summaryCard;
    summaryCard->setAccentColor(accentColorForMetric(primaryMetricKeyForPlugin(pluginId)));

    for (const MetricValue& value : pluginValues) {
        updateMetricCard(ensureMetricCard(pluginId, value.key), value);
    }

    const QString primaryKey = primaryMetricKeyForPlugin(pluginId);
    if (pluginValues.contains(primaryKey)) {
        const MetricValue value = pluginValues.value(primaryKey);
        summaryCard->setValueText(displayValueForMetric(value));
        summaryCard->setHintText(summaryHintForPlugin(pluginId));

        const int progress = progressForMetric(value);
        if (progress >= 0) {
            summaryCard->setProgressValue(progress);
        } else {
            summaryCard->clearProgress();
        }
        updateGroupHeader(pluginId);
        return;
    }

    const MetricValue fallback = pluginValues.cbegin().value();
    summaryCard->setValueText(displayValueForMetric(fallback));
    summaryCard->setHintText(summaryHintForPlugin(pluginId));
    summaryCard->clearProgress();
    updateGroupHeader(pluginId);
}

void DashboardWidget::updateGroupHeader(const QString& pluginId)
{
    PluginGroup& group = ensurePluginGroup(pluginId);
    const QString marker = group.expanded ? QStringLiteral("v") : QStringLiteral(">");
    group.toggleButton->setText(QStringLiteral("%1 %2").arg(marker, pluginTitle(pluginId)));

    const int metricCount = m_pluginMetrics.value(pluginId).size();
    group.subtitleLabel->setText(text(QStringLiteral("dashboard.metricCount"), QStringLiteral("%1 metrics")).arg(metricCount));
}

void DashboardWidget::updateMetricCard(CardWidget* card, const MetricValue& value)
{
    card->setValueText(displayValueForMetric(value));
    card->setHintText(displayHintForMetric(value));
    card->setAccentColor(accentColorForMetric(value.key));

    const int progress = progressForMetric(value);
    if (progress >= 0) {
        card->setProgressValue(progress);
    } else {
        card->clearProgress();
    }
}

QString DashboardWidget::pluginTitle(const QString& pluginId) const
{
    if (pluginId == QStringLiteral("com.vitals.cpu")) return text(QStringLiteral("plugin.com.vitals.cpu"), QStringLiteral("CPU Monitor"));
    if (pluginId == QStringLiteral("com.vitals.memory")) return text(QStringLiteral("plugin.com.vitals.memory"), QStringLiteral("Memory Monitor"));
    if (pluginId == QStringLiteral("com.vitals.network")) return text(QStringLiteral("plugin.com.vitals.network"), QStringLiteral("Network Monitor"));
    if (pluginId == QStringLiteral("com.vitals.disk")) return text(QStringLiteral("plugin.com.vitals.disk"), QStringLiteral("Disk Monitor"));
    if (pluginId == QStringLiteral("com.vitals.systeminfo")) return text(QStringLiteral("plugin.com.vitals.systeminfo"), QStringLiteral("System Info"));
    return pluginId;
}

QString DashboardWidget::primaryMetricKeyForPlugin(const QString& pluginId) const
{
    if (pluginId == QStringLiteral("com.vitals.cpu")) return QStringLiteral("cpu.usage.total");
    if (pluginId == QStringLiteral("com.vitals.memory")) return QStringLiteral("memory.usage.percent");
    if (pluginId == QStringLiteral("com.vitals.network")) return QStringLiteral("network.download.rate");
    if (pluginId == QStringLiteral("com.vitals.disk")) return QStringLiteral("disk.usage.percent");
    if (pluginId == QStringLiteral("com.vitals.systeminfo")) return QStringLiteral("system.memory.total.bytes");
    return {};
}

QString DashboardWidget::summaryHintForPlugin(const QString& pluginId) const
{
    const QHash<QString, MetricValue> pluginValues = m_pluginMetrics.value(pluginId);
    if (pluginId == QStringLiteral("com.vitals.cpu")) {
        const int cores = pluginValues.value(QStringLiteral("cpu.logical.cores")).value.toInt();
        double peak = 0.0;
        for (int index = 0; index < cores; ++index) {
            peak = qMax(peak, pluginValues.value(QStringLiteral("cpu.usage.core%1").arg(index)).value.toDouble());
        }
        if (cores > 0) {
            return text(QStringLiteral("dashboard.coresPeak"), QStringLiteral("%1 cores | Peak %2%"))
                .arg(cores)
                .arg(peak, 0, 'f', 0);
        }
    }

    if (pluginId == QStringLiteral("com.vitals.systeminfo")) {
        const QString device = pluginValues.value(QStringLiteral("system.device.name")).value.toString();
        const qint64 uptime = pluginValues.value(QStringLiteral("system.uptime.seconds")).value.toLongLong();
        if (!device.isEmpty() && uptime > 0) {
            return text(QStringLiteral("dashboard.uptime"), QStringLiteral("%1 | Uptime %2h %3m"))
                .arg(device)
                .arg(uptime / 3600)
                .arg((uptime % 3600) / 60);
        }
        if (!device.isEmpty()) {
            return device;
        }
    }

    if (pluginId == QStringLiteral("com.vitals.network")) {
        const QString primary = pluginValues.value(QStringLiteral("network.interface.primary")).value.toString();
        const double up = pluginValues.value(QStringLiteral("network.upload.rate")).value.toDouble();
        if (!primary.isEmpty()) {
            return text(QStringLiteral("dashboard.networkUpWithInterface"), QStringLiteral("%1 | Up %2/s")).arg(primary, formatBytes(up));
        }
        if (up > 0.0) {
            return text(QStringLiteral("dashboard.networkUp"), QStringLiteral("Up %1/s")).arg(formatBytes(up));
        }
    }

    if (pluginId == QStringLiteral("com.vitals.memory")) {
        const quint64 used = pluginValues.value(QStringLiteral("memory.used.bytes")).value.toULongLong();
        const quint64 total = pluginValues.value(QStringLiteral("memory.total.bytes")).value.toULongLong();
        if (total > 0) {
            return text(QStringLiteral("dashboard.memoryUsed"), QStringLiteral("%1 of %2 used")).arg(formatBytes(used), formatBytes(total));
        }
    }

    if (pluginId == QStringLiteral("com.vitals.disk")) {
        const QString name = pluginValues.value(QStringLiteral("disk.selected.name")).value.toString();
        const qint64 available = pluginValues.value(QStringLiteral("disk.bytes.available")).value.toLongLong();
        const qint64 total = pluginValues.value(QStringLiteral("disk.bytes.total")).value.toLongLong();
        if (!name.isEmpty() && total > 0) {
            return text(QStringLiteral("dashboard.diskAvailable"), QStringLiteral("%1 | %2 available of %3"))
                .arg(name, formatBytes(available), formatBytes(total));
        }
    }

    const auto values = pluginValues.values();
    if (!values.isEmpty()) {
        return displayHintForMetric(values.first());
    }
    return text(QStringLiteral("dashboard.pluginSummary"), QStringLiteral("Plugin summary"));
}

QString DashboardWidget::displayTitleForMetric(const QString& key) const
{
    if (key.startsWith(QStringLiteral("com.vitals."))) return pluginTitle(key);
    if (key == QStringLiteral("framework.status")) return text(QStringLiteral("metric.framework.status"), QStringLiteral("Framework"));
    if (key == QStringLiteral("cpu.usage.total")) return text(QStringLiteral("metric.cpu.usage.total"), QStringLiteral("CPU"));
    if (key == QStringLiteral("cpu.logical.cores")) return text(QStringLiteral("metric.cpu.logical.cores"), QStringLiteral("Logical Cores"));
    if (key == QStringLiteral("cpu.model")) return text(QStringLiteral("metric.cpu.model"), QStringLiteral("Processor"));
    if (key.startsWith(QStringLiteral("cpu.usage.core"))) {
        bool ok = false;
        const int coreIndex = key.mid(QStringLiteral("cpu.usage.core").size()).toInt(&ok);
        if (ok) {
            return text(QStringLiteral("metric.cpu.usage.core"), QStringLiteral("Core %1")).arg(coreIndex + 1);
        }
    }
    if (key == QStringLiteral("memory.usage.percent")) return text(QStringLiteral("metric.memory.usage.percent"), QStringLiteral("Memory"));
    if (key == QStringLiteral("memory.total.bytes")) return text(QStringLiteral("metric.memory.total.bytes"), QStringLiteral("Total Memory"));
    if (key == QStringLiteral("memory.used.bytes")) return text(QStringLiteral("metric.memory.used.bytes"), QStringLiteral("Used Memory"));
    if (key == QStringLiteral("memory.free.bytes")) return text(QStringLiteral("metric.memory.free.bytes"), QStringLiteral("Available Memory"));
    if (key == QStringLiteral("network.interface.primary")) return text(QStringLiteral("metric.network.interface.primary"), QStringLiteral("Primary Interface"));
    if (key == QStringLiteral("network.interfaces.active")) return text(QStringLiteral("metric.network.interfaces.active"), QStringLiteral("Active Interfaces"));
    if (key == QStringLiteral("network.upload.speed")) return text(QStringLiteral("metric.network.upload"), QStringLiteral("Upload"));
    if (key == QStringLiteral("network.download.speed")) return text(QStringLiteral("metric.network.download"), QStringLiteral("Download"));
    if (key == QStringLiteral("network.upload.rate")) return text(QStringLiteral("metric.network.upload"), QStringLiteral("Upload"));
    if (key == QStringLiteral("network.download.rate")) return text(QStringLiteral("metric.network.download"), QStringLiteral("Download"));
    if (key == QStringLiteral("network.download.total.bytes")) return text(QStringLiteral("metric.network.download.total.bytes"), QStringLiteral("Total Received"));
    if (key == QStringLiteral("network.upload.total.bytes")) return text(QStringLiteral("metric.network.upload.total.bytes"), QStringLiteral("Total Sent"));
    if (key == QStringLiteral("disk.selected.name")) return text(QStringLiteral("metric.disk.selected.name"), QStringLiteral("Selected Disk"));
    if (key == QStringLiteral("disk.selected.root")) return text(QStringLiteral("metric.disk.selected.root"), QStringLiteral("Mount Path"));
    if (key == QStringLiteral("disk.selected.device")) return text(QStringLiteral("metric.disk.selected.device"), QStringLiteral("Device"));
    if (key == QStringLiteral("disk.selected.filesystem")) return text(QStringLiteral("metric.disk.selected.filesystem"), QStringLiteral("File System"));
    if (key == QStringLiteral("disk.selected.kind")) return text(QStringLiteral("metric.disk.selected.kind"), QStringLiteral("Disk Kind"));
    if (key == QStringLiteral("disk.usage.percent")) return text(QStringLiteral("metric.disk.usage.percent"), QStringLiteral("Disk Usage"));
    if (key == QStringLiteral("disk.bytes.total")) return text(QStringLiteral("metric.disk.bytes.total"), QStringLiteral("Total Capacity"));
    if (key == QStringLiteral("disk.bytes.used")) return text(QStringLiteral("metric.disk.bytes.used"), QStringLiteral("Used Capacity"));
    if (key == QStringLiteral("disk.bytes.available")) return text(QStringLiteral("metric.disk.bytes.available"), QStringLiteral("Available Capacity"));
    if (key == QStringLiteral("disk.count")) return text(QStringLiteral("metric.disk.count"), QStringLiteral("Mounted Disks"));
    if (key == QStringLiteral("disk.read.speed")) return text(QStringLiteral("metric.disk.read.speed"), QStringLiteral("Disk Read"));
    if (key == QStringLiteral("disk.write.speed")) return text(QStringLiteral("metric.disk.write.speed"), QStringLiteral("Disk Write"));
    if (key == QStringLiteral("battery.level.percent")) return text(QStringLiteral("metric.battery.level.percent"), QStringLiteral("Battery"));
    if (key == QStringLiteral("system.memory.total.bytes")) return text(QStringLiteral("metric.system.memory.total.bytes"), QStringLiteral("Memory"));
    if (key == QStringLiteral("system.gpu.model")) return text(QStringLiteral("metric.system.gpu.model"), QStringLiteral("GPU"));
    if (key == QStringLiteral("system.uptime.seconds")) return text(QStringLiteral("metric.system.uptime.seconds"), QStringLiteral("Uptime"));
    return key;
}

QString DashboardWidget::displayValueForMetric(const MetricValue& value) const
{
    const QString key = value.key;
    if (key.endsWith(QStringLiteral(".percent"))
        || key == QStringLiteral("cpu.usage.total")
        || key.startsWith(QStringLiteral("cpu.usage.core"))) {
        return QStringLiteral("%1%").arg(value.value.toDouble(), 0, 'f', 0);
    }
    if (key.endsWith(QStringLiteral(".speed"))) {
        const double bytesPerSecond = value.value.toDouble();
        return QStringLiteral("%1/s").arg(formatBytes(bytesPerSecond));
    }
    if (key.endsWith(QStringLiteral(".rate"))) {
        return QStringLiteral("%1/s").arg(formatBytes(value.value.toDouble()));
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
    if (value.timestamp.isValid()) {
        return text(QStringLiteral("dashboard.updatedAt"), QStringLiteral("Updated %1")).arg(value.timestamp.toString(QStringLiteral("hh:mm:ss")));
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

QString DashboardWidget::text(const QString& key, const QString& fallback) const
{
    return m_languageManager ? m_languageManager->translate(key, fallback) : fallback;
}

} // namespace Vitals
