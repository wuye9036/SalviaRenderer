#include <sasl/include/semantic/semantic_analyser.h>

#include <sasl/enums/operators.h>

#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/semantic_diags.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol_scope.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/semantic/caster.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/make_tree.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/utility.h>

#include <salviar/include/enums.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/metaprog/util.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/any.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/assign/list_inserter.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/bind/apply.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/scoped_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SASL_SEMANTIC();

using salviar::semantic_value;

using ::sasl::common::diag_chat;
using ::sasl::common::token_t;

using ::sasl::syntax_tree::alias_type;
using ::sasl::syntax_tree::binary_expression;
using ::sasl::syntax_tree::builtin_type;
using ::sasl::syntax_tree::call_expression;
using ::sasl::syntax_tree::case_label;
using ::sasl::syntax_tree::cast_expression;
using ::sasl::syntax_tree::cond_expression;
using ::sasl::syntax_tree::create_builtin_type;
using ::sasl::syntax_tree::create_node;
using ::sasl::syntax_tree::compound_statement;
using ::sasl::syntax_tree::constant_expression;
using ::sasl::syntax_tree::declaration;
using ::sasl::syntax_tree::declarator;
using ::sasl::syntax_tree::declaration_statement;
using ::sasl::syntax_tree::dowhile_statement;
using ::sasl::syntax_tree::expression;
using ::sasl::syntax_tree::expression_initializer;
using ::sasl::syntax_tree::expression_statement;
using ::sasl::syntax_tree::for_statement;
using ::sasl::syntax_tree::function_type;
using ::sasl::syntax_tree::if_statement;
using ::sasl::syntax_tree::jump_statement;
using ::sasl::syntax_tree::label;
using ::sasl::syntax_tree::labeled_statement;
using ::sasl::syntax_tree::member_expression;
using ::sasl::syntax_tree::node;
using ::sasl::syntax_tree::parameter;
using ::sasl::syntax_tree::program;
using ::sasl::syntax_tree::statement;
using ::sasl::syntax_tree::struct_type;
using ::sasl::syntax_tree::switch_statement;
using ::sasl::syntax_tree::tynode;
using ::sasl::syntax_tree::unary_expression;
using ::sasl::syntax_tree::variable_declaration;
using ::sasl::syntax_tree::variable_expression;
using ::sasl::syntax_tree::while_statement;

using ::sasl::syntax_tree::dfunction_combinator;

using namespace boost::assign;
using namespace sasl::utility;

using boost::any;
using boost::any_cast;
using boost::format;
using boost::shared_ptr;
using boost::weak_ptr;
using boost::unordered_map;

using std::vector;
using std::string;
using std::pair;
using std::make_pair;

#define SASL_GET_OR_CREATE_SI( si_type, si_var, node ) shared_ptr<si_type> si_var = get_or_create_semantic_info<si_type>(node);
#define SASL_GET_OR_CREATE_SI_P( si_type, si_var, node, param ) shared_ptr<si_type> si_var = get_or_create_semantic_info<si_type>( node, param );

#define SASL_EXTRACT_SI( si_type, si_var, node ) shared_ptr<si_type> si_var = extract_semantic_info<si_type>(node);

// semantic analysis context
struct sacontext{
	sacontext(): declarator_type_id(-1), is_global(true), member_index(-1), lbl_list(NULL){
	}

	shared_ptr<symbol>			parent_sym;
	shared_ptr<function_type>	parent_fn;

	std::vector< weak_ptr<labeled_statement> >* lbl_list;

	shared_ptr<node> generated_node;
	shared_ptr<node> variable_to_fill; // for initializer only.
	tid_t declarator_type_id;

	int member_index;

	bool is_global;
};

sacontext* ctxt_ptr( any& any_val ){
	return any_cast<sacontext>(&any_val);
}

sacontext const * ctxt_ptr( const any& any_val ){
	return any_cast<sacontext>(&any_val);
}

#define data_cptr() ( ctxt_ptr(*data) )

// utility functions

shared_ptr<tynode> type_info_of( shared_ptr<node> n ){
	shared_ptr<type_info_si> typesi = extract_semantic_info<type_info_si>( n );
	if ( typesi ){
		return typesi->type_info();
	}
	return shared_ptr<tynode>();
}

semantic_analyser::semantic_analyser()
{
	caster.reset( new caster_t() );
}

#define SASL_VISITOR_TYPE_NAME semantic_analyser

template <typename NodeT> any& semantic_analyser::visit_child( any& child_ctxt, const any& init_data, shared_ptr<NodeT> child )
{
	child_ctxt = init_data;
	return visit_child( child_ctxt, child );
}

template <typename NodeT> any& semantic_analyser::visit_child( any& child_ctxt, shared_ptr<NodeT> child )
{
	child->accept( this, &child_ctxt );
	return child_ctxt;
}

template <typename NodeT> any& semantic_analyser::visit_child(
	any& child_ctxt, const any& init_data,
	shared_ptr<NodeT> child, shared_ptr<NodeT>& generated_node )
{
	visit_child( child_ctxt, init_data, child );
	if( ctxt_ptr( child_ctxt )->generated_node ){
		generated_node = ctxt_ptr(child_ctxt)->generated_node->as_handle<NodeT>();
	}
	return child_ctxt;
}

void semantic_analyser::parse_semantic(
	shared_ptr<token_t> const& sem_tok,
	shared_ptr<token_t> const& sem_idx_tok,
	shared_ptr<storage_si> const& ssi
	)
{
	if( sem_tok ){
		salviar::semantic_value sem( salviar::sv_none );
		string semstr = sem_tok->str;
		size_t index = 0;
		if( sem_idx_tok ){
			index = boost::lexical_cast<size_t>(sem_idx_tok->str);
		} else {
			// Try to get last digitals for generate index.
			string::const_reverse_iterator it = semstr.rbegin();

			size_t num_tail_length = 0;
			char ch = '\0';
			while( ch = boost::is_digit()(*it) ){
				++it;
				++num_tail_length;
			}

			if( num_tail_length > 0 ){
				size_t split_pos = semstr.size() - num_tail_length;

				index = boost::lexical_cast<size_t>( semstr.substr( split_pos ) );
				semstr = semstr.substr( 0, split_pos );
			}
		}
		ssi->set_semantic( semantic_value( semstr, static_cast<uint32_t>(index ) ) );
	}
}

SASL_VISIT_DEF( unary_expression ){
	any child_ctxt_init = *data;

	shared_ptr<unary_expression> dup_expr = duplicate( v.as_handle() )->as_handle<unary_expression>();
	
	any child_ctxt;
	visit_child( child_ctxt, child_ctxt_init, v.expr, dup_expr->expr );

	type_info_si* inner_tisi = dup_expr->expr->si_ptr<type_info_si>();
	if( v.op == operators::prefix_incr || v.op == operators::postfix_incr || v.op == operators::prefix_decr || v.op == operators::postfix_decr ){
		if( !is_integer( inner_tisi->type_info()->tycode ) ){
			// REPORT ERROR
			EFLIB_ASSERT_UNIMPLEMENTED();
			return;
		}
	}

	SASL_GET_OR_CREATE_SI_P( storage_si, ssi, dup_expr, msi->pety() );
	ssi->entry_id( inner_tisi->entry_id() );

	data_cptr()->generated_node = dup_expr->as_handle();
}

SASL_VISIT_DEF( cast_expression ){
	any child_ctxt_init = *data;
	
	shared_ptr<cast_expression> dup_cexpr = duplicate(v.as_handle())->as_handle<cast_expression>();
	
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.casted_type, dup_cexpr->casted_type );
	visit_child( child_ctxt, child_ctxt_init, v.expr, dup_cexpr->expr );

	shared_ptr<type_info_si> src_tsi = extract_semantic_info<type_info_si>( dup_cexpr->expr );
	shared_ptr<type_info_si> casted_tsi = extract_semantic_info<type_info_si>( dup_cexpr->casted_type );

	if( src_tsi->entry_id() != casted_tsi->entry_id() ){
		if( caster->try_cast( casted_tsi->entry_id(), src_tsi->entry_id() ) == caster_t::nocast ){
			// Here is code error. Compiler should report it.
			EFLIB_ASSERT_UNIMPLEMENTED();
			return;
		}
	}

	SASL_GET_OR_CREATE_SI_P( storage_si, ssi, dup_cexpr, msi->pety() );
	ssi->entry_id( casted_tsi->entry_id() );

	data_cptr()->generated_node = dup_cexpr->as_handle();
}

