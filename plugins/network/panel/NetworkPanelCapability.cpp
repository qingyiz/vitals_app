#include "panel/NetworkPanelCapability.h"

#include "IAppContext.h"
#include "NetworkPanelWidget.h"

namespace Vitals {

NetworkPanelCapability::NetworkPanelCapability(IAppContext* context)
    : m_context(context)
{
}

QString NetworkPanelCapability::panelId() const
{
    return QStringLiteral("network");
}

QString NetworkPanelCapability::panelName() const
{
    return m_context ? m_context->translate(QStringLiteral("nav.network"), QStringLiteral("Network"))
                     : QStringLiteral("Network");
}

QString NetworkPanelCapability::panelIconKey() const
{
    return QStringLiteral("network");
}

QWidget* NetworkPanelCapability::createPanel(QWidget* parent)
{
    auto* panel = new NetworkPanelWidget(m_context, parent);
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
