# On Windows, `cmake --install` also runs windeployqt, so the installed folder runs on a machine
# without Qt. The extensions are deployed explicitly, with their QML directories: they need Qt
# libraries, QML modules (QtQuick3D, ...) and plugins (qjpeg, ...) that scanning the host alone
# would miss.
if(NOT WIN32)
    return()
endif()

find_program(QTPLUGINS_WINDEPLOYQT windeployqt
    HINTS "${QT6_INSTALL_PREFIX}/${QT6_INSTALL_BINS}" "${Qt6_DIR}/../../../bin")
if(NOT QTPLUGINS_WINDEPLOYQT)
    message(WARNING "windeployqt not found: `cmake --install` will not deploy Qt")
    return()
endif()

set(_qtplugins_extension_files "")
set(_qtplugins_qmldirs "")
foreach(extension IN ITEMS tictactoe mustardmayo)
    string(APPEND _qtplugins_extension_files
        " \"\${CMAKE_INSTALL_PREFIX}/${QTPLUGINS_INSTALL_EXTENSIONSDIR}/$<TARGET_FILE_NAME:${extension}>\"")
    string(APPEND _qtplugins_qmldirs " --qmldir \"${CMAKE_SOURCE_DIR}/plugins/${extension}/qml\"")
endforeach()

install(CODE "
    set(bin \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\")
    message(STATUS \"Deploying Qt into \${bin}\")
    execute_process(
        COMMAND \"${QTPLUGINS_WINDEPLOYQT}\" --no-translations --dir \"\${bin}\"
            \"\${bin}/$<TARGET_FILE_NAME:qtplugins-host>\"
        COMMAND_ERROR_IS_FATAL ANY)
    execute_process(
        COMMAND \"${QTPLUGINS_WINDEPLOYQT}\" --no-translations --dir \"\${bin}\"
            ${_qtplugins_qmldirs} ${_qtplugins_extension_files}
        COMMAND_ERROR_IS_FATAL ANY)
")
