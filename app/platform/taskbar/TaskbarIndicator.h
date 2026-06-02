#pragma once

#include "MetricData.h"

#include "ITaskbarCapability.h"
#include "ITaskbarDetailPlugin.h"
#include "ITaskbarDisplayPlugin.h"

#include <QColor>
#include <QHash>
#include <QList>
#include <QObject>
#include <QRect>
#include <QStringList>

class QAction;
class QMenu;
class QPixmap;
class QSystemTrayIcon;
class QWidgetAction;
class QWidget;

namespace Vitals {

class MetricCenter;
class TaskbarMenuDetailWidget;

/**
 * \if ENGLISH
 * @brief One loaded plugin contribution that may appear in the taskbar surface
 * \endif
 *
 * \if CHINESE
 * @brief 一个可能出现在任务栏表面中的已加载插件显示贡献
 * \endif
 */
struct TaskbarPluginDisplay
{
    QString pluginId;
    QString pluginName;
    QString filePath;
    ITaskbarCapability* capability = nullptr;
    ITaskbarDisplayPlugin* provider = nullptr;
    ITaskbarDetailPlugin* detailProvider = nullptr;
};

/**
 * \if ENGLISH
 * @brief Host-side taskbar, tray, or menu-bar indicator abstraction
 *
 * Provides a unified bridge between MetricCenter and platform-specific host UI
 * surfaces such as Windows tray icons, macOS menu bar extras, or Linux tray
 * integrations. It does not collect system data itself; it only renders host
 * summaries derived from the shared metric model.
 * \endif
 *
 * \if CHINESE
 * @brief 宿主侧任务栏、托盘或菜单栏指示器抽象
 *
 * 该类在 MetricCenter 与平台相关的宿主 UI 表现层之间提供统一桥接，例如
 * Windows 托盘图标、macOS 菜单栏状态项或 Linux 托盘集成。它自身不采集
 * 系统数据，只负责渲染来自统一 Metric 模型的宿主摘要信息。
 * \endif
 */
class TaskbarIndicator : public QObject
{
    Q_OBJECT

public:
    explicit TaskbarIndicator(QObject* parent = nullptr);
    ~TaskbarIndicator() override;

    /**
     * \if ENGLISH
     * @brief Creates tray resources and binds host-level user actions
     * \endif
     *
     * \if CHINESE
     * @brief 创建托盘资源并绑定宿主级交互动作
     * \endif
     */
    virtual void initialize(QWidget* mainWindow);

    /**
     * \if ENGLISH
     * @brief Subscribes to MetricCenter updates for indicator refresh
     * \endif
     *
     * \if CHINESE
     * @brief 订阅 MetricCenter 更新以刷新指示器显示
     * \endif
     */
    void bindMetricCenter(MetricCenter* metricCenter);

    /**
     * \if ENGLISH
     * @brief Replaces the current set of plugin-provided taskbar display sources
     * \endif
     *
     * \if CHINESE
     * @brief 替换当前参与任务栏显示的插件来源集合
     * \endif
     */
    void setPluginDisplays(const QList<TaskbarPluginDisplay>& pluginDisplays);

    /**
     * \if ENGLISH
     * @brief Temporarily hides taskbar rendering without discarding display sources
     *
     * Used for host-level monitoring pause so platform items can keep their
     * native identity and position, especially macOS menu-bar status items.
     * \endif
     *
     * \if CHINESE
     * @brief 临时隐藏任务栏渲染，但不丢弃显示来源
     *
     * 用于宿主级监控暂停，让平台原生项保留自身身份和位置，尤其是 macOS
     * 菜单栏状态项。
     * \endif
     */
    void setDisplaySuppressed(bool suppressed);

    /**
     * \if ENGLISH
     * @brief Returns whether this platform indicator can reopen the hidden main window
     * \endif
     *
     * \if CHINESE
     * @brief 返回该平台指示器是否可用于重新打开隐藏后的主窗口
     * \endif
     */
    bool isAvailable() const;

    /// Returns whether the platform supports toggling the application Dock icon.
    virtual bool supportsDockIconVisibility() const;

    /// Applies the platform Dock icon visibility policy when supported.
    virtual void setDockIconVisible(bool visible);

    /**
     * \if ENGLISH
     * @brief Updates localized host action text used by tray/menu-bar entries
     * \endif
     *
     * \if CHINESE
     * @brief 更新托盘 / 菜单栏入口使用的宿主动作本地化文案
     * \endif
     */
    void setHostActionTexts(
        const QString& showWindowText,
        const QString& quitText,
        const QString& runningText,
        const QString& pausedText);

