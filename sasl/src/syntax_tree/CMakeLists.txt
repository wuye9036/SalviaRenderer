set( HEADER_LIST
	../../include/sasl/syntax_tree/declaration.h
	../../include/sasl/syntax_tree/expression.h
	../../include/sasl/syntax_tree/identifier.h
	../../include/sasl/syntax_tree/node.h
	../../include/sasl/syntax_tree/node_creation.h
	../../include/sasl/syntax_tree/parse_api.h
	../../include/sasl/syntax_tree/program.h
	../../include/sasl/syntax_tree/statement.h
	../../include/sasl/syntax_tree/syntax_tree_builder.h
	../../include/sasl/syntax_tree/syntax_tree_fwd.h
	../../include/sasl/syntax_tree/visitor.h
	../../include/sasl/syntax_tree/utility.h
)

set( SOURCE_LIST
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

add_library(sasl_syntax_tree ${HEADER_LIST} ${SOURCE_LIST})
target_include_directories(sasl_syntax_tree PUBLIC ../../include)
target_link_libraries(sasl_syntax_tree eflib sasl_enums Boost::boost)
target_compile_features(sasl_syntax_tree PUBLIC cxx_std_20)

set_target_properties(sasl_syntax_tree PROPERTIES FOLDER "Shader Libs")

