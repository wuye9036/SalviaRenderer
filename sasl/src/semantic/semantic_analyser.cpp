#include <sasl/include/semantic/semantic_analyser.h>

#include <sasl/enums/operators.h>

#include <sasl/include/semantic/semantic_diags.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/semantic/caster.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/utility.h>
#include <sasl/include/common/diag_chat.h>

#include <salviar/include/enums.h>
#include <salviar/include/shader_reflection.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/utility/scoped_value.h>
#include <eflib/include/utility/unref_declarator.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/assign/list_of.hpp>
#include <boost/assign/list_inserter.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/bind/apply.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <eflib/include/platform/boost_end.h>

BEGIN_NS_SASL_SEMANTIC();

using salviar::semantic_value;

using sasl::common::diag_chat;
EFLIB_USING_SHARED_PTR(sasl::common, token_t);

using sasl::syntax_tree::alias_type;
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, array_type);
using sasl::syntax_tree::binary_expression;
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, builtin_type);
using sasl::syntax_tree::call_expression;
using sasl::syntax_tree::case_label;
using sasl::syntax_tree::cast_expression;
using sasl::syntax_tree::cond_expression;
using sasl::syntax_tree::create_builtin_type;
using sasl::syntax_tree::create_node;
using sasl::syntax_tree::compound_statement;
using sasl::syntax_tree::constant_expression;
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, declaration);
using sasl::syntax_tree::declarator;
using sasl::syntax_tree::declaration_statement;
using sasl::syntax_tree::dowhile_statement;
using sasl::syntax_tree::expression;
using sasl::syntax_tree::expression_initializer;
using sasl::syntax_tree::expression_statement;
using sasl::syntax_tree::for_statement;
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, function_def);
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, function_full_def);
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, function_type);
using sasl::syntax_tree::if_statement;
using sasl::syntax_tree::index_expression;
using sasl::syntax_tree::jump_statement;
using sasl::syntax_tree::label;
using sasl::syntax_tree::labeled_statement;
using sasl::syntax_tree::member_expression;
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, node);
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, parameter);
using sasl::syntax_tree::parameter_full;
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, program);
using sasl::syntax_tree::statement;
using sasl::syntax_tree::struct_type;
using sasl::syntax_tree::switch_statement;
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, tynode);
using sasl::syntax_tree::unary_expression;
using sasl::syntax_tree::variable_declaration;
using sasl::syntax_tree::variable_expression;
using sasl::syntax_tree::while_statement;
using sasl::syntax_tree::list_of_builtin_type;

using namespace boost::assign;
using namespace sasl::utility;

using eflib::scoped_value;
using eflib::fixed_string;

using boost::format;
using boost::shared_ptr;
using boost::weak_ptr;
using boost::unordered_map;

using std::vector;
using std::string;
using std::pair;
using std::make_pair;

#define FUNCTION_SCOPE( new_fn ) \
	scoped_value<function_def_ptr> __sasl_fn_scope_##__LINE__( current_function, (new_fn) );
#define SYMBOL_SCOPE( new_sym ) \
	scoped_value<symbol*> __sasl_sym_scope_##__LINE__( current_symbol, (new_sym) );
#define GLOBAL_FLAG_SCOPE( new_global_flag ) \
	scoped_value<bool> __sasl_global_flag_scope_##__LINE__( is_global_scope, (new_global_flag) );
#define LABEL_LIST_SCOPE( new_label_list ) \
	scoped_value<label_list_t*> __sasl_label_list_scope_##__LINE__( label_list, (new_label_list) );
#define VARIABLE_TO_INIT_SCOPE( var_to_init ) \
	scoped_value<node_ptr> __sasl_var_to_init_scope_##__LINE__( variable_to_initialized, (var_to_init) );
#define MEMBER_COUNTER_SCOPE(); \
	scoped_value<int> __sasl_member_counter_scope_##__LINE__(member_counter, 0);
#define DECLARATION_TID_SCOPE( tid ) \
	scoped_value<int> __sasl_decl_tid_scope_##__LINE__( declaration_tid, (tid) );

semantic_analyser::semantic_analyser()
{
	caster.reset( new caster_t() );

	initialize_operator_parameter_lrvs();

	// Initialize global state
	lang = salviar::lang_none;
	is_global_scope = true;
	declaration_tid = -1;
	member_counter = -1;
	label_list = NULL;
	generated_sem = NULL;
}

#define SASL_VISITOR_TYPE_NAME semantic_analyser

template <typename ReturnNodeT, typename NodeT>
shared_ptr<ReturnNodeT> semantic_analyser::visit_child( shared_ptr<NodeT> const& child, node_semantic** return_sem )
{
	node_ptr old_generated_node = generated_node;
	node_semantic* old_semantic = generated_sem;

	generated_node.reset();
	generated_sem = NULL;

	child->accept(this, NULL);

	// Node semantic is looked up by node pointer, 
	// And Windows 7 used pool based object allocation,
	// So every node if its semantic is created, it must not be deleted.
	// We assume that generated node is always holded by its parent.
	shared_ptr<ReturnNodeT> ret = generated_node->as_handle<ReturnNodeT>();
	if(return_sem) { *return_sem = generated_sem; }

	generated_node = old_generated_node;
	generated_sem = old_semantic;

	return ret;
}

template <typename NodeT>
shared_ptr<NodeT> semantic_analyser::visit_child( shared_ptr<NodeT> const& child, node_semantic** return_sem )
{
	return visit_child<NodeT, NodeT>(child, return_sem);
}

void semantic_analyser::parse_semantic(token_t_ptr const& sem_tok, token_t_ptr const& sem_idx_tok, node_semantic* ssi)
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
		ssi->semantic_value( semantic_value( semstr, static_cast<uint32_t>(index) ) );
	}
}

SASL_VISIT_DEF( unary_expression ){
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<unary_expression> dup_expr = duplicate( v.as_handle() )->as_handle<unary_expression>();
	generated_node = dup_expr;

	node_semantic* inner_sem = NULL;
	dup_expr->expr = visit_child(v.expr, &inner_sem);
	assert(inner_sem);

	if( v.op == operators::prefix_incr || v.op == operators::postfix_incr || v.op == operators::prefix_decr || v.op == operators::postfix_decr ){
		if( !is_integer(inner_sem->ty_proto()->tycode) ){
			// REPORT ERROR
			EFLIB_ASSERT_UNIMPLEMENTED();
			return;
		}
	}

	// Verify L-Value and R-Value of operand.
	parameter_lrvs& operator_lrvs = operator_parameter_lrvs_[v.op];
	if( (operator_lrvs.param_lrvs[0] & lvalue_or_rvalue::rvalue) == 0)
	{
		if( (inner_sem->lr_value() & lvalue_or_rvalue::lvalue) == 0)
		{
			diags
				->report(operator_needs_lvalue)
				->token_range( *v.token_begin(), *v.token_end() )
				->p(v.op_token->str);
			return;
		}
	}

	generated_sem = create_node_semantic(dup_expr);
	generated_sem->tid( inner_sem->tid() );
	generated_sem->lr_value(operator_lrvs.ret_lrv);
}

SASL_VISIT_DEF( cast_expression ){
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<cast_expression> dup_cexpr = duplicate(v.as_handle())->as_handle<cast_expression>();

	node_semantic* src_tsi		= NULL;
	node_semantic* casted_tsi	= NULL;

	dup_cexpr->casted_type = visit_child(v.casted_type, &casted_tsi);
	dup_cexpr->expr = visit_child(v.expr, &src_tsi);

	assert(src_tsi);
	assert(casted_tsi);

	if( src_tsi->tid() != casted_tsi->tid() ){
		if( caster->try_cast( casted_tsi->tid(), src_tsi->tid() ) == caster_t::nocast ){
			// Here is code error. Compiler should report it.
			EFLIB_ASSERT_UNIMPLEMENTED();
			return;
		}
	}

	generated_sem = create_node_semantic(dup_cexpr);
	generated_sem->tid( casted_tsi->tid() );
	generated_sem->lr_value(lvalue_or_rvalue::rvalue);

	generated_node = dup_cexpr;
}

