#include "ast_node.h"

using namespace std;

h_ast_node_list ast_node_list::create(){
	return h_ast_node_list( new ast_node_list() );
}
ast_node_list::ast_node_list(){
}

string ast_node_list::get_typename() const{
	return string();
}

void ast_node_list::visit(ast_visitor *visitor){
}

void ast_node_list::add_node( h_ast_node node ){
	node_list_.push_back( node );
}

