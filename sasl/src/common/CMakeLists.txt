SALVIA_CHECK_BUILD_WITH_UNICODE()

set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/common/common_fwd.h
	${SASL_HOME_DIR}/sasl/include/common/diag_chat.h
	${SASL_HOME_DIR}/sasl/include/common/diag_item.h
	${SASL_HOME_DIR}/sasl/include/common/diag_formatter.h
	${SASL_HOME_DIR}/sasl/include/common/lex_context.h
	${SASL_HOME_DIR}/sasl/include/common/token.h
)

set( SOURCE_FILES diag_chat.cpp diag_item.cpp token.cpp diag_formatter.cpp)

SASL_CONFIG_LIBRARY( sasl_common )

target_link_libraries(sasl_common EFLIB)

