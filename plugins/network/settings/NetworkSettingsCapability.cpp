#include "settings/NetworkSettingsCapability.h"

#include "IAppContext.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

namespace Vitals {

NetworkSettingsCapability::NetworkSettingsCapability(IAppContext* context)
    : m_context(context)
{
}

QString NetworkSettingsCapability::settingsId() const
{
    return QStringLiteral("network");
}

QString NetworkSettingsCapability::settingsTitle() const
{
    return text(QStringLiteral("network.title"), QStringLiteral("Network Monitor"));
}

QWidget* NetworkSettingsCapability::createSettingsWidget(QWidget* parent)
{
    auto* root = new QWidget(parent);
    auto* layout = new QVBoxLayout(root);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* label = new QLabel(
        text(QStringLiteral("network.settingsReady"), QStringLiteral("Network settings capability is wired and ready for future plugin-specific controls.")),
        root);
    label->setWordWrap(true);
    layout->addWidget(label);
    layout->addStretch(1);
    return root;
}

QString NetworkSettingsCapability::text(const QString& key, const QString& fallback) const
{
    return m_context ? m_context->translate(key, fallback) : fallback;
}

} // namespace Vitals
