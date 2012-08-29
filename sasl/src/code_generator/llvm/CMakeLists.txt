SALVIA_CHECK_BUILD_WITH_UNICODE()

set( ADDITIONAL_INCLUDE_DIRECTORIES ${SALVIA_LLVM_INCLUDE_PATH} )
	
set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/utility.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/ty_cache.h
	
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cg_extension.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgs.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgs_objects.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgs_sisd.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgs_simd.h
	
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_api.h
	
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_impl.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_impl.imp.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_sisd.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_general.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_vs.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_simd.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_ps.h
	
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_jit.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_contexts.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_module_impl.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_caster.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_intrins.h
	${SASL_HOME_DIR}/sasl/include/code_generator/llvm/cgllvm_tbe.h
)

set( SOURCE_FILES
	utility.cpp
	ty_cache.cpp
	
	cg_extension.cpp
	cgs.cpp
	cgs_objects.cpp
	cgs_sisd.cpp
	cgs_simd.cpp
	
	cgllvm_api.cpp
	
	cgllvm_impl.cpp
	cgllvm_sisd.cpp
	cgllvm_general.cpp
	cgllvm_vs.cpp
	cgllvm_simd.cpp
	
	cgllvm_jit.cpp
	cgllvm_contexts.cpp
	cgllvm_module_impl.cpp
	
	cgllvm_caster.cpp
	cgllvm_opt.cpp
	
	cgllvm_intrins.cpp
)

SASL_CONFIG_LIBRARY( sasl_cgllvm )
