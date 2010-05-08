#include <sasl/include/syntax_tree/node_creation.h>
#include <sasl/include/syntax_tree/program.h>

BEGIN_NS_SASL_SYNTAX_TREE();

void instantiate(){
	create_node<program>( (const char*)0 );
}

END_NS_SASL_SYNTAX_TREE();