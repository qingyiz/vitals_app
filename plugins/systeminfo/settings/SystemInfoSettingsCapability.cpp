#include "settings/SystemInfoSettingsCapability.h"

#include "IAppContext.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

namespace Vitals {

SystemInfoSettingsCapability::SystemInfoSettingsCapability(IAppContext* context)
    : m_context(context)
{
}

QString SystemInfoSettingsCapability::settingsId() const
{
    return QStringLiteral("systeminfo");
}

QString SystemInfoSettingsCapability::settingsTitle() const
{
    return text(QStringLiteral("systemInfo.title"), QStringLiteral("System Information"));
}

QWidget* SystemInfoSettingsCapability::createSettingsWidget(QWidget* parent)
{
    auto* root = new QWidget(parent);
    auto* layout = new QVBoxLayout(root);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* label = new QLabel(
        text(QStringLiteral("systemInfo.settingsReady"), QStringLiteral("System information settings capability is wired and ready for future plugin-specific controls.")),
        root);
    label->setWordWrap(true);
    layout->addWidget(label);
    layout->addStretch(1);
    return root;
}

QString SystemInfoSettingsCapability::text(const QString& key, const QString& fallback) const
{
    return m_context ? m_context->translate(key, fallback) : fallback;
}

} // namespace Vitals