SASL_VISIT_DEF( binary_expression )
{
	any child_ctxt_init = *data;
	ctxt_ptr(child_ctxt_init)->generated_node.reset();

	shared_ptr<binary_expression> dup_expr = duplicate( v.as_handle() )->as_handle<binary_expression>();
	data_cptr()->generated_node = dup_expr->as_handle();

	any child_ctxt;
	visit_child( child_ctxt, child_ctxt_init, v.left_expr, dup_expr->left_expr );
	visit_child( child_ctxt, child_ctxt_init, v.right_expr, dup_expr->right_expr );

	std::string opname = operator_name( v.op );
	vector< shared_ptr<expression> > exprs;
	exprs += dup_expr->left_expr, dup_expr->right_expr;
	
	if( dup_expr->left_expr->si_ptr<semantic_info>() == NULL 
		|| dup_expr->right_expr->si_ptr<semantic_info>() == NULL )
	{
		return;
	}

	vector< shared_ptr<symbol> > overloads;

	if( is_assign(v.op) || is_arith_assign(v.op) ){
		overloads = data_cptr()->parent_sym->find_assign_overloads( opname, caster, exprs, get_diags() );
	} else {
		overloads = data_cptr()->parent_sym->find_overloads( opname, caster, exprs, get_diags() );
	}
	
	if( overloads.empty() )
	{
		args_type_repr atr;
		for( size_t i = 0; i < 2 /*binary operator*/; ++i )
		{
			atr.arg( exprs[i]->si_ptr<type_info_si>()->type_info() );
		}
		diags->report( operator_param_unmatched )->token_range( *v.token_begin(), *v.token_end() )->p( atr.str() );
	}
	else if ( overloads.size() > 1 )
	{
		diags->report( operator_multi_overloads )->token_range( *v.token_begin(), *v.token_end() )->p( overloads.size() );
	}
	else
	{
		tid_t result_tid = extract_semantic_info<type_info_si>( overloads[0]->node() )->entry_id();
		get_or_create_semantic_info<storage_si>( dup_expr, msi->pety() )->entry_id( result_tid );
	}
}

SASL_VISIT_DEF_UNIMPL( expression_list );

SASL_VISIT_DEF( cond_expression ){
	any child_ctxt_init = *data;
	ctxt_ptr(child_ctxt_init)->generated_node.reset();

	shared_ptr<cond_expression> dup_expr
		= duplicate( v.as_handle() )->as_handle<cond_expression>();

	any child_ctxt;
	visit_child( child_ctxt, child_ctxt_init, v.cond_expr, dup_expr->cond_expr );
	visit_child( child_ctxt, child_ctxt_init, v.yes_expr, dup_expr->yes_expr );
	visit_child( child_ctxt, child_ctxt_init, v.no_expr, dup_expr->no_expr );
	
	// SEMANTIC_TODO Test conversation between type of yes expression and no expression.
	type_info_si* cond_tisi = dup_expr->cond_expr->si_ptr<type_info_si>();
	type_info_si* yes_tisi = dup_expr->yes_expr->si_ptr<type_info_si>();
	type_info_si* no_tisi = dup_expr->no_expr->si_ptr<type_info_si>();
 
	assert( cond_tisi && yes_tisi && no_tisi );

	if( !(cond_tisi && yes_tisi && no_tisi) ) {
		return;
	}

	tid_t bool_tid = msi->pety()->get( builtin_types::_boolean );
	tid_t cond_tid = cond_tisi->entry_id();

	if( cond_tid != bool_tid && !caster->try_implicit( cond_tisi->entry_id(), bool_tid ) ){
		diags->report( cannot_convert_type_from )
			->token_range( *dup_expr->cond_expr->token_begin(), *dup_expr->cond_expr->token_end() )
			->p("?")->p(type_repr(cond_tisi->type_info()).str())->p("bool");
	}
	
	SASL_GET_OR_CREATE_SI_P( storage_si, ssi, dup_expr, msi->pety() );

	tid_t yes_tid = yes_tisi->entry_id();
	tid_t no_tid = no_tisi->entry_id();

	if( yes_tid == no_tid ){
		ssi->entry_id( yes_tid );
	} else if( caster->try_implicit(yes_tid, no_tid) ){
		ssi->entry_id( yes_tid );
	} else if( caster->try_implicit(no_tid, yes_tid) ){
		ssi->entry_id( no_tid );
	} else {
		diags->report(cannot_convert_type_from)
			->token_range( *dup_expr->yes_expr->token_begin(), *dup_expr->no_expr->token_end() )
			->p(":")->p(type_repr(no_tisi->type_info()).str())->p(type_repr(yes_tisi->type_info()).str());
	}

	data_cptr()->generated_node = dup_expr;
}

SASL_VISIT_DEF_UNIMPL( index_expression );

SASL_VISIT_DEF( call_expression )
{
	any child_ctxt_init = *data;
	ctxt_ptr( child_ctxt_init )->generated_node.reset();

	any child_ctxt;

	shared_ptr<call_expression> dup_callexpr = duplicate(v.as_handle())->as_handle<call_expression>();
	visit_child( child_ctxt, child_ctxt_init, v.expr, dup_callexpr->expr );

	dup_callexpr->args.clear();
	BOOST_FOREACH( shared_ptr<expression> arg_expr, v.args )
	{
		visit_child( child_ctxt, child_ctxt_init, arg_expr );
		dup_callexpr->args.push_back( ctxt_ptr(child_ctxt)->generated_node->as_handle<expression>() );
	}

	fnvar_si* fnsi = dup_callexpr->expr->dyn_siptr<fnvar_si>();
	if( fnsi == NULL ){
		EFLIB_ASSERT_UNIMPLEMENTED();
		// Maybe pointer of function.
	} else {
		// Overload
		vector< shared_ptr<symbol> > syms = fnsi->scope()->find_overloads( fnsi->name(), caster, dup_callexpr->args, get_diags() );
		
		if( syms.empty() )
		{
			args_type_repr atr;
			for( size_t i = 0; i < dup_callexpr->args.size(); ++i )
			{
				atr.arg( dup_callexpr->args[i]->si_ptr<type_info_si>()->type_info() );
			}
			diags->report( function_param_unmatched )->token_range( *v.token_begin(), *v.token_end() )->p( fnsi->name() )->p( atr.str() );
		}
		else if ( syms.size() > 1 )
		{
			diags->report( function_multi_overloads )->token_range( *v.token_begin(), *v.token_end() )->p( fnsi->name() )->p( syms.size() );
		}
		else
		{
			shared_ptr<symbol> func_sym = syms[0];
			assert( func_sym );

			mark_intrin_invoked_recursive( func_sym );
			storage_si* ssi = func_sym->node()->si_ptr<storage_si>();
			SASL_GET_OR_CREATE_SI_P( call_si, csi, dup_callexpr, msi->pety() );

			csi->entry_id( ssi->entry_id() );
			csi->is_function_pointer(false);
			csi->overloaded_function( func_sym.get() );
		}
	}

	data_cptr()->generated_node = dup_callexpr->as_handle();
}

int check_swizzle( builtin_types btc, std::string const& mask, int32_t& swizzle_code ){
	swizzle_code = 0;

	if( mask.length() > 4 ){
		return 0;
	}

	size_t agg_size = 0;
	if( is_scalar(btc) ){
		agg_size = 1;
	} else if( is_vector(btc) ){
		agg_size = vector_size( btc );
	} else if( is_matrix(btc) ){
		agg_size = vector_count( btc );
	}

	if( agg_size == 0 ){ return 0; }

	int min_src_size = 0;
	int dest_size = 0;
	swizzle_code = encode_swizzle( dest_size, min_src_size, mask.c_str() );
	
	if( min_src_size > static_cast<int>(agg_size) )
	{
		return 0;
	}

	return dest_size;
}

