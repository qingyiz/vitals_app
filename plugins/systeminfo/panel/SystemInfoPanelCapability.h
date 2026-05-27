#pragma once

#include "IPanelCapability.h"
#include "ISystemInfoCollector.h"

#include <QPointer>

namespace Vitals {

class SystemInfoPanelWidget;
class IAppContext;

class SystemInfoPanelCapability : public IPanelCapability
{
public:
    explicit SystemInfoPanelCapability(IAppContext* context);

    QString panelId() const override;
    QString panelName() const override;
    QString panelIconKey() const override;
    QWidget* createPanel(QWidget* parent = nullptr) override;

    void updateSnapshot(const SystemInfoSnapshot& snapshot);

private:
    IAppContext* m_context = nullptr;
    SystemInfoSnapshot m_lastSnapshot;
    QPointer<SystemInfoPanelWidget> m_panel;
};

} // namespace Vitals
