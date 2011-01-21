check_build_with_unicode()
set_environment()

INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${BOOST_HOME_DIR}
	${SOFTART_HOME_DIR}
)

set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/parser/grammars.h
	${SASL_HOME_DIR}/sasl/include/parser/instancing.h
	${SASL_HOME_DIR}/sasl/include/parser/parser_forward.h
	${SASL_HOME_DIR}/sasl/include/parser/parse_api.h
	${SASL_HOME_DIR}/sasl/include/parser/grammars/declaration.h
	${SASL_HOME_DIR}/sasl/include/parser/grammars/declaration_specifier.h
	${SASL_HOME_DIR}/sasl/include/parser/grammars/expression.h
	${SASL_HOME_DIR}/sasl/include/parser/grammars/initialized_declarator_list.h
	${SASL_HOME_DIR}/sasl/include/parser/grammars/initializer.h
	${SASL_HOME_DIR}/sasl/include/parser/grammars/literal_constant.h
	${SASL_HOME_DIR}/sasl/include/parser/grammars/program.h
	${SASL_HOME_DIR}/sasl/include/parser/grammars/statement.h
	${SASL_HOME_DIR}/sasl/include/parser/grammars/token.h
	${SASL_HOME_DIR}/sasl/include/parser/detail/assignment_expression.h
	${SASL_HOME_DIR}/sasl/include/parser/detail/binary_expression.h
	${SASL_HOME_DIR}/sasl/include/parser/detail/cast_expression.h
	${SASL_HOME_DIR}/sasl/include/parser/detail/declaration.h
	${SASL_HOME_DIR}/sasl/include/parser/detail/declaration_specifier.h
	${SASL_HOME_DIR}/sasl/include/parser/detail/expression.h
	${SASL_HOME_DIR}/sasl/include/parser/detail/expression_list.h
	${SASL_HOME_DIR}/sasl/include/parser/detail/initialized_declarator_list.h
	${SASL_HOME_DIR}/sasl/include/parser/detail/initializer.h
	${SASL_HOME_DIR}/sasl/include/parser/detail/literal_constant.h
	${SASL_HOME_DIR}/sasl/include/parser/detail/primary_expression.h
	${SASL_HOME_DIR}/sasl/include/parser/detail/program.h
	${SASL_HOME_DIR}/sasl/include/parser/detail/statement.h
	${SASL_HOME_DIR}/sasl/include/parser/detail/struct_declaration.h
	${SASL_HOME_DIR}/sasl/include/parser/detail/token.h
	${SASL_HOME_DIR}/sasl/include/parser/detail/variable_declaration.h
	${SASL_HOME_DIR}/sasl/include/parser_tree/declaration.h
	${SASL_HOME_DIR}/sasl/include/parser_tree/declaration_specifier.h
	${SASL_HOME_DIR}/sasl/include/parser_tree/declarator.h
	${SASL_HOME_DIR}/sasl/include/parser_tree/expression.h
	${SASL_HOME_DIR}/sasl/include/parser_tree/initializer.h
	${SASL_HOME_DIR}/sasl/include/parser_tree/literal.h
	${SASL_HOME_DIR}/sasl/include/parser_tree/parser_tree_forward.h
	${SASL_HOME_DIR}/sasl/include/parser_tree/program.h
	${SASL_HOME_DIR}/sasl/include/parser_tree/statement.h
)

set( SOURCE_FILES
	assignment_expression.cpp
	binary_expression.cpp
	cast_expression.cpp
	declaration.cpp
	declaration_specifier.cpp
	expression.cpp
	expression_list.cpp
	initialization_declarator_list.cpp
	initializer.cpp
	literal_constant.cpp
	primary_expression.cpp
	program.cpp
	statement.cpp
	struct_declaration.cpp
	token.cpp
	variable_declaration.cpp
	parse_api.cpp
)

ADD_LIBRARY( sasl_parser STATIC
	${HEADER_FILES} ${SOURCE_FILES}
)

config_output_paths( sasl_parser )

