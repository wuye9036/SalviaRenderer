#include "ast_node_literal_expression.h"


using namespace std;

h_ast_node_literal_expression ast_node_literal_expression::create( const std::string& literal, const std::string& type_suffix, const literal_types& lit_type ){
	return h_ast_node_literal_expression( new ast_node_literal_expression( literal, type_suffix, lit_type ) );
}

ast_node_literal_expression::ast_node_literal_expression(const std::string& literal, const std::string& type_suffix, const literal_types& lit_type )
:literal_(literal), type_suffix_(type_suffix), lit_type_(lit_type){
}

string ast_node_literal_expression::get_literal(){
	return literal_;
}

string ast_node_literal_expression::get_type_suffix(){
	return string();
}

string ast_node_literal_expression::get_typename() const{
	return string();
}

void ast_node_literal_expression::release(){
	delete this;
}

void ast_node_literal_expression::visit(ast_visitor* visitor){
	//do nothing
}