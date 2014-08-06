
#include "./node_ids.h"


void register_enum_name( std::function<void (char const*, node_ids)> const& reg_fn )
{
	reg_fn("expression_statement", node_ids::expression_statement);
	reg_fn("member_expression", node_ids::member_expression);
	reg_fn("tynode", node_ids::tynode);
	reg_fn("unary_expression", node_ids::unary_expression);
	reg_fn("for_statement", node_ids::for_statement);
	reg_fn("initializer", node_ids::initializer);
	reg_fn("function_type", node_ids::function_type);
	reg_fn("variable_declaration", node_ids::variable_declaration);
	reg_fn("cond_expression", node_ids::cond_expression);
	reg_fn("case_label", node_ids::case_label);
	reg_fn("compound_statement", node_ids::compound_statement);
	reg_fn("typedef_definition", node_ids::typedef_definition);
	reg_fn("struct_type", node_ids::struct_type);
	reg_fn("label", node_ids::label);
	reg_fn("function_full_def", node_ids::function_full_def);
	reg_fn("while_statement", node_ids::while_statement);
	reg_fn("program", node_ids::program);
	reg_fn("builtin_type", node_ids::builtin_type);
	reg_fn("switch_statement", node_ids::switch_statement);
	reg_fn("statement", node_ids::statement);
	reg_fn("expression_initializer", node_ids::expression_initializer);
	reg_fn("cast_expression", node_ids::cast_expression);
	reg_fn("if_statement", node_ids::if_statement);
	reg_fn("parameter", node_ids::parameter);
	reg_fn("constant_expression", node_ids::constant_expression);
	reg_fn("node", node_ids::node);
	reg_fn("variable_expression", node_ids::variable_expression);
	reg_fn("dowhile_statement", node_ids::dowhile_statement);
	reg_fn("function_def", node_ids::function_def);
	reg_fn("parameter_full", node_ids::parameter_full);
	reg_fn("ident_label", node_ids::ident_label);
	reg_fn("declaration", node_ids::declaration);
	reg_fn("array_type", node_ids::array_type);
	reg_fn("jump_statement", node_ids::jump_statement);
	reg_fn("alias_type", node_ids::alias_type);
	reg_fn("binary_expression", node_ids::binary_expression);
	reg_fn("expression_list", node_ids::expression_list);
	reg_fn("member_initializer", node_ids::member_initializer);
	reg_fn("labeled_statement", node_ids::labeled_statement);
	reg_fn("declaration_statement", node_ids::declaration_statement);
	reg_fn("index_expression", node_ids::index_expression);
	reg_fn("declarator", node_ids::declarator);
	reg_fn("null_declaration", node_ids::null_declaration);
	reg_fn("identifier", node_ids::identifier);
	reg_fn("expression", node_ids::expression);
	reg_fn("call_expression", node_ids::call_expression);

}

