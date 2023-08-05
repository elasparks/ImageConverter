add_library(image_types STATIC image_types/PCX.h image_types/Bitmap.h)
set_target_properties(image_types PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(image_types PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(image_types color_formats)