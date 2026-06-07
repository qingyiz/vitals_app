add_executable(VitalsApp
    main.cpp
    AppContext.h
    AppContext.cpp
    MainWindow.h
    MainWindow.cpp
    DashboardWidget.h
    DashboardWidget.cpp
    NavigationWidget.h
    NavigationWidget.cpp
    PluginCenterWidget.h
    PluginCenterWidget.cpp
    TaskbarDisplaySettingsWidget.h
    TaskbarDisplaySettingsWidget.cpp
    ToggleSwitch.h
    ToggleSwitch.cpp
    resources.qrc
    platform/taskbar/TaskbarIndicator.h
    platform/taskbar/TaskbarIndicator.cpp
    platform/taskbar/TaskbarMenuDetailWidget.h
    platform/taskbar/TaskbarMenuDetailWidget.cpp
    platform/taskbar/TaskbarIndicatorFactory.h
    platform/taskbar/TaskbarIndicatorFactory.cpp
)

if(WIN32)
    target_sources(VitalsApp
        PRIVATE
            platform/taskbar/WindowsTaskbarIndicator.h
            platform/taskbar/WindowsTaskbarIndicator.cpp
            platform/taskbar/WindowsTaskbarOverlayWidget.h
            platform/taskbar/WindowsTaskbarOverlayWidget.cpp
    )
elseif(APPLE)
    target_sources(VitalsApp
        PRIVATE
            platform/taskbar/MacTaskbarIndicator.h
            platform/taskbar/MacTaskbarIndicator.mm
    )
else()
    target_sources(VitalsApp
        PRIVATE
            platform/taskbar/LinuxTaskbarIndicator.h
            platform/taskbar/LinuxTaskbarIndicator.cpp
    )
endif()

set_target_properties(VitalsApp PROPERTIES
    OUTPUT_NAME "Vitals"
)

if(APPLE)
    set_target_properties(VitalsApp PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Vitals"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.vitals.app"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    )
endif()

target_include_directories(VitalsApp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(VitalsApp
    PRIVATE
        Vitals::Core
        Vitals::Widgets
        Vitals::Sdk
        ${VITALS_QT_LIBS}
)

add_custom_command(TARGET VitalsApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets/languages"
        "$<TARGET_FILE_DIR:VitalsApp>/languages"
)

if(APPLE)
    target_link_libraries(VitalsApp PRIVATE "-framework Cocoa")
endif()

if(WIN32)
    target_link_libraries(VitalsApp PRIVATE shell32 user32)
endif()
