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

if(WIN32)
    target_sources(NetworkMonitorPlugin
        PRIVATE
            platform/windows/WindowsNetworkCollector.h
            platform/windows/WindowsNetworkCollector.cpp
    )

    target_compile_definitions(NetworkMonitorPlugin
        PRIVATE
            _WIN32_WINNT=0x0600
            NTDDI_VERSION=0x06000000
    )

    target_link_libraries(NetworkMonitorPlugin
        PRIVATE
            iphlpapi
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
    LIBRARY_OUTPUT_DIRECTORY "${VITALS_PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${VITALS_PLUGIN_OUTPUT_DIR}"
)

if(MSVC)
    target_compile_options(NetworkMonitorPlugin PRIVATE /utf-8)
endif()

if(WIN32 AND CMAKE_CONFIGURATION_TYPES)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER "${config}" config_upper)
        set_target_properties(NetworkMonitorPlugin PROPERTIES
            "LIBRARY_OUTPUT_DIRECTORY_${config_upper}" "${VITALS_OUTPUT_DIR}/${config}/plugins"
            "RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${VITALS_OUTPUT_DIR}/${config}/plugins"
        )
    endforeach()
endif()
