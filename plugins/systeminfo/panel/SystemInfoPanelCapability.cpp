#include "panel/SystemInfoPanelCapability.h"

#include "IAppContext.h"
#include "SystemInfoPanelWidget.h"

namespace Vitals {

SystemInfoPanelCapability::SystemInfoPanelCapability(IAppContext* context)
    : m_context(context)
{
}

QString SystemInfoPanelCapability::panelId() const
{
    return QStringLiteral("systeminfo");
}

QString SystemInfoPanelCapability::panelName() const
{
    return m_context ? m_context->translate(QStringLiteral("nav.systemInfo"), QStringLiteral("System Info"))
                     : QStringLiteral("System Info");
}

QString SystemInfoPanelCapability::panelIconKey() const
{
    return QStringLiteral("system");
}

QWidget* SystemInfoPanelCapability::createPanel(QWidget* parent)
{
    auto* panel = new SystemInfoPanelWidget(m_context, parent);
    panel->applySnapshot(m_lastSnapshot);
    m_panel = panel;
    return panel;
}

void SystemInfoPanelCapability::updateSnapshot(const SystemInfoSnapshot& snapshot)
{
    m_lastSnapshot = snapshot;
    if (m_panel && m_panel->isVisible()) {
        m_panel->applySnapshot(m_lastSnapshot);
    }
}

} // namespace Vitals