SASL_VISIT_DEF( member_expression ){
	shared_ptr<member_expression> dup_expr =
		duplicate( v.as_handle() )->as_handle<member_expression>();

	any child_ctxt = *data;
	visit_child( child_ctxt, v.expr );
	dup_expr->expr = ctxt_ptr(child_ctxt)->generated_node->as_handle<expression>();

	type_info_si* arg_tisi = dup_expr->expr->dyn_siptr<type_info_si>();

	if( arg_tisi )
	{
		shared_ptr<tynode> agg_type = arg_tisi->type_info();
		tid_t mem_typeid = -1;

		int32_t swizzle_code = 0;
		int32_t member_index = -1;

		if( agg_type->node_class() == node_ids::struct_type )
		{
			// Aggregated is struct
			shared_ptr<symbol> struct_sym = agg_type->as_handle<struct_type>()->symbol();
			shared_ptr<symbol> mem_sym = struct_sym->find_this( v.member->str );

			if( !mem_sym )
			{
				diags->report( not_a_member_of )
					->token_range( *v.member, *v.member )
					->p( v.member->str )
					->p( struct_sym->unmangled_name() );
			}
			else
			{
				shared_ptr<declarator> mem_declr = mem_sym->node()->as_handle<declarator>();
				assert( mem_declr );
				SASL_EXTRACT_SI( storage_si, mem_si, mem_declr );
				mem_typeid = mem_si->entry_id();
				member_index = mem_si->mem_index();
				assert( mem_typeid != -1 );
			}
		}
		else if( agg_type->is_builtin() && is_storagable( agg_type->tycode ) )
		{
			// Aggregated class is vector & matrix
			builtin_types agg_btc = agg_type->tycode;
			int field_count = check_swizzle( agg_btc, v.member->str, swizzle_code );
			if( field_count > 0 ){
				builtin_types elem_btc = scalar_of( agg_btc );
				builtin_types swizzled_btc = builtin_types::none;

				if( is_scalar(agg_btc) || is_vector(agg_btc) ) {
					swizzled_btc = vector_of(
						elem_btc,
						static_cast<size_t>( field_count )
						);
				} else {
					// matrix only
					swizzled_btc = matrix_of(
						elem_btc,
						vector_size( agg_btc ),
						static_cast<size_t>( field_count )
						);
				}

				mem_typeid = msi->pety()->get( swizzled_btc );
			} else {
				diags->report( invalid_swizzle )->token_range( *v.member, *v.member )->p( v.member->str )->p( type_repr(agg_type).str() );
				return;
			}
		}
		else
		{
			diags->report( member_left_must_have_struct )->token_range( *v.member, *v.member )->p( v.member->str )->p( type_repr(agg_type).str() );
			return;
		}

		SASL_GET_OR_CREATE_SI_P( storage_si, ssi, dup_expr, msi->pety() );
		ssi->entry_id( mem_typeid );
		ssi->swizzle( swizzle_code );
	}
	else
	{
		diags->report( member_left_must_have_struct )->token_range( *v.member, *v.member )->p( v.member->str )->p( "<unknown>" );
	}

	data_cptr()->generated_node = dup_expr;
}

SASL_VISIT_DEF( constant_expression )
{
	shared_ptr<constant_expression> dup_cexpr = duplicate( v.as_handle() )->as_handle<constant_expression>();
	
	SASL_GET_OR_CREATE_SI_P( const_value_si, vsi, dup_cexpr, msi->pety() );
	vsi->set_literal( v.value_tok->str, v.ctype );
	data_cptr()->generated_node = dup_cexpr->as_handle();	
}

SASL_VISIT_DEF( variable_expression ){
	std::string name = v.var_name->str;

	shared_ptr<symbol> vdecl = data_cptr()->parent_sym->find( name );
	shared_ptr<variable_expression> dup_vexpr = duplicate( v.as_handle() )->as_handle<variable_expression>();

	if( vdecl ){
		shared_ptr<node> node = vdecl->node();
		shared_ptr<tynode> ty_node = node->as_handle<tynode>();
		shared_ptr<declarator> decl_node = node->as_handle<declarator>();
		shared_ptr<parameter> param_node = node->as_handle<parameter>();

		if( ty_node )
		{
			diags->report( illegal_use_type_as_expr )
				->token_range( *v.token_begin(), *v.token_end() )
				->p( name );
		}
		else if ( decl_node || param_node )
		{
			dup_vexpr->semantic_info( node->semantic_info() );
			dup_vexpr->si_ptr<storage_si>()->declarator( vdecl.get() );
		}
		else
		{
			diags->report( unknown_semantic_error )->token_range(*v.token_begin(), *v.token_end())->p(__FILE__)->p(__LINE__);
		}
	} else {
		// Function
		bool is_function = ! data_cptr()->parent_sym->find_overloads( name ).empty();
		if( is_function ){
			SASL_GET_OR_CREATE_SI( fnvar_si, fvsi, dup_vexpr );
			fvsi->scope( data_cptr()->parent_sym );
			fvsi->name( name );
		} else {
			diags->report( undeclared_identifier )->token_range( *v.token_begin(), *v.token_end() )->p( name );
		}
	}

	data_cptr()->generated_node = dup_vexpr->as_handle();
	return;
}

// declaration & type specifier
SASL_VISIT_DEF_UNIMPL( initializer );
SASL_VISIT_DEF( expression_initializer )
{
	shared_ptr<expression_initializer> dup_exprinit = duplicate( v.as_handle() )->as_handle<expression_initializer>();
	data_cptr()->generated_node = dup_exprinit->as_handle();

	any child_ctxt_init = *data;
	ctxt_ptr(child_ctxt_init)->generated_node.reset();

	any child_ctxt;
	visit_child( child_ctxt, child_ctxt_init, v.init_expr, dup_exprinit->init_expr );

	type_info_si* init_expr_tisi = dup_exprinit->init_expr->si_ptr<type_info_si>();
	if( !init_expr_tisi ) { return; }

	type_info_si* var_tsi = data_cptr()->variable_to_fill->si_ptr<type_info_si>();
	if( !var_tsi || var_tsi->entry_id() == -1 ) { return; }

	if ( var_tsi->entry_id() != init_expr_tisi->entry_id() ){
		if( !caster->try_implicit( var_tsi->entry_id(), init_expr_tisi->entry_id() ) ){
			diags->report( cannot_convert_type_from )
				->token_range( *dup_exprinit->init_expr->token_begin(), *dup_exprinit->init_expr->token_end() )
				->p( type_repr(init_expr_tisi->type_info()).str() )->p( type_repr(var_tsi->type_info()).str() );
		}
	}

	SASL_GET_OR_CREATE_SI_P( storage_si, ssi, dup_exprinit, msi->pety() );
	ssi->entry_id( init_expr_tisi->entry_id() );
}

SASL_VISIT_DEF_UNIMPL( member_initializer );
SASL_VISIT_DEF_UNIMPL( declaration );
SASL_VISIT_DEF( declarator ){

	any child_ctxt_init = *data;
	ctxt_ptr(child_ctxt_init)->generated_node.reset();
	any child_ctxt;

	shared_ptr<declarator> dup_decl = duplicate( v.as_handle() )->as_handle<declarator>();

	SASL_GET_OR_CREATE_SI_P( storage_si, ssi, dup_decl, msi->pety() );
	ssi->entry_id( data_cptr()->declarator_type_id );

	if( data_cptr()->member_index >= 0 ){
		ssi->mem_index( data_cptr()->member_index++ );
	}

	ctxt_ptr(child_ctxt_init)->variable_to_fill = dup_decl;
	if ( v.init ){
		visit_child( child_ctxt, child_ctxt_init, v.init, dup_decl->init );
	}

	if( data_cptr()->declarator_type_id != -1 )
	{
		shared_ptr<symbol> nodesym = data_cptr()->parent_sym->add_child( v.name->str, dup_decl );

		parse_semantic( v.semantic, v.semantic_index, ssi );

		if(	data_cptr()->is_global )
		{
			msi->globals().push_back( nodesym );
		}
	}

	data_cptr()->generated_node = dup_decl->as_handle();
}

SASL_VISIT_DEF( variable_declaration )
{
	any child_ctxt_init = *data;
	ctxt_ptr(child_ctxt_init)->generated_node.reset();
	any child_ctxt;

	shared_ptr<variable_declaration> dup_vdecl = duplicate( v.as_handle() )->as_handle<variable_declaration>();
	data_cptr()->generated_node = dup_vdecl->as_handle();

	visit_child( child_ctxt, child_ctxt_init, v.type_info, dup_vdecl->type_info );
	
	type_info_si* decl_tisi = dup_vdecl->type_info->si_ptr<type_info_si>();
	if( decl_tisi ) {
		ctxt_ptr(child_ctxt_init)->declarator_type_id = decl_tisi->entry_id();
	} else {
		ctxt_ptr(child_ctxt_init)->declarator_type_id = -1;
	}
	ctxt_ptr(child_ctxt_init)->variable_to_fill = dup_vdecl;

	dup_vdecl->declarators.clear();
	BOOST_FOREACH( shared_ptr<declarator> decl, v.declarators ){
		shared_ptr<declarator> gen_decl;
		visit_child( child_ctxt, child_ctxt_init, decl, gen_decl );
		assert(gen_decl);
		dup_vdecl->declarators.push_back( gen_decl );
		data_cptr()->member_index = ctxt_ptr(child_ctxt)->member_index;
		ctxt_ptr(child_ctxt_init)->member_index = ctxt_ptr(child_ctxt)->member_index;
	}
}

