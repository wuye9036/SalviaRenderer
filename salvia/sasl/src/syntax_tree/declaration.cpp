#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/visitor.h>

BEGIN_NS_SASL_SYNTAX_TREE();

initializer::initializer( syntax_node_types type_id, boost::shared_ptr<token_t> tok )
	: node( type_id, tok ){
}

expression_initializer::expression_initializer( boost::shared_ptr<token_t> tok )
	: initializer( syntax_node_types::expression_initializer, tok ) {
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( expression_initializer );

member_initializer::member_initializer( boost::shared_ptr< token_t > tok )
	: initializer( syntax_node_types::member_initializer, tok ) { }

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( member_initializer );

declaration::declaration( syntax_node_types type_id, boost::shared_ptr<token_t> tok )
	: node( type_id, tok )
{
}

declarator::declarator( boost::shared_ptr<token_t> tok ): node(syntax_node_types::declarator, tok) {}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( declarator );

variable_declaration::variable_declaration( boost::shared_ptr<token_t> tok )
	: declaration(syntax_node_types::variable_declaration, tok ) {
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( variable_declaration );

type_definition::type_definition( boost::shared_ptr<token_t> tok )
	: declaration( syntax_node_types::typedef_definition, tok ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( type_definition );

type_specifier::type_specifier( syntax_node_types type_id, boost::shared_ptr<token_t> tok )
	: declaration( type_id, tok ),
	value_typecode( builtin_type_code::none),
	qual( type_qualifiers::none )
{ }

bool type_specifier::is_builtin() const{
	return node_class() == syntax_node_types::builtin_type;
}

bool type_specifier::is_struct() const{
	return node_class() == syntax_node_types::struct_type;
}

bool type_specifier::is_array() const{
	return node_class() == syntax_node_types::array_type;
}

bool type_specifier::is_function() const{
	return node_class() == syntax_node_types::function_type;
}

bool type_specifier::is_alias() const{
	return node_class() == syntax_node_types::alias_type;
}

bool type_specifier::is_uniform() const
{
	return qual.included( type_qualifiers::_uniform );
}

builtin_type::builtin_type( boost::shared_ptr<token_t> tok )
	: type_specifier( syntax_node_types::builtin_type, tok ){ }

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( builtin_type );

array_type::array_type( boost::shared_ptr<token_t> tok )
	: type_specifier( syntax_node_types::array_type, tok ) { }

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( array_type );

struct_type::struct_type( boost::shared_ptr<token_t> tok )
	: type_specifier( syntax_node_types::struct_type, tok ) {}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( struct_type );

parameter::parameter( boost::shared_ptr<token_t> tok )
	: declaration( syntax_node_types::parameter, tok ) {
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( parameter );

function_type::function_type( boost::shared_ptr<token_t> tok )
	: type_specifier( syntax_node_types::function_type, tok )
{
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( function_type );

bool function_type::declaration_only(){
	return body || body->stmts.empty();
}

null_declaration::null_declaration( boost::shared_ptr<token_t> tok )
: declaration( syntax_node_types::null_declaration, tok ){
}

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( null_declaration );

SASL_SYNTAX_NODE_ACCEPT_METHOD_IMPL( alias_type );

alias_type::alias_type( boost::shared_ptr<token_t> tok )
: type_specifier( syntax_node_types::alias_type, tok ){
}
END_NS_SASL_SYNTAX_TREE();