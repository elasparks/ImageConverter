add_subdirectory(utils)
add_subdirectory(image_types)
add_subdirectory(image_formats)

add_executable(converter main.cpp)

set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML COMPONENTS system window graphics)
target_link_libraries(converter PUBLIC sfml-system sfml-graphics sfml-window image_types image_formats)