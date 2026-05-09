#pragma once

#include "INetworkCollector.h"
#include "IPanelCapability.h"

#include <QPointer>

namespace Vitals {

class NetworkPanelWidget;

class NetworkPanelCapability : public IPanelCapability
{
public:
    QString panelId() const override;
    QString panelName() const override;
    QString panelIconKey() const override;
    QWidget* createPanel(QWidget* parent = nullptr) override;

    void updateSnapshot(const NetworkSnapshot& snapshot);

private:
    NetworkSnapshot m_lastSnapshot;
    QPointer<NetworkPanelWidget> m_panel;
};

} // namespace Vitals
