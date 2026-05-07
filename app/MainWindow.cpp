#include "MainWindow.h"

#include "AppContext.h"
#include "DashboardWidget.h"
#include "IPanelPlugin.h"
#include "ITaskbarDisplayPlugin.h"
#include "NavigationWidget.h"
#include "PluginCenterWidget.h"
#include "config/ConfigManager.h"
#include "metric/MetricCenter.h"
#include "platform/taskbar/TaskbarIndicator.h"
#include "platform/taskbar/TaskbarIndicatorFactory.h"
#include "plugin/PluginManager.h"

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QStackedWidget>
#include <QStatusBar>
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
    if (key == QStringLiteral("hello")) {
        return {QStringLiteral("H"), QColor(QStringLiteral("#ff9f0a")), Qt::white};
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

} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_metricCenter(new MetricCenter(this))
    , m_configManager(new ConfigManager())
    , m_appContext(new AppContext(m_metricCenter, m_configManager))
    , m_pluginManager(new PluginManager(this))
    , m_taskbarIndicator(createTaskbarIndicator(this))
{
    setupUi();
    m_taskbarIndicator->initialize(this);
    m_taskbarIndicator->bindMetricCenter(m_metricCenter);
    connect(m_taskbarIndicator, &TaskbarIndicator::showRequested, this, [this]() {
        showNormal();
        raise();
        activateWindow();
    });
    connect(m_taskbarIndicator, &TaskbarIndicator::quitRequested,
        qApp, &QApplication::quit);
    loadPlugins();
    rebuildPages();
    m_pluginManager->startAll();
}

MainWindow::~MainWindow()
{
    m_pluginManager->shutdownAll();
    delete m_appContext;
    delete m_configManager;
}

void MainWindow::setupUi()
{
    setWindowTitle(QStringLiteral("Vitals"));
    applyStyle();

    auto* central = new QWidget(this);
    auto* layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_navigation = new NavigationWidget(central);
    m_pages = new QStackedWidget(central);

    layout->addWidget(m_navigation);
    layout->addWidget(m_pages, 1);
    setCentralWidget(central);

    connect(m_navigation, &QListWidget::currentRowChanged,
        m_pages, &QStackedWidget::setCurrentIndex);
    rebuildPages();

    statusBar()->showMessage(QStringLiteral("Framework ready"));
}

void MainWindow::loadPlugins()
{
    connect(m_pluginManager, &PluginManager::pluginLoaded, this,
        [this](const QString& pluginId, const QString& pluginName) {
            statusBar()->showMessage(QStringLiteral("Loaded plugin: %1").arg(pluginName), 3000);
            Q_UNUSED(pluginId)
        }, Qt::UniqueConnection);
    connect(m_pluginManager, &PluginManager::pluginSkipped, this,
        [this](const QString& path, const QString& reason) {
            statusBar()->showMessage(QStringLiteral("Skipped plugin: %1").arg(reason), 3000);
            Q_UNUSED(path)
        }, Qt::UniqueConnection);

    const QString pluginsDir = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("plugins"));
    m_pluginManager->loadAllPlugins(pluginsDir, m_appContext, m_configManager);
    syncTaskbarDisplays();
    if (m_pluginCenterPage) {
        m_pluginCenterPage->setPluginInfos(m_pluginManager->pluginInfos());
    }

    statusBar()->showMessage(QStringLiteral("Loaded %1 plugin(s)").arg(m_pluginManager->loadedPluginCount()));
}

void MainWindow::reloadPlugins()
{
    const QString preferredPageId = m_navigation ? m_navigation->currentItemId() : QString();

    m_pluginManager->stopAll();
    clearLoadedPluginMetrics();
    m_pluginManager->shutdownAll();
    loadPlugins();
    rebuildPages(preferredPageId);
    m_pluginManager->startAll();
    statusBar()->showMessage(QStringLiteral("Plugin enable state updated"), 3000);
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
        auto* provider = dynamic_cast<ITaskbarDisplayPlugin*>(plugin);
        if (!provider) {
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
                meta.id, filePath, provider->isTaskbarDisplayEnabledByDefault())) {
            continue;
        }

        displays.append({
            meta.id,
            meta.name.isEmpty() ? meta.id : meta.name,
            filePath,
            provider
        });
    }

    m_taskbarIndicator->setPluginDisplays(displays);
}