SASL_VISIT_DEF_UNIMPL( type_definition );
SASL_VISIT_DEF_UNIMPL( tynode );
SASL_VISIT_DEF( builtin_type ){
	// create type information on current symbol.
	// for e.g. create type info onto a variable node.
	SASL_GET_OR_CREATE_SI_P( type_si, tsi, v, msi->pety() );
	tsi->type_info( v.as_handle<tynode>(), data_cptr()->parent_sym );

	data_cptr()->generated_node = tsi->type_info()->as_handle();
}

SASL_VISIT_DEF_UNIMPL( array_type );

SASL_VISIT_DEF( struct_type ){
	// Struct Type are 3 sorts:
	//	* unnamed structure
	//	* struct declaration
	//	* struct definition.

	std::string name;
	if( !v.name ){
		name = data_cptr()->parent_sym->unique_name( symbol::unnamed_struct );
		v.name = token_t::from_string( name );
	}

	// Get from type pool or insert a new one.
	tid_t dup_struct_id
		= msi->pety()->get( v.as_handle<tynode>(), data_cptr()->parent_sym );

	assert( dup_struct_id != -1 );

	shared_ptr<struct_type> dup_struct
		= msi->pety()->get( dup_struct_id )->as_handle<struct_type>();
	data_cptr()->generated_node = dup_struct;

	SASL_EXTRACT_SI( type_info_si, tisi, dup_struct );
	dup_struct = tisi->type_info()->as_handle<struct_type>();

	// If v is declaration only, just return.
	if( !v.has_body ){ return; }

	// If v has body, try to update body, or redefinition.
	if( dup_struct->has_body )
	{
		diags->report( type_redefinition )
			->token_range( *v.token_begin(), *v.token_end() )
			->p(v.name->str)->p("struct");
		return;
	} 

	// Update struct body.
	else
	{
		dup_struct->decls.clear();

		shared_ptr<symbol> sym = dup_struct->symbol();

		any child_ctxt;
		any child_ctxt_init = *data;
		ctxt_ptr(child_ctxt_init)->parent_sym = sym;
		ctxt_ptr(child_ctxt_init)->member_index = 0;
		ctxt_ptr(child_ctxt_init)->is_global = false;

		dup_struct->has_body = true;
		BOOST_FOREACH( shared_ptr<declaration> const& decl, v.decls ){
			visit_child( child_ctxt, child_ctxt_init, decl );
			dup_struct->decls.push_back(
				ctxt_ptr( child_ctxt )->generated_node->as_handle<declaration>()
				);

			// Update member index
			ctxt_ptr(child_ctxt_init)->member_index = ctxt_ptr(child_ctxt)->member_index;
		}
	}
}

SASL_VISIT_DEF( alias_type ){
	tid_t dup_struct_id = -1;
	if( v.alias->str == "sampler" ){
		dup_struct_id = msi->pety()->get( builtin_types::_sampler );
	} else {
		dup_struct_id = msi->pety()->get( v.as_handle<tynode>(), data_cptr()->parent_sym );
	}
	
	if( dup_struct_id == -1 )
	{
		diags->report( undeclared_identifier )->token_range( *v.alias, *v.alias )->p(v.alias->str);
	}

	data_cptr()->generated_node = msi->pety()->get(dup_struct_id);
}

SASL_VISIT_DEF( parameter )
{
	shared_ptr<parameter> dup_par = duplicate( v.as_handle() )->as_handle<parameter>();
	data_cptr()->generated_node = dup_par->as_handle();

	data_cptr()->parent_sym->add_child( v.name ? v.name->str : std::string(), dup_par );

	any child_ctxt;
	visit_child( child_ctxt, *data, v.param_type, dup_par->param_type );

	if ( v.init ){
		visit_child( child_ctxt, *data, v.init, dup_par->init );
	}

	type_info_si* par_tisi = dup_par->param_type->si_ptr<type_info_si>();
	tid_t tid = par_tisi ? par_tisi->entry_id() : -1;
	
	if( tid == -1 ) { return; }

	shared_ptr<storage_si> ssi = get_or_create_semantic_info<storage_si>( dup_par, msi->pety() );
	ssi->entry_id( tid );
	// SEMANTIC_TODO: Nonsupports reference yet.
	ssi->is_reference( false );
	parse_semantic( v.semantic, v.semantic_index, ssi );
}

SASL_VISIT_DEF( function_type )
{
	// Copy node
	shared_ptr<node> dup_node = duplicate( v.as_handle() );
	EFLIB_ASSERT_AND_IF( dup_node, "Node swallow duplicated error !"){
		return;
	}
	shared_ptr<function_type> dup_fn = dup_node->as_handle<function_type>();
	data_cptr()->generated_node = dup_fn;
	dup_fn->params.clear();

	shared_ptr<symbol> sym = data_cptr()->parent_sym->add_function_begin( dup_fn );

	any child_ctxt_init = *data;
	ctxt_ptr(child_ctxt_init)->parent_sym = sym;
	ctxt_ptr(child_ctxt_init)->parent_fn = dup_fn;

	any child_ctxt;

	dup_fn->retval_type
		= ctxt_ptr( visit_child( child_ctxt, child_ctxt_init, v.retval_type ) )->generated_node->as_handle<tynode>();
	if( !dup_fn->retval_type->si_ptr<semantic_info>() ) { return; }
	
	bool successful = true;
	for( vector< shared_ptr<parameter> >::iterator it = v.params.begin();
		it != v.params.end(); ++it )
	{
		shared_ptr<parameter> fn_param;
		visit_child(child_ctxt, child_ctxt_init, *it, fn_param);
		dup_fn->params.push_back(fn_param);

		type_info_si* param_tisi = fn_param->si_ptr<type_info_si>();
		if( !param_tisi || param_tisi->entry_id() == -1 ){ successful = false; }
	}
	if( !successful ){ return; }
	
	data_cptr()->parent_sym->add_function_end( sym );

	shared_ptr<storage_si> ssi = get_or_create_semantic_info<storage_si>( dup_fn, msi->pety() );
	tid_t ret_tid = dup_fn->retval_type->si_ptr<type_info_si>()->entry_id();
	ssi->entry_id( ret_tid );

	// SEMANTIC_TODO judge the true abi.
	ssi->c_compatible( true );

	parse_semantic( v.semantic, v.semantic_index, ssi );

	ctxt_ptr(child_ctxt_init)->is_global = false;

	if ( v.body )
	{
		visit_child( child_ctxt, child_ctxt_init, v.body, dup_fn->body );
		msi->functions().push_back( sym );
	}

	
}

// statement
SASL_VISIT_DEF_UNIMPL( statement );

SASL_VISIT_DEF( declaration_statement )
{
	any child_ctxt_init = *data;
	ctxt_ptr(child_ctxt_init)->generated_node.reset();
	any child_ctxt;

	shared_ptr<declaration_statement> dup_declstmt = duplicate( v.as_handle() )->as_handle<declaration_statement>();

	visit_child( child_ctxt, child_ctxt_init, v.decl, dup_declstmt->decl );

	get_or_create_semantic_info<statement_si>(dup_declstmt);

	data_cptr()->generated_node = dup_declstmt;
}

SASL_VISIT_DEF( if_statement )
{
	any child_ctxt_init = *data;
	any child_ctxt;

	shared_ptr<if_statement> dup_ifstmt = duplicate(v.as_handle())->as_handle<if_statement>();

	visit_child( child_ctxt, child_ctxt_init, v.cond, dup_ifstmt->cond );
	shared_ptr<type_info_si> cond_tsi = extract_semantic_info<type_info_si>(dup_ifstmt->cond);
	assert( cond_tsi );
	tid_t bool_tid = msi->pety()->get( builtin_types::_boolean );
	assert( cond_tsi->entry_id() == bool_tid || caster->try_implicit( bool_tid, cond_tsi->entry_id() ) );

	visit_child( child_ctxt, child_ctxt_init, v.yes_stmt, dup_ifstmt->yes_stmt );
	extract_semantic_info<statement_si>(dup_ifstmt->yes_stmt)->parent_block( dup_ifstmt );

	if( !dup_ifstmt->yes_stmt->symbol() ){
		data_cptr()->parent_sym->add_anonymous_child( dup_ifstmt->yes_stmt );
	}

	if( dup_ifstmt->no_stmt ){
		visit_child( child_ctxt, child_ctxt_init, v.no_stmt, dup_ifstmt->no_stmt );
		extract_semantic_info<statement_si>(dup_ifstmt->no_stmt)->parent_block( dup_ifstmt );

		if( !dup_ifstmt->no_stmt->symbol() ){
			data_cptr()->parent_sym->add_anonymous_child( dup_ifstmt->yes_stmt );
		}
	}

	SASL_GET_OR_CREATE_SI( statement_si, si, dup_ifstmt);

	data_cptr()->generated_node = dup_ifstmt;
}

