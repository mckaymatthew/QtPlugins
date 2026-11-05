cmake_minimum_required(VERSION 3.22)

project(QtPlugins VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Sources are UTF-8; without this MSVC reads them in the local code page and mangles string literals.
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup()

# Qt <= 6.5.3 always links the AGL framework, which newer macOS SDKs no longer ship
# ("ld: framework 'AGL' not found"). Nothing uses it, so drop it from Qt's OpenGL target.
if(APPLE AND TARGET WrapOpenGL::WrapOpenGL)
    get_target_property(_qtplugins_gl_libs WrapOpenGL::WrapOpenGL INTERFACE_LINK_LIBRARIES)
    if(_qtplugins_gl_libs)
        list(FILTER _qtplugins_gl_libs EXCLUDE REGEX "AGL")
        set_target_properties(WrapOpenGL::WrapOpenGL PROPERTIES
            INTERFACE_LINK_LIBRARIES "${_qtplugins_gl_libs}")
    endif()
endif()

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

include(QtPluginsDeploy)
