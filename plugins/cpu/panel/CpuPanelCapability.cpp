#include "panel/CpuPanelCapability.h"

#include "CpuPanelWidget.h"

namespace Vitals {

QString CpuPanelCapability::panelId() const
{
    return QStringLiteral("cpu");
}

QString CpuPanelCapability::panelName() const
{
    return QStringLiteral("CPU Monitor");
}

QString CpuPanelCapability::panelIconKey() const
{
    return QStringLiteral("cpu");
}

QWidget* CpuPanelCapability::createPanel(QWidget* parent)
{
    auto* panel = new CpuPanelWidget(parent);
    panel->applySnapshot(m_lastSnapshot);
    m_panel = panel;
    return panel;
}

void CpuPanelCapability::updateSnapshot(const CpuSnapshot& snapshot)
{
    m_lastSnapshot = snapshot;
    if (m_panel) {
        m_panel->applySnapshot(m_lastSnapshot);
    }
}

} // namespace Vitals