SASL_VISIT_DEF( while_statement ){
	any child_ctxt_init = *data;
	any child_ctxt;

	shared_ptr<while_statement> dup_while = duplicate( v.as_handle() )->as_handle<while_statement>();

	visit_child( child_ctxt, child_ctxt_init, v.cond, dup_while->cond );
	shared_ptr<type_info_si> cond_tsi = extract_semantic_info<type_info_si>(dup_while->cond);
	assert( cond_tsi );
	tid_t bool_tid = msi->pety()->get( builtin_types::_boolean );
	assert( cond_tsi->entry_id() == bool_tid || caster->try_implicit( bool_tid, cond_tsi->entry_id() ) );

	visit_child( child_ctxt, child_ctxt_init, v.body, dup_while->body );
	extract_semantic_info<statement_si>(dup_while->body)->parent_block( dup_while );

	data_cptr()->generated_node = dup_while;
}

SASL_VISIT_DEF( dowhile_statement ){

	any child_ctxt_init = *data;
	any child_ctxt;

	shared_ptr<dowhile_statement> dup_dowhile = duplicate( v.as_handle() )->as_handle<dowhile_statement>();

	visit_child( child_ctxt, child_ctxt_init, v.body, dup_dowhile->body );
	visit_child( child_ctxt, child_ctxt_init, v.cond, dup_dowhile->cond );
	shared_ptr<type_info_si> cond_tsi = extract_semantic_info<type_info_si>(dup_dowhile->cond);
	assert( cond_tsi );
	tid_t bool_tid = msi->pety()->get( builtin_types::_boolean );
	assert( cond_tsi->entry_id() == bool_tid || caster->try_implicit( bool_tid, cond_tsi->entry_id() ) );

	extract_semantic_info<statement_si>(dup_dowhile->body)->parent_block( dup_dowhile );

	data_cptr()->generated_node = dup_dowhile;
}

SASL_VISIT_DEF( labeled_statement ){
	any child_ctxt_init = *data;
	any child_ctxt;

	shared_ptr<labeled_statement> dup_lbl_stmt = duplicate( v.as_handle() )->as_handle<labeled_statement>();
	
	assert( data_cptr()->lbl_list );

	dup_lbl_stmt->labels.clear();
	BOOST_FOREACH( shared_ptr<label> const& lbl, v.labels ){
		shared_ptr<label> dup_lbl;
		visit_child( child_ctxt, child_ctxt_init, lbl, dup_lbl );
		if( dup_lbl )
		{
			dup_lbl_stmt->labels.push_back( dup_lbl );
		}
	}
	visit_child( child_ctxt, child_ctxt_init, v.stmt, dup_lbl_stmt->stmt );
	data_cptr()->lbl_list->push_back( dup_lbl_stmt );

	data_cptr()->generated_node = dup_lbl_stmt;
}

SASL_VISIT_DEF( case_label ){
	any child_ctxt_init = *data;
	any child_ctxt;

	shared_ptr<case_label> dup_case = duplicate( v.as_handle() )->as_handle<case_label>();
	
	if( v.expr )
	{
		visit_child( child_ctxt, child_ctxt_init, v.expr, dup_case->expr );

		if( v.expr->node_class() != node_ids::constant_expression )
		{
			diags->report( case_expr_not_constant )
				->token_range( *v.expr->token_begin(), *v.expr->token_end() );
		}
		else
		{
			type_info_si* expr_tisi = dup_case->expr->si_ptr<type_info_si>();
			if( !expr_tisi ){ return; }

			builtin_types expr_bt = expr_tisi->type_info()->tycode;
			if( !is_integer( expr_bt ) && expr_bt != builtin_types::_boolean )
			{
				diags->report( illegal_type_for_case_expr )
					->token_range( *v.expr->token_begin(), *v.expr->token_end() )
					->p( type_repr(expr_tisi->type_info()).str() );
			}
		}
	}

	data_cptr()->generated_node = dup_case;
}

SASL_VISIT_DEF( ident_label ){
	EFLIB_ASSERT_UNIMPLEMENTED();
}

SASL_VISIT_DEF( switch_statement ){
	any child_ctxt_init = *data;
	any child_ctxt;

	shared_ptr<switch_statement> dup_switch = duplicate( v.as_handle() )->as_handle<switch_statement>();

	visit_child( child_ctxt, child_ctxt_init, v.cond, dup_switch->cond );
	shared_ptr<type_info_si> cond_tsi = extract_semantic_info<type_info_si>(dup_switch->cond);
	assert( cond_tsi );
	tid_t int_tid = msi->pety()->get( builtin_types::_sint32 );
	builtin_types cond_bt = cond_tsi->type_info()->tycode;
	assert( is_integer( cond_bt ) || caster->try_implicit( int_tid, cond_tsi->entry_id() ) );

	SASL_GET_OR_CREATE_SI( statement_si, ssi, dup_switch );
	
	ctxt_ptr(child_ctxt_init)->lbl_list = &ssi->labels();
	visit_child( child_ctxt, child_ctxt_init, v.stmts, dup_switch->stmts );
	
	data_cptr()->generated_node = dup_switch;
}

SASL_VISIT_DEF( compound_statement )
{
	shared_ptr<compound_statement> dup_stmt = duplicate(v.as_handle())->as_handle<compound_statement>();
	dup_stmt->stmts.clear();

	any child_ctxt_init = *data;
	ctxt_ptr(child_ctxt_init)->parent_sym = data_cptr()->parent_sym->add_anonymous_child( dup_stmt );
	ctxt_ptr(child_ctxt_init)->generated_node.reset();

	any child_ctxt;
	for( vector< shared_ptr<statement> >::iterator it = v.stmts.begin();
		it != v.stmts.end(); ++it)
	{
		shared_ptr<statement> child_gen;
		visit_child( child_ctxt, child_ctxt_init, (*it), child_gen );
		
		if( child_gen ){
			dup_stmt->stmts.push_back(child_gen);
		}
	}

	get_or_create_semantic_info<statement_si>(dup_stmt);
	data_cptr()->generated_node = dup_stmt->as_handle();
}

SASL_VISIT_DEF( expression_statement ){
	
	shared_ptr<expression_statement> dup_exprstmt = duplicate( v.as_handle() )->as_handle<expression_statement>();
	
	any child_ctxt_init = *data;
	ctxt_ptr(child_ctxt_init)->generated_node.reset();

	any child_ctxt;
	visit_child( child_ctxt, child_ctxt_init, v.expr, dup_exprstmt->expr );

	get_or_create_semantic_info<statement_si>(dup_exprstmt);

	data_cptr()->generated_node = dup_exprstmt->as_handle();

}

SASL_VISIT_DEF( jump_statement )
{
	shared_ptr<jump_statement> dup_jump = duplicate(v.as_handle())->as_handle<jump_statement>();

	any child_ctxt_init = *data;
	ctxt_ptr( child_ctxt_init )->generated_node.reset();
	data_cptr()->generated_node = dup_jump;

	if (v.code == jump_mode::_return){
		if( v.jump_expr ){
			any child_ctxt;
			visit_child( child_ctxt, child_ctxt_init, v.jump_expr, dup_jump->jump_expr );
		}

		type_info_si* expr_tisi = dup_jump->jump_expr->si_ptr<type_info_si>();
		type_info_si* fret_tisi = ctxt_ptr(*data)->parent_fn->retval_type->si_ptr<type_info_si>();

		if( !expr_tisi || !fret_tisi ){ return; }

		tid_t expr_tid = expr_tisi->entry_id();
		tid_t fret_tid = fret_tisi->entry_id();

		if( expr_tid == -1 || fret_tid == -1 ){ return; }

		if( expr_tid != fret_tid && !caster->try_implicit(fret_tid, expr_tid) )
		{
			diags->report( cannot_convert_type_from )
				->token_range( *dup_jump->jump_expr->token_begin(), *dup_jump->jump_expr->token_end() )
				->p("return")
				->p( type_repr(expr_tisi->type_info()).str() )
				->p( type_repr(fret_tisi->type_info()).str() );
		}
	}
}

