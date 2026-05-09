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
    ToggleSwitch.h
    ToggleSwitch.cpp
    platform/taskbar/TaskbarIndicator.h
    platform/taskbar/TaskbarIndicator.cpp
    platform/taskbar/TaskbarMenuDetailWidget.h
    platform/taskbar/TaskbarMenuDetailWidget.cpp
    platform/taskbar/TaskbarIndicatorFactory.h
    platform/taskbar/TaskbarIndicatorFactory.cpp
    platform/taskbar/WindowsTaskbarIndicator.h
    platform/taskbar/WindowsTaskbarIndicator.cpp
    platform/taskbar/MacTaskbarIndicator.h
    platform/taskbar/MacTaskbarIndicator.mm
    platform/taskbar/LinuxTaskbarIndicator.h
    platform/taskbar/LinuxTaskbarIndicator.cpp
)

set_target_properties(VitalsApp PROPERTIES
    OUTPUT_NAME "Vitals"
)

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

if(APPLE)
    target_link_libraries(VitalsApp PRIVATE "-framework Cocoa")
endif()