SASL_VISIT_DEF( binary_expression )
{
	EFLIB_UNREF_DECLARATOR(data);

	// Duplicate node from syntax tree.
	shared_ptr<binary_expression> dup_expr = duplicate( v.as_handle() )->as_handle<binary_expression>();
	generated_node = dup_expr;

	// Visit child
	node_semantic* left_expr_sem = NULL;
	node_semantic* right_expr_sem = NULL;

	dup_expr->left_expr  = visit_child(v.left_expr, &left_expr_sem);
	dup_expr->right_expr = visit_child(v.right_expr, &right_expr_sem);

	fixed_string opname = module_semantic_->pety()->operator_name( v.op );
	vector<expression*> exprs;
	exprs += dup_expr->left_expr.get(), dup_expr->right_expr.get();
	
	if(	left_expr_sem == NULL ||  right_expr_sem == NULL )
	{
		return;
	}

	// Find overloads
	vector<symbol*> overloads;

	bool is_assign_operation = is_general_assign(v.op);

	if(is_assign_operation){
		overloads = current_symbol->find_assign_overloads(opname, caster.get(), exprs);
	} else {
		overloads = current_symbol->find_overloads(opname, caster.get(), exprs);
	}
	
	if( overloads.empty() )
	{
		args_type_repr atr;
		for( size_t i = 0; i < 2 /*binary operator*/; ++i )
		{
			atr.arg( get_node_semantic(exprs[i])->ty_proto() );
		}
		diags->report(operator_param_unmatched)->token_range( *v.token_begin(), *v.token_end() )->p( atr.str() );
		return;
	}
	
	if ( overloads.size() > 1 )
	{
		diags->report(operator_multi_overloads)->token_range( *v.token_begin(), *v.token_end() )->p( overloads.size() );
		return;
	}

	// Compute semantics.

	// Verify L-Value And R-Value
	assert(left_expr_sem ->lr_value() != lvalue_or_rvalue::unknown);
	assert(right_expr_sem->lr_value() != lvalue_or_rvalue::unknown);

	// rvalue we consider that it is always mactched suceeded.
	parameter_lrvs& operator_lrvs( operator_parameter_lrvs_[v.op] );
	if( (operator_lrvs.param_lrvs[0] & lvalue_or_rvalue::rvalue) == 0)
	{
		// TODO: Try to report error.
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	// rvalue we consider that it is always mactched suceeded.
	if( (operator_lrvs.param_lrvs[1] & lvalue_or_rvalue::rvalue) == 0)
	{
		if (is_assign_operation)
		{
			if( (right_expr_sem->lr_value() & lvalue_or_rvalue::lvalue) == 0)
			{
				diags
					->report(left_operand_must_be_lvalue)
					->token_range( *v.token_begin(), *v.token_end() )
					->p(v.op_token->str);
				return;
			}
		}
		else
		{
			// TODO: Try to report error.
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	}

	if(is_assign_operation){
		mark_modified( dup_expr->right_expr.get() );
	}
	tid_t result_tid = get_node_semantic( overloads[0]->associated_node() )->tid();
	generated_sem = create_node_semantic(dup_expr);
	generated_sem->tid(result_tid);
	generated_sem->lr_value(operator_lrvs.ret_lrv);
}

SASL_VISIT_DEF_UNIMPL( expression_list );

SASL_VISIT_DEF( cond_expression ){
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<cond_expression> dup_expr
		= duplicate( v.as_handle() )->as_handle<cond_expression>();
	generated_node = dup_expr;

	node_semantic* cond_sem= NULL;
	node_semantic* yes_sem	= NULL;
	node_semantic* no_sem	= NULL;

	dup_expr->cond_expr	= visit_child(v.cond_expr,	&cond_sem);
	dup_expr->yes_expr	= visit_child(v.yes_expr,	&yes_sem);
	dup_expr->no_expr	= visit_child(v.no_expr,	&no_sem);
	
	// SEMANTIC_TODO Test conversation between type of yes expression and no expression.
	assert( cond_sem && yes_sem && no_sem );
	if( !(cond_sem && yes_sem && no_sem) ) {
		return;
	}

	tid_t bool_tid = module_semantic_->pety()->get( builtin_types::_boolean );
	tid_t cond_tid = cond_sem->tid();

	if( cond_tid != bool_tid && !caster->try_implicit( cond_sem->tid(), bool_tid ) ){
		diags->report( cannot_convert_type_from )
			->token_range( *dup_expr->cond_expr->token_begin(), *dup_expr->cond_expr->token_end() )
			->p("?")->p(type_repr(cond_sem->ty_proto()).str())->p("bool");
		return;
	}
	
	// Compute tid of conditional expression
	tid_t yes_tid = yes_sem->tid();
	tid_t no_tid = no_sem->tid();
	tid_t expr_tid = -1;
	if( yes_tid == no_tid ){
		expr_tid = yes_tid;
	} else if( caster->try_implicit(yes_tid, no_tid) ){
		expr_tid = yes_tid;
	} else if( caster->try_implicit(no_tid, yes_tid) ){
		expr_tid = no_tid;
	} else {
		diags->report(cannot_convert_type_from)
			->token_range( *dup_expr->yes_expr->token_begin(), *dup_expr->no_expr->token_end() )
			->p(":")->p(type_repr(no_sem->ty_proto()).str())->p(type_repr(yes_sem->ty_proto()).str());
		return;
	}

	// Compute L-R Value of conditional expression.
	assert(yes_sem->lr_value() != lvalue_or_rvalue::unknown);
	assert(yes_sem->lr_value() != lvalue_or_rvalue::lrvalue);

	assert(no_sem->lr_value() != lvalue_or_rvalue::unknown);
	assert(no_sem->lr_value() != lvalue_or_rvalue::lrvalue);

	generated_sem = create_node_semantic(dup_expr);
	generated_sem->tid(expr_tid);

	if( (yes_sem->lr_value() & lvalue_or_rvalue::lvalue) == lvalue_or_rvalue::lvalue
		&& (no_sem->lr_value() & lvalue_or_rvalue::lvalue) == lvalue_or_rvalue::lvalue )
	{
		generated_sem->lr_value(lvalue_or_rvalue::lvalue);
	}
	else
	{
		generated_sem->lr_value(lvalue_or_rvalue::rvalue);
	}
}

SASL_VISIT_DEF( index_expression )
{
	EFLIB_UNREF_DECLARATOR(data);
	
	shared_ptr<index_expression> dup_idxexpr = duplicate(v.as_handle())->as_handle<index_expression>();
	generated_node = dup_idxexpr;
	dup_idxexpr->expr = visit_child(v.expr);	
	
	node_semantic* agg_sem = get_node_semantic(dup_idxexpr->expr);
	if( !agg_sem ){ return; }
	tynode* agg_tyn = agg_sem->ty_proto();
	builtin_types agg_tycode = agg_tyn->tycode;
	if( !( agg_tyn->is_array() || is_vector(agg_tycode) || is_matrix(agg_tycode) ) )
	{	
		diags->report(not_an_acceptable_operator)
			->token_range( *v.token_begin(), *v.token_end() )
			->p("[")->p( type_repr(agg_tyn).str() );
		agg_sem = NULL;
	}

	dup_idxexpr->index_expr = visit_child(v.index_expr);
	node_semantic* index_sem = get_node_semantic(dup_idxexpr->index_expr);

	if( !index_sem ){ return; }
	tynode* idx_tyn = index_sem->ty_proto();
	builtin_types idx_tycode = idx_tyn->tycode;

	if( !is_integer(idx_tycode) )
	{
		diags->report( subscript_not_integral )
			->token_range( *v.token_begin(), *v.token_end() );
		index_sem = NULL;
	}
	
	if( !(agg_sem && index_sem) ) { return; }

	generated_sem = create_node_semantic(dup_idxexpr);
	generated_sem->lr_value( agg_sem->lr_value() );
	if( agg_tyn->is_array() )
	{
		array_type_ptr array_tyn = agg_tyn->as_handle<array_type>();
		tid_t elem_tid  = get_node_semantic(array_tyn->elem_type)->tid();
		tid_t inner_tid = module_semantic_->pety()->get_array(elem_tid, array_tyn->array_lens.size()-1);
		generated_sem->tid(inner_tid);
	}
	else if( is_vector(agg_tycode) )
	{
		generated_sem->tid( module_semantic_->pety()->get( scalar_of(agg_tycode) ) );
	}
	else if( is_matrix(agg_tycode) )
	{
		builtin_types vector_tycode = row_vector_of(agg_tycode);
		generated_sem->tid( module_semantic_->pety()->get(vector_tycode) );
	}
}

SASL_VISIT_DEF( call_expression )
{
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<call_expression> dup_callexpr = duplicate(v.as_handle())->as_handle<call_expression>();
	generated_node = dup_callexpr;

	dup_callexpr->expr = visit_child(v.expr);
	
	dup_callexpr->args.clear();
	BOOST_FOREACH( shared_ptr<expression> arg_expr, v.args )
	{
		dup_callexpr->args.push_back( visit_child(arg_expr) );
	}

	node_semantic* expr_sem = get_node_semantic(dup_callexpr->expr);
	node_semantic* fnsi = expr_sem;

	if( expr_sem == NULL ){ return; }
	
	if( expr_sem->is_function_pointer() ) {
		EFLIB_ASSERT_UNIMPLEMENTED();
		// Maybe pointer of function.
	} else {
		// Overload
		vector<expression*> args;
		BOOST_FOREACH( shared_ptr<expression> const& arg, dup_callexpr->args )
		{
			args.push_back( arg.get() );
		}
		vector<symbol*> syms
			= fnsi->associated_symbol()->find_overloads(fnsi->function_name(), caster.get(), args);
		
		if( syms.empty() )
		{
			args_type_repr atr;
			for( size_t i = 0; i < dup_callexpr->args.size(); ++i )
			{
				node_semantic* arg_sem = get_node_semantic(dup_callexpr->args[i]);
				if( arg_sem )
				{
					atr.arg( arg_sem->ty_proto() );
				}
				else
				{
					atr.arg( shared_ptr<tynode>() );
				}
			}
			diags->report( function_param_unmatched )
				->token_range( *v.token_begin(), *v.token_end() )
				->p( fnsi->function_name() )->p( atr.str() );
		}
		else if ( syms.size() > 1 )
		{
			diags->report( function_multi_overloads )
				->token_range( *v.token_begin(), *v.token_end() )
				->p( fnsi->function_name() )->p( syms.size() );
		}
		else
		{
			symbol* func_sym = syms[0];
			assert( func_sym );

			mark_intrin_invoked_recursive( func_sym );
			node_semantic* func_sem = get_node_semantic( func_sym->associated_node() );
			generated_sem = create_node_semantic(dup_callexpr);

			generated_sem->tid( func_sem->tid() );
			generated_sem->is_function_pointer(false);
			generated_sem->overloaded_function(func_sym);
			generated_sem->lr_value(lvalue_or_rvalue::rvalue);
		}
	}
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
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<member_expression> dup_expr = duplicate( v.as_handle() )->as_handle<member_expression>();
	generated_node = dup_expr;

	dup_expr->expr = visit_child(v.expr);
	
	node_semantic* agg_sem = get_node_semantic(dup_expr->expr);
	if( !agg_sem )
	{
		diags
			->report(member_left_must_have_struct)
			->token_range(*v.member, *v.member)
			->p(v.member->str)
			->p( "<unknown>" );
		return;
	}

	tynode* agg_type = agg_sem->ty_proto();
	tid_t mem_typeid = -1;

	int32_t swizzle_code = 0;
	int32_t member_index = -1;

	if( agg_type->is_struct() )
	{
		// Aggregated is struct
		symbol* struct_sym = get_symbol( agg_type );
		symbol* mem_sym = struct_sym->find_this( v.member->str );

		if( !mem_sym )
		{
			diags->report( not_a_member_of )
				->token_range( *v.member, *v.member )
				->p( v.member->str )
				->p( struct_sym->unmangled_name() );
		}
		else
		{
			shared_ptr<declarator> mem_declr = mem_sym->associated_node()->as_handle<declarator>();
			assert( mem_declr );
			node_semantic* mem_si = get_node_semantic(mem_declr);
			mem_typeid = mem_si->tid();
			member_index = mem_si->member_index();
			assert( mem_typeid != -1 );
		}
	}
	else if( agg_type->is_builtin() 
		&& ( is_scalar(agg_type->tycode) || is_vector(agg_type->tycode) ) )
	{
		// Aggregated class is scalar or vector: Member expression is 'SWIZZLE'
		builtin_types agg_btc = agg_type->tycode;
		int field_count = check_swizzle( agg_btc, v.member->str, swizzle_code );
		if( field_count > 0 ){
			builtin_types elem_btc = scalar_of( agg_btc );
			builtin_types swizzled_btc
				= vector_of( elem_btc, static_cast<size_t>(field_count) );
			mem_typeid = module_semantic_->pety()->get( swizzled_btc );
		} else {
			diags->report( invalid_swizzle )
				->token_range( *v.member, *v.member )
				->p( v.member->str )->p( type_repr(agg_type).str() );
			return;
		}
	}
	else
	{
		diags->report( member_left_must_have_struct )
			->token_range( *v.member, *v.member )
			->p( v.member->str )->p( type_repr(agg_type).str() );
		return;
	}

	generated_sem = create_node_semantic(dup_expr);

	generated_sem->tid(mem_typeid);
	generated_sem->swizzle(swizzle_code);
	generated_sem->lr_value( agg_sem->lr_value() );
}

SASL_VISIT_DEF(constant_expression)
{
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<constant_expression> dup_cexpr = duplicate( v.as_handle() )->as_handle<constant_expression>();
	generated_sem = create_node_semantic(dup_cexpr);
	generated_sem->const_value(v.value_tok->str, v.ctype);
	generated_sem->lr_value(lvalue_or_rvalue::rvalue);
	generated_node = dup_cexpr;	
}

SASL_VISIT_DEF( variable_expression ){
	EFLIB_UNREF_DECLARATOR(data);

	std::string name = v.var_name->str;

	symbol* vdecl = current_symbol->find( name );
	shared_ptr<variable_expression> dup_vexpr = duplicate( v.as_handle() )->as_handle<variable_expression>();

	if( vdecl ){
		node* node = vdecl->associated_node();
		shared_ptr<tynode> ty_node = node->as_handle<tynode>();
		shared_ptr<declarator> decl_node = node->as_handle<declarator>();
		shared_ptr<parameter> param_node = node->as_handle<parameter>();

		if( ty_node )
		{
			diags->report(illegal_use_type_as_expr)
				->token_range( *v.token_begin(), *v.token_end() )
				->p(name);
		}
		else if ( decl_node || param_node )
		{
			generated_sem = create_node_semantic(dup_vexpr);
			*generated_sem = *get_node_semantic(node);
			generated_sem->associated_symbol(vdecl);
			generated_sem->lr_value(lvalue_or_rvalue::lvalue);
			generated_sem->referenced_declarator(node);
		}
		else
		{
			diags->report( unknown_semantic_error )->token_range(*v.token_begin(), *v.token_end())->p(__FILE__)->p(__LINE__);
		}
	} else {
		// Function
		bool is_function = !current_symbol->find_overloads(name).empty();
		if( is_function ){
			generated_sem = create_node_semantic(dup_vexpr);
			generated_sem->associated_symbol(current_symbol);
			generated_sem->function_name(name);
		} else {
			diags->report(undeclared_identifier)->token_range( *v.token_begin(), *v.token_end() )->p(name);
		}
	}

	generated_node = dup_vexpr->as_handle();
}

// declaration & type specifier
SASL_VISIT_DEF_UNIMPL( initializer );
SASL_VISIT_DEF( expression_initializer )
{
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<expression_initializer> dup_exprinit = duplicate( v.as_handle() )->as_handle<expression_initializer>();
	generated_node = dup_exprinit->as_handle();

	dup_exprinit->init_expr = visit_child(v.init_expr);

	node_semantic* init_expr_sem = get_node_semantic(dup_exprinit->init_expr);
	if( !init_expr_sem ) { return; }

	node_semantic* var_tsi = get_node_semantic(variable_to_initialized);
	if( !var_tsi || var_tsi->tid() == -1 ) { return; }

	if ( var_tsi->tid() != init_expr_sem->tid() ){
		if( !caster->try_implicit( var_tsi->tid(), init_expr_sem->tid() ) ){
			diags->report( cannot_convert_type_from )
				->token_range( *dup_exprinit->init_expr->token_begin(), *dup_exprinit->init_expr->token_end() )
				->p( type_repr(init_expr_sem->ty_proto()).str() )->p( type_repr(var_tsi->ty_proto()).str() );
		}
	}

	generated_sem = create_node_semantic(dup_exprinit);
	generated_sem->tid( init_expr_sem->tid() );
}

SASL_VISIT_DEF_UNIMPL( member_initializer );
SASL_VISIT_DEF_UNIMPL( declaration );
SASL_VISIT_DEF( declarator ){
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<declarator> dup_decl = duplicate( v.as_handle() )->as_handle<declarator>();

	generated_sem = create_node_semantic(dup_decl);
	generated_sem->tid(declaration_tid);

	if( member_counter >= 0 )
	{
		generated_sem->member_index(member_counter++);
	}

	if ( v.init ){
		VARIABLE_TO_INIT_SCOPE(dup_decl);
		dup_decl->init = visit_child(v.init);
	}

	if( declaration_tid != -1 )
	{
		symbol* nodesym = current_symbol->add_named_child( v.name->str, dup_decl.get() );
		parse_semantic( v.semantic, v.semantic_index, generated_sem );
		if(is_global_scope)
		{
			module_semantic_->global_vars().push_back(nodesym);
		}
	}

	generated_node = dup_decl;
}

SASL_VISIT_DEF( variable_declaration )
{
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<variable_declaration> dup_vdecl = duplicate( v.as_handle() )->as_handle<variable_declaration>();
	generated_node = dup_vdecl;

	dup_vdecl->type_info = visit_child(v.type_info);
	
	node_semantic* decl_sem = get_node_semantic(dup_vdecl->type_info);

	int decl_tid = decl_sem ? decl_sem->tid() : -1;
	
	dup_vdecl->declarators.clear();

	{
		DECLARATION_TID_SCOPE(decl_tid);
		VARIABLE_TO_INIT_SCOPE(dup_vdecl);

		BOOST_FOREACH( shared_ptr<declarator> decl, v.declarators ){
			shared_ptr<declarator> gen_decl = visit_child(decl);
			assert(gen_decl);
			dup_vdecl->declarators.push_back(gen_decl);
		}
	}

}

SASL_VISIT_DEF_UNIMPL( type_definition );
SASL_VISIT_DEF_UNIMPL( tynode );
SASL_VISIT_DEF( builtin_type ){
	EFLIB_UNREF_DECLARATOR(data);

	// create type information on current symbol.
	// for e.g. create type info onto a variable node.
	tynode*			proto_node = NULL;
	node_semantic*	proto_sem = NULL;
	
	module_semantic_->pety()->get2(v.tycode, &proto_node, &proto_sem);

	generated_node = proto_node->as_handle();
	generated_sem = proto_sem;
}

SASL_VISIT_DEF(array_type)
{
	EFLIB_UNREF_DECLARATOR(data);

	tid_t array_tid = module_semantic_->pety()->get( &v, current_symbol );
	assert(array_tid != -1);

	tynode* proto_tyn = NULL;
	module_semantic_->pety()->get2(array_tid, &proto_tyn, &generated_sem);
	array_type_ptr dup_array = proto_tyn->as_handle<array_type>();
	if( !get_node_semantic(dup_array->elem_type) )
	{
		dup_array->elem_type = visit_child(v.elem_type);
	}
	generated_node = dup_array;
}

SASL_VISIT_DEF( struct_type ){
	EFLIB_UNREF_DECLARATOR(data);

	// struct type are 3 sorts:
	//	* unnamed structure
	//	* struct declaration
	//	* struct definition.

	std::string name;
	if( !v.name ){
		name = unique_structure_name();
		v.name = token_t::from_string(name);
	}

	// Get from type pool or insert a new one.
	tid_t dup_struct_id
		= module_semantic_->pety()->get( &v, current_symbol );

	assert( dup_struct_id != -1 );

	tynode* tyn = NULL;
	module_semantic_->pety()->get2(dup_struct_id, &tyn, &generated_sem);

	shared_ptr<struct_type> dup_struct = tyn->as_handle<struct_type>();
	generated_node = dup_struct;

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

		SYMBOL_SCOPE( get_symbol(dup_struct) );
		GLOBAL_FLAG_SCOPE(false);
		MEMBER_COUNTER_SCOPE();

		dup_struct->has_body = true;
		BOOST_FOREACH( shared_ptr<declaration> const& decl, v.decls ){
			dup_struct->decls.push_back( visit_child(decl) );
		}
	}
}

SASL_VISIT_DEF( alias_type ){
	EFLIB_UNREF_DECLARATOR(data);

	tid_t dup_struct_id = -1;
	if( v.alias->str == "sampler" ){
		dup_struct_id = module_semantic_->pety()->get(builtin_types::_sampler);
	} else {
		dup_struct_id = module_semantic_->pety()->get(&v, current_symbol);
	}
	
	if( dup_struct_id == -1 )
	{
		generated_node = duplicate( v.as_handle() );
		diags->report( undeclared_identifier )->token_range(*v.alias, *v.alias)->p(v.alias->str);
		return;
	}

	tynode* proto_node = NULL;
	module_semantic_->pety()->get2(dup_struct_id, &proto_node, &generated_sem);
	generated_node = proto_node->as_handle();
}

SASL_VISIT_DEF(function_type)
{
	EFLIB_UNREF_DECLARATOR(data);
	EFLIB_UNREF_DECLARATOR(v);
	EFLIB_ASSERT_UNIMPLEMENTED();
}

SASL_VISIT_DEF(parameter)
{
	EFLIB_UNREF_DECLARATOR(data);
	EFLIB_UNREF_DECLARATOR(v);
	EFLIB_ASSERT_UNIMPLEMENTED();
}

SASL_VISIT_DEF(function_def)
{
	EFLIB_UNREF_DECLARATOR(data);
	EFLIB_UNREF_DECLARATOR(v);
	EFLIB_ASSERT_UNIMPLEMENTED();
}

SASL_VISIT_DEF( parameter_full )
{
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<parameter> dup_par = create_node<parameter>( v.token_begin(), v.token_end() );
	dup_par->semantic = v.semantic;
	dup_par->semantic_index = v.semantic_index;
	dup_par->name = v.name;

	generated_node = dup_par;

	symbol* sym = NULL;
	if( v.name )
	{
		sym = current_symbol->add_named_child( v.name->str, dup_par.get() );
	}

	node_semantic* par_sem = NULL;
	visit_child(v.param_type, &par_sem);

	if ( v.init ){ dup_par->init = visit_child(v.init); }
	tid_t tid = par_sem ? par_sem->tid() : -1;
	
	if( tid == -1 ) { return; }

	generated_sem = create_node_semantic(dup_par);
	generated_sem->tid(tid);
	// TODO: Nonsupports reference yet.
	generated_sem->is_reference(false);
	if(sym)
	{
		generated_sem->associated_symbol(sym);
	}

	parse_semantic(v.semantic, v.semantic_index, generated_sem);
}

SASL_VISIT_DEF( function_full_def )
{
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<function_def> def_node = create_node<function_def>(
		v.token_begin(), v.token_end()
		);
	def_node->name = v.name;
	def_node->semantic = v.semantic;
	def_node->semantic_index = v.semantic_index;

	generated_node = def_node;
	
	node_semantic* result_type_sem = NULL;
	tid_t fn_tid = -1;
	symbol* sym = current_symbol->add_function_begin( def_node.get() );
	{
		SYMBOL_SCOPE(sym);
		FUNCTION_SCOPE(def_node);

		vector<tid_t> fn_tids;
		visit_child(v.retval_type, &result_type_sem);
		if( !result_type_sem ) { return; }
		fn_tids.push_back( result_type_sem->tid() );

		bool successful = true;
		for(size_t i_param = 0; i_param < v.params.size(); ++i_param)
		{
			node_semantic* param_sem = NULL;
			shared_ptr<parameter> param = visit_child<parameter>(v.params[i_param], &param_sem);
			def_node->params.push_back(param);
			fn_tids.push_back( param_sem->tid() );

			if( !param_sem || param_sem->tid() == -1 )
			{
				successful = false; 
			}
		}

		if(!successful)
		{
			current_symbol->cancel_function(sym);
			return;
		}

		fn_tid = module_semantic_->pety()->get_function_type(fn_tids);
		assert(fn_tid != -1);
		def_node->type = module_semantic_->pety()->get_proto(fn_tid)->as_handle<function_type>();
	}

	current_symbol->add_function_end(sym, fn_tid);

	generated_sem = create_node_semantic(def_node);
	generated_sem->tid( result_type_sem->tid() );
	generated_sem->associated_symbol(sym);
	// SEMANTIC_TODO judge the true abi.
	generated_sem->msc_compatible(true);

	parse_semantic(def_node->semantic, def_node->semantic_index, generated_sem);

	if ( v.body )
	{
		SYMBOL_SCOPE(sym);
		FUNCTION_SCOPE(def_node);
		GLOBAL_FLAG_SCOPE(false);

		def_node->body = visit_child(v.body);
		module_semantic_->functions().push_back(sym);
	}
}

// statement
SASL_VISIT_DEF_UNIMPL( statement );

SASL_VISIT_DEF( declaration_statement )
{
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<declaration_statement> dup_declstmt = duplicate( v.as_handle() )->as_handle<declaration_statement>();

	dup_declstmt->decls.clear();
	BOOST_FOREACH( shared_ptr<declaration> const& decl, v.decls )
	{
		shared_ptr<declaration> dup_decl = visit_child(decl);
		if( dup_decl ){ dup_declstmt->decls.push_back(dup_decl); }
	}
	
	generated_sem = create_node_semantic(dup_declstmt);
	generated_node = dup_declstmt;
}

SASL_VISIT_DEF( if_statement )
{
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<if_statement> dup_ifstmt = duplicate(v.as_handle())->as_handle<if_statement>();

	dup_ifstmt->cond = visit_child(v.cond);

#if defined(EFLIB_DEBUG)
	node_semantic* cond_tsi = get_node_semantic(dup_ifstmt->cond);
	assert( cond_tsi );
	tid_t bool_tid = module_semantic_->pety()->get( builtin_types::_boolean );
	assert( cond_tsi->tid() == bool_tid || caster->try_implicit( bool_tid, cond_tsi->tid() ) );
#endif

	dup_ifstmt->yes_stmt = visit_child(v.yes_stmt);
	//get_node_semantic( dup_ifstmt->yes_stmt.get() )->parent_block( dup_ifstmt.get() );

	if( ! get_symbol(dup_ifstmt->yes_stmt) ){
		current_symbol->add_child( dup_ifstmt->yes_stmt.get() );
	}

	if( dup_ifstmt->no_stmt ){
		dup_ifstmt->no_stmt = visit_child(v.no_stmt);
		//get_node_semantic( dup_ifstmt->no_stmt.get() )->parent_block(dup_ifstmt.get());

		if( ! get_symbol(dup_ifstmt->no_stmt) ){
			current_symbol->add_child( dup_ifstmt->yes_stmt.get() );
		}
	}

	generated_node = dup_ifstmt;
}

SASL_VISIT_DEF( while_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<while_statement> dup_while = duplicate( v.as_handle() )->as_handle<while_statement>();

	dup_while->cond = visit_child(v.cond);

#ifdef EFLIB_DEBUG
	node_semantic* cond_tsi = get_node_semantic(dup_while->cond);
	assert( cond_tsi );
	tid_t bool_tid = module_semantic_->pety()->get( builtin_types::_boolean );
	assert( cond_tsi->tid() == bool_tid || caster->try_implicit( bool_tid, cond_tsi->tid() ) );
#endif

	dup_while->body = visit_child(v.body);
	//get_node_semantic( dup_while->body.get() )->parent_block( dup_while.get() );

	generated_node = dup_while;
}

SASL_VISIT_DEF( dowhile_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<dowhile_statement> dup_dowhile = duplicate( v.as_handle() )->as_handle<dowhile_statement>();

	dup_dowhile->body = visit_child(v.body);
	dup_dowhile->cond = visit_child(v.cond);

#if defined(EFLIB_DEBUG)
	node_semantic* cond_tsi = get_node_semantic(dup_dowhile->cond);
	assert( cond_tsi );
	tid_t bool_tid = module_semantic_->pety()->get( builtin_types::_boolean );
	assert( cond_tsi->tid() == bool_tid || caster->try_implicit( bool_tid, cond_tsi->tid() ) );
#endif

	//get_node_semantic(dup_dowhile->body)->parent_block( dup_dowhile.get() );

	generated_node = dup_dowhile;
}

SASL_VISIT_DEF( labeled_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<labeled_statement> dup_lbl_stmt = duplicate( v.as_handle() )->as_handle<labeled_statement>();
	
	assert( label_list );

	dup_lbl_stmt->labels.clear();
	BOOST_FOREACH( shared_ptr<label> const& lbl, v.labels ){
		shared_ptr<label> dup_lbl = visit_child(lbl);
		if( dup_lbl )
		{
			dup_lbl_stmt->labels.push_back( dup_lbl );
		}
	}
	dup_lbl_stmt->stmt = visit_child(v.stmt);
	label_list->push_back( dup_lbl_stmt );

	generated_node = dup_lbl_stmt;
}

SASL_VISIT_DEF( case_label ){
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<case_label> dup_case = duplicate( v.as_handle() )->as_handle<case_label>();
	
	if( v.expr )
	{
		dup_case->expr = visit_child(v.expr);

		if( v.expr->node_class() != node_ids::constant_expression )
		{
			diags->report( case_expr_not_constant )
				->token_range( *v.expr->token_begin(), *v.expr->token_end() );
		}
		else
		{
			node_semantic* expr_sem = get_node_semantic(dup_case->expr);
			if( !expr_sem ){ return; }

			builtin_types expr_bt = expr_sem->ty_proto()->tycode;
			if( !is_integer( expr_bt ) && expr_bt != builtin_types::_boolean )
			{
				diags->report( illegal_type_for_case_expr )
					->token_range( *v.expr->token_begin(), *v.expr->token_end() )
					->p( type_repr(expr_sem->ty_proto()).str() );
			}
		}
	}

	generated_node = dup_case;
}

SASL_VISIT_DEF( ident_label ){
	EFLIB_UNREF_DECLARATOR(v);
	EFLIB_UNREF_DECLARATOR(data);
	EFLIB_ASSERT_UNIMPLEMENTED();
}

SASL_VISIT_DEF( switch_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<switch_statement> dup_switch = duplicate( v.as_handle() )->as_handle<switch_statement>();

	dup_switch->cond = visit_child(v.cond);

#if defined(EFLIB_DEBUG)
	node_semantic* cond_tsi = get_node_semantic(dup_switch->cond);
	assert( cond_tsi );
	tid_t int_tid = module_semantic_->pety()->get( builtin_types::_sint32 );
	builtin_types cond_bt = cond_tsi->ty_proto()->tycode;
	assert( is_integer( cond_bt ) || caster->try_implicit( int_tid, cond_tsi->tid() ) );
#endif

	generated_sem = create_node_semantic(dup_switch);
	
	LABEL_LIST_SCOPE( &generated_sem->labeled_statements() );
	dup_switch->stmts = visit_child(v.stmts);
	
	generated_node = dup_switch;
}

SASL_VISIT_DEF( compound_statement )
{
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<compound_statement> dup_stmt = duplicate(v.as_handle())->as_handle<compound_statement>();
	dup_stmt->stmts.clear();

	SYMBOL_SCOPE( current_symbol->add_child( dup_stmt.get() ) );

	for( vector< shared_ptr<statement> >::iterator it = v.stmts.begin();
		it != v.stmts.end(); ++it)
	{
		shared_ptr<statement> child_gen = visit_child(*it);
		if( child_gen ){
			dup_stmt->stmts.push_back(child_gen);
		}
	}

	generated_sem = create_node_semantic(dup_stmt);
	generated_node = dup_stmt;
}

SASL_VISIT_DEF( expression_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<expression_statement> dup_exprstmt = duplicate( v.as_handle() )->as_handle<expression_statement>();
	dup_exprstmt->expr = visit_child(v.expr);
	generated_sem = create_node_semantic(dup_exprstmt);
	generated_node = dup_exprstmt->as_handle();
}

SASL_VISIT_DEF( jump_statement )
{
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<jump_statement> dup_jump = duplicate(v.as_handle())->as_handle<jump_statement>();
	generated_node = dup_jump;

	if (v.code == jump_mode::_return){
		if( v.jump_expr ){
			dup_jump->jump_expr = visit_child(v.jump_expr);
		}

		node_semantic* expr_sem = get_node_semantic(dup_jump->jump_expr);
		node_semantic* func_sem = get_node_semantic(current_function);

		if(!expr_sem){ return; }

		tid_t expr_tid = expr_sem->tid();
		tid_t fret_tid = func_sem->tid();

		if( expr_tid == -1 || fret_tid == -1 ){ return; }

		if( expr_tid != fret_tid && !caster->try_implicit(fret_tid, expr_tid) )
		{
			diags->report( cannot_convert_type_from )
				->token_range( *dup_jump->jump_expr->token_begin(), *dup_jump->jump_expr->token_end() )
				->p("return")
				->p( type_repr(expr_sem->ty_proto()).str() )
				->p( type_repr(func_sem->ty_proto()).str() );
		}
	}
}

SASL_VISIT_DEF( for_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	shared_ptr<for_statement> dup_for = duplicate(v.as_handle())->as_handle<for_statement>();

	SYMBOL_SCOPE( current_symbol->add_child( dup_for.get() ) );
	dup_for->init = visit_child(v.init);
	if( v.cond ){
		dup_for->cond = visit_child(v.cond);
	}
	if( v.iter ){
		dup_for->iter = visit_child(v.iter);
	}
	dup_for->body = visit_child(v.body);

	generated_node = dup_for;
}

// program
SASL_VISIT_DEF( program ){
	EFLIB_UNREF_DECLARATOR(data);

	// create semantic info
	module_semantic_ = module_semantic::create();
	diags = diag_chat::create();

	SYMBOL_SCOPE( module_semantic_->root_symbol() );
	
	register_builtin_types();
	initialize_casts();
	caster->set_function_get_tynode( boost::bind( &pety_t::get_proto, module_semantic_->pety(), _1) );
	register_builtin_functions2();

	program_ptr dup_prog = duplicate( v.as_handle() )->as_handle<program>();
	prog_ = dup_prog.get();
	dup_prog->decls.clear();

	// analysis declarations.
	for( vector<declaration_ptr>::iterator it = v.decls.begin(); it != v.decls.end(); ++it )
	{
		declaration_ptr node_gen = visit_child(*it);
		dup_prog->decls.push_back( node_gen );
	}

	module_semantic_->set_language( static_cast<salviar::languages>(lang) );
	module_semantic_->link_symbol(dup_prog.get(), module_semantic_->root_symbol() );
	module_semantic_->set_program(dup_prog);
}

void semantic_analyser::empty_caster( node* /*lhs*/, node* /*rhs*/ )
{
}

// Add casters for scalar, vector, matrix.
void add_svm_casters(
	shared_ptr<caster_t> const& caster, caster_t::casts casts, 
	builtin_types src, builtin_types dst,
	caster_t::cast_t fn, pety_t* pety
	)
{
	caster->add_cast_auto_prior( casts, pety->get(src), pety->get(dst), fn );
	for( size_t i = 1; i <= 4; ++i )
	{
		caster->add_cast_auto_prior( casts,
			pety->get( vector_of(src, i) ), pety->get( vector_of(dst, i) ),
			fn );
		for( size_t j = 1; j <= 4; ++j )
		{
			caster->add_cast_auto_prior( casts,
				pety->get( matrix_of(src, i, j) ), pety->get( matrix_of(dst, i, j) ),
				fn );
		}
	}
}

void semantic_analyser::initialize_casts(){
	// register default type converter
	pety_t* pety = module_semantic_->pety();

	builtin_types sint8_bt	= builtin_types::_sint8	;
	builtin_types sint16_bt	= builtin_types::_sint16;
	builtin_types sint32_bt	= builtin_types::_sint32;
	builtin_types sint64_bt	= builtin_types::_sint64;

	builtin_types uint8_bt	= builtin_types::_uint8	;
	builtin_types uint16_bt	= builtin_types::_uint16;
	builtin_types uint32_bt	= builtin_types::_uint32;
	builtin_types uint64_bt	= builtin_types::_uint64;

	builtin_types float_bt	= builtin_types::_float	;
	builtin_types double_bt	= builtin_types::_double;

	builtin_types bool_bt	= builtin_types::_boolean;

	// default conversation will do nothing.
	caster_t::cast_t default_conv = bind(&semantic_analyser::empty_caster, this, _1, _2);

	add_svm_casters( caster, caster_t::imp, sint8_bt, sint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, sint8_bt, sint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, sint8_bt, sint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, sint8_bt, float_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::imp, sint8_bt, double_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, sint8_bt, bool_bt,   default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint8_bt, uint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint8_bt, uint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint8_bt, uint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint8_bt, uint64_bt, default_conv, pety );

	add_svm_casters( caster, caster_t::exp, sint16_bt, sint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::imp, sint16_bt, sint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, sint16_bt, sint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint16_bt, uint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint16_bt, uint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint16_bt, uint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint16_bt, uint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, sint16_bt, float_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::imp, sint16_bt, double_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, sint16_bt, bool_bt,   default_conv, pety );

	add_svm_casters( caster, caster_t::exp, sint32_bt, sint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint32_bt, sint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, sint32_bt, sint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint32_bt, uint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint32_bt, uint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint32_bt, uint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint32_bt, uint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, sint32_bt, float_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::imp, sint32_bt, double_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, sint32_bt, bool_bt,   default_conv, pety );

	add_svm_casters( caster, caster_t::exp, sint64_bt, sint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint64_bt, sint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint64_bt, sint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint64_bt, uint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint64_bt, uint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint64_bt, uint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, sint64_bt, uint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, sint64_bt, float_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::imp, sint64_bt, double_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, sint64_bt, bool_bt,   default_conv, pety );

	add_svm_casters( caster, caster_t::exp, uint8_bt, sint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint8_bt, sint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint8_bt, sint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint8_bt, sint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint8_bt, uint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint8_bt, uint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint8_bt, uint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint8_bt, float_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint8_bt, double_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint8_bt, bool_bt,   default_conv, pety );

	add_svm_casters( caster, caster_t::exp, uint16_bt, sint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, uint16_bt, sint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint16_bt, sint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint16_bt, sint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, uint16_bt, uint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint16_bt, uint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint16_bt, uint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint16_bt, float_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint16_bt, double_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint16_bt, bool_bt,   default_conv, pety );

	add_svm_casters( caster, caster_t::imp, uint32_bt, uint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint32_bt, sint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint32_bt, float_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint32_bt, double_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint32_bt, bool_bt,   default_conv, pety );
	add_svm_casters( caster, caster_t::exp, uint32_bt, sint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, uint32_bt, sint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, uint32_bt, sint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, uint32_bt, uint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, uint32_bt, uint16_bt, default_conv, pety );

	add_svm_casters( caster, caster_t::exp, uint64_bt, sint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, uint64_bt, sint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, uint64_bt, sint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, uint64_bt, sint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, uint64_bt, uint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, uint64_bt, uint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, uint64_bt, uint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint64_bt, float_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint64_bt, double_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, uint64_bt, bool_bt,   default_conv, pety );
 
	add_svm_casters( caster, caster_t::exp, float_bt, sint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, float_bt, sint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, float_bt, sint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, float_bt, sint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, float_bt, uint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, float_bt, uint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, float_bt, uint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, float_bt, uint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, float_bt, double_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::imp, float_bt, bool_bt,   default_conv, pety );

	add_svm_casters( caster, caster_t::exp, double_bt, sint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, double_bt, sint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, double_bt, sint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, double_bt, sint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, double_bt, uint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, double_bt, uint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, double_bt, uint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, double_bt, uint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, double_bt, float_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::imp, double_bt, bool_bt,   default_conv, pety );

	add_svm_casters( caster, caster_t::exp, bool_bt, sint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, bool_bt, sint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, bool_bt, sint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, bool_bt, sint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, bool_bt, uint8_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, bool_bt, uint16_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, bool_bt, uint32_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, bool_bt, uint64_bt, default_conv, pety );
	add_svm_casters( caster, caster_t::exp, bool_bt, float_bt,  default_conv, pety );
	add_svm_casters( caster, caster_t::exp, bool_bt, double_bt, default_conv, pety );

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
		tid_t bt_tid = pety->get( bt );
		tid_t v1bt_tid = pety->get( v1bt );
		caster->add_cast( caster_t::eql, bt_tid, v1bt_tid, default_conv );
		caster->add_cast( caster_t::eql, v1bt_tid, bt_tid, default_conv );
	}
}

