#include "MainWindow.h"

#include "AppContext.h"
#include "DashboardWidget.h"
#include "IPanelCapability.h"
#include "IPanelPlugin.h"
#include "ITaskbarCapability.h"
#include "ITaskbarDetailPlugin.h"
#include "ITaskbarDisplayPlugin.h"
#include "NavigationWidget.h"
#include "PluginCenterWidget.h"
#include "ToggleSwitch.h"
#include "TaskbarDisplaySettingsWidget.h"
#include "config/ConfigManager.h"
#include "language/LanguageManager.h"
#include "metric/MetricCenter.h"
#include "platform/taskbar/TaskbarIndicator.h"
#include "platform/taskbar/TaskbarIndicatorFactory.h"
#include "plugin/PluginManager.h"

#include <QApplication>
#include <QCloseEvent>
#include <QColor>
#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QHash>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QSizePolicy>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

namespace Vitals {

namespace {

struct NavigationIconSpec
{
    QString glyph;
    QColor background;
    QColor foreground;
};

NavigationIconSpec iconSpecForKey(const QString& key)
{
    if (key == QStringLiteral("dashboard")) {
        return {QStringLiteral("D"), QColor(QStringLiteral("#0a84ff")), Qt::white};
    }
    if (key == QStringLiteral("plugins")) {
        return {QStringLiteral("P"), QColor(QStringLiteral("#5e5ce6")), Qt::white};
    }
    if (key == QStringLiteral("system")) {
        return {QStringLiteral("S"), QColor(QStringLiteral("#64d2ff")), QColor(QStringLiteral("#0b2230"))};
    }
    if (key == QStringLiteral("cpu")) {
        return {QStringLiteral("C"), QColor(QStringLiteral("#ff453a")), Qt::white};
    }
    if (key == QStringLiteral("memory")) {
        return {QStringLiteral("M"), QColor(QStringLiteral("#32d74b")), QColor(QStringLiteral("#0b2230"))};
    }
    if (key == QStringLiteral("network")) {
        return {QStringLiteral("N"), QColor(QStringLiteral("#0a84ff")), Qt::white};
    }
    if (key == QStringLiteral("disk")) {
        return {QStringLiteral("D"), QColor(QStringLiteral("#bf5af2")), Qt::white};
    }
    if (key == QStringLiteral("battery")) {
        return {QStringLiteral("B"), QColor(QStringLiteral("#ffd60a")), QColor(QStringLiteral("#0b2230"))};
    }
    return {key.left(1).toUpper(), QColor(QStringLiteral("#8e8e93")), Qt::white};
}

enum class SidebarActionIcon
{
    Bug,
    Plugins,
    Pause,
    Resume,
    Quit
};

QIcon createSidebarActionIcon(SidebarActionIcon type, const QColor& color = QColor(QStringLiteral("#5f6368")))
{
    QPixmap pixmap(48, 48);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(color, 3.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    switch (type) {
    case SidebarActionIcon::Bug:
        painter.drawEllipse(QRectF(15, 17, 18, 18));
        painter.drawLine(QPointF(24, 12), QPointF(24, 17));
        painter.drawLine(QPointF(18, 14), QPointF(21, 18));
        painter.drawLine(QPointF(30, 14), QPointF(27, 18));
        painter.drawLine(QPointF(14, 22), QPointF(9, 20));
        painter.drawLine(QPointF(14, 28), QPointF(9, 30));
        painter.drawLine(QPointF(34, 22), QPointF(39, 20));
        painter.drawLine(QPointF(34, 28), QPointF(39, 30));
        painter.drawLine(QPointF(24, 18), QPointF(24, 34));
        break;
    case SidebarActionIcon::Plugins:
        painter.drawRoundedRect(QRectF(11, 11, 11, 11), 2.5, 2.5);
        painter.drawRoundedRect(QRectF(26, 11, 11, 11), 2.5, 2.5);
        painter.drawRoundedRect(QRectF(11, 26, 11, 11), 2.5, 2.5);
        painter.drawRoundedRect(QRectF(26, 26, 11, 11), 2.5, 2.5);
        break;
    case SidebarActionIcon::Pause:
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawRoundedRect(QRectF(17, 13, 5.5, 22), 2.6, 2.6);
        painter.drawRoundedRect(QRectF(25.5, 13, 5.5, 22), 2.6, 2.6);
        break;
    case SidebarActionIcon::Resume:
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        {
            QPainterPath playPath;
            playPath.moveTo(18, 12);
            playPath.lineTo(36, 24);
            playPath.lineTo(18, 36);
            playPath.closeSubpath();
            painter.drawPath(playPath);
        }
        break;
    case SidebarActionIcon::Quit:
        painter.drawLine(QPointF(24, 10), QPointF(24, 22));

        {
            QPainterPath powerPath;
            powerPath.moveTo(17.5, 17);
            powerPath.cubicTo(13.4, 20.5, 11.7, 26.6, 14.2, 31.8);
            powerPath.cubicTo(17.3, 38.2, 25.4, 40.5, 31.4, 36.7);
            powerPath.cubicTo(37.5, 32.8, 38.3, 23.6, 30.5, 17);
            painter.drawPath(powerPath);
        }
        break;
    }

    return QIcon(pixmap);
}

QHash<QString, bool> taskbarDefaultEnabledByFilePath(const PluginManager* pluginManager)
{
    QHash<QString, bool> defaults;
    if (!pluginManager) {
        return defaults;
    }

    QHash<QString, QString> loadedFilePathByPluginId;
    for (const PluginRuntimeInfo& pluginInfo : pluginManager->pluginInfos()) {
        if (pluginInfo.status == PluginRuntimeInfo::Status::Loaded) {
            loadedFilePathByPluginId.insert(pluginInfo.metaInfo.id, pluginInfo.filePath);
        }
    }

    for (IPlugin* plugin : pluginManager->plugins()) {
        if (!plugin) {
            continue;
        }

        ITaskbarCapability* capability = plugin->taskbarCapability();
        auto* provider = capability ? nullptr : dynamic_cast<ITaskbarDisplayPlugin*>(plugin);
        if (!capability && !provider) {
            continue;
        }

        const PluginMetaInfo meta = plugin->metaInfo();
        const QString filePath = loadedFilePathByPluginId.value(meta.id);
        if (filePath.isEmpty()) {
            continue;
        }

        defaults.insert(filePath,
            capability ? capability->isEnabledByDefault() : provider->isTaskbarDisplayEnabledByDefault());
    }

    return defaults;
}

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_metricCenter(new MetricCenter(this))
    , m_configManager(new ConfigManager())
    , m_languageManager(new LanguageManager(this))
    , m_appContext(new AppContext(m_metricCenter, m_configManager, m_languageManager))
    , m_pluginManager(new PluginManager(this))
    , m_taskbarIndicator(createTaskbarIndicator(this))
{
    m_languageManager->initialize(m_configManager->language());
    refreshTaskbarHostActions();
    setupUi();
    m_taskbarIndicator->initialize(this);
    m_taskbarIndicator->bindMetricCenter(m_metricCenter);
    connect(m_taskbarIndicator, &TaskbarIndicator::showRequested, this, [this]() {
        if (m_taskbarIndicator && m_taskbarIndicator->supportsDockIconVisibility()) {
            m_taskbarIndicator->setDockIconVisible(true);
        }
        showNormal();
        raise();
        activateWindow();
    });
    connect(m_taskbarIndicator, &TaskbarIndicator::quitRequested,
        qApp, &QApplication::quit);
    loadPlugins();
    rebuildPages();
    m_pluginManager->startAll();
    m_monitoringPaused = false;
}

MainWindow::~MainWindow()
{
    m_pluginManager->shutdownAll();
    delete m_appContext;
    delete m_configManager;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    event->ignore();

    if (m_taskbarIndicator && m_taskbarIndicator->isAvailable()) {
        hide();
        applyWindowPresencePolicy();
        showStatusMessage(text(QStringLiteral("status.windowHidden"),
            QStringLiteral("Vitals is still running in the background")), 3000);
        return;
    }

    showMinimized();
    showStatusMessage(text(QStringLiteral("status.windowMinimized"),
        QStringLiteral("Vitals was minimized because no tray entry is available")), 3000);
}

void MainWindow::setupUi()
{
    setWindowTitle(text(QStringLiteral("app.title"), QStringLiteral("Vitals")));
    applyStyle();

    auto* central = new QWidget(this);
    auto* layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* sidebar = new QWidget(central);
    sidebar->setObjectName(QStringLiteral("sidebar"));
    sidebar->setFixedWidth(172);
    auto* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 0, 0, 6);
    sidebarLayout->setSpacing(0);

    auto* content = new QWidget(central);
    content->setObjectName(QStringLiteral("contentShell"));
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    m_navigation = new NavigationWidget(sidebar);
    m_pages = new QStackedWidget(content);
    m_pages->setMinimumSize(0, 0);
    m_pages->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    sidebarLayout->addWidget(m_navigation, 1);
    sidebarLayout->addWidget(createSidebarActions(sidebar));
    contentLayout->addWidget(m_pages, 1);
    contentLayout->addWidget(createContentStatusBar(content));

    layout->addWidget(sidebar);
    layout->addWidget(content, 1);
    setCentralWidget(central);

    connect(m_navigation, &QListWidget::currentRowChanged,
        m_pages, &QStackedWidget::setCurrentIndex);
    rebuildPages();

    showStatusMessage(text(QStringLiteral("status.frameworkReady"), QStringLiteral("Framework ready")));
}

void MainWindow::loadPlugins()
{
    connect(m_pluginManager, &PluginManager::pluginLoaded, this,
        [this](const QString& pluginId, const QString& pluginName) {
            showStatusMessage(text(QStringLiteral("status.pluginLoaded"), QStringLiteral("Loaded plugin: %1")).arg(pluginName), 3000);
            Q_UNUSED(pluginId)
        }, Qt::UniqueConnection);
    connect(m_pluginManager, &PluginManager::pluginSkipped, this,
        [this](const QString& path, const QString& reason) {
            showStatusMessage(text(QStringLiteral("status.pluginSkipped"), QStringLiteral("Skipped plugin: %1")).arg(reason), 3000);
            Q_UNUSED(path)
        }, Qt::UniqueConnection);

    const QString pluginsDir = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("plugins"));
    m_pluginManager->loadAllPlugins(pluginsDir, m_appContext, m_configManager);
    syncTaskbarDisplays();
    if (m_pluginCenterPage) {
        m_pluginCenterPage->setPluginInfos(
            m_pluginManager->pluginInfos(),
            taskbarDefaultEnabledByFilePath(m_pluginManager));
    }

