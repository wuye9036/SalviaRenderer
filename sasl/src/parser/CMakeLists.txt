SALVIA_CHECK_BUILD_WITH_UNICODE()

set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/parser/diags.h
	${SASL_HOME_DIR}/sasl/include/parser/error_handlers.h
	${SASL_HOME_DIR}/sasl/include/parser/lexer.h
	${SASL_HOME_DIR}/sasl/include/parser/generator.h
	${SASL_HOME_DIR}/sasl/include/parser/grammars.h
	${SASL_HOME_DIR}/sasl/include/parser/parse_api.h
	${SASL_HOME_DIR}/sasl/include/parser/parser_forward.h
)

set( SOURCE_FILES
	diags.cpp
	error_handlers.cpp
	generator.cpp
	grammars.cpp
	lexer.cpp
	parse_api.cpp
)

SASL_CONFIG_LIBRARY( sasl_parser )

