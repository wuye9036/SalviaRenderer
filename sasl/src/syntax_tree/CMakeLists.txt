file(GLOB HEADER_LIST CONFIGURE_DEPENDS "../../include/sasl/syntax_tree/*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")

add_library(sasl_syntax_tree ${HEADER_LIST} ${SOURCE_LIST})
target_include_directories(sasl_syntax_tree PUBLIC ../../include)
target_link_libraries(sasl_syntax_tree sasl_enums sasl_common eflib Boost::boost range-v3 range-v3-meta range-v3::meta range-v3-concepts)
target_compile_features(sasl_syntax_tree PUBLIC cxx_std_20)
