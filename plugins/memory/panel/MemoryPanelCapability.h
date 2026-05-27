#pragma once

#include "IMemoryCollector.h"
#include "IPanelCapability.h"

#include <QPointer>

namespace Vitals {

class MemoryPanelWidget;
class IAppContext;

class MemoryPanelCapability : public IPanelCapability
{
public:
    explicit MemoryPanelCapability(IAppContext* context);

    QString panelId() const override;
    QString panelName() const override;
    QString panelIconKey() const override;
    QWidget* createPanel(QWidget* parent = nullptr) override;

    void updateSnapshot(const MemorySnapshot& snapshot);

private:
    IAppContext* m_context = nullptr;
    MemorySnapshot m_lastSnapshot;
    QPointer<MemoryPanelWidget> m_panel;
};

} // namespace Vitals
