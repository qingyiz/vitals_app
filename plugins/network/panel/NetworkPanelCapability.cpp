#include "panel/NetworkPanelCapability.h"

#include "NetworkPanelWidget.h"

namespace Vitals {

QString NetworkPanelCapability::panelId() const
{
    return QStringLiteral("network");
}

QString NetworkPanelCapability::panelName() const
{
    return QStringLiteral("Network Monitor");
}

QString NetworkPanelCapability::panelIconKey() const
{
    return QStringLiteral("network");
}

QWidget* NetworkPanelCapability::createPanel(QWidget* parent)
{
    auto* panel = new NetworkPanelWidget(parent);
    panel->applySnapshot(m_lastSnapshot);
    m_panel = panel;
    return panel;
}

void NetworkPanelCapability::updateSnapshot(const NetworkSnapshot& snapshot)
{
    m_lastSnapshot = snapshot;
    if (m_panel) {
        m_panel->applySnapshot(m_lastSnapshot);
    }
}

} // namespace Vitals
