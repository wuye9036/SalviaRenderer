SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SALVIA_BOOST_HOME_DIR}
	${SALVIA_HOME_DIR}
)
	
set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_forward.h
	${SASL_HOME_DIR}/sasl/include/semantic/abi_analyser.h
	${SASL_HOME_DIR}/sasl/include/semantic/abi_info.h
	${SASL_HOME_DIR}/sasl/include/semantic/name_mangler.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_analyser.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_api.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_error.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_info.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_infos.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_infos.imp.h
	${SASL_HOME_DIR}/sasl/include/semantic/symbol.h
	${SASL_HOME_DIR}/sasl/include/semantic/symbol_scope.h
	${SASL_HOME_DIR}/sasl/include/semantic/ssa_context.h
	${SASL_HOME_DIR}/sasl/include/semantic/ssa_constructor.h
	${SASL_HOME_DIR}/sasl/include/semantic/type_checker.h
	${SASL_HOME_DIR}/sasl/include/semantic/caster.h
	${SASL_HOME_DIR}/sasl/include/semantic/pety.h
	${SASL_HOME_DIR}/sasl/include/semantic/deps_graph.h
)

set( SOURCE_FILES
	abi_analyser.cpp
	abi_info.cpp
	name_mangler.cpp
	semantic_analyser.cpp
	semantic_api.cpp
	semantic_error.cpp
	semantic_infos/module_si.cpp
	semantic_infos/semantic_infos.cpp
	ssa_context.cpp
	ssa_constructor.cpp
	symbol.cpp
	symbol_scope.cpp
	type_checker.cpp
	caster.cpp
	pety.cpp
	deps_graph.cpp
)

ADD_LIBRARY( sasl_semantic STATIC
	${HEADER_FILES} ${SOURCE_FILES}
)

SALVIA_CONFIG_OUTPUT_PATHS( sasl_semantic )
