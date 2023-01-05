file(GLOB HEADER_LIST CONFIGURE_DEPENDS "./include/enum/*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")

add_library( sasl_enums STATIC ${HEADER_LIST} ${SOURCE_LIST})
target_include_directories(sasl_enums PUBLIC ../../include)
target_link_libraries(sasl_enums eflib salvia_shader)