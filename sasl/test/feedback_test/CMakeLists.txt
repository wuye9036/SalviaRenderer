SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${boost_home_dir}
	${softart_home_dir}
	${sasl_home_dir}
	${SALVIA_LLVM_INCLUDE_PATH}
)

LINK_DIRECTORIES(
	${boost_lib_dir}
	${SALVIA_LLVM_LIB_PATH}
)

set( HEADER_FILES
)

set( SOURCE_FILES
	llvm_test.cpp
	program_test.cpp
	function_test_basic.cpp
)

ADD_EXECUTABLE( sasl_test_cgllvm ${HEADER_FILES} ${SOURCE_FILES} )
TARGET_LINK_LIBRARIES( sasl_test_cgllvm
	sasl_enums sasl_common sasl_semantic sasl_syntaxtree sasl_cgllvm LLVMCore LLVMSystem LLVMSupport
)

SET_TARGET_PROPERTIES( sasl_test_cgllvm PROPERTIES FOLDER "Shader Tests")
SALVIA_CONFIG_OUTPUT_PATHS( sasl_test_cgllvm )