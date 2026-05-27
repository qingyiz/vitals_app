#pragma once

#include "ISystemInfoCollector.h"

#include <QWidget>

namespace Vitals {

class IAppContext;
class InfoPanelWidget;

/**
 * \if ENGLISH
 * @brief Read-only panel widget that presents normalized system information
 * \endif
 *
 * \if CHINESE
 * @brief 用于展示统一系统信息快照的只读详情面板
 * \endif
 */
class SystemInfoPanelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SystemInfoPanelWidget(IAppContext* context, QWidget* parent = nullptr);

    /**
     * \if ENGLISH
     * @brief Refreshes the widget contents from a newly collected snapshot
     * \endif
     *
     * \if CHINESE
     * @brief 使用新的采集快照刷新页面内容
     * \endif
     */
    void applySnapshot(const SystemInfoSnapshot& snapshot);

private:
    static QString formatBytes(quint64 bytes);
    QString formatUptime(qint64 seconds) const;
    QString text(const QString& key, const QString& fallback) const;

    IAppContext* m_context = nullptr;
    InfoPanelWidget* m_infoPanel = nullptr;
};

} // namespace Vitals
