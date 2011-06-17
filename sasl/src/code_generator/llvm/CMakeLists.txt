SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${SALVIA_BOOST_HOME_DIR}
	${SALVIA_HOME_DIR}
	${SALVIA_LLVM_INCLUDE_PATH}
)
	
set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_llext.h
	
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_api.h
	
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_impl.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_impl.imp.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_sisd.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_general.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_vs.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_simd.h
	
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_jit.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_contexts.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_globalctxt.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_type_converters.h
)

set( SOURCE_FILES
	cgllvm_llext.cpp
	
	cgllvm_api.cpp
	
	cgllvm_impl.cpp
	cgllvm_sisd.cpp
	cgllvm_general.cpp
	cgllvm_vs.cpp
	cgllvm_simd.cpp
	
	cgllvm_jit.cpp
	cgllvm_contexts.cpp
	cgllvm_globalctxt.cpp
	
	cgllvm_type_converters.cpp
	cgllvm_opt.cpp
)

ADD_LIBRARY( sasl_cgllvm STATIC	${HEADER_FILES} ${SOURCE_FILES} )

SALVIA_CONFIG_OUTPUT_PATHS( sasl_cgllvm )