SASL_VISIT_DEF( for_statement ){
	any child_ctxt_init = *data;

	shared_ptr<for_statement> dup_for = duplicate(v.as_handle())->as_handle<for_statement>();

	ctxt_ptr( child_ctxt_init )->generated_node.reset();
	ctxt_ptr(child_ctxt_init)->parent_sym = data_cptr()->parent_sym->add_anonymous_child( dup_for );

	any child_ctxt;
	visit_child( child_ctxt, child_ctxt_init, v.init, dup_for->init );
	
	if( v.cond ){
		visit_child( child_ctxt, child_ctxt_init, v.cond, dup_for->cond );
	}

	if( v.iter ){
		visit_child( child_ctxt, child_ctxt_init, v.iter, dup_for->iter );
	}

	visit_child( child_ctxt, child_ctxt_init, v.body, dup_for->body );

	data_cptr()->generated_node = dup_for;
}

// program
SASL_VISIT_DEF( program ){
	data = data;

	// create semantic info
	msi.reset( new module_si() );
	diags = diag_chat::create();

	any child_ctxt_init = sacontext();
	ctxt_ptr(child_ctxt_init)->parent_sym = msi->root();

	any child_ctxt = child_ctxt_init;

	register_builtin_types();
	add_cast( child_ctxt_init );
	caster->set_tynode_getter(
		boost::bind(
		static_cast<shared_ptr<tynode> (pety_t::*)(tid_t)>(&pety_t::get),
		msi->pety().get(), _1)
		);
	register_builtin_functions( child_ctxt_init );

	shared_ptr<program> dup_prog = duplicate( v.as_handle() )->as_handle<program>();
	dup_prog->decls.clear();

	// analysis declarations.
	for( vector< shared_ptr<declaration> >::iterator it = v.decls.begin(); it != v.decls.end(); ++it ){
		shared_ptr<declaration> node_gen;
		visit_child( child_ctxt, child_ctxt_init, (*it), node_gen );
		dup_prog->decls.push_back( node_gen );
	}

	msi->root()->relink( dup_prog->as_handle() );
}

void semantic_analyser::empty_caster( shared_ptr<node> lhs, shared_ptr<node> rhs ){
	// do nothing
}

void semantic_analyser::add_cast( const boost::any& /*ctxt*/ ){
	// register default type converter
	pety_t* typemgr = msi->pety().get();

	tid_t sint8_ts	= typemgr->get( builtin_types::_sint8	);
	tid_t sint16_ts	= typemgr->get( builtin_types::_sint16	);
	tid_t sint32_ts	= typemgr->get( builtin_types::_sint32	);
	tid_t sint64_ts	= typemgr->get( builtin_types::_sint64	);

	tid_t uint8_ts	= typemgr->get( builtin_types::_uint8	);
	tid_t uint16_ts	= typemgr->get( builtin_types::_uint16	);
	tid_t uint32_ts	= typemgr->get( builtin_types::_uint32	);
	tid_t uint64_ts	= typemgr->get( builtin_types::_uint64	);

	tid_t float_ts	= typemgr->get( builtin_types::_float	);
	tid_t double_ts	= typemgr->get( builtin_types::_double	);

	tid_t bool_ts	= typemgr->get( builtin_types::_boolean	);

	// default conversation will do nothing.
	caster_t::cast_t default_conv = bind(&semantic_analyser::empty_caster, this, _1, _2);

	caster->add_cast_auto_prior( caster_t::imp, sint8_ts, sint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, sint8_ts, sint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, sint8_ts, sint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, sint8_ts, float_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::imp, sint8_ts, double_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, sint8_ts, bool_ts,   default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint8_ts, uint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint8_ts, uint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint8_ts, uint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint8_ts, uint64_ts, default_conv );

	caster->add_cast_auto_prior( caster_t::exp, sint16_ts, sint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::imp, sint16_ts, sint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, sint16_ts, sint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint16_ts, uint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint16_ts, uint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint16_ts, uint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint16_ts, uint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, sint16_ts, float_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::imp, sint16_ts, double_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, sint16_ts, bool_ts,   default_conv );

	caster->add_cast_auto_prior( caster_t::exp, sint32_ts, sint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint32_ts, sint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, sint32_ts, sint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint32_ts, uint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint32_ts, uint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint32_ts, uint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint32_ts, uint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, sint32_ts, float_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::imp, sint32_ts, double_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, sint32_ts, bool_ts,   default_conv );

	caster->add_cast_auto_prior( caster_t::exp, sint64_ts, sint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint64_ts, sint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint64_ts, sint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint64_ts, uint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint64_ts, uint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint64_ts, uint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, sint64_ts, uint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, sint64_ts, float_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::imp, sint64_ts, double_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, sint64_ts, bool_ts,   default_conv );

	caster->add_cast_auto_prior( caster_t::exp, uint8_ts, sint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint8_ts, sint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint8_ts, sint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint8_ts, sint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint8_ts, uint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint8_ts, uint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint8_ts, uint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint8_ts, float_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint8_ts, double_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint8_ts, bool_ts,   default_conv );

	caster->add_cast_auto_prior( caster_t::exp, uint16_ts, sint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, uint16_ts, sint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint16_ts, sint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint16_ts, sint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, uint16_ts, uint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint16_ts, uint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint16_ts, uint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint16_ts, float_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint16_ts, double_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint16_ts, bool_ts,   default_conv );

	caster->add_cast_auto_prior( caster_t::imp, uint32_ts, uint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint32_ts, sint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint32_ts, float_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint32_ts, double_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint32_ts, bool_ts,   default_conv );
	caster->add_cast_auto_prior( caster_t::exp, uint32_ts, sint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, uint32_ts, sint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, uint32_ts, sint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, uint32_ts, uint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, uint32_ts, uint16_ts, default_conv );
	

	caster->add_cast_auto_prior( caster_t::exp, uint64_ts, sint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, uint64_ts, sint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, uint64_ts, sint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, uint64_ts, sint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, uint64_ts, uint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, uint64_ts, uint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, uint64_ts, uint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint64_ts, float_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint64_ts, double_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, uint64_ts, bool_ts,   default_conv );

	caster->add_cast_auto_prior( caster_t::exp, float_ts, sint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, float_ts, sint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, float_ts, sint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, float_ts, sint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, float_ts, uint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, float_ts, uint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, float_ts, uint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, float_ts, uint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, float_ts, double_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::imp, float_ts, bool_ts,   default_conv );

	caster->add_cast_auto_prior( caster_t::exp, double_ts, sint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, double_ts, sint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, double_ts, sint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, double_ts, sint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, double_ts, uint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, double_ts, uint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, double_ts, uint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, double_ts, uint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, double_ts, float_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::imp, double_ts, bool_ts,   default_conv );

	caster->add_cast_auto_prior( caster_t::exp, bool_ts, sint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, bool_ts, sint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, bool_ts, sint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, bool_ts, sint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, bool_ts, uint8_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, bool_ts, uint16_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, bool_ts, uint32_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, bool_ts, uint64_ts, default_conv );
	caster->add_cast_auto_prior( caster_t::exp, bool_ts, float_ts,  default_conv );
	caster->add_cast_auto_prior( caster_t::exp, bool_ts, double_ts, default_conv );

	// Add scalar-vector1 and vec1-scalar cast
	vector<builtin_types> scalar_bts;
	scalar_bts.push_back( builtin_types::_sint8	);
	scalar_bts.push_back( builtin_types::_sint16 );
	scalar_bts.push_back( builtin_types::_sint32 );
	scalar_bts.push_back( builtin_types::_sint64 );
	scalar_bts.push_back( builtin_types::_uint8 );
	scalar_bts.push_back( builtin_types::_uint16 );
	scalar_bts.push_back( builtin_types::_uint32 );
	scalar_bts.push_back( builtin_types::_uint64 );
	scalar_bts.push_back( builtin_types::_float );
	scalar_bts.push_back( builtin_types::_double );
	scalar_bts.push_back( builtin_types::_boolean );

	BOOST_FOREACH( builtin_types bt, scalar_bts ){
		builtin_types v1bt = vector_of( bt, 1 );
		tid_t bt_tid = typemgr->get( bt );
		tid_t v1bt_tid = typemgr->get( v1bt );
		caster->add_cast( caster_t::eql, bt_tid, v1bt_tid, default_conv );
		caster->add_cast( caster_t::eql, v1bt_tid, bt_tid, default_conv );
	}
}

