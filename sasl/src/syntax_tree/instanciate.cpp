#include <sasl/syntax_tree/declaration.h>
#include <sasl/syntax_tree/expression.h>
#include <sasl/syntax_tree/node_creation.h>
#include <sasl/syntax_tree/statement.h>
#include <sasl/syntax_tree/program.h>

namespace sasl::syntax_tree {

using sasl::common::token;

void instantiate(){
	create_node<program>( token::make_empty(), token::make_empty() );
	create_node<variable_declaration>( token::make_empty(), token::make_empty() );
	create_node<parameter_full>( token::make_empty(), token::make_empty() );
	create_node<expression_list>( token::make_empty(), token::make_empty() );
	create_node<labeled_statement>( token::make_empty(), token::make_empty() );
	create_node<function_def>( token::make_empty(), token::make_empty() );
	create_node<parameter>( token::make_empty(), token::make_empty() );
}

}
