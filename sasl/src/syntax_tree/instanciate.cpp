#include <sasl/syntax_tree/declaration.h>
#include <sasl/syntax_tree/expression.h>
#include <sasl/syntax_tree/node_creation.h>
#include <sasl/syntax_tree/statement.h>
#include <sasl/syntax_tree/program.h>

namespace sasl::syntax_tree {

using sasl::common::token;

void instantiate(){
	create_node<program>( token::null(), token::null() );
	create_node<variable_declaration>( token::null(), token::null() );
	create_node<parameter_full>( token::null(), token::null() );
	create_node<expression_list>( token::null(), token::null() );
	create_node<labeled_statement>( token::null(), token::null() );
	create_node<function_def>( token::null(), token::null() );
	create_node<parameter>( token::null(), token::null() );
}

}
