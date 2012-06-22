SALVIA_CHECK_BUILD_WITH_UNICODE()
	
set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_forward.h
	${SASL_HOME_DIR}/sasl/include/semantic/abi_analyser.h
	${SASL_HOME_DIR}/sasl/include/semantic/abi_info.h
	${SASL_HOME_DIR}/sasl/include/semantic/name_mangler.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_analyser.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_api.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_diags.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantics.h
	${SASL_HOME_DIR}/sasl/include/semantic/symbol.h
	${SASL_HOME_DIR}/sasl/include/semantic/type_checker.h
	${SASL_HOME_DIR}/sasl/include/semantic/caster.h
	${SASL_HOME_DIR}/sasl/include/semantic/pety.h
	
	${SASL_HOME_DIR}/sasl/include/semantic/ssa_constructor.h
	${SASL_HOME_DIR}/sasl/include/semantic/ssa_context.h
	${SASL_HOME_DIR}/sasl/include/semantic/ssa_graph.h
	${SASL_HOME_DIR}/sasl/include/semantic/ssa_nodes.h
	${SASL_HOME_DIR}/sasl/include/semantic/ssa_vmap.h
)

set( SOURCE_FILES
	abi_analyser.cpp
	abi_info.cpp
	name_mangler.cpp
	semantic_analyser.cpp
	semantic_api.cpp
	semantic_diags.cpp
	symbol.cpp
	type_checker.cpp
	caster.cpp
	pety.cpp
	
	# ssa_constructor.cpp
	# ssa_context.cpp
	# ssa_graph.cpp
	# ssa_nodes.cpp
	# ssa_vmap.cpp
	
	semantics.cpp
)

SASL_CONFIG_LIBRARY( sasl_semantic )
