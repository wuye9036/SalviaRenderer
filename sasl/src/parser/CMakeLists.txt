
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "../../include/sasl/parser/*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")

add_library(sasl_parser ${HEADER_LIST} ${SOURCE_LIST})
target_include_directories(sasl_parser PUBLIC ../../include)
target_link_libraries(sasl_parser eflib range-v3 range-v3-meta range-v3::meta range-v3-concepts fmt::fmt)
target_compile_features(sasl_parser PUBLIC cxx_std_20)