    showStatusMessage(text(QStringLiteral("status.pluginsLoaded"), QStringLiteral("Loaded %1 plugin(s)"))
        .arg(m_pluginManager->loadedPluginCount()));
}

void MainWindow::reloadPlugins()
{
    const QString preferredPageId = m_navigation ? m_navigation->currentItemId() : QString();

    m_pluginManager->stopAll();
    clearLoadedPluginMetrics();
    m_pluginManager->shutdownAll();
    loadPlugins();
    rebuildPages(preferredPageId);
    if (!m_monitoringPaused) {
        m_pluginManager->startAll();
    }
    showStatusMessage(text(QStringLiteral("status.pluginEnableUpdated"), QStringLiteral("Plugin enable state updated")), 3000);
}

void MainWindow::clearLoadedPluginMetrics()
{
    for (IPlugin* plugin : m_pluginManager->plugins()) {
        if (!plugin) {
            continue;
        }

        const QString pluginId = plugin->metaInfo().id;
        if (!pluginId.isEmpty()) {
            m_metricCenter->removePluginMetrics(pluginId);
        }
    }
}

void MainWindow::syncTaskbarDisplays()
{
    QList<TaskbarPluginDisplay> displays;
    const QList<PluginRuntimeInfo> pluginInfos = m_pluginManager->pluginInfos();

    for (IPlugin* plugin : m_pluginManager->plugins()) {
        ITaskbarCapability* capability = plugin ? plugin->taskbarCapability() : nullptr;
        auto* provider = capability ? nullptr : dynamic_cast<ITaskbarDisplayPlugin*>(plugin);
        if (!capability && !provider) {
            continue;
        }

        const PluginMetaInfo meta = plugin->metaInfo();
        QString filePath;
        for (const PluginRuntimeInfo& pluginInfo : pluginInfos) {
            if (pluginInfo.metaInfo.id == meta.id
                && pluginInfo.status == PluginRuntimeInfo::Status::Loaded) {
                filePath = pluginInfo.filePath;
                break;
            }
        }

        if (!m_configManager->isPluginTaskbarEnabled(
                meta.id,
                filePath,
                capability ? capability->isEnabledByDefault() : provider->isTaskbarDisplayEnabledByDefault())) {
            continue;
        }

        displays.append({
            meta.id,
            meta.name.isEmpty() ? meta.id : meta.name,
            filePath,
            capability,
            provider,
            capability ? nullptr : dynamic_cast<ITaskbarDetailPlugin*>(plugin)
        });
    }

    m_taskbarIndicator->setPluginDisplays(displays);
}

void MainWindow::addPage(const QString& id, const QString& title, QWidget* page, const QIcon& icon)
{
    m_navigation->addNavigationItem(id, title, icon);
    m_pages->addWidget(page);
}

QWidget* MainWindow::createContentStatusBar(QWidget* parent)
{
    auto* bar = new QWidget(parent);
    bar->setObjectName(QStringLiteral("contentStatusBar"));

    auto* layout = new QHBoxLayout(bar);
    layout->setContentsMargins(16, 6, 16, 6);
    layout->setSpacing(10);

    m_statusMessageLabel = new QLabel(bar);
    m_statusMessageLabel->setObjectName(QStringLiteral("contentStatusMessage"));
    m_statusMessageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_statusMessageLabel->setMinimumWidth(0);

    setupLanguageSelector(bar);
    setupDockIconSelector(bar);

    layout->addWidget(m_statusMessageLabel, 1);
    if (m_dockIconLabel && m_dockIconSwitch) {
        layout->addWidget(m_dockIconLabel);
        layout->addWidget(m_dockIconSwitch);
    }
    layout->addWidget(m_languageLabel);
    layout->addWidget(m_languageCombo);

    return bar;
}

void MainWindow::setupLanguageSelector(QWidget* parent)
{
    m_languageLabel = new QLabel(text(QStringLiteral("language.label"), QStringLiteral("Language")), parent);
    m_languageLabel->setObjectName(QStringLiteral("languageSelectorLabel"));
    m_languageCombo = new QComboBox(parent);
    m_languageCombo->setMinimumWidth(132);

    refreshLanguageSelector();

    connect(m_languageCombo, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        const QString languageCode = m_languageCombo->itemData(index).toString();
        if (languageCode.isEmpty() || languageCode == m_languageManager->currentLanguage()) {
            return;
        }
        if (!m_languageManager->setLanguage(languageCode)) {
            return;
        }

        m_configManager->setLanguage(languageCode);
        const QString currentPageId = m_navigation ? m_navigation->currentItemId() : QString();
        setWindowTitle(text(QStringLiteral("app.title"), QStringLiteral("Vitals")));
        m_languageLabel->setText(text(QStringLiteral("language.label"), QStringLiteral("Language")));
        if (m_dockIconLabel) {
            const QString dockText = text(QStringLiteral("settings.showDockIcon"), QStringLiteral("Dock"));
            const QString dockTooltip = text(QStringLiteral("settings.showDockIcon.tooltip"),
                QStringLiteral("Show Vitals in the macOS Dock. When disabled, reopen the window from the menu bar item."));
            m_dockIconLabel->setText(dockText);
            m_dockIconLabel->setToolTip(dockTooltip);
            if (m_dockIconSwitch) {
                m_dockIconSwitch->setToolTip(dockTooltip);
                m_dockIconSwitch->setAccessibleName(dockText);
            }
        }
        refreshSidebarActions();
        refreshTaskbarHostActions();
        refreshLanguageSelector();
        rebuildPages(currentPageId);
        showStatusMessage(text(QStringLiteral("status.languageChanged"), QStringLiteral("Language switched to %1"))
                .arg(m_languageCombo->currentText()),
            3000);
    });
}

