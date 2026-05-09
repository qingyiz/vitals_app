#include "settings/CpuSettingsCapability.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

namespace Vitals {

QString CpuSettingsCapability::settingsId() const
{
    return QStringLiteral("cpu");
}

QString CpuSettingsCapability::settingsTitle() const
{
    return QStringLiteral("CPU Monitor");
}

QWidget* CpuSettingsCapability::createSettingsWidget(QWidget* parent)
{
    auto* root = new QWidget(parent);
    auto* layout = new QVBoxLayout(root);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* label = new QLabel(
        QStringLiteral("CPU settings capability is wired and ready for future plugin-specific controls."),
        root);
    label->setWordWrap(true);
    layout->addWidget(label);
    layout->addStretch(1);
    return root;
}

} // namespace Vitals
