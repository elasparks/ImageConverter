add_library(color_formats STATIC color_formats/ColorFormats.h)
set_target_properties(color_formats PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(color_formats PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})