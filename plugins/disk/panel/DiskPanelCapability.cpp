#include "panel/DiskPanelCapability.h"

#include "DiskPanelWidget.h"
#include "IAppContext.h"
#include "monitor/DiskMonitorCapability.h"

namespace Vitals {

DiskPanelCapability::DiskPanelCapability(DiskMonitorCapability* monitorCapability, IAppContext* context)
    : m_context(context)
    , m_monitorCapability(monitorCapability)
{
}

QString DiskPanelCapability::panelId() const
{
    return QStringLiteral("disk");
}

QString DiskPanelCapability::panelName() const
{
    return m_context ? m_context->translate(QStringLiteral("nav.disk"), QStringLiteral("Disk"))
                     : QStringLiteral("Disk");
}

QString DiskPanelCapability::panelIconKey() const
{
    return QStringLiteral("disk");
}

QWidget* DiskPanelCapability::createPanel(QWidget* parent)
{
    auto* panel = new DiskPanelWidget(m_context, parent);
    if (m_monitorCapability) {
        QObject::connect(panel, &DiskPanelWidget::diskSelected,
            m_monitorCapability, &DiskMonitorCapability::setSelectedRootPath);
    }
    panel->applySnapshot(m_lastSnapshot);
    m_panel = panel;
    return panel;
}

void DiskPanelCapability::updateSnapshot(const DiskSnapshot& snapshot)
{
    m_lastSnapshot = snapshot;
    if (m_panel) {
        m_panel->applySnapshot(m_lastSnapshot);
    }
}

} // namespace Vitals
