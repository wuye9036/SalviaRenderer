#include "ast_node_type_qualifier.h"

using namespace std;

h_ast_node_type_qualifier ast_node_type_qualifier::create(const type_qualifiers& qual){
	return h_ast_node_type_qualifier( new ast_node_type_qualifier( qual ) );
}

string ast_node_type_qualifier::get_typename() const{
	return string();
}

void ast_node_type_qualifier::visit(ast_visitor* visitor){
	//do nothing
}

void ast_node_type_qualifier::release(){
	delete this;
}

type_qualifiers ast_node_type_qualifier::get_qualifier() const{
	return qual_;
}