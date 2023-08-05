add_library(image_formats STATIC image_formats/pcx/PCXFormat.h image_formats/pcx/PCXPalette16Color.h)
set_target_properties(image_formats PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(image_formats PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(image_formats image_types)