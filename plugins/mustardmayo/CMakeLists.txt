# Game rules and simulation, kept free of rendering so they can be unit tested.
add_library(mustardmayo_core STATIC
    TypingSession.h TypingSession.cpp
    DrivingSession.h DrivingSession.cpp
    MustardMayoGame.h MustardMayoGame.cpp)
target_include_directories(mustardmayo_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mustardmayo_core PUBLIC Qt6::Core)
set_target_properties(mustardmayo_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Quick3D ShaderTools)

qtplugins_add_extension(mustardmayo MustardMayoExtension.h MustardMayoExtension.cpp)
target_link_libraries(mustardmayo PRIVATE mustardmayo_core Qt6::Quick Qt6::Widgets)

# QML and the compiled blur shader live inside the plugin under a prefix unique to it.
qt_add_resources(mustardmayo mustardmayo_qml
    PREFIX /qtplugins/mustardmayo
    BASE qml
    FILES
        qml/Main.qml
        qml/Blur.qml
        qml/OverlayButton.qml
        qml/ProgressBar.qml
        qml/Ring.qml
        qml/WorkScene.qml
        qml/DriveScene.qml
        qml/DestinationScene.qml
        qml/HomeScene.qml
        qml/AsleepScene.qml
        qml/BurgerScene.qml
        qml/Lightning.qml)
# Burger photos, one per bite, served as assets/* next to the QML. JPEG keeps the plugin small but
# needs Qt's qjpeg image-format plugin at runtime (deployed by default by windeployqt/macdeployqt).
qt_add_resources(mustardmayo mustardmayo_assets
    PREFIX /qtplugins/mustardmayo
    FILES
        assets/whataburger0.jpg assets/whataburger1.jpg assets/whataburger2.jpg
        assets/whataburger3.jpg assets/whataburger4.jpg assets/whataburger5.jpg
        assets/whataburger6.jpg assets/whataburger7.jpg)
qt_add_shaders(mustardmayo mustardmayo_shaders
    PREFIX /qtplugins/mustardmayo
    FILES shaders/blur.frag)
