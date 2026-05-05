#pragma once

#include "IMonitorPlugin.h"
#include "IPanelPlugin.h"
#include "ISystemInfoCollector.h"

#include <QObject>
#include <QPointer>
#include <memory>

class QTimer;

namespace Vitals {

class IAppContext;
class SystemInfoPanelWidget;

/**
 * \if ENGLISH
 * @brief macOS-first system information plugin for the Vitals host
 *
 * Publishes normalized host identity metrics and provides a read-only details
 * panel. The first implementation targets macOS only, with platform support
 * declared explicitly through plugin metadata and collector factory dispatch.
 * \endif
 *
 * \if CHINESE
 * @brief 面向 Vitals 宿主的 macOS 优先系统信息插件
 *
 * 该插件发布统一格式的宿主基础系统信息指标，并提供只读详情面板。当前首版
 * 仅支持 macOS，平台支持范围通过插件元信息与采集器工厂分发明确声明。
 * \endif
 */
class SystemInfoPlugin : public QObject, public IMonitorPlugin, public IPanelPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Vitals_IPlugin_iid FILE "systeminfo_plugin.json")
    Q_INTERFACES(Vitals::IPlugin Vitals::IMonitorPlugin Vitals::IPanelPlugin)

public:
    explicit SystemInfoPlugin(QObject* parent = nullptr);
    ~SystemInfoPlugin() override;

    PluginMetaInfo metaInfo() const override;
    bool initialize(IAppContext* context) override;
    void start() override;
    void stop() override;
    void shutdown() override;

    QList<MetricDescriptor> metricDescriptors() const override;
    int defaultIntervalMs() const override;
    void setIntervalMs(int intervalMs) override;

    QString panelId() const override;
    QString panelName() const override;
    QString panelIconKey() const override;
    QWidget* createPanel(QWidget* parent = nullptr) override;

private:
    void collectAndPublish();
    void publishSnapshot(const SystemInfoSnapshot& snapshot) const;

    IAppContext* m_context = nullptr;
    std::unique_ptr<ISystemInfoCollector> m_collector;
    QTimer* m_timer = nullptr;
    int m_intervalMs = 5000;
    SystemInfoSnapshot m_lastSnapshot;
    QPointer<SystemInfoPanelWidget> m_panel;
};

} // namespace Vitals
