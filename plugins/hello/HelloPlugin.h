#pragma once

#include "IPanelPlugin.h"

#include <QObject>

namespace Vitals {

class IAppContext;

class HelloPlugin : public QObject, public IPanelPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID Vitals_IPlugin_iid FILE "hello_plugin.json")
    Q_INTERFACES(Vitals::IPlugin Vitals::IPanelPlugin)

public:
    PluginMetaInfo metaInfo() const override;
    bool initialize(IAppContext* context) override;
    void start() override;
    void stop() override;
    void shutdown() override;

    QString panelId() const override;
    QString panelName() const override;
    QString panelIconKey() const override;
    QWidget* createPanel(QWidget* parent = nullptr) override;

private:
    IAppContext* m_context = nullptr;
};

} // namespace Vitals
