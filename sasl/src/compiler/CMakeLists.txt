check_build_with_unicode()
set_environment()

INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${BOOST_HOME_DIR}
	${SALVIA_HOME_DIR}
	${3rd_party_llvm_include_path}
)

LINK_DIRECTORIES(
	${BOOST_LIB_DIR}
	${3rd_party_llvm_lib_path}
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

config_output_paths( sasl_compiler )