void semantic_analyser::register_builtin_functions( const boost::any& child_ctxt_init ){
	any child_ctxt;

	// Operators
	typedef unordered_map<
		builtin_types, shared_ptr<builtin_type>, enum_hasher
		> bt_table_t;
	bt_table_t standard_bttbl;
	bt_table_t storage_bttbl;

	map_of_builtin_type( standard_bttbl, &is_standard );
	map_of_builtin_type( storage_bttbl, &is_storagable );

#define BUILTIN_TYPE( btc ) (storage_bttbl[ builtin_types::btc ])

	shared_ptr<builtin_type> bt_bool = BUILTIN_TYPE( _boolean );
	shared_ptr<builtin_type> bt_i32  = BUILTIN_TYPE( _sint32 );

	// Arithmetic operators
	vector<std::string> op_tbl;
	const vector<operators>& oplist = list_of_operators();

	for( size_t i_op = 0; i_op < oplist.size(); ++i_op ){
		operators op = oplist[i_op];
		std::string op_name( operator_name(op) );

		if ( is_arithmetic(op) )
		{
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type )
			{
				shared_ptr<builtin_type> ty = it_type->second;
				register_function( child_ctxt_init, op_name ) % ty % ty >> ty;
			}
		}

		if( is_arith_assign(op) )
		{
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				shared_ptr<builtin_type> ty = it_type->second;
				register_function( child_ctxt_init, op_name ) % ty % ty >> ty;
			}
		}

		if( is_relationship(op) ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type )
			{
				builtin_types tycode = it_type->first;
				shared_ptr<builtin_type> ty = it_type->second;
				register_function(child_ctxt_init, op_name) % ty % ty >> storage_bttbl[replace_scalar(tycode, builtin_types::_boolean)];
			}
		}

		if( is_bit(op) || is_bit_assign(op) ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				if ( is_integer(it_type->first) ){
					shared_ptr<builtin_type> ty = it_type->second;
					register_function( child_ctxt_init, op_name ) % ty % ty >> ty;
				}
			}
		}

		if( is_shift(op) || is_shift_assign(op) ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				if ( is_scalar(it_type->first) && is_integer(it_type->first) ){
					shared_ptr<builtin_type> ty = it_type->second;
					register_function( child_ctxt_init, op_name ) % ty % BUILTIN_TYPE(_uint32) >> ty;
					register_function( child_ctxt_init, op_name ) % ty % BUILTIN_TYPE(_uint64) >> ty;
				}
			}
		}

		if( is_bool_arith(op) ){
			register_function( child_ctxt_init, op_name ) % bt_bool % bt_bool >> bt_bool;

			builtin_types tycode( builtin_types::none );
			shared_ptr<builtin_type> ty;
			for( size_t vsize = 1; vsize <= 4; ++vsize )
			{
				tycode = vector_of(builtin_types::_boolean, vsize);
				ty = storage_bttbl[tycode];
				register_function( child_ctxt_init, op_name ) % ty % ty >> ty;
				for( size_t vcnt = 1; vcnt <= 4; ++vcnt )
				{
					tycode = matrix_of(builtin_types::_boolean, vsize, vcnt);
					ty = storage_bttbl[tycode];
					register_function( child_ctxt_init, op_name ) % ty % ty >> ty;
				}
			}
		}

		if( is_prefix(op) || is_postfix(op) || op == operators::positive ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				if ( is_integer(it_type->first) )
				{
					shared_ptr<builtin_type> ty = it_type->second;
					register_function( child_ctxt_init, op_name ) % ty >> ty;
				}
			}
		}

		if( op == operators::bit_not ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				if ( is_integer(it_type->first) ){
					shared_ptr<builtin_type> ty = it_type->second;
					register_function( child_ctxt_init, op_name ) % ty >> ty;
				}
			}
		}

		if( op == operators::logic_not ){
			register_function( child_ctxt_init, op_name ) % bt_bool >> bt_bool;
		}

		if( op == operators::negative ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				if ( it_type->first != builtin_types::_uint64 ){
					shared_ptr<builtin_type> ty = it_type->second;
					register_function( child_ctxt_init, op_name ) % ty >> ty;
				}
			}
		}

		if ( op == operators::assign ){
			for( bt_table_t::iterator it_type = storage_bttbl.begin(); it_type != storage_bttbl.end(); ++it_type ){
				shared_ptr<builtin_type> ty = it_type->second;
				register_function( child_ctxt_init, op_name ) % ty % ty >> ty;
			}
		}
	}

	{
		/** @{ Intrinsics */
		shared_ptr<builtin_type> fvec_ts[5];
		for( int i = 1; i <= 4; ++i ){
			fvec_ts[i] = storage_bttbl[ vector_of( builtin_types::_float, i ) ];
		}

		shared_ptr<builtin_type> sampler_ty = create_builtin_type( builtin_types::_sampler );

		// External and Intrinsic are Same signatures
		{
			register_intrinsic( child_ctxt_init, "tex2Dlod", true, true )
				% sampler_ty % fvec_ts[4]
			>> fvec_ts[4];

			register_intrinsic( child_ctxt_init, "tex2Dgrad", true, true ) 
				% sampler_ty % fvec_ts[2] /*coord*/
				% fvec_ts[2] % fvec_ts[2] /*ddx, ddy*/
			>> fvec_ts[4];
		}

		// External Signatures
		{
			register_intrinsic( child_ctxt_init, "__tex2Dbias", true, true ) 
				% sampler_ty % fvec_ts[4] /*coord with bias*/
				% fvec_ts[2] % fvec_ts[2] /*ddx, ddy*/
			>> fvec_ts[4];

			register_intrinsic( child_ctxt_init, "__tex2Dproj", true, true ) 
				% sampler_ty % fvec_ts[4] /*coord with proj*/
				% fvec_ts[4] % fvec_ts[4] /*ddx, ddy*/
			>> fvec_ts[4];
		}


		// Intrinsic Signatures
		{
			register_intrinsic( child_ctxt_init, "tex2D", false, true ).deps("tex2Dgrad")
				% sampler_ty % fvec_ts[2] 
			>> fvec_ts[4];
			
			register_intrinsic( child_ctxt_init, "tex2Dbias", false, true ).deps("__tex2Dbias")
				% sampler_ty % fvec_ts[4] /*coord with bias*/
			>> fvec_ts[4];

			register_intrinsic( child_ctxt_init, "tex2Dproj", false, true ).deps("__tex2Dproj")
				% sampler_ty % fvec_ts[4] /*coord with proj*/
			>> fvec_ts[4];
		}

		// LLVM Workaround for some intrinsics are NULL pointer.
		{
			register_intrinsic( child_ctxt_init, "__wa_expf", true, false ) % BUILTIN_TYPE(_float) >> BUILTIN_TYPE(_float);
		}

		shared_ptr<builtin_type> fmat_ts[5][5];
		for( int vec_size = 1; vec_size < 5; ++vec_size ){
			for( int n_vec = 1; n_vec < 5; ++n_vec ){
				fmat_ts[vec_size][n_vec] = storage_bttbl[
					matrix_of( builtin_types::_float, vec_size, n_vec )
				];
			}
		}

		for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
			shared_ptr<builtin_type> ty = it_type->second;
			register_intrinsic(child_ctxt_init, "ddx") % ty >> ty;
			register_intrinsic(child_ctxt_init, "ddy") % ty >> ty;
		}

		for( size_t vec_size = 1; vec_size <= 4; ++vec_size){
			
			for( size_t n_vec = 1; n_vec <= 4; ++n_vec ){
				register_intrinsic(child_ctxt_init, "mul")
					% fvec_ts[n_vec] % fmat_ts[vec_size][n_vec]
				>> fvec_ts[vec_size];

				register_intrinsic(child_ctxt_init, "mul")
					% fmat_ts[vec_size][n_vec] % fvec_ts[vec_size]
				>> fvec_ts[n_vec];
			}

			register_intrinsic(child_ctxt_init, "dot")
				% fvec_ts[vec_size] % fvec_ts[vec_size]
			>> BUILTIN_TYPE(_float);

			register_intrinsic( child_ctxt_init, "sqrt" ) % fvec_ts[vec_size] >> fvec_ts[vec_size];
		}

		register_intrinsic( child_ctxt_init, "abs" ) % BUILTIN_TYPE(_float) >> BUILTIN_TYPE(_float);
		register_intrinsic( child_ctxt_init, "abs" ) % BUILTIN_TYPE(_sint32) >> BUILTIN_TYPE(_sint32);
		// WORKAROUND_TODO LLVM 3.0: Intrinsic didn't generate correct native call.
		// register_intrinsic( child_ctxt_init, "exp" ) % BUILTIN_TYPE(_float) >> BUILTIN_TYPE(_float);
		register_intrinsic( child_ctxt_init, "exp" ).deps("__wa_expf") % BUILTIN_TYPE(_float) >> BUILTIN_TYPE(_float);

		register_intrinsic( child_ctxt_init, "sqrt" ) % BUILTIN_TYPE(_float) >> BUILTIN_TYPE(_float);

		register_intrinsic( child_ctxt_init, "cross" ) % fvec_ts[3] % fvec_ts[3] >> fvec_ts[3];
		/**@}*/
	}
	
	{
		shared_ptr<builtin_type> fvec_ts[5];
		fvec_ts[1] = storage_bttbl[builtin_types::_float];
		for( int i = 2; i <= 4; ++i ){
			fvec_ts[i] = storage_bttbl[ vector_of( builtin_types::_float, i ) ];
		}

		shared_ptr<builtin_type> ivec_ts[5];
		ivec_ts[1] = storage_bttbl[builtin_types::_sint32];
		for( int i = 2; i <= 4; ++i ){
			ivec_ts[i] = storage_bttbl[ vector_of( builtin_types::_sint32, i ) ];
		}

		// Constructors
		typedef pair<char const*, shared_ptr<builtin_type>* > name_ty_pair_t;
		vector< name_ty_pair_t > scalar_name_tys;
		scalar_name_tys.push_back( make_pair( "int"		, ivec_ts	) );
		scalar_name_tys.push_back( make_pair( "float"	, fvec_ts	) );

		BOOST_FOREACH( name_ty_pair_t const& name_ty, scalar_name_tys ){
			for( int i = 2; i <= 4; ++i ){
				std::string name( name_ty.first );
				char index_to_str[] = "0";
				index_to_str[0] += (char)i;
				name.append( index_to_str );

				register_constructor( child_ctxt_init, name, name_ty.second, i );
			}
		}
	}
}

