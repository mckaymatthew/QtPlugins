qtplugins_add_extension(hello HelloExtension.h HelloExtension.cpp)
target_link_libraries(hello PRIVATE Qt6::Widgets)
