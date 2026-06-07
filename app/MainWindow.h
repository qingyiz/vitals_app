#pragma once

#include <QIcon>
#include <QMainWindow>

class QCloseEvent;
class QStackedWidget;
class QComboBox;
class QLabel;
class QPushButton;
class QWidget;

namespace Vitals {

class AppContext;
class ConfigManager;
class DashboardWidget;
class IPlugin;
class LanguageManager;
class MetricCenter;
class NavigationWidget;
class PluginCenterWidget;
class PluginManager;
class TaskbarIndicator;
class ToggleSwitch;

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

protected:
    void closeEvent(QCloseEvent* event) override;

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

    /// Wraps a plugin panel with host-owned generic controls declared by capabilities.
    QWidget* createPluginPanelPage(IPlugin* plugin, QWidget* panel);

    /// Applies the current host visual theme and widget stylesheet.
    void applyStyle();

    /// Builds the content-owned status bar that does not occupy the sidebar.
    QWidget* createContentStatusBar(QWidget* parent);

    /// Builds and wires the language selector for the content status bar.
    void setupLanguageSelector(QWidget* parent);

    /// Builds the macOS-only Dock visibility switch when the platform supports it.
    void setupDockIconSelector(QWidget* parent);

    /// Builds the compact host action buttons at the bottom of the sidebar.
    QWidget* createSidebarActions(QWidget* parent);

    /// Refreshes sidebar action labels and tooltips after language changes.
    void refreshSidebarActions();

    /// Refreshes localized host actions exposed by tray/menu-bar entry points.
    void refreshTaskbarHostActions();

    /// Opens the host plugin center page from the compact sidebar action.
    void openPluginCenter();

    /// Toggles host-level monitoring dispatch without changing plugin enable state.
    void toggleMonitoring();

    /// Updates the language selector to match the active catalog.
    void refreshLanguageSelector();

    /// Applies the persisted platform-level window presence policy.
    void applyWindowPresencePolicy();

    /// Updates the content status message with optional timeout restoration.
    void showStatusMessage(const QString& message, int timeoutMs = 0);

    /// Creates a consistent host-owned navigation icon from a semantic key.
    QIcon createNavigationIcon(const QString& iconKey) const;

    /// Removes all pages and rebuilds the host and plugin navigation entries.
    void rebuildPages(const QString& preferredPageId = QString());

    QString text(const QString& key, const QString& fallback) const;
    QString navigationTitleForPage(const QString& id, const QString& title) const;

    MetricCenter* m_metricCenter = nullptr;
    ConfigManager* m_configManager = nullptr;
    LanguageManager* m_languageManager = nullptr;
    AppContext* m_appContext = nullptr;
    PluginManager* m_pluginManager = nullptr;
    TaskbarIndicator* m_taskbarIndicator = nullptr;
    PluginCenterWidget* m_pluginCenterPage = nullptr;
    NavigationWidget* m_navigation = nullptr;
    QStackedWidget* m_pages = nullptr;
    QLabel* m_statusMessageLabel = nullptr;
    QLabel* m_languageLabel = nullptr;
    QComboBox* m_languageCombo = nullptr;
    QLabel* m_dockIconLabel = nullptr;
    ToggleSwitch* m_dockIconSwitch = nullptr;
    QPushButton* m_bugReportButton = nullptr;
    QPushButton* m_pluginCenterButton = nullptr;
    QPushButton* m_monitorToggleButton = nullptr;
    QPushButton* m_quitButton = nullptr;
    int m_statusMessageSerial = 0;
    bool m_monitoringPaused = false;
};

} // namespace Vitals
