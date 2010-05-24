#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/visitor.h>

BEGIN_NS_SASL_SYNTAX_TREE();

initializer::initializer( syntax_node_types type_id, boost::shared_ptr<token_attr> tok )
	: node( type_id, tok ){
}

expression_initializer::expression_initializer( boost::shared_ptr<token_attr> tok )
	: initializer( syntax_node_types::expression_initializer, tok ) {
}

void expression_initializer::accept( syntax_tree_visitor* v ){
	v->visit(*this);
}

member_initializer::member_initializer( boost::shared_ptr< token_attr > tok )
	: initializer( syntax_node_types::member_initializer, tok ) { }

void member_initializer::accept( syntax_tree_visitor* v ){
	v->visit(*this);
}

declaration::declaration( syntax_node_types type_id, boost::shared_ptr<token_attr> tok )
	: node( type_id, tok )
{
}

variable_declaration::variable_declaration( boost::shared_ptr<token_attr> tok )
	: declaration(syntax_node_types::variable_declaration, tok ) {
}

void variable_declaration::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

type_definition::type_definition( boost::shared_ptr<token_attr> tok )
	: declaration( syntax_node_types::typedef_definition, tok ){
}

void type_definition::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

type_specifier::type_specifier( syntax_node_types type_id, boost::shared_ptr<token_attr> tok )
	: declaration( type_id, tok ), value_typecode( buildin_type_code::none) { }

bool type_specifier::is_buildin() const{
	return node_class() == syntax_node_types::buildin_type;
}

buildin_type::buildin_type( boost::shared_ptr<token_attr> tok )
	: type_specifier( syntax_node_types::buildin_type, tok ){ }

void buildin_type::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

qualified_type::qualified_type( boost::shared_ptr<token_attr> tok )
	: type_specifier( syntax_node_types::qualified_type, tok ), qual(type_qualifiers::none) { }

void qualified_type::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

array_type::array_type( boost::shared_ptr<token_attr> tok )
	: type_specifier( syntax_node_types::array_type, tok ) { }

void array_type::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

struct_type::struct_type( boost::shared_ptr<token_attr> tok )
	: type_specifier( syntax_node_types::struct_type, tok ) {}

void struct_type::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

parameter::parameter( boost::shared_ptr<token_attr> tok )
	: declaration( syntax_node_types::parameter, tok ) {
}

void parameter::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}

function_type::function_type( boost::shared_ptr<token_attr> tok )
	: type_specifier( syntax_node_types::function_type, tok ), is_declaration(false)
{
}

void function_type::accept( syntax_tree_visitor* v ){
	v->visit( *this );
}
END_NS_SASL_SYNTAX_TREE();