#pragma once

#include "IDiskCollector.h"
#include "IPanelCapability.h"

#include <QPointer>

namespace Vitals {

class DiskPanelWidget;
class DiskMonitorCapability;
class IAppContext;

/**
 * \if ENGLISH
 * @brief Panel capability exposing the disk monitor page
 * \endif
 *
 * \if CHINESE
 * @brief 暴露磁盘监控页面的面板能力
 * \endif
 */
class DiskPanelCapability : public IPanelCapability
{
public:
    DiskPanelCapability(DiskMonitorCapability* monitorCapability, IAppContext* context);

    QString panelId() const override;
    QString panelName() const override;
    QString panelIconKey() const override;
    QWidget* createPanel(QWidget* parent = nullptr) override;

    void updateSnapshot(const DiskSnapshot& snapshot);

private:
    IAppContext* m_context = nullptr;
    DiskMonitorCapability* m_monitorCapability = nullptr;
    QPointer<DiskPanelWidget> m_panel;
    DiskSnapshot m_lastSnapshot;
};

} // namespace Vitals
