SALVIA_CHECK_BUILD_WITH_UNICODE()

set( ADDITIONAL_INCLUDE_DIRECTORIES
	${SALVIA_LLVM_INCLUDE_PATH}
	)
	
set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/codegen/utility.h
	${SASL_HOME_DIR}/sasl/include/codegen/ty_cache.h
	${SASL_HOME_DIR}/sasl/include/codegen/generate_entry.h
	
	${SASL_HOME_DIR}/sasl/include/codegen/cg_extension.h
	${SASL_HOME_DIR}/sasl/include/codegen/cgs.h
	${SASL_HOME_DIR}/sasl/include/codegen/cgs_objects.h
	${SASL_HOME_DIR}/sasl/include/codegen/cgs_sisd.h
	${SASL_HOME_DIR}/sasl/include/codegen/cgs_simd.h
	
	${SASL_HOME_DIR}/sasl/include/codegen/cg_api.h
	
	${SASL_HOME_DIR}/sasl/include/codegen/cg_impl.h
	${SASL_HOME_DIR}/sasl/include/codegen/cg_impl.imp.h
	${SASL_HOME_DIR}/sasl/include/codegen/cg_sisd.h
	${SASL_HOME_DIR}/sasl/include/codegen/cg_general.h
	${SASL_HOME_DIR}/sasl/include/codegen/cg_vs.h
	${SASL_HOME_DIR}/sasl/include/codegen/cg_simd.h
	${SASL_HOME_DIR}/sasl/include/codegen/cg_ps.h
	
	${SASL_HOME_DIR}/sasl/include/codegen/cg_jit.h
	${SASL_HOME_DIR}/sasl/include/codegen/cg_contexts.h
	${SASL_HOME_DIR}/sasl/include/codegen/cg_module_impl.h
	${SASL_HOME_DIR}/sasl/include/codegen/cg_caster.h
	${SASL_HOME_DIR}/sasl/include/codegen/cg_intrins.h
	${SASL_HOME_DIR}/sasl/include/codegen/cg_type_builder.h
)

set( SOURCE_FILES
	utility.cpp
	ty_cache.cpp
	generate_entry.cpp
	
	cg_extension.cpp
	cgs.cpp
	cgs_objects.cpp
	cgs_sisd.cpp
	cgs_simd.cpp
	
	cg_api.cpp
	
	cg_impl.cpp
	cg_sisd.cpp
	cg_general.cpp
	cg_vs.cpp
	cg_simd.cpp
	
	cg_jit.cpp
	cg_contexts.cpp
	cg_module_impl.cpp
	
	cg_caster.cpp
	cg_opt.cpp
	
	cg_intrins.cpp
)

SASL_CONFIG_LIBRARY(sasl_codegen)
