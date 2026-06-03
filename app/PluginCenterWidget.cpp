#include "PluginCenterWidget.h"

#include "config/ConfigManager.h"
#include "language/LanguageManager.h"
#include "ToggleSwitch.h"

#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLayoutItem>
#include <QScrollArea>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace Vitals {

namespace {

QLabel* createBodyLabel(const QString& text, const QString& objectName, QWidget* parent)
{
    auto* label = new QLabel(text, parent);
    label->setObjectName(objectName);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    return label;
}

} // namespace

PluginCenterWidget::PluginCenterWidget(ConfigManager* configManager, LanguageManager* languageManager, QWidget* parent)
    : QWidget(parent)
    , m_configManager(configManager)
    , m_languageManager(languageManager)
{
    setMinimumSize(0, 0);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(22, 18, 22, 20);
    rootLayout->setSpacing(10);

    m_titleLabel = new QLabel(text(QStringLiteral("plugins.title"), QStringLiteral("Plugins")), this);
    m_titleLabel->setObjectName(QStringLiteral("pageTitle"));

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setObjectName(QStringLiteral("pageSubtitle"));
    m_summaryLabel->setWordWrap(true);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setMinimumSize(0, 0);
    scrollArea->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    auto* cardsContainer = new QWidget(scrollArea);
    cardsContainer->setMinimumWidth(560);
    cardsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_cardsLayout = new QVBoxLayout(cardsContainer);
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setSpacing(8);
    m_cardsLayout->setAlignment(Qt::AlignTop);

    scrollArea->setWidget(cardsContainer);

    rootLayout->addWidget(m_titleLabel);
    rootLayout->addWidget(m_summaryLabel);
    rootLayout->addWidget(scrollArea, 1);
}

void PluginCenterWidget::setPluginInfos(
    const QList<PluginRuntimeInfo>& pluginInfos,
    const QHash<QString, bool>& taskbarDefaultEnabledByFilePath)
{
    m_taskbarDefaultEnabledByFilePath = taskbarDefaultEnabledByFilePath;
    clearLayout(m_cardsLayout);

    int loadedCount = 0;
    int disabledCount = 0;
    int skippedCount = 0;
    int failedCount = 0;

    for (const PluginRuntimeInfo& pluginInfo : pluginInfos) {
        if (pluginInfo.status == PluginRuntimeInfo::Status::Loaded) {
            ++loadedCount;
        } else if (pluginInfo.status == PluginRuntimeInfo::Status::Disabled) {
            ++disabledCount;
        } else if (pluginInfo.status == PluginRuntimeInfo::Status::Skipped) {
            ++skippedCount;
        } else {
            ++failedCount;
        }
        m_cardsLayout->addWidget(createPluginCard(pluginInfo));
    }

    m_summaryLabel->setText(text(QStringLiteral("plugins.summary"),
        QStringLiteral("%1 loaded, %2 disabled, %3 skipped, %4 failed. Plugin switches apply immediately."))
        .arg(loadedCount)
        .arg(disabledCount)
        .arg(skippedCount)
        .arg(failedCount));

    if (pluginInfos.isEmpty()) {
        auto* empty = new QLabel(text(QStringLiteral("plugins.empty"),
            QStringLiteral("No plugin binaries were discovered in the runtime plugins directory.")), this);
        empty->setObjectName(QStringLiteral("panelBody"));
        empty->setWordWrap(true);
        m_cardsLayout->addWidget(empty);
    }

    m_cardsLayout->addStretch(1);
}

