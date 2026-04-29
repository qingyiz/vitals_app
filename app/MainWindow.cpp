#include "MainWindow.h"

#include "AppContext.h"
#include "DashboardWidget.h"
#include "IPanelPlugin.h"
#include "NavigationWidget.h"
#include "config/ConfigManager.h"
#include "metric/MetricCenter.h"
#include "platform/taskbar/TaskbarIndicator.h"
#include "platform/taskbar/TaskbarIndicatorFactory.h"
#include "plugin/PluginManager.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QStatusBar>
#include <QVBoxLayout>

namespace Vitals {

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

    auto* dashboard = new DashboardWidget(this);
    dashboard->bindMetricCenter(m_metricCenter);
    addPage(QStringLiteral("dashboard"), QStringLiteral("Dashboard"), dashboard);

    addPage(QStringLiteral("plugins"), QStringLiteral("Plugins"), createPluginManagerPage());

    connect(m_navigation, &QListWidget::currentRowChanged,
        m_pages, &QStackedWidget::setCurrentIndex);
    m_navigation->setCurrentRow(0);

    statusBar()->showMessage(QStringLiteral("Framework ready"));
}

void MainWindow::loadPlugins()
{
    connect(m_pluginManager, &PluginManager::pluginLoaded, this,
        [this](const QString& pluginId, const QString& pluginName) {
            statusBar()->showMessage(QStringLiteral("Loaded plugin: %1").arg(pluginName), 3000);
            Q_UNUSED(pluginId)
        });
    connect(m_pluginManager, &PluginManager::pluginSkipped, this,
        [this](const QString& path, const QString& reason) {
            statusBar()->showMessage(QStringLiteral("Skipped plugin: %1").arg(reason), 3000);
            Q_UNUSED(path)
        });

    const QString pluginsDir = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("plugins"));
    m_pluginManager->loadAllPlugins(pluginsDir, m_appContext);

    for (IPlugin* plugin : m_pluginManager->plugins()) {
        auto* panelPlugin = dynamic_cast<IPanelPlugin*>(plugin);
        if (!panelPlugin) {
            continue;
        }
        addPage(panelPlugin->panelId(), panelPlugin->panelName(), panelPlugin->createPanel(this));
    }

    statusBar()->showMessage(QStringLiteral("Loaded %1 plugin(s)").arg(m_pluginManager->loadedPluginCount()));
}

void MainWindow::addPage(const QString& id, const QString& title, QWidget* page)
{
    m_navigation->addNavigationItem(id, title);
    m_pages->addWidget(page);
}

QWidget* MainWindow::createPluginManagerPage()
{
    auto* page = new QWidget(this);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 24, 28, 28);
    layout->setSpacing(14);

    auto* title = new QLabel(QStringLiteral("Plugins"), page);
    title->setObjectName(QStringLiteral("pageTitle"));
    auto* summary = new QLabel(QStringLiteral("Loaded plugins, platform compatibility, and runtime status will be managed here."), page);
    summary->setObjectName(QStringLiteral("pageSubtitle"));
    summary->setWordWrap(true);

    auto* panel = new QFrame(page);
    panel->setObjectName(QStringLiteral("emptyPanel"));
    auto* panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(18, 16, 18, 16);
    panelLayout->setSpacing(6);
    auto* panelTitle = new QLabel(QStringLiteral("Plugin Center"), panel);
    panelTitle->setObjectName(QStringLiteral("panelTitle"));
    auto* panelBody = new QLabel(QStringLiteral("The next milestone will list loaded, skipped, and failed plugins with their declared supportedPlatforms."), panel);
    panelBody->setObjectName(QStringLiteral("panelBody"));
    panelBody->setWordWrap(true);
    panelLayout->addWidget(panelTitle);
    panelLayout->addWidget(panelBody);

    layout->addWidget(title);
    layout->addWidget(summary);
    layout->addWidget(panel);
    layout->addStretch();
    return page;
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
