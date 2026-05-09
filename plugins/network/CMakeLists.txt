add_library(NetworkMonitorPlugin MODULE
    NetworkMonitorPlugin.h
    NetworkMonitorPlugin.cpp
    monitor/NetworkMonitorCapability.h
    monitor/NetworkMonitorCapability.cpp
    panel/NetworkPanelCapability.h
    panel/NetworkPanelCapability.cpp
    taskbar/NetworkTaskbarCapability.h
    taskbar/NetworkTaskbarCapability.cpp
    settings/NetworkSettingsCapability.h
    settings/NetworkSettingsCapability.cpp
    NetworkPanelWidget.h
    NetworkPanelWidget.cpp
    INetworkCollector.h
    NetworkCollectorFactory.h
    NetworkCollectorFactory.cpp
    network_plugin.json
)

if(APPLE)
    target_sources(NetworkMonitorPlugin
        PRIVATE
            platform/macos/MacNetworkCollector.h
            platform/macos/MacNetworkCollector.mm
    )
endif()

target_include_directories(NetworkMonitorPlugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(NetworkMonitorPlugin
    PRIVATE
        Vitals::Sdk
        Vitals::Widgets
        ${VITALS_QT_LIBS}
)

set_target_properties(NetworkMonitorPlugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}/plugins"
)
