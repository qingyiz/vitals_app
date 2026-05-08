#include "HelloPlugin.h"

#include "IAppContext.h"
#include "IMetricSink.h"

#include <QDateTime>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

namespace Vitals {

PluginMetaInfo HelloPlugin::metaInfo() const
{
    return {
        QStringLiteral("com.vitals.hello"),
        QStringLiteral("Hello Plugin"),
        QStringLiteral("Minimal panel plugin used to verify the host/plugin contract."),
        QStringLiteral("0.1.0"),
        QStringLiteral("Vitals"),
        QStringLiteral("panel"),
        {QStringLiteral("windows"), QStringLiteral("macos"), QStringLiteral("linux")},
        QStringLiteral("0.1.0"),
        false
    };
}

bool HelloPlugin::initialize(IAppContext* context)
{
    m_context = context;
    return m_context != nullptr;
}

void HelloPlugin::start()
{
    if (!m_context || !m_context->metricSink()) {
        return;
    }

    MetricFrame frame;
    frame.pluginId = metaInfo().id;
    frame.timestamp = QDateTime::currentDateTime();
    frame.values.append({
        QStringLiteral("hello.plugin.status"),
        QStringLiteral("Loaded"),
        frame.timestamp,
        {}
    });
    m_context->metricSink()->publishFrame(frame);
}

void HelloPlugin::stop()
{
}

void HelloPlugin::shutdown()
{
    m_context = nullptr;
}

QString HelloPlugin::panelId() const
{
    return QStringLiteral("hello");
}

QString HelloPlugin::panelName() const
{
    return QStringLiteral("Hello Plugin");
}

QString HelloPlugin::panelIconKey() const
{
    return QStringLiteral("hello");
}

QWidget* HelloPlugin::createPanel(QWidget* parent)
{
    auto* page = new QWidget(parent);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 20, 24, 24);
    layout->setSpacing(12);

    auto* title = new QLabel(QStringLiteral("Hello Plugin"), page);
    title->setObjectName(QStringLiteral("pageTitle"));
    auto* body = new QLabel(QStringLiteral("This page is provided by a dynamically loaded plugin."), page);
    body->setWordWrap(true);

    layout->addWidget(title);
    layout->addWidget(body);
    layout->addStretch();
    return page;
}

} // namespace Vitals