    /// Emits the host-level show request.
    void emitShowRequested();

    /// Emits the host-level quit request.
    void emitQuitRequested();

Q_SIGNALS:
    void showRequested();
    void quitRequested();

protected:
    /// Returns a human-readable platform label for tooltips and diagnostics.
    virtual QString platformName() const = 0;

    /// Returns the compact label shown before live metrics become available.
    virtual QString idleText() const;

    /// Returns the short text rendered inside the tray icon.
    virtual QString iconLabel(const QHash<QString, MetricValue>& latestValues) const;

    /// Returns the multi-line tooltip summarizing the latest metrics.
    virtual QString tooltipText(const QHash<QString, MetricValue>& latestValues) const;

    /// Returns the accent color used when rendering the dynamic icon.
    virtual QColor accentColor() const;

    /// Returns whether the platform prefers plain text rendering without a filled badge.
    virtual bool prefersTextOnlyDisplay() const;

    /// Returns whether the platform expects monochrome text for system tinting.
    virtual bool prefersSystemTintedText() const;

    /// Returns the maximum number of characters that should be rendered visibly.
    virtual int maximumVisibleLabelLength() const;

    /// Returns whether plugin text should be rendered into the tray icon itself.
    virtual bool usesDynamicTrayIcon() const;

    /// Returns whether the system tray icon should own the detail context menu.
    virtual bool usesTrayContextMenu() const;

    /// Returns the host window associated with this indicator.
    QWidget* mainWindow() const;

    QString showWindowText() const;
    QString quitText() const;
    QString runningText() const;
    QString pausedText() const;

    /// Binds the host window without creating any tray resources.
    void setMainWindow(QWidget* mainWindow);

    /// Marks whether the platform indicator can be used as a background entry point.
    void setAvailable(bool available);

    /// Returns whether any plugin currently participates in taskbar display.
    bool hasPluginDisplays() const;

    /// Returns whether host policy currently suppresses taskbar rendering.
    bool isDisplaySuppressed() const;

    const QList<TaskbarPluginDisplay>& pluginDisplays() const;
    const QHash<QString, MetricValue>& latestValues() const;

    /// Returns the current compact text that should be shown in the taskbar surface.
    QString currentLabel() const;

    /// Returns the current tooltip text for the active taskbar state.
    QString currentTooltip() const;

    /// Returns structured detail content for the current menu snapshot.
    QList<TaskbarDetailContent> currentDetailContents() const;
    void showDetailMenuNear(const QRect& anchorRect);
    void showDetailMenuNear(const QList<TaskbarDetailContent>& contents, const QRect& anchorRect);
    QString labelForDisplay(const TaskbarPluginDisplay& display, const QHash<QString, MetricValue>& latestValues) const;
    QString tooltipForDisplay(const TaskbarPluginDisplay& display, const QHash<QString, MetricValue>& latestValues) const;
    TaskbarDetailContent detailContentForDisplay(const TaskbarPluginDisplay& display, const QHash<QString, MetricValue>& latestValues) const;

    /// Formats one metric value for human-readable summary display.
    QString formatMetricValue(const MetricValue& value) const;
    QString humanizedMetricName(const QString& key) const;
    QPixmap buildPixmap(const QString& label) const;

    virtual void refresh();
    virtual void hostActionTextsChanged();

private:
    void handleMetricUpdated(const MetricValue& value);
    void handleMetricRemoved(const QString& key);
    QStringList orderedMetricKeys() const;
    QIcon buildIcon(const QString& label) const;

    QWidget* m_mainWindow = nullptr;
    QSystemTrayIcon* m_trayIcon = nullptr;
    QMenu* m_menu = nullptr;
    QWidgetAction* m_detailAction = nullptr;
    TaskbarMenuDetailWidget* m_detailWidget = nullptr;
    QAction* m_summaryAction = nullptr;
    QAction* m_showAction = nullptr;
    QAction* m_quitAction = nullptr;
    QHash<QString, MetricValue> m_latestValues;
    QList<TaskbarPluginDisplay> m_pluginDisplays;
    QString m_showWindowText = QStringLiteral("Show Vitals");
    QString m_quitText = QStringLiteral("Quit Vitals");
    QString m_runningText = QStringLiteral("Running in background");
    QString m_pausedText = QStringLiteral("Monitoring display paused");
    bool m_displaySuppressed = false;
    bool m_available = false;
};

} // namespace Vitals
