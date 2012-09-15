SALVIA_CHECK_BUILD_WITH_UNICODE()

set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/syntax_tree/declaration.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/expression.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/identifier.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/node.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/node_creation.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/parse_api.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/program.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/statement.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/syntax_tree_builder.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/syntax_tree_fwd.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/visitor.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/utility.h
)

set( SOURCE_FILES
	declaration.cpp
	expression.cpp
	identifier.cpp
	instanciate.cpp
	node.cpp
	parse_api.cpp
	program.cpp
	statement.cpp
	syntax_tree_builder.cpp
	utility.cpp
)

SASL_CONFIG_LIBRARY( sasl_syntaxtree )

