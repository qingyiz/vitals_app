#pragma once

#include "ISettingsCapability.h"

namespace Vitals {

class NetworkSettingsCapability : public ISettingsCapability
{
public:
    QString settingsId() const override;
    QString settingsTitle() const override;
    QWidget* createSettingsWidget(QWidget* parent = nullptr) override;
};

} // namespace Vitals
