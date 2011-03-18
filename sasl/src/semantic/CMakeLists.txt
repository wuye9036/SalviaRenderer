check_build_with_unicode()
set_environment()

INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${BOOST_HOME_DIR}
	${SOFTART_HOME_DIR}
)
	
set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_forward.h
	${SASL_HOME_DIR}/sasl/include/semantic/name_mangler.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_analyser.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_analyser_impl.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_error.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_info.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_infos.h
	${SASL_HOME_DIR}/sasl/include/semantic/semantic_infos.imp.h
	${SASL_HOME_DIR}/sasl/include/semantic/symbol.h
	${SASL_HOME_DIR}/sasl/include/semantic/symbol_scope.h
	${SASL_HOME_DIR}/sasl/include/semantic/type_checker.h
	${SASL_HOME_DIR}/sasl/include/semantic/type_converter.h
	${SASL_HOME_DIR}/sasl/include/semantic/type_manager.h
)

set( SOURCE_FILES
	name_mangler.cpp
	semantic_analyser_impl.cpp
	semantic_error.cpp
	semantic_infos/module_si.cpp
	semantic_infos/semantic_infos.cpp
	semantic_analyser.cpp
	symbol.cpp
	symbol_scope.cpp
	type_checker.cpp
	type_converter.cpp
	type_manager.cpp
)

ADD_LIBRARY( sasl_semantic STATIC
	${HEADER_FILES} ${SOURCE_FILES}
)

config_output_paths( sasl_common )