void semantic_analyser::initialize_operator_parameter_lrvs()
{
	using lvalue_or_rvalue::lvalue;
	using lvalue_or_rvalue::rvalue;

	// Binary operators
	operator_parameter_lrvs_.insert( make_pair( operators::add, parameter_lrvs(rvalue, rvalue, rvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::sub, parameter_lrvs(rvalue, rvalue, rvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::mul, parameter_lrvs(rvalue, rvalue, rvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::div, parameter_lrvs(rvalue, rvalue, rvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::mod, parameter_lrvs(rvalue, rvalue, rvalue) ) );
											    
	operator_parameter_lrvs_.insert( make_pair( operators::bit_and, parameter_lrvs(rvalue, rvalue, rvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::bit_or , parameter_lrvs(rvalue, rvalue, rvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::bit_xor, parameter_lrvs(rvalue, rvalue, rvalue) ) );
											    
	operator_parameter_lrvs_.insert( make_pair( operators::logic_and, parameter_lrvs(rvalue, rvalue, rvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::logic_or , parameter_lrvs(rvalue, rvalue, rvalue) ) );
											    
	operator_parameter_lrvs_.insert( make_pair( operators::left_shift , parameter_lrvs(rvalue, rvalue, rvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::right_shift, parameter_lrvs(rvalue, rvalue, rvalue) ) );
										    
	operator_parameter_lrvs_.insert( make_pair( operators::less			, parameter_lrvs(rvalue, rvalue, rvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::less_equal	, parameter_lrvs(rvalue, rvalue, rvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::equal		, parameter_lrvs(rvalue, rvalue, rvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::greater_equal, parameter_lrvs(rvalue, rvalue, rvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::greater		, parameter_lrvs(rvalue, rvalue, rvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::not_equal	, parameter_lrvs(rvalue, rvalue, rvalue) ) );

	// Assigns
	operator_parameter_lrvs_.insert( make_pair( operators::assign, parameter_lrvs(lvalue, rvalue, lvalue) ) );

	operator_parameter_lrvs_.insert( make_pair( operators::add_assign, parameter_lrvs(lvalue, rvalue, lvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::sub_assign, parameter_lrvs(lvalue, rvalue, lvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::mul_assign, parameter_lrvs(lvalue, rvalue, lvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::div_assign, parameter_lrvs(lvalue, rvalue, lvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::mod_assign, parameter_lrvs(lvalue, rvalue, lvalue) ) );

	operator_parameter_lrvs_.insert( make_pair( operators::bit_and_assign, parameter_lrvs(lvalue, rvalue, lvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::bit_or_assign , parameter_lrvs(lvalue, rvalue, lvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::bit_xor_assign, parameter_lrvs(lvalue, rvalue, lvalue) ) );
										    
	operator_parameter_lrvs_.insert( make_pair( operators::lshift_assign, parameter_lrvs(lvalue, rvalue, lvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::rshift_assign, parameter_lrvs(lvalue, rvalue, lvalue) ) );

	// Unary operators
	operator_parameter_lrvs_.insert( make_pair( operators::positive, parameter_lrvs(rvalue, rvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::negative, parameter_lrvs(rvalue, rvalue) ) );

	operator_parameter_lrvs_.insert( make_pair( operators::prefix_incr, parameter_lrvs(lvalue, lvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::prefix_decr, parameter_lrvs(lvalue, lvalue) ) );

	operator_parameter_lrvs_.insert( make_pair( operators::postfix_incr, parameter_lrvs(rvalue, lvalue) ) );
	operator_parameter_lrvs_.insert( make_pair( operators::postfix_decr, parameter_lrvs(rvalue, lvalue) ) );
}

struct proto_info
{
public:
	proto_info() {}
	proto_info(tid_t fn_tid, std::vector<tid_t> const& params_tid)
		: fn_tid(fn_tid), ret_tid(params_tid[0]), params_count( params_tid.size()-1 )
	{
		memset( this->params_tid, -1, sizeof(this->params_tid) );
		assert( params_count <= 5 );
		memcpy( &(this->params_tid[0]), &params_tid[1], sizeof(tid_t) * params_count );
	}

	proto_info(proto_info const& rhs)
	{
		*this = rhs;
	}

	proto_info& operator = (proto_info const& rhs)
	{
		memcpy( this, &rhs, sizeof(proto_info) );
		return *this;
	}

	tid_t	fn_tid;
	tid_t	ret_tid;
	tid_t	params_tid[5];
	size_t	params_count;
};

class proto_cache
{
public:
	class proto_cache_inserter
	{
	public:
		friend class proto_cache;

		proto_cache_inserter(pety_t* pety, vector<proto_info>& protos, vector<tid_t>& result_param_tids)
			: pety_(pety), protos_(protos), result_param_tids_(result_param_tids)
		{
		}

		size_t result(tid_t result_tid)
		{
			size_t proto_index = protos_.size();
			result_param_tids_[0] = result_tid;
			tid_t fn_tid = pety_->get_function_type(result_param_tids_);
			protos_.push_back( proto_info(fn_tid, result_param_tids_) );
			return proto_index;
		}

	private:
		proto_cache_inserter(proto_cache_inserter const& rhs)
			: pety_(rhs.pety_), protos_(rhs.protos_), result_param_tids_(rhs.result_param_tids_)
		{
		}

		proto_cache_inserter& operator = (proto_cache_inserter const&);

		pety_t*				pety_;
		vector<proto_info>&	protos_;
		vector<tid_t>&		result_param_tids_;
	};

	proto_cache(pety_t* pety): pety_(pety) {}

	proto_cache_inserter add_proto(tid_t p0_tid)
	{
		result_param_tids_.resize(2);
		result_param_tids_[1] = p0_tid;
		return proto_cache_inserter(pety_, protos_, result_param_tids_);
	}

	proto_cache_inserter add_proto(tid_t p0_tid, tid_t p1_tid)
	{
		result_param_tids_.resize(3);
		result_param_tids_[1] = p0_tid;
		result_param_tids_[2] = p1_tid;
		return proto_cache_inserter(pety_, protos_, result_param_tids_);
	}

	proto_cache_inserter add_proto(tid_t p0_tid, tid_t p1_tid, tid_t p2_tid)
	{
		result_param_tids_.resize(4);
		result_param_tids_[1] = p0_tid;
		result_param_tids_[2] = p1_tid;
		result_param_tids_[3] = p2_tid;
		return proto_cache_inserter(pety_, protos_, result_param_tids_);
	}

	proto_cache_inserter add_proto(tid_t p0_tid, tid_t p1_tid, tid_t p2_tid, tid_t p3_tid)
	{
		result_param_tids_.resize(5);
		result_param_tids_[1] = p0_tid;
		result_param_tids_[2] = p1_tid;
		result_param_tids_[3] = p2_tid;
		result_param_tids_[4] = p3_tid;
		return proto_cache_inserter(pety_, protos_, result_param_tids_);
	}

	proto_cache_inserter add_proto(vector<tid_t> const& param_tids)
	{
		result_param_tids_.resize(param_tids.size()+1);
		std::copy( param_tids.begin(), param_tids.end(), result_param_tids_.begin()+1 );
		return proto_cache_inserter(pety_, protos_, result_param_tids_);
	}

	vector<proto_info> const& protos() const
	{
		return protos_;
	}
private:
	pety_t*				pety_;
	vector<proto_info>	protos_;
	vector<tid_t>		result_param_tids_;
};

void semantic_analyser::register_builtin_functions2()
{
	pety_t* pety = module_semantic_->pety();
	vector<builtin_types>	builtins;
	vector<tid_t>			tids;

	// Proto groups
	proto_cache protos(pety);

	vector<size_t>	arith_fns;
	vector<size_t>	arith_assign_fns;
	vector<size_t>	relationship_fns;
	vector<size_t>	bit_shift_fns;
	vector<size_t>	bool_arith_fns;
	vector<size_t>	prefix_postfix_positive_fns;
	vector<size_t>	bit_not_fns;
	vector<size_t>	logic_not_fns;
	vector<size_t>	abs_negative_fns;
	vector<size_t>	assign_fns;
	vector<size_t>	min_max_fns;
	vector<size_t>	all_any_fns;
	vector<size_t>	mad_clamp_fns;
	vector<size_t>	ddx_ddy_fns;
	vector<size_t>	vf_vf_intrins;
	vector<size_t>	vf_vfvf_intrins;
	vector<size_t>	vf_vfvfvf_intrins;
	vector<size_t>	vb_vf_intrins;

	// Function groups
	vector<fixed_string>	vf_vf_intrin_names;
	vf_vf_intrin_names.reserve(25);
	vf_vf_intrin_names +=
		fixed_string("degrees"), fixed_string("radians"), 
		fixed_string("sqrt"), fixed_string("exp"), fixed_string("exp2"),
		fixed_string("sin"), fixed_string("cos"), fixed_string("tan"),
		fixed_string("asin"), fixed_string("acos"), fixed_string("atan"),
		fixed_string("ceil"), fixed_string("floor"),
		fixed_string("log"), fixed_string("log2"), fixed_string("log10"),
		fixed_string("sinh"), fixed_string("cosh"), fixed_string("tanh"),
		fixed_string("frac"), fixed_string("saturate"), fixed_string("round"), fixed_string("trunc"), 
		fixed_string("rsqrt"), fixed_string("rcp");

	vector<fixed_string>	vf_vfvf_intrin_names;
	vf_vfvf_intrin_names.reserve(4);
	vf_vfvf_intrin_names +=
		fixed_string("fmod"), fixed_string("ldexp"), fixed_string("pow"), fixed_string("step");

	vector<fixed_string>	vf_vfvfvf_intrin_names;
	vf_vfvfvf_intrin_names.reserve(4);
	vf_vfvfvf_intrin_names +=
		fixed_string("lerp"), fixed_string("smoothstep");

	vector<fixed_string>	vb_vf_intrin_names;
	vb_vf_intrin_names.reserve(3);
	vb_vf_intrin_names +=
		fixed_string("isinf"), fixed_string("isfinite"), fixed_string("isnan");

	// Initialize builtins and tids.
	list_of_builtin_type(builtins, &is_storagable);
	for(size_t i_builtin = 0; i_builtin < builtins.size(); ++i_builtin)
	{
		tids.push_back( pety->get(builtins[i_builtin]) );
	}

	tid_t bool_tid = pety->get(builtin_types::_boolean);
	for(size_t i_builtin = 0; i_builtin < builtins.size(); ++i_builtin)
	{
		builtin_types	btc = builtins[i_builtin];
		tid_t			bttid = tids[i_builtin];
		tid_t			same_dimension_bool = pety->get( replace_scalar(btc, builtin_types::_boolean) );

		// Generate protos
		size_t b_v  = protos.add_proto(bttid).result(same_dimension_bool);
		size_t sb_v = protos.add_proto(bttid).result(bool_tid);
		size_t v_v  = protos.add_proto(bttid).result(bttid);
		size_t b_vv = protos.add_proto(bttid, bttid).result(same_dimension_bool);
		size_t v_vv = protos.add_proto(bttid, bttid).result(bttid);
		
		// Add proto to groups

		// Numeric only
		if( scalar_of(btc) != builtin_types::_boolean )
		{
			size_t v_vvv = protos.add_proto(bttid, bttid, bttid).result(bttid);

			arith_fns.push_back(v_vv);
			arith_assign_fns.push_back(v_vv);
			mad_clamp_fns.push_back(v_vvv);

			// PS Only
			if( lang == salviar::lang_pixel_shader )
			{
				ddx_ddy_fns.push_back(v_v);
			}

			// Integer only
			if( is_integer(btc) )
			{
				bit_shift_fns.push_back(v_vv);
				if( is_standard(btc) )
				{
					prefix_postfix_positive_fns.push_back(v_v);
					bit_not_fns.push_back(v_v);
				}
			}
			// Float only.
			// TODO they all need support double in future.
			else if( scalar_of(btc) == builtin_types::_float )
			{
				vf_vf_intrins.push_back(v_v);
				vf_vfvf_intrins.push_back(v_vv);
				vf_vfvfvf_intrins.push_back(v_vvv);
				vb_vf_intrins.push_back(b_v);
			}

			// Signed and float only.
			if( is_signed(btc) || is_real(btc) )
			{
				abs_negative_fns.push_back(v_v);
			}

			// Vector or Matrix only.
			if(    ( is_vector(btc) && vector_size(btc) > 1 )
				|| ( is_matrix(btc) && (vector_size(btc) * vector_count(btc) > 1) ) )
			{
				tid_t scalar_bttid = pety->get( scalar_of(btc) );
				size_t v_sv = protos.add_proto(scalar_bttid, bttid).result(bttid);
				size_t v_vs = protos.add_proto(bttid, scalar_bttid).result(bttid);
				
				arith_assign_fns.push_back(v_vs);
				arith_fns.push_back(v_vs);
				arith_fns.push_back(v_sv);
			}
		}
		// Boolean only
		else
		{
			bool_arith_fns.push_back(v_vv);
			logic_not_fns.push_back(v_v);
		}

		// All types available
		assign_fns.push_back(v_vv);
		relationship_fns.push_back(b_vv);
		min_max_fns.push_back(v_vv);
		all_any_fns.push_back(sb_v);
	}

	// Register operators
	{
		vector<operators> const& oplist = list_of_operators();
		for( size_t i_op = 0; i_op < oplist.size(); ++i_op )
		{
			operators op = oplist[i_op];
			fixed_string op_name = module_semantic_->pety()->operator_name(op);

			if ( is_arithmetic(op) )
			{
				register_function2(op_name, arith_fns, protos.protos());
			}
			else if( is_arith_assign(op) )
			{
				register_function2(op_name, arith_assign_fns, protos.protos());
			}
			else if( is_relationship(op) )
			{
				register_function2(op_name, relationship_fns, protos.protos());
			}
			else if( is_bit(op) || is_bit_assign(op) || is_shift(op) || is_shift_assign(op) )
			{
				register_function2(op_name, bit_shift_fns, protos.protos());
			}
			else if( is_bool_arith(op) )
			{
				register_function2(op_name, bool_arith_fns, protos.protos());
			}
			else if( is_prefix(op) || is_postfix(op) || op == operators::positive )
			{
				register_function2(op_name, prefix_postfix_positive_fns, protos.protos());
			}
			else if( op == operators::bit_not )
			{
				register_function2(op_name, bit_not_fns, protos.protos());
			}
			else if( op == operators::logic_not )
			{
				register_function2(op_name, logic_not_fns, protos.protos());
			}
			else if( op == operators::negative )
			{
				register_function2(op_name, abs_negative_fns, protos.protos());
			}
			else if ( op == operators::assign )
			{
				register_function2(op_name, assign_fns, protos.protos());
			}
		}
	}

	// Register intrinsics
	{
		for(size_t i = 0; i < vb_vf_intrin_names.size(); ++i)
		{
			register_intrinsic2( vb_vf_intrin_names[i], vb_vf_intrins, protos.protos() );
		}

		for(size_t i = 0; i < vf_vf_intrin_names.size(); ++i)
		{
			register_intrinsic2( vf_vf_intrin_names[i], vf_vf_intrins, protos.protos() );
		}

		for(size_t i = 0; i < vf_vfvf_intrin_names.size(); ++i)
		{
			register_intrinsic2( vf_vfvf_intrin_names[i], vf_vfvf_intrins, protos.protos() );
		}
		
		for(size_t i = 0; i < vf_vfvfvf_intrin_names.size(); ++i)
		{
			register_intrinsic2( vf_vfvfvf_intrin_names[i], vf_vfvfvf_intrins, protos.protos() );
		}

		register_intrinsic2( "abs",		abs_negative_fns,	protos.protos() );
		register_intrinsic2( "min",		min_max_fns,		protos.protos() );
		register_intrinsic2( "max",		min_max_fns,		protos.protos() );
		register_intrinsic2( "all",		all_any_fns,		protos.protos() );
		register_intrinsic2( "any",		all_any_fns,		protos.protos() );
		register_intrinsic2( "mad",		mad_clamp_fns,		protos.protos() );
		register_intrinsic2( "clamp",	mad_clamp_fns,		protos.protos() );
		register_intrinsic2( "ddx",		ddx_ddy_fns,		protos.protos() );
		register_intrinsic2( "ddy",		ddx_ddy_fns,		protos.protos() );
	}

	// Register functions with share-less signatures
	{
		tid_t float_tid = pety->get(builtin_types::_float);
		tid_t fvec_tids[5];
		tid_t float_matrix_tids[5][5];

		for(size_t vec_size = 1; vec_size <= 4; ++vec_size)
		{
			fvec_tids[vec_size] = pety->get( vector_of(builtin_types::_float, vec_size) );
			for(size_t vec_cnt = 1; vec_cnt <= 4; ++vec_cnt)
			{
				float_matrix_tids[vec_size][vec_cnt] =  pety->get(
					matrix_of(builtin_types::_float, vec_size, vec_cnt)
				);
			}
		}

		vector<size_t>	norm_fns;
		vector<size_t>	length_fns;
		vector<size_t>	dist_dot_fns;
		vector<size_t>	reflect_fns;
		vector<size_t>	refract_fns;
		vector<size_t>	faceforward_fns;

		for( size_t i = 1; i <= 4; ++i )
		{
			norm_fns.push_back(
				protos.add_proto(fvec_tids[i]).result(fvec_tids[i]) );
			length_fns.push_back(
				protos.add_proto(fvec_tids[i]).result(float_tid) );
			dist_dot_fns.push_back(
				protos.add_proto(fvec_tids[i], fvec_tids[i]).result(float_tid) );
			reflect_fns.push_back(
				protos.add_proto(fvec_tids[i], fvec_tids[i]).result(fvec_tids[i]) );
			refract_fns.push_back(
				protos.add_proto(fvec_tids[i], fvec_tids[i], float_tid).result(fvec_tids[i]) );
			faceforward_fns.push_back(
				protos.add_proto(fvec_tids[i], fvec_tids[i], fvec_tids[i]).result(fvec_tids[i]) );
		}
							 
		register_intrinsic2( "dot",			dist_dot_fns,		protos.protos() );
		register_intrinsic2( "distance",	dist_dot_fns,		protos.protos() );
		register_intrinsic2( "reflect",		reflect_fns,		protos.protos() );
		register_intrinsic2( "refract",		refract_fns,		protos.protos() );
		register_intrinsic2( "length",		length_fns,			protos.protos() );
		register_intrinsic2( "normalize",	norm_fns,			protos.protos() );
		register_intrinsic2( "faceforward",	faceforward_fns,	protos.protos() );
							 
		vector<size_t> dst_fns;
		dst_fns.push_back(
			protos.add_proto(fvec_tids[4], fvec_tids[4]).result(fvec_tids[4])
			);
		register_intrinsic2( "dst", dst_fns, protos.protos() );

		vector<size_t> cross_fns;
		cross_fns.push_back(
			protos.add_proto(fvec_tids[3], fvec_tids[3]).result(fvec_tids[3])
			);
		register_intrinsic2( "cross", cross_fns, protos.protos() );

		
		vector<size_t> lit_fns;
		lit_fns.push_back(
			protos.add_proto(float_tid, float_tid, float_tid).result(fvec_tids[4])
			);
		register_intrinsic2( "lit", lit_fns, protos.protos() );

		tid_t sampler_tid = pety->get(builtin_types::_sampler);

		vector<size_t> tex_fns(1);
		if( lang == salviar::lang_pixel_shader || lang == salviar::lang_vertex_shader )
		{
			tex_fns[0] = protos.add_proto(sampler_tid, fvec_tids[4]).result(fvec_tids[4]);

			register_intrinsic2("tex2Dlod",   tex_fns, protos.protos(), lang == salviar::lang_pixel_shader);
			register_intrinsic2("texCUBElod", tex_fns, protos.protos(), lang == salviar::lang_pixel_shader);

			if(lang == salviar::lang_pixel_shader)
			{
				register_intrinsic2("tex2Dbias", tex_fns, protos.protos(), true);
				register_intrinsic2("tex2Dproj", tex_fns, protos.protos(), true);

				tex_fns[0] = protos.add_proto(sampler_tid, fvec_tids[2]).result(fvec_tids[4]);
				register_intrinsic2("tex2D", tex_fns, protos.protos(), true);

				tex_fns[0] = protos
					.add_proto(sampler_tid, fvec_tids[2], fvec_tids[2], fvec_tids[2])
					.result(fvec_tids[4]);
				register_intrinsic2("tex2Dgrad", tex_fns, protos.protos(), true);
			}
		}
		
		vector<size_t> mul_fns;
		for( size_t vec_size = 1; vec_size <= 4; ++vec_size){
			for( size_t vec_cnt = 1; vec_cnt <= 4; ++vec_cnt ){
				mul_fns.push_back(
					protos.add_proto(fvec_tids[vec_cnt], float_matrix_tids[vec_size][vec_cnt])
					.result(fvec_tids[vec_size])
					);
				mul_fns.push_back(
					protos.add_proto(float_matrix_tids[vec_size][vec_cnt], fvec_tids[vec_size])
					.result(fvec_tids[vec_cnt])
					);
			}
		}
		register_intrinsic2( "mul", mul_fns, protos.protos() );

		vector<tid_t> int_tids;
		vector<tid_t> uint_tids;
		vector<tid_t> float_tids;

		int_tids.push_back	( pety->get(builtin_types::_sint32) );
		uint_tids.push_back	( pety->get(builtin_types::_uint32) );
		float_tids.push_back( pety->get(builtin_types::_float)	);

		for( size_t vsize = 1; vsize <= 4; ++vsize )
		{
			int_tids.push_back	( pety->get( vector_of(builtin_types::_sint32, vsize) ) );
			uint_tids.push_back	( pety->get( vector_of(builtin_types::_uint32, vsize) ) );
			float_tids.push_back( pety->get( vector_of(builtin_types::_float , vsize) ) );

			for( size_t vcnt = 1; vcnt <= 4; ++vcnt )
			{
				int_tids.push_back	( pety->get( matrix_of(builtin_types::_sint32, vsize, vcnt) ) );
				uint_tids.push_back	( pety->get( matrix_of(builtin_types::_uint32, vsize, vcnt) ) );
				float_tids.push_back( pety->get( matrix_of(builtin_types::_float,  vsize, vcnt) ) );
			}
		}

		vector<size_t> asint_fns;
		vector<size_t> asuint_fns;
		vector<size_t> asfloat_fns;
		vector<size_t> bits_fns;
		vector<size_t> sign_fns;

		for(size_t i_tid = 0; i_tid < int_tids.size(); ++i_tid)
		{
			tid_t int_tid = int_tids[i_tid];
			tid_t uint_tid = uint_tids[i_tid];
			tid_t float_tid = float_tids[i_tid];

			asint_fns.push_back( protos.add_proto(uint_tid).result(int_tid) );
			asint_fns.push_back( protos.add_proto(float_tid).result(int_tid) );

			asuint_fns.push_back( protos.add_proto(int_tid).result(uint_tid) );
			asuint_fns.push_back( protos.add_proto(float_tid).result(uint_tid) );

			asfloat_fns.push_back( protos.add_proto(int_tid).result(float_tid) );
			asfloat_fns.push_back( protos.add_proto(uint_tid).result(float_tid) );

			bits_fns.push_back( protos.add_proto(uint_tid).result(uint_tid) );
			bits_fns.push_back( protos.add_proto(int_tid).result(int_tid) );

			sign_fns.push_back( protos.add_proto(float_tid).result(int_tid) );
		}

		register_intrinsic2( "asint",   asint_fns,   protos.protos() );
		register_intrinsic2( "asuint",  asuint_fns,  protos.protos() );
		register_intrinsic2( "asfloat", asfloat_fns, protos.protos() );

		register_intrinsic2( "countbits",   bits_fns, protos.protos() );
		register_intrinsic2( "count_bits",  bits_fns, protos.protos() );
		register_intrinsic2( "firstbithigh",bits_fns, protos.protos() );
		register_intrinsic2( "firstbitlow", bits_fns, protos.protos() );
		register_intrinsic2( "reversebits", bits_fns, protos.protos() );
		register_intrinsic2( "sign",		sign_fns, protos.protos() );
	}

	// Register constructors
	{
		vector< pair<builtin_types, char const*> > scalar_bts;
		vector<size_t> constructor_protos;

		scalar_bts.push_back( make_pair(builtin_types::_boolean, "bool" ) );
		scalar_bts.push_back( make_pair(builtin_types::_sint32,  "int"  ) );
		scalar_bts.push_back( make_pair(builtin_types::_float,   "float") );

		vector<size_t>	param_indexes;
		vector<tid_t>	param_tids;
		param_indexes.reserve(4);
		param_tids.reserve(4);

		vector<size_t>	constructor_fns;

		for(size_t i_scalar = 0; i_scalar < scalar_bts.size(); ++i_scalar)
		{
			builtin_types scalar_bt = scalar_bts[i_scalar].first;
			tid_t vec_tids[5] = 
			{
				0,
				pety->get( scalar_bt ),
				pety->get( vector_of(scalar_bt, 2) ),
				pety->get( vector_of(scalar_bt, 3) ),
				pety->get( vector_of(scalar_bt, 4) )
			};
			
			for(size_t scalar_count = 2; scalar_count <= 4; ++scalar_count)
			{
				fixed_string constructor_name(scalar_bts[i_scalar].second);
				constructor_name.mutable_raw_string().append( 1, static_cast<char>('0' + scalar_count) );
				constructor_fns.clear();
				
				// Enumerate all constructor paramters.
				size_t used_scalar_count = 0;
				param_indexes.push_back(0);
				while( !param_indexes.empty() )
				{
					param_indexes.back() += 1;
					used_scalar_count += 1;

					if(used_scalar_count == scalar_count)
					{
						// Output parameter tids
						param_tids.clear();
						for(size_t i_param = 0; i_param < param_indexes.size(); ++i_param)
						{
							param_tids.push_back(vec_tids[ param_indexes[i_param] ]);
						}
						constructor_fns.push_back( protos.add_proto(param_tids).result(vec_tids[scalar_count]) );
						used_scalar_count -= param_indexes.back();
						param_indexes.pop_back();
						continue;
					}
					else
					{
						param_indexes.push_back(0);
					}
				}
				
				register_constructor2( constructor_name, constructor_fns, protos.protos() );
			}
		}
	}
}

void semantic_analyser::register_function2(
	fixed_string const& name,
	vector<size_t> const& proto_indexes,
	vector<proto_info> const& protos,
	bool is_intrinsic,
	bool is_partial_exec,
	bool is_constructor
	)
{
	pety_t* pety = module_semantic_->pety();
	symbol::overload_position overload_pos
		= current_symbol->get_overload_position(name);

	for(size_t i_proto = 0; i_proto < proto_indexes.size(); ++i_proto)
	{
		proto_info const& proto = protos[ proto_indexes[i_proto] ];

		shared_ptr<function_def> fn_def = create_node<function_def>(
			shared_ptr<token_t>(), shared_ptr<token_t>()
			);
		hold_generated_node(fn_def);

		fn_def->name = token_t::from_string(name);
		fn_def->type
			= pety->get_proto(proto.fn_tid)->as_handle<function_type>();
		assert(fn_def->type);

		for(size_t i_param = 0; i_param < proto.params_count; ++i_param)
		{
			shared_ptr<parameter> param = create_node<parameter>( token_t_ptr(), token_t_ptr() );
			fn_def->params.push_back(param);
			node_semantic* param_sem = create_node_semantic(param);
			param_sem->tid( proto.params_tid[i_param] );
		}

		node_semantic* fn_sem = create_node_semantic(fn_def);
		
		fn_sem->tid(proto.ret_tid);

		fn_sem->is_intrinsic(is_intrinsic);
		fn_sem->msc_compatible(!is_intrinsic);
		fn_sem->partial_execution(is_partial_exec);
		fn_sem->is_constructor(is_constructor);
		
		symbol* sym = current_symbol->unchecked_insert_overload(overload_pos, fn_def.get(), proto.fn_tid);

		if( is_intrinsic )
		{
			module_semantic_->intrinsics().push_back(sym);
		}
	}
}

void semantic_analyser::register_intrinsic2(
	eflib::fixed_string		const& name,
	std::vector<size_t>		const& proto_indexes,
	std::vector<proto_info> const& protos,
	bool partial_exec
	)
{
	register_function2(name, proto_indexes, protos, true, partial_exec, false);
}

void semantic_analyser::register_constructor2(
	eflib::fixed_string		const& name,
	std::vector<size_t>		const& proto_indexes,
	std::vector<proto_info> const& protos
	)
{
	register_function2(name, proto_indexes, protos, true, false, true);
}

void semantic_analyser::register_builtin_types(){
	BOOST_FOREACH( builtin_types const & btc, list_of_builtin_types() ){
		if( module_semantic_->pety()->get( btc ) == -1 ){
			assert( !"Register builtin type failed!" );
		}
	}
}

module_semantic_ptr const& semantic_analyser::get_module_semantic() const{
	return module_semantic_;
}

void semantic_analyser::mark_intrin_invoked_recursive(symbol* sym)
{
	node_semantic* ssi = get_node_semantic( sym->associated_node() );
	
	assert( ssi ); // Function must be analyzed.
	
	if( ssi->is_invoked() ){ return; }
		
	ssi->is_invoked( true );
}

diag_chat* semantic_analyser::get_diags() const
{
	return diags.get();
}

uint32_t semantic_analyser::language() const
{
	return lang;
}

void semantic_analyser::language( uint32_t v )
{
	lang = v;
}

node_semantic* semantic_analyser::get_node_semantic(sasl::syntax_tree::node* v)
{
	return module_semantic_->get_semantic(v);
}

node_semantic* semantic_analyser::get_node_semantic(sasl::syntax_tree::node_ptr const& v)
{
	return module_semantic_->get_semantic(v);
}

node_semantic* semantic_analyser::create_node_semantic(sasl::syntax_tree::node* v)
{
	return module_semantic_->create_semantic(v);
}

node_semantic* semantic_analyser::create_node_semantic( sasl::syntax_tree::node_ptr const& v )
{
	return module_semantic_->create_semantic( v.get() );
}

node_semantic* semantic_analyser::get_or_create_semantic( sasl::syntax_tree::node* v )
{
	return module_semantic_->get_or_create_semantic(v);
}

node_semantic* semantic_analyser::get_or_create_semantic( sasl::syntax_tree::node_ptr const& v )
{
	return module_semantic_->get_or_create_semantic(v);
}

std::string semantic_analyser::unique_structure_name()
{
	boost::uuids::random_generator gen;
	boost::uuids::uuid u = gen();
	return std::string("__unnamed_struct_") + boost::uuids::to_string(u);
}

symbol* semantic_analyser::get_symbol(node* v)
{
	return module_semantic_->get_symbol(v);
}

symbol* semantic_analyser::get_symbol(node_ptr const& v)
{
	return module_semantic_->get_symbol( v.get() );
}

void semantic_analyser::hold_generated_node(node_ptr const& v)
{
	module_semantic_->hold_generated_node(v);
}

void semantic_analyser::mark_modified(expression* expr)
{
	node_semantic* sem = get_node_semantic(expr);
	assert(sem);

	if(expr->node_class() == node_ids::variable_expression)
	{
		node* decl = sem->referenced_declarator();
		symbol* decl_sym = get_symbol(decl);
		module_semantic_->modify(decl_sym);
		return;
	}

	if(expr->node_class() == node_ids::member_expression)
	{
		member_expression* mem_expr = static_cast<member_expression*>(expr);
		mark_modified( mem_expr->expr.get() );
		return;
	}

	if(expr->node_class() == node_ids::binary_expression)
	{
		binary_expression* bin_expr = static_cast<binary_expression*>(expr);
		if( is_general_assign(bin_expr->op) )
		{
			// Marked as modified when sub expression is evaluated.
			return;
		}

		EFLIB_ASSERT_UNIMPLEMENTED();
		return;
	}
	EFLIB_ASSERT_UNIMPLEMENTED();
}

semantic_analyser::parameter_lrvs::parameter_lrvs(
	lvalue_or_rvalue::id ret_lrv,
	lvalue_or_rvalue::id lrv_p0,
	lvalue_or_rvalue::id lrv_p1,
	lvalue_or_rvalue::id lrv_p2 )
	: ret_lrv(ret_lrv)
{
	param_lrvs[0] = lrv_p0;
	param_lrvs[1] = lrv_p1;
	param_lrvs[2] = lrv_p2;
}

END_NS_SASL_SEMANTIC();
