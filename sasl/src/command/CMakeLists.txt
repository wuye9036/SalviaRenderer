
set(SOURCE_LIST command.cpp)
add_executable(sasl_command ${SOURCE_LIST})
target_link_libraries(sasl_command sasl_common eflib)
target_compile_features(sasl_semantic PUBLIC cxx_std_20)

