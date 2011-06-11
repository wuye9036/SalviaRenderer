check_build_with_unicode()
set_environment()

INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${BOOST_HOME_DIR}
	${SOFTART_HOME_DIR}
)

set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/parser/generator.h
	${SASL_HOME_DIR}/sasl/include/parser/parser_forward.h
	${SASL_HOME_DIR}/sasl/include/parser/lexer.h
	${SASL_HOME_DIR}/sasl/include/parser/parse_api.h
	${SASL_HOME_DIR}/sasl/include/parser/grammars.h
)

set( SOURCE_FILES
	lexer.cpp
	generator.cpp
	grammars.cpp
	parse_api.cpp
)

ADD_LIBRARY( sasl_parser STATIC
	${HEADER_FILES} ${SOURCE_FILES}
)

config_output_paths( sasl_parser )

