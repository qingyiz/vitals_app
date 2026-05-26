#include "PluginCenterWidget.h"

#include "config/ConfigManager.h"
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

PluginCenterWidget::PluginCenterWidget(ConfigManager* configManager, QWidget* parent)
    : QWidget(parent)
    , m_configManager(configManager)
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(22, 18, 22, 20);
    rootLayout->setSpacing(10);

    auto* title = new QLabel(QStringLiteral("Plugins"), this);
    title->setObjectName(QStringLiteral("pageTitle"));

    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setObjectName(QStringLiteral("pageSubtitle"));
    m_summaryLabel->setWordWrap(true);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* cardsContainer = new QWidget(scrollArea);
    m_cardsLayout = new QVBoxLayout(cardsContainer);
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setSpacing(8);
    m_cardsLayout->setAlignment(Qt::AlignTop);

    rootLayout->addWidget(title);
    rootLayout->addWidget(m_summaryLabel);
    scrollArea->setWidget(cardsContainer);
    rootLayout->addWidget(scrollArea, 1);
}

void PluginCenterWidget::setPluginInfos(const QList<PluginRuntimeInfo>& pluginInfos)
{
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

    m_summaryLabel->setText(QStringLiteral(
        "%1 loaded, %2 disabled, %3 skipped, %4 failed. Plugin switches apply immediately.")
        .arg(loadedCount)
        .arg(disabledCount)
        .arg(skippedCount)
        .arg(failedCount));

    if (pluginInfos.isEmpty()) {
        auto* empty = new QLabel(QStringLiteral("No plugin binaries were discovered in the runtime plugins directory."), this);
        empty->setObjectName(QStringLiteral("panelBody"));
        empty->setWordWrap(true);
        m_cardsLayout->addWidget(empty);
    }
}

QWidget* PluginCenterWidget::createPluginCard(const PluginRuntimeInfo& pluginInfo)
{
    auto* card = new QFrame(this);
    card->setObjectName(QStringLiteral("emptyPanel"));

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

    auto* toggleContainer = new QWidget(card);
    auto* toggleLayout = new QHBoxLayout(toggleContainer);
    toggleLayout->setContentsMargins(0, 0, 0, 0);
    toggleLayout->setSpacing(8);

    auto* enabledLabel = new QLabel(QStringLiteral("Enabled"), card);
    enabledLabel->setObjectName(QStringLiteral("panelBody"));
    auto* enabledSwitch = new ToggleSwitch(card);
    enabledSwitch->setChecked(m_configManager->isPluginEnabled(pluginInfo.metaInfo.id, pluginInfo.filePath));
    connect(enabledSwitch, &ToggleSwitch::toggled, card, [this, pluginInfo](bool checked) {
        Q_EMIT pluginEnabledChanged(pluginInfo.metaInfo.id, pluginInfo.filePath, checked);
    });
    toggleLayout->addWidget(enabledLabel);
    toggleLayout->addWidget(enabledSwitch);

    if (pluginInfo.metaInfo.supportsTaskbarDisplay) {
        auto* taskbarLabel = new QLabel(QStringLiteral("Menu Bar"), card);
        taskbarLabel->setObjectName(QStringLiteral("panelBody"));
        auto* taskbarSwitch = new ToggleSwitch(card);
        taskbarSwitch->setChecked(
            m_configManager->isPluginTaskbarEnabled(pluginInfo.metaInfo.id, pluginInfo.filePath, true));
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
        QStringLiteral("ID: %1\nCategory: %2\nVersion: %3\nAuthor: %4\nSupported Platforms: %5\nMenu Bar: %6")
            .arg(pluginInfo.metaInfo.id.isEmpty() ? QStringLiteral("--") : pluginInfo.metaInfo.id)
            .arg(pluginInfo.metaInfo.category.isEmpty() ? QStringLiteral("--") : pluginInfo.metaInfo.category)
            .arg(pluginInfo.metaInfo.version.isEmpty() ? QStringLiteral("--") : pluginInfo.metaInfo.version)
            .arg(pluginInfo.metaInfo.author.isEmpty() ? QStringLiteral("--") : pluginInfo.metaInfo.author)
            .arg(supportedPlatformsText(pluginInfo))
            .arg(pluginInfo.metaInfo.supportsTaskbarDisplay ? QStringLiteral("Supported") : QStringLiteral("Not supported")),
        QStringLiteral("panelBody"), card);

    auto* path = createBodyLabel(
        QStringLiteral("Binary: %1").arg(pluginInfo.filePath),
        QStringLiteral("pluginPathLabel"), card);

    layout->addLayout(header);
    layout->addWidget(meta);

    if (!pluginInfo.reason.isEmpty()) {
        auto* reason = createBodyLabel(
            QStringLiteral("Reason: %1").arg(pluginInfo.reason),
            QStringLiteral("pluginReasonLabel"), card);
        layout->addWidget(reason);
    }

    layout->addWidget(path);
    return card;
}

QString PluginCenterWidget::statusText(PluginRuntimeInfo::Status status)
{
    switch (status) {
    case PluginRuntimeInfo::Status::Loaded:
        return QStringLiteral("Loaded");
    case PluginRuntimeInfo::Status::Disabled:
        return QStringLiteral("Disabled");
    case PluginRuntimeInfo::Status::Skipped:
        return QStringLiteral("Skipped");
    case PluginRuntimeInfo::Status::Failed:
        return QStringLiteral("Failed");
    }
    return QStringLiteral("Unknown");
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

} // namespace Vitals
