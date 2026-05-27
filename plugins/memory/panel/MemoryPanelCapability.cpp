#include "panel/MemoryPanelCapability.h"

#include "IAppContext.h"
#include "MemoryPanelWidget.h"

namespace Vitals {

MemoryPanelCapability::MemoryPanelCapability(IAppContext* context)
    : m_context(context)
{
}

QString MemoryPanelCapability::panelId() const
{
    return QStringLiteral("memory");
}

QString MemoryPanelCapability::panelName() const
{
    return m_context ? m_context->translate(QStringLiteral("nav.memory"), QStringLiteral("Memory"))
                     : QStringLiteral("Memory");
}

QString MemoryPanelCapability::panelIconKey() const
{
    return QStringLiteral("memory");
}

QWidget* MemoryPanelCapability::createPanel(QWidget* parent)
{
    auto* panel = new MemoryPanelWidget(m_context, parent);
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
