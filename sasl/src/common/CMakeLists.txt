SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${SALVIA_BOOST_HOME_DIR}
	${SALVIA_HOME_DIR}
)

set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/common/common_fwd.h
	${SASL_HOME_DIR}/sasl/include/common/compiler_info_manager.h
	${SASL_HOME_DIR}/sasl/include/common/compiler_information.h
	${SASL_HOME_DIR}/sasl/include/common/lex_context.h
	${SASL_HOME_DIR}/sasl/include/common/token.h
)

set( SOURCE_FILES
	compiler_info_manager.cpp
	compiler_information_impl.cpp
)

ADD_LIBRARY( sasl_common STATIC
	${HEADER_FILES} ${SOURCE_FILES}
)

SALVIA_CONFIG_OUTPUT_PATHS( sasl_common )

