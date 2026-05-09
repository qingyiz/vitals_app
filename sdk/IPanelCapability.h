#pragma once

#include <QIcon>
#include <QString>

class QWidget;

namespace Vitals {

class IPanelCapability
{
public:
    virtual ~IPanelCapability() = default;

    virtual QString panelId() const = 0;
    virtual QString panelName() const = 0;
    virtual QString panelIconKey() const = 0;

    virtual QIcon panelIcon() const
    {
        return {};
    }

    virtual QWidget* createPanel(QWidget* parent = nullptr) = 0;
};

} // namespace Vitals
