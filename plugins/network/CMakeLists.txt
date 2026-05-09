add_library(NetworkMonitorPlugin MODULE
    NetworkMonitorPlugin.h
    NetworkMonitorPlugin.cpp
    NetworkPanelWidget.h
    NetworkPanelWidget.cpp
    INetworkCollector.h
    NetworkCollectorFactory.h
    NetworkCollectorFactory.cpp
    network_plugin.json
)

target_sources(NetworkMonitorPlugin
    PRIVATE
        platform/macos/MacNetworkCollector.h
        platform/macos/MacNetworkCollector.mm
)

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
