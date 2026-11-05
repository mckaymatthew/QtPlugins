find_package(Qt6 REQUIRED COMPONENTS Quick Quick3D)

# Game rules, kept free of rendering so they can be unit tested.
add_library(tictactoe_core STATIC GameModel.h GameModel.cpp)
target_include_directories(tictactoe_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tictactoe_core PUBLIC Qt6::Core)
set_target_properties(tictactoe_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

qtplugins_add_extension(tictactoe TicTacToeExtension.h TicTacToeExtension.cpp)
target_link_libraries(tictactoe PRIVATE tictactoe_core Qt6::Quick Qt6::Widgets)

# QML is compiled into the plugin so deployment is a single file. The resource prefix must be
# unique per extension: all loaded plugins share one resource namespace.
qt_add_resources(tictactoe tictactoe_qml
    PREFIX /qtplugins/tictactoe
    BASE qml
    FILES qml/Board.qml qml/XPiece.qml qml/OPiece.qml qml/OverlayButton.qml)
