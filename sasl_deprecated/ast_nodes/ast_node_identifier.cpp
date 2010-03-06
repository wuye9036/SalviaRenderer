#include "ast_node.h"


using namespace std;

h_ast_node_identifier ast_node_identifier::create(const std::string& ident){
	return h_ast_node_identifier( new ast_node_identifier( ident ) );
}

ast_node_identifier::ast_node_identifier( const std::string& ident )
: ident_(ident){
}

const string& ast_node_identifier::get_ident(){
	return ident_;
}

string ast_node_identifier::get_typename() const{
	return string();
}

void ast_node_identifier::release(){
	delete this;
}

void ast_node_identifier::visit(ast_visitor *visitor){
}