cmake_minimum_required(VERSION 3.22)

project(QtPlugins VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup()

option(QTPLUGINS_BUILD_TESTS "Build unit tests" ON)

# Every plugin's shared library is written here so the host can be pointed straight at it:
#   build/host/qtplugins-host --extensions-dir build/extensions
set(QTPLUGINS_EXTENSIONS_DIR "${CMAKE_BINARY_DIR}/extensions")
