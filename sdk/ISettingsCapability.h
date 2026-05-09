#pragma once

#include <QString>

class QWidget;

namespace Vitals {

class ISettingsCapability
{
public:
    virtual ~ISettingsCapability() = default;

    virtual QString settingsId() const = 0;
    virtual QString settingsTitle() const = 0;
    virtual QWidget* createSettingsWidget(QWidget* parent = nullptr) = 0;
};

} // namespace Vitals
