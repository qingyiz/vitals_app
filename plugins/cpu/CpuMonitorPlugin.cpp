#include "CpuMonitorPlugin.h"

#include "monitor/CpuMonitorCapability.h"
#include "panel/CpuPanelCapability.h"
#include "settings/CpuSettingsCapability.h"
#include "taskbar/CpuTaskbarCapability.h"

namespace Vitals {

CpuMonitorPlugin::CpuMonitorPlugin(QObject* parent)
    : QObject(parent)
{
}

CpuMonitorPlugin::~CpuMonitorPlugin() = default;

PluginMetaInfo CpuMonitorPlugin::metaInfo() const
{
    return {
        QStringLiteral("com.vitals.cpu"),
        QStringLiteral("CPU Monitor"),
        QStringLiteral("Live CPU usage monitor plugin for macOS."),
        QStringLiteral("0.1.0"),
        QStringLiteral("Vitals"),
        QStringLiteral("monitor"),
        {QStringLiteral("macos")},
        QStringLiteral("0.1.0"),
        true
    };
}

bool CpuMonitorPlugin::initialize(IAppContext* context)
{
    m_context = context;

    m_monitorCapability = std::make_unique<CpuMonitorCapability>();
    m_panelCapability = std::make_unique<CpuPanelCapability>();
    m_taskbarCapability = std::make_unique<CpuTaskbarCapability>(m_monitorCapability.get());
    m_settingsCapability = std::make_unique<CpuSettingsCapability>();

    if (!m_monitorCapability->initialize(context)) {
        shutdown();
        return false;
    }

    connect(m_monitorCapability.get(), &CpuMonitorCapability::snapshotUpdated, this,
        [this](const CpuSnapshot& snapshot) {
            if (m_panelCapability) {
                m_panelCapability->updateSnapshot(snapshot);
            }
        });

    if (m_panelCapability) {
        m_panelCapability->updateSnapshot(m_monitorCapability->lastSnapshot());
    }

    return true;
}

void CpuMonitorPlugin::start()
{
    if (m_monitorCapability) {
        m_monitorCapability->startMonitoring();
    }
}

void CpuMonitorPlugin::stop()
{
    if (m_monitorCapability) {
        m_monitorCapability->stopMonitoring();
    }
}

void CpuMonitorPlugin::shutdown()
{
    if (m_monitorCapability) {
        m_monitorCapability->stopMonitoring();
    }

    m_settingsCapability.reset();
    m_taskbarCapability.reset();
    m_panelCapability.reset();
    m_monitorCapability.reset();
    m_context = nullptr;
}

IMonitorCapability* CpuMonitorPlugin::monitorCapability()
{
    return m_monitorCapability.get();
}

IPanelCapability* CpuMonitorPlugin::panelCapability()
{
    return m_panelCapability.get();
}

ITaskbarCapability* CpuMonitorPlugin::taskbarCapability()
{
    return m_taskbarCapability.get();
}

ISettingsCapability* CpuMonitorPlugin::settingsCapability()
{
    return m_settingsCapability.get();
}

} // namespace Vitals
