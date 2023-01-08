file(GLOB HEADER_LIST CONFIGURE_DEPENDS "../../include/drivers/*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")

# Configure Dynamic Link Driver.
add_library(sasl_drivers SHARED ${HEADER_LIST} ${SOURCE_LIST})
# disable compiler error caused by __DATE__ and __TIME__ which is used in Boost.Wave
target_compile_options_if_applicable(sasl_drivers "-Wno-date-time")
target_compile_features(sasl_drivers PUBLIC cxx_std_20)
target_include_directories(sasl_drivers PUBLIC ../../include)
target_link_libraries( sasl_drivers PUBLIC
	sasl_shims sasl_codegen sasl_semantic sasl_syntax_tree sasl_parser sasl_enums sasl_common eflib
    Boost::wave Boost::program_options
)