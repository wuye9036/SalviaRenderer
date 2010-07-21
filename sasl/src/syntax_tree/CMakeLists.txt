check_build_with_unicode()
set_environment()

INCLUDE_DIRECTORIES(
	${sasl_home_dir}
	${boost_home_dir}
	${softart_home_dir}
)
	
set( HEADER_FILES
	${sasl_home_dir}/sasl/include/syntax_tree/declaration.h
	${sasl_home_dir}/sasl/include/syntax_tree/expression.h
	${sasl_home_dir}/sasl/include/syntax_tree/identifier.h
	${sasl_home_dir}/sasl/include/syntax_tree/node.h
	${sasl_home_dir}/sasl/include/syntax_tree/node_creation.h
	${sasl_home_dir}/sasl/include/syntax_tree/operator_table.h
	${sasl_home_dir}/sasl/include/syntax_tree/program.h
	${sasl_home_dir}/sasl/include/syntax_tree/statement.h
	${sasl_home_dir}/sasl/include/syntax_tree/syntax_tree_fwd.h
	${sasl_home_dir}/sasl/include/syntax_tree/syntax_tree_generator.h
	${sasl_home_dir}/sasl/include/syntax_tree/visitor.h
)

set( SOURCE_FILES
	declaration.cpp
	expression.cpp
	identifier.cpp
	instanciate.cpp
	node.cpp
	operator_table.cpp
	program.cpp
	statement.cpp
)

ADD_LIBRARY( sasl_syntaxtree STATIC
	${HEADER_FILES} ${SOURCE_FILES}
)

config_output_paths( sasl_common )

