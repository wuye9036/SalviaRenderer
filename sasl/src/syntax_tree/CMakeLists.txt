check_build_with_unicode()
set_environment()

INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${BOOST_HOME_DIR}
	${SOFTART_HOME_DIR}
)
	
set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/syntax_tree/declaration.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/expression.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/identifier.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/node.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/node_creation.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/program.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/statement.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/syntax_tree_fwd.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/syntax_tree_generator.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/visitor.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/make_tree.h
	${SASL_HOME_DIR}/sasl/include/syntax_tree/utility.h
)

set( SOURCE_FILES
	declaration.cpp
	expression.cpp
	identifier.cpp
	instanciate.cpp
	node.cpp
	program.cpp
	statement.cpp
	make_tree.cpp
	utility.cpp
)

ADD_LIBRARY( sasl_syntaxtree STATIC
	${HEADER_FILES} ${SOURCE_FILES}
)

config_output_paths( sasl_common )