void MainWindow::setupDockIconSelector(QWidget* parent)
{
    if (!m_taskbarIndicator || !m_taskbarIndicator->supportsDockIconVisibility()) {
        return;
    }

    const QString dockText = text(QStringLiteral("settings.showDockIcon"), QStringLiteral("Dock"));
    const QString dockTooltip = text(QStringLiteral("settings.showDockIcon.tooltip"),
        QStringLiteral("Show Vitals in the macOS Dock. When disabled, reopen the window from the menu bar item."));
    m_dockIconLabel = new QLabel(dockText, parent);
    m_dockIconLabel->setObjectName(QStringLiteral("languageSelectorLabel"));
    m_dockIconLabel->setToolTip(dockTooltip);

    m_dockIconSwitch = new ToggleSwitch(parent);
    m_dockIconSwitch->setChecked(m_configManager->showDockIcon());
    m_dockIconSwitch->setToolTip(dockTooltip);
    m_dockIconSwitch->setAccessibleName(dockText);

    connect(m_dockIconSwitch, &ToggleSwitch::toggled, this, [this](bool checked) {
        m_configManager->setShowDockIcon(checked);
        applyWindowPresencePolicy();
        if (checked) {
            showStatusMessage(text(QStringLiteral("status.dockIconShown"),
                QStringLiteral("Vitals will stay visible in the Dock while running in the background")), 3000);
            return;
        }

        showStatusMessage(isVisible()
                ? text(QStringLiteral("status.dockIconHiddenInBackground"),
                    QStringLiteral("Vitals will hide from the Dock after the main window is closed"))
                : text(QStringLiteral("status.dockIconHidden"),
                    QStringLiteral("Vitals is hidden from the Dock. Use the menu bar item to reopen it.")),
            3000);
    });
}

