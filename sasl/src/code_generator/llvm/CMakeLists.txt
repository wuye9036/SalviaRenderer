check_build_with_unicode()
set_environment()

INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${BOOST_HOME_DIR}
	${SOFTART_HOME_DIR}
	${3rd_party_llvm_include_path}
)
	
set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_api.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_common.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_jit.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_contexts.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_globalctxt.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_impl.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_type_converters.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_vs.h
)

set( SOURCE_FILES
	cgllvm_api.cpp
	cgllvm_common.cpp
	cgllvm_vs.cpp
	cgllvm_jit.cpp
	cgllvm_contexts.cpp
	cgllvm_globalctxt.cpp
	cgllvm_impl.cpp
	cgllvm_type_converters.cpp
	cgllvm_opt.cpp
)

ADD_LIBRARY( sasl_cgllvm STATIC	${HEADER_FILES} ${SOURCE_FILES} )

config_output_paths( sasl_cgllvm )
