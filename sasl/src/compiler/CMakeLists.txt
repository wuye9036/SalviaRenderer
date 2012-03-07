SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${SALVIA_BOOST_HOME_DIR}
	${SALVIA_HOME_DIR}
	${SALVIA_LLVM_INCLUDE_PATH}
)

LINK_DIRECTORIES(
	${SALVIA_BOOST_LIB_DIR}
	${SALVIA_LLVM_LIB_PATH}
)

set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/parser/parse_api.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/parse_api.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_analyser.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_api.h
	${SASL_HOME_DIR}/sasl/include/compiler/options.h
	${SASL_HOME_DIR}/sasl/include/compiler/compiler_forward.h
)

set( SOURCE_FILES
	compiler.cpp
	options.cpp
)

ADD_EXECUTABLE( sasl_compiler ${HEADER_FILES} ${SOURCE_FILES} )

TARGET_LINK_LIBRARIES( sasl_compiler
	sasl_cgllvm
	sasl_semantic sasl_syntaxtree sasl_parser
	sasl_enums sasl_common
	${SASL_LLVM_LIBS}
	${SALVIA_BOOST_LIBS}
)

SET_TARGET_PROPERTIES( sasl_compiler PROPERTIES FOLDER "Shader Drivers")
SALVIA_CONFIG_OUTPUT_PATHS( sasl_compiler )

