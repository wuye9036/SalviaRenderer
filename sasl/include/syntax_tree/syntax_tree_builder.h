#ifndef SASL_SYNTAX_TREE_BUILDER_H
#define SASL_SYNTAX_TREE_BUILDER_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>

#include <eflib/diagnostics/assert.h>

#include <memory>
#include <unordered_map>
#include <variant>

namespace sasl{
	namespace parser{
		class attribute;
		class lexer;
		class grammars;
	}
}

BEGIN_NS_SASL_SYNTAX_TREE();

class builder_context{
	builder_context( const builder_context& rhs, bool reset_gen_node = true )
		: parent(rhs.parent), gen_node( reset_gen_node ? std::shared_ptr<node>() : rhs.gen_node )
	{
	}

	builder_context(
		std::shared_ptr<node> parent = std::shared_ptr<node>(),
		std::shared_ptr<node> gen_node = std::shared_ptr<node>()
		): parent( parent ), gen_node( gen_node )
	{
	}

	std::shared_ptr<node> unqual_type;
	std::shared_ptr<node> parent;
	std::shared_ptr<node> gen_node;

};
class syntax_tree_builder{
public:
	syntax_tree_builder( sasl::parser::lexer& l, sasl::parser::grammars& g );
	std::shared_ptr<program> build_prog( std::shared_ptr< sasl::parser::attribute > attr );
	std::shared_ptr<function_full_def> build_fndef( std::shared_ptr<sasl::parser::attribute> attr );
	std::vector< std::shared_ptr<declaration> >
		build_decl( std::shared_ptr<sasl::parser::attribute> attr );
	std::vector< std::shared_ptr<declaration> > 
		build_basic_decl( std::shared_ptr<sasl::parser::attribute> attr );
	std::vector< std::shared_ptr<variable_declaration> >
		build_vardecl( std::shared_ptr<sasl::parser::attribute> attr );
	std::vector< std::shared_ptr<declarator> >
		build_declarators(
			std::shared_ptr<sasl::parser::attribute> attr,
			std::shared_ptr<sasl::syntax_tree::tynode> tyn,
			std::vector< std::shared_ptr<sasl::syntax_tree::variable_declaration> >& new_decls
			);

	void build_initdecl(
		std::shared_ptr<sasl::parser::attribute> attr,
		std::shared_ptr<sasl::syntax_tree::tynode> tyn,
		std::vector< std::shared_ptr<sasl::syntax_tree::declarator> >&			declarators,
		std::vector< std::shared_ptr<sasl::syntax_tree::variable_declaration> >&	declarations
		);

	std::shared_ptr<function_full_def> build_fndecl( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<parameter_full> build_param( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<struct_type> build_struct( std::shared_ptr<sasl::parser::attribute> attr );
	void build_struct_body( std::shared_ptr<sasl::parser::attribute> attr, std::shared_ptr<struct_type> out );

	std::shared_ptr<expression> build_expr( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<expression_list> build_exprlst( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<expression> build_assignexpr( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<expression> build_lcomb_expr( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<expression> dispatch_lcomb_expr( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<expression> build_rhsexpr( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<expression> build_condexpr( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<expression> build_castexpr( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<expression> build_unaryexpr( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<cast_expression> build_typecastedexpr( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<unary_expression> build_unariedexpr( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<expression> build_postexpr( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<expression> build_callexpr(
		std::shared_ptr<sasl::parser::attribute> attr,
		std::shared_ptr<expression> expr );
	std::shared_ptr<expression> build_indexexpr(
		std::shared_ptr<sasl::parser::attribute> attr,
		std::shared_ptr<expression> expr );
	std::shared_ptr<expression> build_memexpr(
		std::shared_ptr<sasl::parser::attribute> attr,
		std::shared_ptr<expression> expr );
	std::shared_ptr<expression> build_pmexpr( std::shared_ptr<sasl::parser::attribute> attr );
	
	std::shared_ptr<tynode> build_typespec( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<tynode> build_unqualedtype( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<tynode> build_prequaledtype( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<tynode> build_postqualedtype( std::shared_ptr<sasl::parser::attribute> attr );

	std::shared_ptr<initializer> build_init( std::shared_ptr<sasl::parser::attribute> attr );
	
	std::shared_ptr<statement> build_stmt( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<compound_statement> build_stmt_compound( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<jump_statement> build_flowctrl( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<expression_statement> build_stmt_expr( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<declaration_statement> build_stmt_decl( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<if_statement> build_stmt_if( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<for_statement> build_stmt_for( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<while_statement> build_stmt_while( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<dowhile_statement> build_stmt_dowhile( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<switch_statement> build_stmt_switch( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<statement> build_stmt_labeled( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<label> build_label( std::shared_ptr<sasl::parser::attribute> attr );

	std::shared_ptr<for_statement> build_for_loop( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<statement> build_for_init_decl( std::shared_ptr<sasl::parser::attribute> attr );
	std::shared_ptr<compound_statement> wrap_to_compound( std::shared_ptr<statement> stmt );

	std::shared_ptr<tynode> bind_typequal(
		std::shared_ptr<tynode> unqual,
		std::shared_ptr<sasl::parser::attribute> qual
		);

	std::shared_ptr<tynode> bind_typequal(
		std::shared_ptr<sasl::parser::attribute> qual,
		std::shared_ptr<tynode> unqual
		);

	operators build_binop(
		std::shared_ptr<sasl::parser::attribute> attr,
		std::shared_ptr<token_t>& op_tok );

	operators build_prefix_op(
		std::shared_ptr<sasl::parser::attribute> attr,
		std::shared_ptr<token_t>& op_tok );
	
	operators build_postfix_op(
		std::shared_ptr<sasl::parser::attribute> attr,
		std::shared_ptr<token_t>& op_tok );

	void build_semantic(
		std::shared_ptr<sasl::parser::attribute> const& attr,
		std::shared_ptr<sasl::common::token_t>& out_semantic,
		std::shared_ptr<sasl::common::token_t>& out_semantic_index
		);
private:
	void initialize_bt_cache();
	std::shared_ptr<builtin_type> get_builtin( std::shared_ptr<sasl::parser::attribute> const& attr );

	syntax_tree_builder& operator = ( syntax_tree_builder const& );

	sasl::parser::lexer& l;
	sasl::parser::grammars& g;

	std::unordered_map< std::string, std::shared_ptr<builtin_type> > bt_cache;
};

END_NS_SASL_SYNTAX_TREE()

#endif