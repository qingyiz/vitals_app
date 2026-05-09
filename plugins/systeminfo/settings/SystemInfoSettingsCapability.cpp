#include "settings/SystemInfoSettingsCapability.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

namespace Vitals {

QString SystemInfoSettingsCapability::settingsId() const
{
    return QStringLiteral("systeminfo");
}

QString SystemInfoSettingsCapability::settingsTitle() const
{
    return QStringLiteral("System Information");
}

QWidget* SystemInfoSettingsCapability::createSettingsWidget(QWidget* parent)
{
    auto* root = new QWidget(parent);
    auto* layout = new QVBoxLayout(root);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* label = new QLabel(
        QStringLiteral("System information settings capability is wired and ready for future plugin-specific controls."),
        root);
    label->setWordWrap(true);
    layout->addWidget(label);
    layout->addStretch(1);
    return root;
}

} // namespace Vitals