QWidget* PluginCenterWidget::createPluginCard(const PluginRuntimeInfo& pluginInfo)
{
    auto* card = new QFrame(this);
    card->setObjectName(QStringLiteral("emptyPanel"));
    card->setMinimumSize(560, 132);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 11, 14, 12);
    layout->setSpacing(7);

    auto* header = new QHBoxLayout();
    header->setSpacing(10);

    const QString pluginName = pluginInfo.metaInfo.name.isEmpty()
        ? QFileInfo(pluginInfo.filePath).baseName()
        : pluginInfo.metaInfo.name;

    auto* title = new QLabel(pluginName, card);
    title->setObjectName(QStringLiteral("panelTitle"));
    title->setWordWrap(true);
    title->setMinimumWidth(120);
    title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto* toggleContainer = new QWidget(card);
    auto* toggleLayout = new QHBoxLayout(toggleContainer);
    toggleLayout->setContentsMargins(0, 0, 0, 0);
    toggleLayout->setSpacing(8);

    auto* enabledLabel = new QLabel(text(QStringLiteral("plugins.enabled"), QStringLiteral("Enabled")), card);
    enabledLabel->setObjectName(QStringLiteral("panelBody"));
    auto* enabledSwitch = new ToggleSwitch(card);
    enabledSwitch->setChecked(m_configManager->isPluginEnabled(pluginInfo.metaInfo.id, pluginInfo.filePath));
    connect(enabledSwitch, &ToggleSwitch::toggled, card, [this, pluginInfo](bool checked) {
        Q_EMIT pluginEnabledChanged(pluginInfo.metaInfo.id, pluginInfo.filePath, checked);
    });
    toggleLayout->addWidget(enabledLabel);
    toggleLayout->addWidget(enabledSwitch);

    if (pluginInfo.metaInfo.supportsTaskbarDisplay) {
        auto* taskbarLabel = new QLabel(text(QStringLiteral("plugins.menuBar"), QStringLiteral("Menu Bar")), card);
        taskbarLabel->setObjectName(QStringLiteral("panelBody"));
        auto* taskbarSwitch = new ToggleSwitch(card);
        const bool taskbarDefaultEnabled = m_taskbarDefaultEnabledByFilePath.value(pluginInfo.filePath, true);
        taskbarSwitch->setChecked(
            m_configManager->isPluginTaskbarEnabled(pluginInfo.metaInfo.id, pluginInfo.filePath, taskbarDefaultEnabled));
        connect(taskbarSwitch, &ToggleSwitch::toggled, card, [this, pluginInfo](bool checked) {
            Q_EMIT pluginTaskbarVisibilityChanged(pluginInfo.metaInfo.id, pluginInfo.filePath, checked);
        });
        toggleLayout->addSpacing(4);
        toggleLayout->addWidget(taskbarLabel);
        toggleLayout->addWidget(taskbarSwitch);
    }

    auto* status = new QLabel(statusText(pluginInfo.status), card);
    status->setObjectName(QStringLiteral("pluginStatusPill"));
    status->setStyleSheet(QStringLiteral(
        "QLabel#pluginStatusPill { background: %1; color: white; border-radius: 9px; padding: 3px 9px; font-size: 11px; font-weight: 700; }")
        .arg(statusColor(pluginInfo.status)));

    header->addWidget(title);
    header->addStretch();
    header->addWidget(toggleContainer);
    header->addWidget(status);

    auto* meta = createBodyLabel(
        text(QStringLiteral("plugins.meta"),
            QStringLiteral("ID: %1\nCategory: %2\nVersion: %3\nAuthor: %4\nSupported Platforms: %5\nMenu Bar: %6"))
            .arg(pluginInfo.metaInfo.id.isEmpty() ? QStringLiteral("--") : pluginInfo.metaInfo.id)
            .arg(pluginInfo.metaInfo.category.isEmpty() ? QStringLiteral("--") : pluginInfo.metaInfo.category)
            .arg(pluginInfo.metaInfo.version.isEmpty() ? QStringLiteral("--") : pluginInfo.metaInfo.version)
            .arg(pluginInfo.metaInfo.author.isEmpty() ? QStringLiteral("--") : pluginInfo.metaInfo.author)
            .arg(supportedPlatformsText(pluginInfo))
            .arg(pluginInfo.metaInfo.supportsTaskbarDisplay
                    ? text(QStringLiteral("plugins.supported"), QStringLiteral("Supported"))
                    : text(QStringLiteral("plugins.notSupported"), QStringLiteral("Not supported"))),
        QStringLiteral("panelBody"), card);

    auto* path = createBodyLabel(
        text(QStringLiteral("plugins.binary"), QStringLiteral("Binary: %1")).arg(pluginInfo.filePath),
        QStringLiteral("pluginPathLabel"), card);

    layout->addLayout(header);
    layout->addWidget(meta);

    if (!pluginInfo.reason.isEmpty()) {
        auto* reason = createBodyLabel(
            text(QStringLiteral("plugins.reason"), QStringLiteral("Reason: %1")).arg(pluginInfo.reason),
            QStringLiteral("pluginReasonLabel"), card);
        layout->addWidget(reason);
    }

    layout->addWidget(path);
    return card;
}

QString PluginCenterWidget::statusText(PluginRuntimeInfo::Status status) const
{
    switch (status) {
    case PluginRuntimeInfo::Status::Loaded:
        return text(QStringLiteral("plugins.status.loaded"), QStringLiteral("Loaded"));
    case PluginRuntimeInfo::Status::Disabled:
        return text(QStringLiteral("plugins.status.disabled"), QStringLiteral("Disabled"));
    case PluginRuntimeInfo::Status::Skipped:
        return text(QStringLiteral("plugins.status.skipped"), QStringLiteral("Skipped"));
    case PluginRuntimeInfo::Status::Failed:
        return text(QStringLiteral("plugins.status.failed"), QStringLiteral("Failed"));
    }
    return text(QStringLiteral("plugins.status.unknown"), QStringLiteral("Unknown"));
}

QString PluginCenterWidget::statusColor(PluginRuntimeInfo::Status status)
{
    switch (status) {
    case PluginRuntimeInfo::Status::Loaded:
        return QStringLiteral("#32a852");
    case PluginRuntimeInfo::Status::Disabled:
        return QStringLiteral("#6e6e73");
    case PluginRuntimeInfo::Status::Skipped:
        return QStringLiteral("#9b8b2f");
    case PluginRuntimeInfo::Status::Failed:
        return QStringLiteral("#d84b3e");
    }
    return QStringLiteral("#8e8e93");
}

QString PluginCenterWidget::supportedPlatformsText(const PluginRuntimeInfo& pluginInfo)
{
    return pluginInfo.metaInfo.supportedPlatforms.isEmpty()
        ? QStringLiteral("--")
        : pluginInfo.metaInfo.supportedPlatforms.join(QStringLiteral(", "));
}

void PluginCenterWidget::clearLayout(QLayout* layout)
{
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        if (QLayout* childLayout = item->layout()) {
            clearLayout(childLayout);
            delete childLayout;
        }
        delete item;
    }
}

QString PluginCenterWidget::text(const QString& key, const QString& fallback) const
{
    return m_languageManager ? m_languageManager->translate(key, fallback) : fallback;
}

} // namespace Vitals
