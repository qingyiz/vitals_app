add_library(SystemInfoPlugin MODULE
    SystemInfoPlugin.h
    SystemInfoPlugin.cpp
    monitor/SystemInfoMonitorCapability.h
    monitor/SystemInfoMonitorCapability.cpp
    panel/SystemInfoPanelCapability.h
    panel/SystemInfoPanelCapability.cpp
    taskbar/SystemInfoTaskbarCapability.h
    taskbar/SystemInfoTaskbarCapability.cpp
    settings/SystemInfoSettingsCapability.h
    settings/SystemInfoSettingsCapability.cpp
    SystemInfoPanelWidget.h
    SystemInfoPanelWidget.cpp
    ISystemInfoCollector.h
    SystemInfoCollectorFactory.h
    SystemInfoCollectorFactory.cpp
    systeminfo_plugin.json
)

if(APPLE)
    target_sources(SystemInfoPlugin
        PRIVATE
            platform/macos/MacSystemInfoCollector.h
            platform/macos/MacSystemInfoCollector.mm
    )
endif()

if(WIN32)
    target_sources(SystemInfoPlugin
        PRIVATE
            platform/windows/WindowsSystemInfoCollector.h
            platform/windows/WindowsSystemInfoCollector.cpp
    )
endif()

target_include_directories(SystemInfoPlugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SystemInfoPlugin
    PRIVATE
        Vitals::Sdk
        Vitals::Widgets
        ${VITALS_QT_LIBS}
)

if(APPLE)
    target_link_libraries(SystemInfoPlugin
        PRIVATE
            "-framework Foundation"
            "-framework Metal"
    )
endif()

if(WIN32)
    target_link_libraries(SystemInfoPlugin
        PRIVATE
            user32
    )
endif()

set_target_properties(SystemInfoPlugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}/plugins"
)

if(WIN32 AND CMAKE_CONFIGURATION_TYPES)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER "${config}" config_upper)
        set_target_properties(SystemInfoPlugin PROPERTIES
            "LIBRARY_OUTPUT_DIRECTORY_${config_upper}" "${VITALS_OUTPUT_DIR}/${config}/plugins"
            "RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${VITALS_OUTPUT_DIR}/${config}/plugins"
        )
    endforeach()
endif()
