add_library(CpuMonitorPlugin MODULE
    CpuMonitorPlugin.h
    CpuMonitorPlugin.cpp
    monitor/CpuMonitorCapability.h
    monitor/CpuMonitorCapability.cpp
    panel/CpuPanelCapability.h
    panel/CpuPanelCapability.cpp
    taskbar/CpuTaskbarCapability.h
    taskbar/CpuTaskbarCapability.cpp
    settings/CpuSettingsCapability.h
    settings/CpuSettingsCapability.cpp
    CpuPanelWidget.h
    CpuPanelWidget.cpp
    ICpuCollector.h
    CpuCollectorFactory.h
    CpuCollectorFactory.cpp
    cpu_plugin.json
)

if(APPLE)
    target_sources(CpuMonitorPlugin
        PRIVATE
            platform/macos/MacCpuCollector.h
            platform/macos/MacCpuCollector.mm
    )
endif()

target_include_directories(CpuMonitorPlugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(CpuMonitorPlugin
    PRIVATE
        Vitals::Sdk
        Vitals::Widgets
        ${VITALS_QT_LIBS}
)

set_target_properties(CpuMonitorPlugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}/plugins"
)
