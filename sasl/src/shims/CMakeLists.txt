file(GLOB HEADER_LIST CONFIGURE_DEPENDS "../../include/sasl/parser/*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")

add_library(sasl_shims ${HEADER_LIST} ${SOURCE_LIST})
target_include_directories(sasl_shims PUBLIC ../../include)
target_link_libraries(sasl_shims eflib shader)
target_compile_features(sasl_shims PUBLIC cxx_std_20)

set_target_properties(sasl_shims PROPERTIES FOLDER "Shader Libs")