void MainWindow::addPage(const QString& id, const QString& title, QWidget* page, const QIcon& icon)
{
    m_navigation->addNavigationItem(id, title, icon);
    m_pages->addWidget(page);
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

QWidget* MainWindow::createPluginManagerPage()
{
    m_pluginCenterPage = new PluginCenterWidget(m_configManager, this);
    connect(m_pluginCenterPage, &PluginCenterWidget::pluginEnabledChanged, this,
        [this](const QString& pluginId, const QString& filePath, bool enabled) {
            m_configManager->setPluginEnabled(pluginId, filePath, enabled);
            reloadPlugins();
        });
    connect(m_pluginCenterPage, &PluginCenterWidget::pluginTaskbarVisibilityChanged, this,
        [this](const QString& pluginId, const QString& filePath, bool enabled) {
            m_configManager->setPluginTaskbarEnabled(pluginId, filePath, enabled);
            syncTaskbarDisplays();
            statusBar()->showMessage(QStringLiteral("Updated menu bar visibility for %1").arg(pluginId), 3000);
        });
    return m_pluginCenterPage;
}

void MainWindow::rebuildPages(const QString& preferredPageId)
{
    m_navigation->clear();

    while (m_pages->count() > 0) {
        QWidget* widget = m_pages->widget(0);
        m_pages->removeWidget(widget);
        widget->deleteLater();
    }

    auto* dashboard = new DashboardWidget(this);
    dashboard->bindMetricCenter(m_metricCenter);
    addPage(QStringLiteral("dashboard"), QStringLiteral("Dashboard"), dashboard,
        createNavigationIcon(QStringLiteral("dashboard")));

    addPage(QStringLiteral("plugins"), QStringLiteral("Plugins"), createPluginManagerPage(),
        createNavigationIcon(QStringLiteral("plugins")));
    if (m_pluginCenterPage) {
        m_pluginCenterPage->setPluginInfos(m_pluginManager->pluginInfos());
    }

    for (IPlugin* plugin : m_pluginManager->plugins()) {
        auto* panelPlugin = dynamic_cast<IPanelPlugin*>(plugin);
        if (!panelPlugin) {
            continue;
        }
        addPage(panelPlugin->panelId(), panelPlugin->panelName(), panelPlugin->createPanel(this),
            panelPlugin->panelIcon().isNull()
                ? createNavigationIcon(panelPlugin->panelIconKey())
                : panelPlugin->panelIcon());
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
        QListWidget#navigation {
            background: #ececf1;
            color: #34343a;
            border: none;
            padding: 16px 12px;
            font-size: 14px;
        }
        QListWidget#navigation::item {
            border-radius: 6px;
            padding: 9px 11px;
            margin: 1px 0;
        }
        QListWidget#navigation::item:selected {
            background: rgba(10, 132, 255, 0.14);
            color: #0057c2;
            font-weight: 600;
        }
        QLabel#pageTitle {
            color: #1d252d;
            font-size: 26px;
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
            border-radius: 10px;
            padding: 4px 10px;
            font-size: 12px;
            font-weight: 600;
        }
        QFrame#metricCard {
            background: rgba(255, 255, 255, 0.92);
            border: 1px solid #dedee3;
            border-radius: 8px;
        }
        QLabel#cardTitle {
            color: #6e6e73;
            font-size: 12px;
            font-weight: 600;
        }
        QLabel#cardValue {
            color: #1d1d1f;
            font-size: 25px;
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
            border-radius: 18px;
        }
        QLabel#systemHeroEyebrow {
            color: #8e8e93;
            font-size: 11px;
            font-weight: 700;
            letter-spacing: 1px;
        }
        QLabel#systemHeroTitle {
            color: #101114;
            font-size: 32px;
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
            border-radius: 12px;
        }
        QLabel#systemBadgeLabel {
            color: #8e8e93;
            font-size: 10px;
            font-weight: 700;
            letter-spacing: 1px;
        }
        QLabel#systemBadgeValue {
            color: #1d1d1f;
            font-size: 17px;
            font-weight: 700;
        }
        QFrame#systemDetailsPanel {
            background: rgba(255, 255, 255, 0.90);
            border: 1px solid #dde0e6;
            border-radius: 18px;
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
            border-radius: 14px;
        }
        QLabel#systemInfoTileEyebrow {
            color: #8e8e93;
            font-size: 10px;
            font-weight: 700;
        }
        QLabel#systemInfoTileValue {
            color: #1d1d1f;
            font-size: 16px;
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
        QStatusBar {
            background: #f5f5f7;
            color: #6e6e73;
        }
    )"));
}

} // namespace Vitals
