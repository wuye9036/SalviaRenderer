check_build_with_unicode()
set_environment()

INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${BOOST_HOME_DIR}
	${SOFTART_HOME_DIR}
)

set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/common/common_fwd.h
	${SASL_HOME_DIR}/sasl/include/common/compiler_info_manager.h
	${SASL_HOME_DIR}/sasl/include/common/compiler_information.h
	${SASL_HOME_DIR}/sasl/include/common/lex_context.h
	${SASL_HOME_DIR}/sasl/include/common/token_attr.h
)

set( SOURCE_FILES
	compiler_info_manager.cpp
	compiler_information_impl.cpp
)

ADD_LIBRARY( sasl_common STATIC
	${HEADER_FILES} ${SOURCE_FILES}
)

config_output_paths( sasl_common )

