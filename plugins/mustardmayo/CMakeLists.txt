# Game rules and simulation, kept free of rendering so they can be unit tested.
add_library(mustardmayo_core STATIC
    TypingSession.h TypingSession.cpp
    DrivingSession.h DrivingSession.cpp
    MustardMayoGame.h MustardMayoGame.cpp)
target_include_directories(mustardmayo_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mustardmayo_core PUBLIC Qt6::Core)
set_target_properties(mustardmayo_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
