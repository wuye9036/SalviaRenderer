set( HEADER_LIST
	../../include/sasl/semantic/semantic_forward.h
	../../include/sasl/semantic/elem_indexes.h
	../../include/sasl/semantic/pety.h
	../../include/sasl/semantic/reflector.h
	../../include/sasl/semantic/reflector2.h
	../../include/sasl/semantic/reflection_impl.h
	../../include/sasl/semantic/semantic_analyser.h
	../../include/sasl/semantic/semantic_api.h
	../../include/sasl/semantic/semantic_diags.h
	../../include/sasl/semantic/semantics.h
	../../include/sasl/semantic/symbol.h
	../../include/sasl/semantic/type_checker.h
	../../include/sasl/semantic/caster.h
)

set( SOURCE_LIST
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

add_library(sasl_semantic ${HEADER_LIST} ${SOURCE_LIST})
target_include_directories(sasl_semantic PUBLIC ../../include)
target_link_libraries(sasl_semantic eflib shader)
target_compile_features(sasl_semantic PUBLIC cxx_std_20)

set_target_properties(sasl_semantic PROPERTIES FOLDER "Shader Libs")