QWidget* MainWindow::createSidebarActions(QWidget* parent)
{
    auto* container = new QWidget(parent);
    container->setObjectName(QStringLiteral("sidebarActions"));

    auto* layout = new QHBoxLayout(container);
    layout->setContentsMargins(11, 4, 11, 0);
    layout->setSpacing(8);
    container->setMinimumHeight(42);

    m_bugReportButton = new QPushButton(container);
    m_bugReportButton->setObjectName(QStringLiteral("sidebarActionButton"));
    m_bugReportButton->setIcon(createSidebarActionIcon(SidebarActionIcon::Bug));
    m_bugReportButton->setFixedSize(30, 30);
    m_bugReportButton->setIconSize(QSize(25, 25));
    m_bugReportButton->setCursor(Qt::PointingHandCursor);
    connect(m_bugReportButton, &QPushButton::clicked, this, [this]() {
        const QUrl issueUrl(QStringLiteral("https://github.com/qingyiz/vitals_app/issues/new"));
        if (!QDesktopServices::openUrl(issueUrl)) {
            showStatusMessage(text(QStringLiteral("status.bugReportOpenFailed"),
                QStringLiteral("Could not open bug report page")), 3000);
            return;
        }
        showStatusMessage(text(QStringLiteral("status.bugReportOpened"),
            QStringLiteral("Opened bug report page")), 3000);
    });

    m_pluginCenterButton = new QPushButton(container);
    m_pluginCenterButton->setObjectName(QStringLiteral("sidebarActionButton"));
    m_pluginCenterButton->setIcon(createSidebarActionIcon(SidebarActionIcon::Plugins));
    m_pluginCenterButton->setFixedSize(30, 30);
    m_pluginCenterButton->setIconSize(QSize(25, 25));
    m_pluginCenterButton->setCursor(Qt::PointingHandCursor);
    connect(m_pluginCenterButton, &QPushButton::clicked, this, &MainWindow::openPluginCenter);

    m_monitorToggleButton = new QPushButton(container);
    m_monitorToggleButton->setObjectName(QStringLiteral("sidebarActionButton"));
    m_monitorToggleButton->setIcon(createSidebarActionIcon(SidebarActionIcon::Pause));
    m_monitorToggleButton->setFixedSize(30, 30);
    m_monitorToggleButton->setIconSize(QSize(25, 25));
    m_monitorToggleButton->setCursor(Qt::PointingHandCursor);
    connect(m_monitorToggleButton, &QPushButton::clicked, this, &MainWindow::toggleMonitoring);

    m_quitButton = new QPushButton(container);
    m_quitButton->setObjectName(QStringLiteral("sidebarQuitButton"));
    m_quitButton->setIcon(createSidebarActionIcon(SidebarActionIcon::Quit));
    m_quitButton->setFixedSize(30, 30);
    m_quitButton->setIconSize(QSize(25, 25));
    m_quitButton->setCursor(Qt::PointingHandCursor);
    connect(m_quitButton, &QPushButton::clicked, qApp, &QApplication::quit);

    layout->addWidget(m_bugReportButton);
    layout->addWidget(m_pluginCenterButton);
    layout->addWidget(m_monitorToggleButton);
    layout->addWidget(m_quitButton);

    refreshSidebarActions();
    return container;
}

