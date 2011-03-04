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

	boost::shared_ptr<expression> build_expr( boost::shared_ptr<sasl::parser::attribute> attr );

	boost::shared_ptr<type_specifier> build_typespec( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<type_specifier> build_unqualedtype( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<type_specifier> build_prequaledtype( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<type_specifier> build_postqualedtype( boost::shared_ptr<sasl::parser::attribute> attr );

	boost::shared_ptr<initializer> build_init( boost::shared_ptr<sasl::parser::attribute> attr );
	
	boost::shared_ptr<statement> build_stmt( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<compound_statement> build_stmt_compound( boost::shared_ptr<sasl::parser::attribute> attr );
	boost::shared_ptr<jump_statement> build_flowctrl( boost::shared_ptr<sasl::parser::attribute> attr );

	boost::shared_ptr<type_specifier> bind_typequal(
		boost::shared_ptr<type_specifier> unqual,
		boost::shared_ptr<sasl::parser::attribute> qual
		);

	boost::shared_ptr<type_specifier> bind_typequal(
		boost::shared_ptr<sasl::parser::attribute> qual,
		boost::shared_ptr<type_specifier> unqual
		);

private:
	syntax_tree_builder& operator = ( syntax_tree_builder const& );
	sasl::parser::lexer& l;
	sasl::parser::grammars& g;
};
//
//struct syntax_tree_builder;
//
//struct ast_builder_context{
//

//};
//
//template<typename ResultT>
//struct parser_tree_variant_visitor: public boost::static_visitor< ResultT >{
//	typedef parser_tree_variant_visitor<ResultT> base_visitor;
//	parser_tree_variant_visitor( syntax_tree_builder* builder, ast_builder_context& ctxt ): builder(builder), ctxt(ctxt){}
//protected:
//	syntax_tree_builder* builder;
//	ast_builder_context& ctxt;
//private:
//	parser_tree_variant_visitor<ResultT>& operator = ( parser_tree_variant_visitor<ResultT> const & );
//	parser_tree_variant_visitor( parser_tree_variant_visitor<ResultT> const & );
//};
//
//template <typename ResultT>
//struct dispatch_visitor: public parser_tree_variant_visitor<ResultT>{
//	dispatch_visitor( syntax_tree_builder* builder, ast_builder_context& ctxt )
//		: base_visitor( builder, ctxt ){}
//	template<typename T>
//	ResultT operator() ( const T& v ){
//		return builder->build(v, ctxt);
//	}
//};
//
//template <typename ResultT>
//struct umimpl_visitor: public parser_tree_variant_visitor<ResultT>{
//	umimpl_visitor() : base_visitor( builder ){}
//	template<typename T>
//	ResultT operator() ( const T& v ){
//		EFLIB_ASSERT_UNIMPLEMENTED();
//		return ResultT();
//	}
//};
//
//// normal expression variant extractor
//struct expression_variant_visitor: public parser_tree_variant_visitor< boost::shared_ptr<expression> >{
//	typedef token constant_t;
//
//	expression_variant_visitor( syntax_tree_builder* builder, ast_builder_context& ctxt );
//
//	// this is an error condition.
//	boost::shared_ptr<expression> operator()( const int& v);
//
//	// for constant
//	boost::shared_ptr<expression> operator()( const constant_t& v );
//
//	// for normal expression
//	template <typename ExpressionT>	boost::shared_ptr<expression> operator()( const ExpressionT& v );
//};
//
//struct postfix_qualified_expression_visitor: public parser_tree_variant_visitor< boost::shared_ptr<expression> >{
//	postfix_qualified_expression_visitor( syntax_tree_builder* builder, ast_builder_context& ctxt );
//
//	boost::shared_ptr<expression> operator()( const sasl::parser_tree::idx_expression& v );
//	boost::shared_ptr<expression> operator()( const sasl::parser_tree::call_expression& v);
//	boost::shared_ptr<expression> operator()( const sasl::parser_tree::mem_expression& v);
//	boost::shared_ptr<expression> operator()( const token& v);
//
//private:
//	template< typename STPostExpressionT, typename ExpressionPostT  >
//	boost::shared_ptr<expression> composite( const ExpressionPostT& v );
//
//	boost::shared_ptr<expression> expr;
//};
//
//struct statement_visitor: public parser_tree_variant_visitor< boost::shared_ptr<statement> >{
//	statement_visitor( syntax_tree_builder* builder, ast_builder_context& ctxt )
//		: base_visitor( builder, ctxt ){}
//
//	template<typename T>
//	boost::shared_ptr<statement> operator() ( const T& /*v*/ ){
//		EFLIB_ASSERT_UNIMPLEMENTED();
//		return boost::shared_ptr<statement>();
//		// return builder->build(v, ctxt);
//	}
//
//	boost::shared_ptr<statement> operator() ( const ::sasl::parser_tree::declaration_statement& v );
//};
//
//struct type_visitor : public parser_tree_variant_visitor< boost::shared_ptr<type_specifier> >
//{
//	type_visitor( syntax_tree_builder* builder, ast_builder_context& ctxt )
//		: base_visitor( builder, ctxt ){}
//
//	template<typename T>
//	boost::shared_ptr<type_specifier> operator() ( T const& v ){
//		return builder->build(v, ctxt)->typed_handle<type_specifier>();
//	}
//
//	boost::shared_ptr<type_specifier> operator() ( token const& v );
//private:
//	static boost::unordered_map< std::string, boost::shared_ptr<buildin_type> > typemap;
//};
//
//struct syntax_tree_builder{
//	/*******************/
//	/* terminals build */
//	/*******************/
//	operators build_operator( const sasl::common::token& v, operators modifier = operators::none );
//	boost::shared_ptr<constant_expression> build_constant( const sasl::common::token& v );
//
//	/*************************/
//	/* non-terminals builder */
//	/*************************/
//
//	//build expressions
//	boost::shared_ptr<expression> build( const sasl::parser_tree::unary_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::cast_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::mul_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::add_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::shf_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::rel_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::eql_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::band_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::bxor_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::bor_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::land_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::lor_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::rhs_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::assign_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::expression_post& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::pm_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::typecast_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::post_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::cond_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::idx_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::call_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::mem_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::unaried_expression& v, ast_builder_context& ctxt );
//	boost::shared_ptr<expression> build( const sasl::parser_tree::paren_expression& v, ast_builder_context& ctxt );
//
//	// build statements
//	boost::shared_ptr<statement> build( const sasl::parser_tree::statement& v, ast_builder_context& ctxt );
//	boost::shared_ptr<statement> build( const sasl::parser_tree::if_statement& v, ast_builder_context& ctxt );
//
//	// build declarations
//	boost::shared_ptr<declaration> build( const sasl::parser_tree::declaration& v, ast_builder_context& ctxt );
//	boost::shared_ptr<declaration> build( const sasl::parser_tree::function_definition& v, ast_builder_context& ctxt );
//	boost::shared_ptr<declaration> build( const sasl::parser_tree::basic_declaration& v, ast_builder_context& ctxt );
//	boost::shared_ptr<declaration> build( const sasl::parser_tree::struct_declaration& v, ast_builder_context& ctxt );
//	boost::shared_ptr<declaration> build( const sasl::parser_tree::variable_declaration& v, ast_builder_context& ctxt );
//	boost::shared_ptr<declaration> build( const sasl::parser_tree::function_declaration& v, ast_builder_context& ctxt );
//	boost::shared_ptr<declaration> build( const sasl::parser_tree::typedef_declaration& v, ast_builder_context& ctxt );
//	boost::shared_ptr<declarator> build( const sasl::parser_tree::initialized_declarator& v, ast_builder_context& ctxt );
//	
//	// build initialzier
//	boost::shared_ptr<initializer> build( sasl::parser_tree::initializer const & v, ast_builder_context& ctxt );
//
//	// build type specifier & declaration specifier
//	boost::shared_ptr<type_specifier> build( const sasl::parser_tree::declaration_specifier& v, ast_builder_context& ctxt );
//	boost::shared_ptr<type_specifier> build( sasl::parser_tree::prefix_qualified_type const & v, ast_builder_context& ctxt );
//	boost::shared_ptr<type_specifier> build( sasl::parser_tree::unqualified_type const & v, ast_builder_context& ctxt );
//	boost::shared_ptr<type_specifier> build( sasl::parser_tree::paren_post_qualified_type const & v, ast_builder_context& ctxt );
//
//	// build program
//	boost::shared_ptr<program> build( const sasl::parser_tree::program& v);
//
//private:
//	template <typename ExpressionT>
//	boost::shared_ptr<expression> build_variant_expression( const ExpressionT& v, ast_builder_context& ctxt ){
//		expression_variant_visitor visitor(this, ctxt);
//		return boost::apply_visitor( visitor, v );
//	}
//
//	template <typename BinaryExpressionT>
//	boost::shared_ptr<expression> build_binary_expression( const BinaryExpressionT& v, ast_builder_context& ctxt ){
//		boost::shared_ptr<expression> ret_expr = build( v.first_expr, ctxt );
//		for (size_t i_exprlist = 0; i_exprlist < v.follow_exprs.size(); ++i_exprlist){
//			operators op = build_operator( boost::fusion::at_c<0>(v.follow_exprs[i_exprlist]) );
//			boost::shared_ptr<expression> expr = build( boost::fusion::at_c<1>( v.follow_exprs[i_exprlist] ), ctxt );
//			// ret_expr = composite_binary_expression( ret_expr, op, expr );
//		}
//		return ret_expr;
//	}
//
//	template <typename RightCombineBinaryExpressionT>
//	boost::shared_ptr<expression> build_right_combine_binary_expression(
//		const RightCombineBinaryExpressionT& v, ast_builder_context& ctxt )
//	{
//		return build_right_combine_binary_expression(
//			v.first_expr,
//			v.follow_exprs.begin(), v.follow_exprs.end(),
//			ctxt
//			);
//	}
//
//	template <typename ExpressionListIteratorT, typename ExpressionT>
//	boost::shared_ptr<expression> build_right_combine_binary_expression(
//		const ExpressionT& lexpr,
//		ExpressionListIteratorT follow_begin, ExpressionListIteratorT follow_end,
//		ast_builder_context& ctxt )
//	{
//		boost::shared_ptr<expression> left_expr = build( lexpr, ctxt );
//		
//		if( follow_begin == follow_end ){
//			return left_expr;
//		}
//
//		boost::shared_ptr<expression> right_expr = build_right_combine_binary_expression(
//			boost::fusion::at_c<1>(*follow_begin),
//			follow_begin + 2, follow_end,
//			ctxt
//			);
//
//		operators op = build_operator( boost::fusion::at_c<0>(*follow_begin) );
//		return boost::shared_ptr<expression>();//composite_binary_expression( left_expr, op, right_expr );
//	}
//
//	boost::shared_ptr<expression> composite_binary_expression(
//		const boost::shared_ptr<expression>& lexpr,
//		const boost::shared_ptr<token>& op,
//		const boost::shared_ptr<expression>& rexpr,
//		ast_builder_context& /*ctxt*/
//		)
//	{
//		boost::shared_ptr<binary_expression> composited_expr = create_node<binary_expression>(op);
//		composited_expr->left_expr = lexpr;
//		composited_expr->right_expr = rexpr;
//		composited_expr->op = build_operator(*op, operators::none);
//
//		return composited_expr->typed_handle<expression>();
//	}
//
//	template <typename ExpressionPostT>
//	boost::shared_ptr<expression> append_expression_post(
//		const boost::shared_ptr<expression> expr,
//		const ExpressionPostT& post,
//		ast_builder_context& ctxt
//		)
//	{
//		postfix_qualified_expression_visitor visitor( this, ctxt );
//		return boost::apply_visitor( visitor, post );
//	}
//
//	boost::shared_ptr<statement> build_for_initializer( const sasl::parser_tree::for_initializer& /*v*/, for_statement* /*for_stmt*/ ){
//		// for_stmt->looper.for_init
//	}
//
//	boost::shared_ptr<type_specifier> qualify( sasl::parser_tree::postfix_qualified_type::qualifiers_t const & quals, boost::shared_ptr<type_specifier> inner );
//	boost::shared_ptr<type_specifier> qualify( sasl::parser_tree::prefix_qualified_type::qualifiers_t const & quals, boost::shared_ptr<type_specifier> inner );
//};

END_NS_SASL_SYNTAX_TREE()

#endif