#pragma once

#include <QIcon>
#include <QMainWindow>

class QStackedWidget;

namespace Vitals {

class AppContext;
class ConfigManager;
class DashboardWidget;
class MetricCenter;
class NavigationWidget;
class PluginCenterWidget;
class PluginManager;
class TaskbarIndicator;

/**
 * \if ENGLISH
 * @brief Main host window for the Vitals desktop application
 *
 * Owns the primary navigation shell, dashboard container, plugin panel pages,
 * plugin runtime services, and platform-level taskbar integration. It does not
 * implement concrete monitoring logic itself.
 * \endif
 *
 * \if CHINESE
 * @brief Vitals 桌面应用的主宿主窗口
 *
 * 负责主导航框架、Dashboard 容器、插件面板页面、插件运行时服务以及宿主
 * 级任务栏集成，但自身不实现具体的系统监控采集逻辑。
 * \endif
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    /// Builds the main shell layout and default host pages.
    void setupUi();

    /// Loads plugins from the runtime plugin directory and mounts panel pages.
    void loadPlugins();

    /// Reloads plugin runtime state after enable/disable changes.
    void reloadPlugins();

    /// Clears cached metrics that belong to the currently loaded plugins.
    void clearLoadedPluginMetrics();

    /// Rebuilds the current set of taskbar display providers from loaded plugins.
    void syncTaskbarDisplays();

    /// Adds a page to both the navigation list and stacked content area.
    void addPage(const QString& id, const QString& title, QWidget* page, const QIcon& icon = QIcon());

    /// Creates the host-owned plugin management placeholder page.
    QWidget* createPluginManagerPage();

    /// Applies the current host visual theme and widget stylesheet.
    void applyStyle();

    /// Creates a consistent host-owned navigation icon from a semantic key.
    QIcon createNavigationIcon(const QString& iconKey) const;

    /// Removes all pages and rebuilds the host and plugin navigation entries.
    void rebuildPages(const QString& preferredPageId = QString());

    MetricCenter* m_metricCenter = nullptr;
    ConfigManager* m_configManager = nullptr;
    AppContext* m_appContext = nullptr;
    PluginManager* m_pluginManager = nullptr;
    TaskbarIndicator* m_taskbarIndicator = nullptr;
    PluginCenterWidget* m_pluginCenterPage = nullptr;
    NavigationWidget* m_navigation = nullptr;
    QStackedWidget* m_pages = nullptr;
};

} // namespace Vitals
