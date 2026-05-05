#pragma once

#include "MetricData.h"

#include <QWidget>
#include <QHash>

class QGridLayout;
class QLabel;

namespace Vitals {

class CardWidget;
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
    explicit DashboardWidget(QWidget* parent = nullptr);

    /// Subscribes the dashboard to MetricCenter updates.
    void bindMetricCenter(MetricCenter* metricCenter);

private Q_SLOTS:
    /// Re-renders one card when a metric value changes.
    void updateMetric(const Vitals::MetricValue& value);
    /// Removes one card when the owning plugin is unloaded.
    void removeMetric(const QString& key);

private:
    CardWidget* ensureCard(const QString& key);
    void relayoutCards();
    QString displayTitleForMetric(const QString& key) const;
    QString displayValueForMetric(const MetricValue& value) const;
    QString displayHintForMetric(const MetricValue& value) const;
    QColor accentColorForMetric(const QString& key) const;
    int progressForMetric(const MetricValue& value) const;

    QLabel* m_statusLabel = nullptr;
    QGridLayout* m_grid = nullptr;
    QHash<QString, CardWidget*> m_cards;
};

} // namespace Vitals
