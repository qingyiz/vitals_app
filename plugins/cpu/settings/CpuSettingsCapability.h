#pragma once

#include "ISettingsCapability.h"

namespace Vitals {

class IAppContext;

class CpuSettingsCapability : public ISettingsCapability
{
public:
    explicit CpuSettingsCapability(IAppContext* context);

    QString settingsId() const override;
    QString settingsTitle() const override;
    QWidget* createSettingsWidget(QWidget* parent = nullptr) override;

private:
    QString text(const QString& key, const QString& fallback) const;

    IAppContext* m_context = nullptr;
};

} // namespace Vitals