void MainWindow::refreshSidebarActions()
{
    if (m_bugReportButton) {
        const QString bugReportText = text(QStringLiteral("sidebar.reportBug"), QStringLiteral("Report Bug"));
        m_bugReportButton->setText(QString());
        m_bugReportButton->setToolTip(bugReportText);
        m_bugReportButton->setAccessibleName(bugReportText);
    }
    if (m_pluginCenterButton) {
        const QString pluginCenterText = text(QStringLiteral("sidebar.pluginCenter"), QStringLiteral("Plugin Center"));
        m_pluginCenterButton->setText(QString());
        m_pluginCenterButton->setToolTip(pluginCenterText);
        m_pluginCenterButton->setAccessibleName(pluginCenterText);
    }
    if (m_monitorToggleButton) {
        const QString monitorText = m_monitoringPaused
            ? text(QStringLiteral("sidebar.resumeMonitoring"), QStringLiteral("Resume Monitoring"))
            : text(QStringLiteral("sidebar.pauseMonitoring"), QStringLiteral("Pause Monitoring"));
        m_monitorToggleButton->setText(QString());
        m_monitorToggleButton->setToolTip(monitorText);
        m_monitorToggleButton->setAccessibleName(monitorText);
        m_monitorToggleButton->setIcon(createSidebarActionIcon(
            m_monitoringPaused ? SidebarActionIcon::Resume : SidebarActionIcon::Pause));
    }
    if (m_quitButton) {
        const QString quitText = text(QStringLiteral("sidebar.quit"), QStringLiteral("Quit Vitals"));
        m_quitButton->setText(QString());
        m_quitButton->setToolTip(quitText);
        m_quitButton->setAccessibleName(quitText);
    }
}

void MainWindow::refreshTaskbarHostActions()
{
    if (!m_taskbarIndicator) {
        return;
    }

    m_taskbarIndicator->setHostActionTexts(
        text(QStringLiteral("taskbar.showWindow"), QStringLiteral("Show Vitals")),
        text(QStringLiteral("sidebar.quit"), QStringLiteral("Quit Vitals")),
        text(QStringLiteral("taskbar.runningInBackground"), QStringLiteral("Running in background")),
        text(QStringLiteral("taskbar.monitoringDisplayPaused"), QStringLiteral("Monitoring display paused")));
}

void MainWindow::openPluginCenter()
{
    if (m_navigation && m_navigation->setCurrentItemById(QStringLiteral("plugins"))) {
        showStatusMessage(text(QStringLiteral("status.pluginCenterOpened"),
            QStringLiteral("Opened plugin center")), 3000);
        return;
    }

    showStatusMessage(text(QStringLiteral("status.pluginCenterUnavailable"),
        QStringLiteral("Plugin center is unavailable")), 3000);
}

