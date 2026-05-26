#include "panel/MemoryPanelCapability.h"

#include "MemoryPanelWidget.h"

namespace Vitals {

QString MemoryPanelCapability::panelId() const
{
    return QStringLiteral("memory");
}

QString MemoryPanelCapability::panelName() const
{
    return QStringLiteral("Memory Monitor");
}

QString MemoryPanelCapability::panelIconKey() const
{
    return QStringLiteral("memory");
}

QWidget* MemoryPanelCapability::createPanel(QWidget* parent)
{
    auto* panel = new MemoryPanelWidget(parent);
    panel->applySnapshot(m_lastSnapshot);
    m_panel = panel;
    return panel;
}

void MemoryPanelCapability::updateSnapshot(const MemorySnapshot& snapshot)
{
    m_lastSnapshot = snapshot;
    if (m_panel) {
        m_panel->applySnapshot(m_lastSnapshot);
    }
}

} // namespace Vitals
