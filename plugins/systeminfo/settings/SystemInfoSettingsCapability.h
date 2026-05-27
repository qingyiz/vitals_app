#pragma once

#include "ISettingsCapability.h"

namespace Vitals {

class IAppContext;

class SystemInfoSettingsCapability : public ISettingsCapability
{
public:
    explicit SystemInfoSettingsCapability(IAppContext* context);

    QString settingsId() const override;
    QString settingsTitle() const override;
    QWidget* createSettingsWidget(QWidget* parent = nullptr) override;

private:
    QString text(const QString& key, const QString& fallback) const;

    IAppContext* m_context = nullptr;
};

} // namespace Vitals