void MainWindow::toggleMonitoring()
{
    if (m_monitoringPaused) {
        m_pluginManager->startAll();
        m_monitoringPaused = false;
        m_taskbarIndicator->setDisplaySuppressed(false);
        syncTaskbarDisplays();
        refreshSidebarActions();
        showStatusMessage(text(QStringLiteral("status.monitoringResumed"),
            QStringLiteral("Monitoring resumed")), 3000);
        return;
    }

    m_pluginManager->stopAll();
    m_monitoringPaused = true;
    m_taskbarIndicator->setDisplaySuppressed(true);
    refreshSidebarActions();
    showStatusMessage(text(QStringLiteral("status.monitoringPaused"),
        QStringLiteral("Monitoring paused")), 3000);
}

void MainWindow::showStatusMessage(const QString& message, int timeoutMs)
{
    if (!m_statusMessageLabel) {
        return;
    }

    ++m_statusMessageSerial;
    const int serial = m_statusMessageSerial;
    m_statusMessageLabel->setText(message);

    if (timeoutMs <= 0) {
        return;
    }

    QTimer::singleShot(timeoutMs, this, [this, serial]() {
        if (serial != m_statusMessageSerial || !m_statusMessageLabel) {
            return;
        }
        m_statusMessageLabel->setText(text(QStringLiteral("status.frameworkReady"), QStringLiteral("Framework ready")));
    });
}

void MainWindow::refreshLanguageSelector()
{
    if (!m_languageCombo) {
        return;
    }

    const QSignalBlocker blocker(m_languageCombo);
    m_languageCombo->clear();
    int currentIndex = -1;
    const QList<LanguageManager::Language> languages = m_languageManager->availableLanguages();
    for (const LanguageManager::Language& language : languages) {
        const QString label = language.nativeName.isEmpty() ? language.name : language.nativeName;
        m_languageCombo->addItem(label, language.code);
        if (language.code == m_languageManager->currentLanguage()) {
            currentIndex = m_languageCombo->count() - 1;
        }
    }
    if (currentIndex >= 0) {
        m_languageCombo->setCurrentIndex(currentIndex);
    }
}

void MainWindow::applyWindowPresencePolicy()
{
    if (!m_taskbarIndicator || !m_taskbarIndicator->supportsDockIconVisibility()) {
        return;
    }

    const bool foregroundWindowVisible = isVisible() && !isHidden();
    m_taskbarIndicator->setDockIconVisible(
        foregroundWindowVisible ? true : m_configManager->showDockIcon());
}

QIcon MainWindow::createNavigationIcon(const QString& iconKey) const
{
    const NavigationIconSpec spec = iconSpecForKey(iconKey);

    QPixmap pixmap(36, 36);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(spec.background);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(QRectF(3, 3, 30, 30), 9, 9);

    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(16);
    painter.setFont(font);
    painter.setPen(spec.foreground);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, spec.glyph.left(1));

    return QIcon(pixmap);
}

QString MainWindow::text(const QString& key, const QString& fallback) const
{
    return m_languageManager ? m_languageManager->translate(key, fallback) : fallback;
}

QString MainWindow::navigationTitleForPage(const QString& id, const QString& title) const
{
    if (id == QStringLiteral("cpu")) {
        return text(QStringLiteral("nav.cpu"), QStringLiteral("CPU"));
    }
    if (id == QStringLiteral("memory")) {
        return text(QStringLiteral("nav.memory"), QStringLiteral("Memory"));
    }
    if (id == QStringLiteral("network")) {
        return text(QStringLiteral("nav.network"), QStringLiteral("Network"));
    }
    if (id == QStringLiteral("disk")) {
        return text(QStringLiteral("nav.disk"), QStringLiteral("Disk"));
    }
    if (id == QStringLiteral("systeminfo")) {
        return text(QStringLiteral("nav.systemInfo"), QStringLiteral("System Info"));
    }
    return title;
}

QWidget* MainWindow::createPluginManagerPage()
{
    m_pluginCenterPage = new PluginCenterWidget(m_configManager, m_languageManager, this);
    connect(m_pluginCenterPage, &PluginCenterWidget::pluginEnabledChanged, this,
        [this](const QString& pluginId, const QString& filePath, bool enabled) {
            m_configManager->setPluginEnabled(pluginId, filePath, enabled);
            reloadPlugins();
        });
    connect(m_pluginCenterPage, &PluginCenterWidget::pluginTaskbarVisibilityChanged, this,
        [this](const QString& pluginId, const QString& filePath, bool enabled) {
            m_configManager->setPluginTaskbarEnabled(pluginId, filePath, enabled);
            syncTaskbarDisplays();
            showStatusMessage(text(QStringLiteral("status.taskbarVisibilityUpdated"),
                QStringLiteral("Updated menu bar visibility for %1")).arg(pluginId), 3000);
        });
    return m_pluginCenterPage;
}

