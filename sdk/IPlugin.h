#pragma once

#include "IAppContext.h"
#include "PluginMetaInfo.h"

#include <QtPlugin>

namespace Vitals {

/**
 * \if ENGLISH
 * @brief Base lifecycle contract for all Vitals plugins
 *
 * Defines the minimal host-plugin contract used by the Vitals application.
 * Every plugin must expose metadata, complete host-aware initialization, and
 * participate in a predictable lifecycle including start, stop, and shutdown.
 * \endif
 *
 * \if CHINESE
 * @brief Vitals 所有插件的基础生命周期接口
 *
 * 定义 Vitals 宿主与插件之间最小且稳定的契约。每个插件都必须提供元信息，
 * 完成带宿主上下文的初始化，并参与可预测的生命周期流程，包括 start、stop
 * 与 shutdown。
 * \endif
 */
class IPlugin
{
public:
    virtual ~IPlugin() = default;

    /**
     * \if ENGLISH
     * @brief Returns the static metadata declared by the plugin
     * \endif
     *
     * \if CHINESE
     * @brief 返回插件声明的静态元信息
     * \endif
     */
    virtual PluginMetaInfo metaInfo() const = 0;

    /**
     * \if ENGLISH
     * @brief Initializes the plugin with host services before it can run
     * @param context Host application context provided by the framework
     * @return True if initialization succeeds and the plugin can be started
     * \endif
     *
     * \if CHINESE
     * @brief 使用宿主提供的服务初始化插件
     * @param context 框架提供的宿主应用上下文
     * @return 初始化成功并可进入启动阶段时返回 true
     * \endif
     */
    virtual bool initialize(IAppContext* context) = 0;

    /**
     * \if ENGLISH
     * @brief Starts the plugin runtime work after successful initialization
     * \endif
     *
     * \if CHINESE
     * @brief 在初始化成功后启动插件运行逻辑
     * \endif
     */
    virtual void start() = 0;

    /**
     * \if ENGLISH
     * @brief Requests the plugin to stop its active runtime work
     * \endif
     *
     * \if CHINESE
     * @brief 请求插件停止当前运行中的逻辑
     * \endif
     */
    virtual void stop() = 0;

    /**
     * \if ENGLISH
     * @brief Releases plugin-owned resources before unloading
     * \endif
     *
     * \if CHINESE
     * @brief 在卸载前释放插件持有的资源
     * \endif
     */
    virtual void shutdown() = 0;
};

} // namespace Vitals

#define Vitals_IPlugin_iid "com.vitals.plugin.IPlugin/1.0"
Q_DECLARE_INTERFACE(Vitals::IPlugin, Vitals_IPlugin_iid)
