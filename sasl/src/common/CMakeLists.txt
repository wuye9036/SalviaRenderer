SALVIA_CHECK_BUILD_WITH_UNICODE()

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

SASL_CONFIG_LIBRARY( sasl_common )

