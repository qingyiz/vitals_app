#include "panel/SystemInfoPanelCapability.h"

#include "SystemInfoPanelWidget.h"

namespace Vitals {

QString SystemInfoPanelCapability::panelId() const
{
    return QStringLiteral("systeminfo");
}

QString SystemInfoPanelCapability::panelName() const
{
    return QStringLiteral("System Info");
}

QString SystemInfoPanelCapability::panelIconKey() const
{
    return QStringLiteral("system");
}

QWidget* SystemInfoPanelCapability::createPanel(QWidget* parent)
{
    auto* panel = new SystemInfoPanelWidget(parent);
    panel->applySnapshot(m_lastSnapshot);
    m_panel = panel;
    return panel;
}

void SystemInfoPanelCapability::updateSnapshot(const SystemInfoSnapshot& snapshot)
{
    m_lastSnapshot = snapshot;
    if (m_panel) {
        m_panel->applySnapshot(m_lastSnapshot);
    }
}

} // namespace Vitals
