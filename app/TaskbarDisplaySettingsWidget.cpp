#include "TaskbarDisplaySettingsWidget.h"

#include "ITaskbarCapability.h"
#include "config/ConfigManager.h"
#include "language/LanguageManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace Vitals {

namespace {

QJsonObject loadConfig(const QString& configPath)
{
    QFile file(configPath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return QJsonDocument::fromJson(file.readAll()).object();
}

bool saveConfig(const QString& configPath, const QJsonObject& root)
{
    QDir dir = QFileInfo(configPath).dir();
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        return false;
    }

    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

} // namespace

TaskbarDisplaySettingsWidget::TaskbarDisplaySettingsWidget(
    const QString& pluginId,
    const ITaskbarCapability* taskbarCapability,
    ConfigManager* configManager,
    LanguageManager* languageManager,
    QWidget* parent)
    : QFrame(parent)
    , m_pluginId(pluginId)
    , m_configManager(configManager)
    , m_languageManager(languageManager)
{
    setObjectName(QStringLiteral("taskbarSettingsPanel"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(18, 10, 18, 10);
    layout->setSpacing(14);

    auto* copyColumn = new QWidget(this);
    copyColumn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    auto* copyLayout = new QVBoxLayout(copyColumn);
    copyLayout->setContentsMargins(0, 0, 0, 0);
    copyLayout->setSpacing(2);

    auto* title = new QLabel(text(QStringLiteral("common.taskbarDisplay"), QStringLiteral("Taskbar Display")), copyColumn);
    title->setObjectName(QStringLiteral("taskbarSettingsTitle"));

    auto* hint = new QLabel(
        text(QStringLiteral("common.taskbarLabelHint"),
            QStringLiteral("Compact label shown before the live value.")),
        copyColumn);
    hint->setObjectName(QStringLiteral("taskbarSettingsHint"));
    hint->setWordWrap(false);
    hint->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    hint->setMinimumWidth(0);
    copyLayout->addWidget(title);
    copyLayout->addWidget(hint);

    const QList<TaskbarLabelDescriptor> descriptors = taskbarCapability
        ? taskbarCapability->taskbarLabelDescriptors()
        : QList<TaskbarLabelDescriptor>{};

    auto* controlGroup = new QWidget(this);
    controlGroup->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    auto* controlLayout = new QVBoxLayout(controlGroup);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->setSpacing(6);

    for (const TaskbarLabelDescriptor& descriptor : descriptors) {
        if (descriptor.configKey.isEmpty()) {
            continue;
        }

        auto* row = new QWidget(controlGroup);
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->setSpacing(8);

        auto* label = new QLabel(text(descriptor.titleKey, descriptor.titleFallback), row);
        label->setObjectName(QStringLiteral("taskbarSettingsFieldLabel"));
        label->setMinimumWidth(62);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        auto* lineEdit = new QLineEdit(row);
        lineEdit->setObjectName(QStringLiteral("taskbarSettingsLineEdit"));
        lineEdit->setMaxLength(12);
        lineEdit->setFixedWidth(118);
        lineEdit->setPlaceholderText(descriptor.defaultLabel);
        lineEdit->setText(currentLabel(descriptor.configKey, descriptor.defaultLabel));
        connect(lineEdit, &QLineEdit::textChanged, this,
            [this, configKey = descriptor.configKey](const QString& value) {
                saveLabel(configKey, value);
                Q_EMIT settingsChanged();
            });

        rowLayout->addWidget(label);
        rowLayout->addWidget(lineEdit);
        controlLayout->addWidget(row);
    }

    layout->addWidget(copyColumn, 1);
    layout->addWidget(controlGroup);
}

QString TaskbarDisplaySettingsWidget::text(const QString& key, const QString& fallback) const
{
    return m_languageManager ? m_languageManager->translate(key, fallback) : fallback;
}

QString TaskbarDisplaySettingsWidget::currentLabel(const QString& configKey, const QString& defaultLabel) const
{
    if (!m_configManager || m_pluginId.isEmpty()) {
        return defaultLabel;
    }

    const QString configPath = m_configManager->pluginConfigPath(m_pluginId);
    const QString label = loadConfig(configPath).value(configKey).toString().trimmed();
    return label.isEmpty() ? defaultLabel : label;
}

void TaskbarDisplaySettingsWidget::saveLabel(const QString& configKey, const QString& label) const
{
    if (!m_configManager || m_pluginId.isEmpty() || configKey.isEmpty()) {
        return;
    }

    const QString configPath = m_configManager->pluginConfigPath(m_pluginId);
    QJsonObject root = loadConfig(configPath);
    const QString trimmed = label.trimmed();
    if (trimmed.isEmpty()) {
        root.remove(configKey);
    } else {
        root.insert(configKey, trimmed);
    }
    saveConfig(configPath, root);
}

} // namespace Vitals