QWidget* MainWindow::createPluginPanelPage(IPlugin* plugin, QWidget* panel)
{
    if (!plugin || !panel) {
        return panel;
    }

    ITaskbarCapability* taskbarCapability = plugin->taskbarCapability();
    if (!taskbarCapability || taskbarCapability->taskbarLabelDescriptors().isEmpty()) {
        return panel;
    }

    auto* page = new QWidget(this);
    page->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* taskbarSettings = new TaskbarDisplaySettingsWidget(
        plugin->metaInfo().id,
        taskbarCapability,
        m_configManager,
        m_languageManager,
        page);
    connect(taskbarSettings, &TaskbarDisplaySettingsWidget::settingsChanged,
        this, &MainWindow::syncTaskbarDisplays);

    panel->setParent(page);
    layout->addWidget(taskbarSettings);
    layout->addWidget(panel, 1);
    return page;
}

void MainWindow::rebuildPages(const QString& preferredPageId)
{
    m_navigation->clear();

    while (m_pages->count() > 0) {
        QWidget* widget = m_pages->widget(0);
        m_pages->removeWidget(widget);
        widget->deleteLater();
    }

    auto* dashboard = new DashboardWidget(m_languageManager, this);
    dashboard->bindMetricCenter(m_metricCenter);
    addPage(QStringLiteral("dashboard"), text(QStringLiteral("nav.dashboard"), QStringLiteral("Dashboard")), dashboard,
        createNavigationIcon(QStringLiteral("dashboard")));

    addPage(QStringLiteral("plugins"), text(QStringLiteral("nav.plugins"), QStringLiteral("Plugins")), createPluginManagerPage(),
        createNavigationIcon(QStringLiteral("plugins")));
    if (m_pluginCenterPage) {
        m_pluginCenterPage->setPluginInfos(
            m_pluginManager->pluginInfos(),
            taskbarDefaultEnabledByFilePath(m_pluginManager));
    }

    for (IPlugin* plugin : m_pluginManager->plugins()) {
        IPanelCapability* panelCapability = plugin ? plugin->panelCapability() : nullptr;
        auto* panelPlugin = panelCapability ? nullptr : dynamic_cast<IPanelPlugin*>(plugin);
        if (!panelCapability && !panelPlugin) {
            continue;
        }
        const QString panelId = panelCapability ? panelCapability->panelId() : panelPlugin->panelId();
        const QString panelName = panelCapability ? panelCapability->panelName() : panelPlugin->panelName();
        const QIcon icon = panelCapability ? panelCapability->panelIcon() : panelPlugin->panelIcon();
        const QString iconKey = panelCapability ? panelCapability->panelIconKey() : panelPlugin->panelIconKey();
        QWidget* page = panelCapability ? panelCapability->createPanel(this) : panelPlugin->createPanel(this);
        addPage(panelId, navigationTitleForPage(panelId, panelName), createPluginPanelPage(plugin, page),
            icon.isNull() ? createNavigationIcon(iconKey) : icon);
    }

    if (!preferredPageId.isEmpty() && m_navigation->setCurrentItemById(preferredPageId)) {
        return;
    }

    if (m_navigation->setCurrentItemById(QStringLiteral("plugins"))) {
        return;
    }

    if (m_navigation->count() > 0) {
        m_navigation->setCurrentRow(0);
    }
}

