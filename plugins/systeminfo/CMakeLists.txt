add_library(SystemInfoPlugin MODULE
    SystemInfoPlugin.h
    SystemInfoPlugin.cpp
    SystemInfoPanelWidget.h
    SystemInfoPanelWidget.cpp
    ISystemInfoCollector.h
    SystemInfoCollectorFactory.h
    SystemInfoCollectorFactory.cpp
    systeminfo_plugin.json
)

target_sources(SystemInfoPlugin
    PRIVATE
        platform/macos/MacSystemInfoCollector.h
        platform/macos/MacSystemInfoCollector.mm
)

target_include_directories(SystemInfoPlugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SystemInfoPlugin
    PRIVATE
        Vitals::Sdk
        Vitals::Widgets
        ${VITALS_QT_LIBS}
        "-framework Foundation"
        "-framework Metal"
)

set_target_properties(SystemInfoPlugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}/plugins"
)
