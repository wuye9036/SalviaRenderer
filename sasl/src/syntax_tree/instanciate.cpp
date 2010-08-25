#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/make_tree.h>
#include <sasl/include/syntax_tree/node_creation.h>
#include <sasl/include/syntax_tree/program.h>

BEGIN_NS_SASL_SYNTAX_TREE();

using sasl::common::token_attr;

void instantiate(){
	create_node<program>( (const char*)0 );
	create_node<variable_declaration>( token_attr::null() );
	create_node<parameter>( token_attr::null() );
}

END_NS_SASL_SYNTAX_TREE();