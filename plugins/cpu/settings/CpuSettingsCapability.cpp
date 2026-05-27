#include "settings/CpuSettingsCapability.h"

#include "IAppContext.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

namespace Vitals {

CpuSettingsCapability::CpuSettingsCapability(IAppContext* context)
    : m_context(context)
{
}

QString CpuSettingsCapability::settingsId() const
{
    return QStringLiteral("cpu");
}

QString CpuSettingsCapability::settingsTitle() const
{
    return text(QStringLiteral("cpu.title"), QStringLiteral("CPU Monitor"));
}

QWidget* CpuSettingsCapability::createSettingsWidget(QWidget* parent)
{
    auto* root = new QWidget(parent);
    auto* layout = new QVBoxLayout(root);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* label = new QLabel(
        text(QStringLiteral("cpu.settingsReady"), QStringLiteral("CPU settings capability is wired and ready for future plugin-specific controls.")),
        root);
    label->setWordWrap(true);
    layout->addWidget(label);
    layout->addStretch(1);
    return root;
}

QString CpuSettingsCapability::text(const QString& key, const QString& fallback) const
{
    return m_context ? m_context->translate(key, fallback) : fallback;
}

} // namespace Vitals
