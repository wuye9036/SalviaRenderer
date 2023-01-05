file(GLOB HEADER_LIST CONFIGURE_DEPENDS "../../include/sasl/semantic/*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")

add_library(sasl_semantic ${HEADER_LIST} ${SOURCE_LIST})
target_include_directories(sasl_semantic PUBLIC ../../include)
target_link_libraries(sasl_semantic eflib salvia_shader)
target_compile_features(sasl_semantic PUBLIC cxx_std_20)
