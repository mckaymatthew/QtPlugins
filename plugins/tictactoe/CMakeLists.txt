# Game rules, kept free of rendering so they can be unit tested.
add_library(tictactoe_core STATIC GameModel.h GameModel.cpp)
target_include_directories(tictactoe_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tictactoe_core PUBLIC Qt6::Core)
set_target_properties(tictactoe_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
