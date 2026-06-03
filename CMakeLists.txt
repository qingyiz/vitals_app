cmake_minimum_required(VERSION 3.20)

project(Vitals
    VERSION 0.1.0
    DESCRIPTION "Cross-platform desktop system monitor framework"
    LANGUAGES CXX
)

if(APPLE)
    enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(VITALS_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")
if(APPLE)
    set(VITALS_PLUGIN_OUTPUT_DIR "${VITALS_OUTPUT_DIR}/Vitals.app/Contents/MacOS/plugins")
else()
    set(VITALS_PLUGIN_OUTPUT_DIR "${VITALS_OUTPUT_DIR}/plugins")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${VITALS_OUTPUT_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

find_package(Qt6 QUIET COMPONENTS Core Widgets)
if(Qt6_FOUND)
    set(VITALS_QT_VERSION 6)
    set(VITALS_QT_LIBS Qt6::Core Qt6::Widgets)
else()
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Widgets)
    set(VITALS_QT_VERSION 5)
    set(VITALS_QT_LIBS Qt5::Core Qt5::Widgets)
endif()

message(STATUS "Using Qt ${VITALS_QT_VERSION}")

add_subdirectory(sdk)
add_subdirectory(core)
add_subdirectory(widgets)
add_subdirectory(plugins)
add_subdirectory(app)
