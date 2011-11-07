#ifndef SASL_SYNTAX_TREE_BUILDER_H
#define SASL_SYNTAX_TREE_BUILDER_H

#include <sasl/include/syntax_tree/syntax_tree_fwd.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/variant.hpp>
#include <eflib/include/platform/boost_end.h>

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
		: parent(rhs.parent), gen_node( reset_gen_node ? boost::shared_ptr<node>() : rhs.gen_node )
	{
	}

	builder_context(
		boost::shared_ptr<node> parent = boost::shared_ptr<node>(),
		boost::shared_ptr<node> gen_node = boost::shared_ptr<node>()
		): parent( parent ), gen_node( gen_node )
	{
	}

	boost::shared_ptr<node> unqual_type;
	boost::shared_ptr<node> parent;
	boost::shared_ptr<node> gen_node;

};
class syntax_tree_builder{
public:
	syntax_tree_builder( sasl::parser::lexer& l, sasl::parser::grammars& g );
	boost::shared_ptr<program> build_prog( boost::shared_ptr< sasl::parser::attribute > attr );
	boost::shared_ptr<declaration> build_decl( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<function_type> build_fndef( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<declaration> build_basic_decl( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<variable_declaration> build_vardecl( boost::shared_ptr<sasl::parser::attribute> attr );
	std::vector< boost::shared_ptr<declarator> > build_declarators( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<declarator> build_initdecl( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<function_type> build_fndecl( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<parameter> build_param( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<struct_type> build_struct( boost::shared_ptr<sasl::parser::attribute> attr );
	void build_struct_body( boost::shared_ptr<sasl::parser::attribute> attr, boost::shared_ptr<struct_type> out );

	boost::shared_ptr<expression> build_expr( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<expression_list> build_exprlst( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<expression> build_assignexpr( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<expression> build_lcomb_expr( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<expression> dispatch_lcomb_expr( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<expression> build_rhsexpr( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<expression> build_condexpr( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<expression> build_castexpr( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<expression> build_unaryexpr( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<cast_expression> build_typecastedexpr( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<unary_expression> build_unariedexpr( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<expression> build_postexpr( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<expression> build_callexpr(
		boost::shared_ptr<sasl::parser::attribute> attr,
		boost::shared_ptr<expression> expr );
	boost::shared_ptr<expression> build_memexpr(
		boost::shared_ptr<sasl::parser::attribute> attr,
		boost::shared_ptr<expression> expr );
	boost::shared_ptr<expression> build_pmexpr( boost::shared_ptr<sasl::parser::attribute> attr );
	
	boost::shared_ptr<tynode> build_typespec( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<tynode> build_unqualedtype( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<tynode> build_prequaledtype( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<tynode> build_postqualedtype( boost::shared_ptr<sasl::parser::attribute> attr );

	boost::shared_ptr<initializer> build_init( boost::shared_ptr<sasl::parser::attribute> attr );
	
	boost::shared_ptr<statement> build_stmt( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<compound_statement> build_stmt_compound( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<jump_statement> build_flowctrl( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<expression_statement> build_stmt_expr( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<declaration_statement> build_stmt_decl( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<if_statement> build_stmt_if( boost::shared_ptr<sasl::parser::attribute> attr );

	boost::shared_ptr<tynode> bind_typequal(
		boost::shared_ptr<tynode> unqual,
		boost::shared_ptr<sasl::parser::attribute> qual
		);

	boost::shared_ptr<tynode> bind_typequal(
		boost::shared_ptr<sasl::parser::attribute> qual,
		boost::shared_ptr<tynode> unqual
		);

	operators build_binop( boost::shared_ptr<sasl::parser::attribute> attr );
	void build_semantic(
		boost::shared_ptr<sasl::parser::attribute> const& attr,
		boost::shared_ptr<sasl::common::token_t>& out_semantic,
		boost::shared_ptr<sasl::common::token_t>& out_semantic_index
		);
private:
	void initialize_bt_cache();

	syntax_tree_builder& operator = ( syntax_tree_builder const& );

	sasl::parser::lexer& l;
	sasl::parser::grammars& g;

	boost::unordered_map< std::string, boost::shared_ptr<builtin_type> > bt_cache;
};

END_NS_SASL_SYNTAX_TREE()

#endif