#pragma once

#include "PluginMetaInfo.h"

#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QVector>

class QPluginLoader;

namespace Vitals {

class IAppContext;
class IPlugin;
class ConfigManager;

/**
 * \if ENGLISH
 * @brief Runtime state summary for one discovered plugin binary
 * \endif
 *
 * \if CHINESE
 * @brief 单个已发现插件二进制的运行时状态摘要
 * \endif
 */
struct PluginRuntimeInfo
{
    enum class Status
    {
        Loaded,
        Disabled,
        Skipped,
        Failed
    };

    QString filePath;
    PluginMetaInfo metaInfo;
    Status status = Status::Failed;
    QString reason;
};

/**
 * \if ENGLISH
 * @brief Runtime manager responsible for plugin discovery and lifecycle control
 *
 * Scans the plugin directory, validates platform compatibility metadata,
 * instantiates supported plugins, and coordinates their host lifecycle.
 * The host application should centralize plugin loading through this class.
 * \endif
 *
 * \if CHINESE
 * @brief 负责插件发现与生命周期控制的运行时管理器
 *
 * 该类负责扫描插件目录、校验平台兼容元信息、实例化受支持的插件，并统一
 * 协调它们在宿主中的生命周期。宿主程序应通过该类集中完成插件加载。
 * \endif
 */
class PluginManager : public QObject
{
    Q_OBJECT

public:
    explicit PluginManager(QObject* parent = nullptr);
    ~PluginManager() override;

    /**
     * \if ENGLISH
     * @brief Discovers and initializes all compatible plugins under a directory
     * \endif
     *
     * \if CHINESE
     * @brief 扫描指定目录并初始化所有与当前平台兼容的插件
     * \endif
     */
    void loadAllPlugins(const QString& pluginsDir, IAppContext* context, const ConfigManager* configManager = nullptr);

    /// Starts every successfully loaded plugin.
    void startAll();

    /// Stops every successfully loaded plugin.
    void stopAll();

    /// Shuts down all loaded plugins and releases their loaders.
    void shutdownAll();

    /// Returns the currently loaded plugin instances.
    QList<IPlugin*> plugins() const;

    /// Returns the number of successfully loaded plugins.
    int loadedPluginCount() const;

    /// Returns runtime summaries for all discovered plugin binaries.
    QList<PluginRuntimeInfo> pluginInfos() const;

Q_SIGNALS:
    void pluginLoaded(const QString& pluginId, const QString& pluginName);
    void pluginLoadFailed(const QString& path, const QString& reason);
    void pluginSkipped(const QString& path, const QString& reason);

private:
    struct LoadedPlugin
    {
        QSharedPointer<QPluginLoader> loader;
        IPlugin* plugin = nullptr;
    };

    static bool isPluginFile(const QString& fileName);
    static QString currentPlatformId();
    static bool supportsCurrentPlatform(const QPluginLoader& loader, QString* reason);
    static PluginMetaInfo metaInfoFromLoader(const QPluginLoader& loader);

    QVector<LoadedPlugin> m_plugins;
    QVector<PluginRuntimeInfo> m_pluginInfos;
};

} // namespace Vitals
