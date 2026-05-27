#include "panel/CpuPanelCapability.h"

#include "CpuPanelWidget.h"
#include "IAppContext.h"

namespace Vitals {

CpuPanelCapability::CpuPanelCapability(IAppContext* context)
    : m_context(context)
{
}

QString CpuPanelCapability::panelId() const
{
    return QStringLiteral("cpu");
}

QString CpuPanelCapability::panelName() const
{
    return m_context ? m_context->translate(QStringLiteral("nav.cpu"), QStringLiteral("CPU"))
                     : QStringLiteral("CPU");
}

QString CpuPanelCapability::panelIconKey() const
{
    return QStringLiteral("cpu");
}

QWidget* CpuPanelCapability::createPanel(QWidget* parent)
{
    auto* panel = new CpuPanelWidget(m_context, parent);
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