void semantic_analyser::register_builtin_types(){
	BOOST_FOREACH( builtin_types const & btc, list_of_builtin_types() ){
		if( msi->pety()->get( btc ) == -1 ){
			assert( !"Register builtin type failed!" );
		}
	}
}

boost::shared_ptr<module_si> const& semantic_analyser::module_semantic_info() const{
	return msi;
}

semantic_analyser::function_register semantic_analyser::register_function( boost::any const& child_ctxt_init, std::string const& name )
{
	shared_ptr<function_type> fn = create_node<function_type>( token_t::null(), token_t::null() );
	fn->name = token_t::from_string( name );

	function_register ret(*this, child_ctxt_init, fn, false, false, false);

	return ret;
}

semantic_analyser::function_register semantic_analyser::register_intrinsic( boost::any const& child_ctxt_init, std::string const& name, bool external, bool partial_exec )
{
	shared_ptr<function_type> fn = create_node<function_type>( token_t::null(), token_t::null() );
	fn->name = token_t::from_string( name );

	function_register ret(*this, child_ctxt_init, fn, true, external, partial_exec);

	return ret;
}

void semantic_analyser::mark_intrin_invoked_recursive( shared_ptr<symbol> const& sym )
{
	storage_si* ssi = sym->node()->si_ptr<storage_si>();
	
	assert( ssi ); // Function must be analyzed.
	
	if( ssi->is_invoked() ){ return; }
		
	ssi->is_invoked( true );
	BOOST_FOREACH( shared_ptr<symbol> const& dep, ssi->intrinsic_deps() ){
		mark_intrin_invoked_recursive( dep );
	}
}

void semantic_analyser::register_constructor( any const& child_ctxt_init, std::string const& name, shared_ptr<builtin_type>* tys, int total )
{
	vector< shared_ptr<builtin_type> > param_tys;
	register_constructor_impl( child_ctxt_init, name, tys, total, 0, param_tys );
}

void semantic_analyser::register_constructor_impl(
	any const& child_ctxt_init, string const& name, shared_ptr<builtin_type>* tys,
	int total, int param_scalar_counts, vector< shared_ptr<builtin_type> >& param_tys )
{
	if( param_scalar_counts == total )
	{
		function_register constructor_reg = register_intrinsic( child_ctxt_init, name );
		constructor_reg.as_constructor();
		BOOST_FOREACH( shared_ptr<builtin_type> const& par_ty, param_tys ){
			constructor_reg % par_ty;
		}
		constructor_reg >> tys[total];
	} 
	else 
	{
		for( int i = 1; i <= total - param_scalar_counts; ++i ){
			param_tys.push_back( tys[i] );
			register_constructor_impl( child_ctxt_init, name, tys, total, param_scalar_counts+i, param_tys );
			param_tys.pop_back();
		}
	}
}

diag_chat* semantic_analyser::get_diags() const
{
	return diags.get();
}

// function_register
semantic_analyser::function_register::function_register(
	semantic_analyser& owner,
	boost::any const& ctxt_init,
	shared_ptr<function_type> const& fn,
	bool is_intrinsic,
	bool is_external,
	bool exec_partial
	) :owner(owner), ctxt_init(ctxt_init), fn(fn), is_intrinsic(is_intrinsic), is_external(is_external), is_partial_exec(exec_partial), is_constr(false)
{
	assert( fn );
}

semantic_analyser::function_register& semantic_analyser::function_register::operator%(
	semantic_analyser::function_register::type_handle_t const& par_type
	)
{
	return p(par_type);
}

void semantic_analyser::function_register::operator >> (
	semantic_analyser::function_register::type_handle_t const& ret_type
	)
{
	r(ret_type);
}

semantic_analyser::function_register& semantic_analyser::function_register::p( semantic_analyser::function_register::type_handle_t const& par_type )
{
	assert( par_type && fn );
	
	shared_ptr<parameter> par = create_node<parameter>( token_t::null(), token_t::null() );
	par->param_type = par_type;
	fn->params.push_back( par );

	return *this;
}

void semantic_analyser::function_register::r(
	semantic_analyser::function_register::type_handle_t const& ret_type 
	)
{
	assert( ret_type && fn );

	fn->retval_type = ret_type;

	any child_ctxt;
	owner.visit_child( child_ctxt, ctxt_init, fn );
	shared_ptr<node> new_node = ctxt_ptr(child_ctxt)->generated_node;

	storage_si* fn_ssi = new_node->si_ptr<storage_si>();

	fn_ssi->is_intrinsic(is_intrinsic);
	fn_ssi->c_compatible(!is_intrinsic);
	fn_ssi->external_compatible(is_external);
	fn_ssi->partial_execution(is_partial_exec);
	fn_ssi->is_constructor(is_constr);

	if( is_intrinsic ){
		shared_ptr<symbol> new_sym = new_node->symbol();
		assert( new_sym );
	
		// Add deps of intrinsic
		BOOST_FOREACH( string const& dep_str, intrinsic_deps ){
			vector< shared_ptr<symbol> > dep_syms = new_sym->find_overloads( dep_str );
			assert( !dep_syms.empty() );
			BOOST_FOREACH( shared_ptr<symbol> const& dep_sym, dep_syms ){
				fn_ssi->add_intrin_dep( dep_sym );
			}
		}

		owner.module_semantic_info()->intrinsics().push_back( new_sym );
	}
	fn.reset();
}

semantic_analyser::function_register::function_register( function_register const& rhs)
	: ctxt_init( rhs.ctxt_init ), fn(rhs.fn), owner(rhs.owner), is_intrinsic(rhs.is_intrinsic), is_external(rhs.is_external), is_partial_exec(rhs.is_partial_exec), is_constr(rhs.is_constr)
{
}

semantic_analyser::function_register& semantic_analyser::function_register::deps( std::string const& name )
{
	intrinsic_deps.push_back( name );
	return *this;
}

semantic_analyser::function_register& semantic_analyser::function_register::as_constructor()
{
	is_constr = true;
	return *this;
}

END_NS_SASL_SEMANTIC();
