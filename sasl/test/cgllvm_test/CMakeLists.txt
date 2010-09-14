check_build_with_unicode()
set_environment()

INCLUDE_DIRECTORIES(
	${boost_home_dir}
	${softart_home_dir}
	${sasl_home_dir}
	${3rd_party_llvm_include_path}
)

LINK_DIRECTORIES(
	${boost_lib_dir}
	${3rd_party_llvm_lib_path}
)

set( HEADER_FILES
)

set( SOURCE_FILES
	llvm_test.cpp
	program_test.cpp
)

ADD_EXECUTABLE( sasl_test_cgllvm ${HEADER_FILES} ${SOURCE_FILES} )
TARGET_LINK_LIBRARIES( sasl_test_cgllvm
	sasl_enums sasl_common sasl_semantic sasl_syntaxtree sasl_cgllvm LLVMCore LLVMSystem LLVMSupport
)

config_output_paths( sasl_test_cgllvm )