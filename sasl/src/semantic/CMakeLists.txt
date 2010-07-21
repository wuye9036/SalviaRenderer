check_build_with_unicode()
set_environment()

INCLUDE_DIRECTORIES(
	${sasl_home_dir}
	${boost_home_dir}
	${softart_home_dir}
)
	
set( HEADER_FILES
	${sasl_home_dir}/sasl/include/semantic/semantic_forward.h
	${sasl_home_dir}/sasl/include/semantic/name_mangler.h
	${sasl_home_dir}/sasl/include/semantic/semantic_analyser.h
	${sasl_home_dir}/sasl/include/semantic/semantic_analyser_impl.h
	${sasl_home_dir}/sasl/include/semantic/semantic_error.h
	${sasl_home_dir}/sasl/include/semantic/semantic_info.h
	${sasl_home_dir}/sasl/include/semantic/semantic_infos.h
	${sasl_home_dir}/sasl/include/semantic/symbol.h
	${sasl_home_dir}/sasl/include/semantic/symbol_scope.h
	${sasl_home_dir}/sasl/include/semantic/type_checker.h
)

set( SOURCE_FILES
	name_mangler.cpp
	semantic_analyser_impl.cpp
	semantic_error.cpp
	semantic_infos.cpp
	semantic_analyser.cpp
	symbol.cpp
	symbol_scope.cpp
	type_checker.cpp
)

ADD_LIBRARY( sasl_semantic STATIC
	${HEADER_FILES} ${SOURCE_FILES}
)

config_output_paths( sasl_common )
