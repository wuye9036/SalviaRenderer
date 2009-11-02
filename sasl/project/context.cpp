#include "context.h"

using namespace std;

context::context(unit* owner) : current_unit_(owner)
{
}

const string& context::get_current_line(){
	return current_line_;
}

void context::set_current_line(const std::string& line_str){
	current_line_ = line_str;
}

int context::get_current_line_idx(){
	return current_line_idx_;
}

void context::set_current_line_idx(int line_idx){
	current_line_idx_ = line_idx;
}

context::code_pos context::get_code_pos(){
	return code_pos(filename_, current_line_idx_);
}

void context::set_code_pos( const code_pos& pos){
	filename_ = pos.filename;
	current_line_idx_ = pos.line_idx;
}

vector<h_ast_node>& context::get_node_stack(){
	return node_stack_;
}