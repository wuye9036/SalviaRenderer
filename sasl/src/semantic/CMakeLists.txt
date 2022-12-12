SALVIA_CHECK_BUILD_WITH_UNICODE()
	
set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/semantic/semantic_forward.h
	${SASL_HOME_DIR}/sasl/semantic/elem_indexes.h
	${SASL_HOME_DIR}/sasl/semantic/pety.h
	${SASL_HOME_DIR}/sasl/semantic/reflector.h
	${SASL_HOME_DIR}/sasl/semantic/reflector2.h
	${SASL_HOME_DIR}/sasl/semantic/reflection_impl.h
	${SASL_HOME_DIR}/sasl/semantic/semantic_analyser.h
	${SASL_HOME_DIR}/sasl/semantic/semantic_api.h
	${SASL_HOME_DIR}/sasl/semantic/semantic_diags.h
	${SASL_HOME_DIR}/sasl/semantic/semantics.h
	${SASL_HOME_DIR}/sasl/semantic/symbol.h
	${SASL_HOME_DIR}/sasl/semantic/type_checker.h
	${SASL_HOME_DIR}/sasl/semantic/caster.h
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