void MainWindow::applyStyle()
{
    qApp->setStyleSheet(QStringLiteral(R"(
        QWidget {
            font-family: -apple-system, BlinkMacSystemFont, "SF Pro Text", "Segoe UI", sans-serif;
            color: #1d1d1f;
        }
        QMainWindow, QStackedWidget {
            background: #f5f5f7;
        }
        QWidget#sidebar {
            background: #e9e9ed;
            border-right: 1px solid #d5d5da;
        }
        QWidget#contentShell {
            background: #f5f5f7;
        }
        QListWidget#navigation {
            background: transparent;
            color: #34343a;
            border: none;
            padding: 12px 10px;
            font-size: 13px;
        }
        QListWidget#navigation::item {
            border-radius: 6px;
            padding: 7px 9px;
            margin: 1px 0;
        }
        QListWidget#navigation::item:selected {
            background: rgba(10, 132, 255, 0.14);
            color: #0057c2;
            font-weight: 600;
        }
        QWidget#sidebarActions {
            background: transparent;
        }
        QPushButton#sidebarActionButton,
        QPushButton#sidebarQuitButton {
            background: transparent;
            border: none;
            border-radius: 16px;
            padding: 0;
            color: #5f6368;
        }
        QPushButton#sidebarActionButton:hover {
            background: rgba(60, 60, 67, 0.08);
        }
        QPushButton#sidebarQuitButton:hover {
            background: rgba(255, 69, 58, 0.14);
        }
        QPushButton#sidebarActionButton:pressed,
        QPushButton#sidebarQuitButton:pressed {
            background: rgba(60, 60, 67, 0.14);
        }
        QWidget#contentStatusBar {
            background: #f5f5f7;
            border-top: 1px solid #dedee3;
        }
        QLabel#contentStatusMessage {
            color: #6e6e73;
            font-size: 12px;
        }
        QLabel#languageSelectorLabel {
            color: #6e6e73;
            font-size: 12px;
            font-weight: 700;
        }
        QLabel#pageTitle {
            color: #1d252d;
            font-size: 22px;
            font-weight: 700;
        }
        QLabel#pageSubtitle {
            color: #6e6e73;
            font-size: 13px;
            line-height: 1.35;
        }
        QLabel#statusPill {
            background: rgba(50, 215, 75, 0.14);
            color: #248a3d;
            border: 1px solid rgba(50, 215, 75, 0.28);
            border-radius: 9px;
            padding: 3px 9px;
            font-size: 11px;
            font-weight: 600;
        }
        QFrame#metricCard {
            background: rgba(255, 255, 255, 0.92);
            border: 1px solid #dedee3;
            border-radius: 8px;
        }
        QLabel#cardTitle {
            color: #6e6e73;
            font-size: 11px;
            font-weight: 600;
        }
        QLabel#cardValue {
            color: #1d1d1f;
            font-size: 21px;
            font-weight: 700;
        }
        QLabel#cardHint {
            color: #86868b;
            font-size: 12px;
        }
        QFrame#emptyPanel {
            background: rgba(255, 255, 255, 0.86);
            border: 1px solid #dedee3;
            border-radius: 8px;
        }
        QFrame#systemOverviewPanel {
            background: transparent;
            border: none;
        }
        QFrame#systemHeroPanel {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(255,255,255,0.98),
                stop:1 rgba(246,247,250,0.98));
            border: 1px solid #d8dbe1;
            border-radius: 12px;
        }
        QLabel#systemHeroEyebrow {
            color: #8e8e93;
            font-size: 11px;
            font-weight: 700;
            letter-spacing: 1px;
        }
        QLabel#systemHeroTitle {
            color: #101114;
            font-size: 26px;
            font-weight: 800;
        }
        QLabel#systemHeroSubtitle {
            color: #5b6068;
            font-size: 13px;
        }
        QLabel#systemHeroMeta {
            color: #7a7f87;
            font-size: 12px;
            font-weight: 600;
        }
        QFrame#systemBadge {
            background: rgba(248,249,251,0.92);
            border: 1px solid #dde0e6;
            border-radius: 9px;
        }
        QLabel#systemBadgeLabel {
            color: #8e8e93;
            font-size: 10px;
            font-weight: 700;
            letter-spacing: 1px;
        }
        QLabel#systemBadgeValue {
            color: #1d1d1f;
            font-size: 15px;
            font-weight: 700;
        }
        QFrame#systemDetailsPanel {
            background: rgba(255, 255, 255, 0.90);
            border: 1px solid #dde0e6;
            border-radius: 12px;
        }
        QLabel#systemSectionTitle {
            color: #1d1d1f;
            font-size: 15px;
            font-weight: 700;
        }
        QLabel#systemInfoKeyLabel {
            color: #7a7f87;
            font-size: 11px;
            font-weight: 600;
            min-width: 56px;
        }
        QLabel#systemInfoValueLabel {
            color: #1d1d1f;
            font-size: 12px;
            font-weight: 600;
        }
        QFrame#systemInfoTile {
            background: rgba(255, 255, 255, 0.94);
            border: 1px solid #dde0e6;
            border-radius: 10px;
        }
        QLabel#systemInfoTileEyebrow {
            color: #8e8e93;
            font-size: 10px;
            font-weight: 700;
        }
        QLabel#systemInfoTileValue {
            color: #1d1d1f;
            font-size: 15px;
            font-weight: 700;
        }
        QLabel#panelTitle {
            color: #1d1d1f;
            font-size: 15px;
            font-weight: 700;
        }
        QLabel#panelBody {
            color: #6e6e73;
            font-size: 13px;
        }
        QFrame#taskbarSettingsPanel {
            background: rgba(255, 255, 255, 0.72);
            border-bottom: 1px solid rgba(210, 213, 219, 0.85);
        }
        QLabel#taskbarSettingsTitle {
            color: #1d1d1f;
            font-size: 13px;
            font-weight: 700;
        }
        QLabel#taskbarSettingsHint {
            color: #7a7f87;
            font-size: 12px;
        }
        QLabel#taskbarSettingsFieldLabel {
            color: #6e6e73;
            font-size: 12px;
            font-weight: 600;
        }
        QLineEdit#taskbarSettingsLineEdit {
            background: rgba(255, 255, 255, 0.95);
            color: #1d1d1f;
            border: 1px solid #cfd3da;
            border-radius: 6px;
            padding: 5px 8px;
            min-height: 20px;
            font-size: 12px;
        }
        QLineEdit#taskbarSettingsLineEdit:focus {
            border: 1px solid #0a84ff;
            background: #ffffff;
        }
    )"));
}

} // namespace Vitals
