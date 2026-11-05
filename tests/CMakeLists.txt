find_package(Qt6 REQUIRED COMPONENTS Test Quick)

# Broken/foreign libraries the ExtensionManager must reject, built into their own directory.
set(FIXTURE_DIR "${CMAKE_CURRENT_BINARY_DIR}/fixture-extensions")
foreach(fixture wrongiid badapi)
    qt_add_plugin(fixture_${fixture} SHARED)
    target_sources(fixture_${fixture} PRIVATE fixtures/${fixture}.cpp)
    target_link_libraries(fixture_${fixture} PRIVATE QtPlugins::Sdk)
    set_target_properties(fixture_${fixture} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "$<1:${FIXTURE_DIR}>")
endforeach()
add_library(fixture_notaplugin SHARED fixtures/notaplugin.cpp)
set_target_properties(fixture_notaplugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "$<1:${FIXTURE_DIR}>"
    RUNTIME_OUTPUT_DIRECTORY "$<1:${FIXTURE_DIR}>")
file(WRITE "${FIXTURE_DIR}/README.txt" "Not a library; must be ignored.\n")

qt_add_executable(tst_extensionmanager tst_extensionmanager.cpp)
target_link_libraries(tst_extensionmanager PRIVATE qtplugins_extensionmanager Qt6::Widgets Qt6::Test)
target_compile_definitions(tst_extensionmanager PRIVATE
    EXTENSIONS_DIR="${QTPLUGINS_EXTENSIONS_DIR}"
    FIXTURE_DIR="${FIXTURE_DIR}")
add_dependencies(tst_extensionmanager hello fixture_wrongiid fixture_badapi fixture_notaplugin)
add_test(NAME tst_extensionmanager COMMAND tst_extensionmanager)
set_tests_properties(tst_extensionmanager PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

qt_add_executable(tst_gamemodel tst_gamemodel.cpp)
target_link_libraries(tst_gamemodel PRIVATE tictactoe_core Qt6::Test)
add_test(NAME tst_gamemodel COMMAND tst_gamemodel)

qt_add_executable(tst_mustardmayo tst_mustardmayo.cpp)
target_link_libraries(tst_mustardmayo PRIVATE mustardmayo_core Qt6::Test)
add_test(NAME tst_mustardmayo COMMAND tst_mustardmayo)

# Rendering smoke tests: need a display with OpenGL, so on headless Linux they run under Xvfb.
find_program(XVFB_RUN xvfb-run)
foreach(game tictactoe mustardmayo)
    qt_add_executable(smoke_${game} smoke_${game}.cpp SmokeUtils.h)
    target_link_libraries(smoke_${game} PRIVATE
        qtplugins_extensionmanager Qt6::Widgets Qt6::Quick Qt6::Test)
    target_compile_definitions(smoke_${game} PRIVATE
        EXTENSIONS_DIR="${QTPLUGINS_EXTENSIONS_DIR}"
        SCREENSHOT_DIR="${CMAKE_CURRENT_BINARY_DIR}")
    add_dependencies(smoke_${game} ${game})
    if(XVFB_RUN)
        add_test(NAME smoke_${game}
                 COMMAND ${XVFB_RUN} -a -s "-screen 0 1280x1024x24" $<TARGET_FILE:smoke_${game}>)
    elseif(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
        add_test(NAME smoke_${game} COMMAND smoke_${game})
    endif()
endforeach()

# On Windows, tests run from the build tree need Qt's DLLs, which live next to Qt's tools in bin/.
if(WIN32)
    get_property(_qtplugins_tests DIRECTORY PROPERTY TESTS)
    set_property(TEST ${_qtplugins_tests} APPEND PROPERTY ENVIRONMENT_MODIFICATION
        "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")
endif()
