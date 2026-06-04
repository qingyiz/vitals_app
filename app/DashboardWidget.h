#pragma once

#include "MetricData.h"

#include <QHash>
#include <QSet>
#include <QWidget>

class QGridLayout;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QFrame;

namespace Vitals {

class CardWidget;
class LanguageManager;
class MetricCenter;

/**
 * \if ENGLISH
 * @brief Host-owned overview dashboard built on top of MetricCenter
 *
 * Converts normalized metric updates into compact visual cards suitable for
 * quick scanning. The dashboard is intentionally data-driven and does not know
 * which platform API or plugin implementation produced a metric.
 * \endif
 *
 * \if CHINESE
 * @brief 构建在 MetricCenter 之上的宿主总览面板
 *
 * 该组件将统一的指标更新转换为便于快速扫读的紧凑卡片展示。它是数据驱动
 * 的，不关心某个指标来自哪个平台 API 或哪一个具体插件实现。
 * \endif
 */
class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(LanguageManager* languageManager, QWidget* parent = nullptr);

    /// Subscribes the dashboard to MetricCenter updates.
    void bindMetricCenter(MetricCenter* metricCenter);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private Q_SLOTS:
    /// Re-renders one plugin summary when a metric frame changes.
    void updateFrame(const Vitals::MetricFrame& frame);
    /// Removes one card when the owning plugin is unloaded.
    void removeMetric(const QString& key);

private:
    struct PluginGroup
    {
        QFrame* container = nullptr;
        QPushButton* toggleButton = nullptr;
        QLabel* subtitleLabel = nullptr;
        CardWidget* summaryCard = nullptr;
        QWidget* detailsContainer = nullptr;
        QGridLayout* detailsGrid = nullptr;
        QHash<QString, CardWidget*> metricCards;
        bool expanded = false;
    };

    PluginGroup& ensurePluginGroup(const QString& pluginId);
    CardWidget* ensureMetricCard(const QString& pluginId, const QString& metricKey);
    void removePluginGroup(const QString& pluginId);
    void relayoutMetricCards(PluginGroup& group);
    int detailColumnCount(const PluginGroup& group) const;
    void updatePluginSummary(const QString& pluginId);
    void updatePendingPluginSummaries();
    void updateGroupHeader(const QString& pluginId);
    void updateMetricCard(CardWidget* card, const MetricValue& value);
    QString pluginTitle(const QString& pluginId) const;
    QString primaryMetricKeyForPlugin(const QString& pluginId) const;
    QString summaryHintForPlugin(const QString& pluginId) const;
    QString displayTitleForMetric(const QString& key) const;
    QString displayValueForMetric(const MetricValue& value) const;
    QString displayHintForMetric(const MetricValue& value) const;
    QColor accentColorForMetric(const QString& key) const;
    int progressForMetric(const MetricValue& value) const;
    QString text(const QString& key, const QString& fallback) const;

    QLabel* m_titleLabel = nullptr;
    QLabel* m_statusLabel = nullptr;
    QVBoxLayout* m_groupLayout = nullptr;
    QLabel* m_emptyLabel = nullptr;
    QHash<QString, PluginGroup> m_groups;
    QHash<QString, QString> m_metricOwners;
    QHash<QString, QHash<QString, MetricValue>> m_pluginMetrics;
    QSet<QString> m_pendingPluginSummaries;
    LanguageManager* m_languageManager = nullptr;
};

} // namespace Vitals
