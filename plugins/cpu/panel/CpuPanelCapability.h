#pragma once

#include "IPanelCapability.h"
#include "ICpuCollector.h"

#include <QPointer>

namespace Vitals {

class CpuPanelWidget;

class CpuPanelCapability : public IPanelCapability
{
public:
    QString panelId() const override;
    QString panelName() const override;
    QString panelIconKey() const override;
    QWidget* createPanel(QWidget* parent = nullptr) override;

    void updateSnapshot(const CpuSnapshot& snapshot);

private:
    CpuSnapshot m_lastSnapshot;
    QPointer<CpuPanelWidget> m_panel;
};

} // namespace Vitals
