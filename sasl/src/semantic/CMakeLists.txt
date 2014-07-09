SALVIA_CHECK_BUILD_WITH_UNICODE()
	
set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_forward.h
	${SASL_HOME_DIR}/sasl/include/semantic/elem_indexes.h
	${SASL_HOME_DIR}/sasl/include/semantic/pety.h
	${SASL_HOME_DIR}/sasl/include/semantic/reflector.h
	${SASL_HOME_DIR}/sasl/include/semantic/reflector2.h
	${SASL_HOME_DIR}/sasl/include/semantic/reflection_impl.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_analyser.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_api.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_diags.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantics.h
	${SASL_HOME_DIR}/sasl/include/semantic/symbol.h
	${SASL_HOME_DIR}/sasl/include/semantic/type_checker.h
	${SASL_HOME_DIR}/sasl/include/semantic/caster.h
)

set( SOURCE_FILES
	reflector.cpp
	reflector2.cpp
	reflection_impl.cpp
	semantic_analyser.cpp
	semantic_api.cpp
	semantic_diags.cpp
	symbol.cpp
	type_checker.cpp
	caster.cpp
	pety.cpp
	semantics.cpp
)

SASL_CONFIG_LIBRARY( sasl_semantic )
