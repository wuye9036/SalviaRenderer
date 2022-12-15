
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "../../include/sasl/codegen/*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "*.cpp")

add_library( sasl_codegen STATIC ${HEADER_LIST} ${SOURCE_LIST})
target_include_directories(sasl_codegen PUBLIC ../../include)
target_link_libraries(sasl_codegen eflib shader)
target_link_llvm(sasl_codegen)
target_compile_features(sasl_semantic PUBLIC cxx_std_20)


set_target_properties(sasl_codegen PROPERTIES FOLDER "Shader Libs")
