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

# Installed layout mirrors where the host looks by default: an "extensions" directory next to the
# executable, or Contents/PlugIns/extensions inside the macOS app bundle.
if(APPLE)
    set(QTPLUGINS_INSTALL_EXTENSIONSDIR "qtplugins-host.app/Contents/PlugIns/extensions")
else()
    set(QTPLUGINS_INSTALL_EXTENSIONSDIR "${CMAKE_INSTALL_BINDIR}/extensions")
endif()

# Keep the path to a non-system Qt (e.g. from the Qt online installer) in installed binaries.
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(QtPluginsExtension)

add_subdirectory(sdk)
add_subdirectory(host)
add_subdirectory(plugins)

if(QTPLUGINS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
