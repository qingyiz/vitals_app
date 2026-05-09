#include "settings/NetworkSettingsCapability.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

namespace Vitals {

QString NetworkSettingsCapability::settingsId() const
{
    return QStringLiteral("network");
}

QString NetworkSettingsCapability::settingsTitle() const
{
    return QStringLiteral("Network Monitor");
}

QWidget* NetworkSettingsCapability::createSettingsWidget(QWidget* parent)
{
    auto* root = new QWidget(parent);
    auto* layout = new QVBoxLayout(root);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* label = new QLabel(
        QStringLiteral("Network settings capability is wired and ready for future plugin-specific controls."),
        root);
    label->setWordWrap(true);
    layout->addWidget(label);
    layout->addStretch(1);
    return root;
}

} // namespace Vitals
