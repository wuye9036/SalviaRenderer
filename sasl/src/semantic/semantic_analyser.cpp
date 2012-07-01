#include <sasl/include/semantic/semantic_analyser.h>

#include <sasl/enums/operators.h>

#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/semantic_diags.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/semantic/caster.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/make_tree.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/utility.h>
#include <sasl/include/common/scope_guard.h>
#include <sasl/include/common/diag_chat.h>

#include <salviar/include/enums.h>
#include <salviar/include/shader_abi.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/metaprog/util.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/assign/list_of.hpp>
#include <boost/assign/list_inserter.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/bind/apply.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
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
using sasl::syntax_tree::declaration;
using sasl::syntax_tree::declarator;
using sasl::syntax_tree::declaration_statement;
using sasl::syntax_tree::dowhile_statement;
using sasl::syntax_tree::expression;
using sasl::syntax_tree::expression_initializer;
using sasl::syntax_tree::expression_statement;
using sasl::syntax_tree::for_statement;
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, function_type);
using sasl::syntax_tree::if_statement;
using sasl::syntax_tree::index_expression;
using sasl::syntax_tree::jump_statement;
using sasl::syntax_tree::label;
using sasl::syntax_tree::labeled_statement;
using sasl::syntax_tree::member_expression;
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, node);
using sasl::syntax_tree::parameter;
using sasl::syntax_tree::program;
using sasl::syntax_tree::statement;
using sasl::syntax_tree::struct_type;
using sasl::syntax_tree::switch_statement;
EFLIB_USING_SHARED_PTR(sasl::syntax_tree, tynode);
using sasl::syntax_tree::unary_expression;
using sasl::syntax_tree::variable_declaration;
using sasl::syntax_tree::variable_expression;
using sasl::syntax_tree::while_statement;

using sasl::common::scope_guard;

using namespace boost::assign;
using namespace sasl::utility;

using boost::format;
using boost::shared_ptr;
using boost::weak_ptr;
using boost::unordered_map;

using std::vector;
using std::string;
using std::pair;
using std::make_pair;

#define FUNCTION_SCOPE( new_fn ) \
	scope_guard<function_type_ptr> __sasl_fn_scope_##__LINE__( current_function, (new_fn) );
#define SYMBOL_SCOPE( new_sym ) \
	scope_guard<symbol*> __sasl_sym_scope_##__LINE__( current_symbol, (new_sym) );
#define GLOBAL_FLAG_SCOPE( new_global_flag ) \
	scope_guard<bool> __sasl_global_flag_scope_##__LINE__( is_global_scope, (new_global_flag) );
#define LABEL_LIST_SCOPE( new_label_list ) \
	scope_guard<label_list_t*> __sasl_label_list_scope_##__LINE__( label_list, (new_label_list) );
#define VARIABLE_TO_INIT_SCOPE( var_to_init ) \
	scope_guard<node_ptr> __sasl_var_to_init_scope_##__LINE__( variable_to_initialized, (var_to_init) );
#define MEMBER_COUNTER_SCOPE(); \
	scope_guard<int> __sasl_member_counter_scope_##__LINE__(member_counter, 0);
#define DECLARATION_TID_SCOPE( tid ) \
	scope_guard<int> __sasl_decl_tid_scope_##__LINE__( declaration_tid, (tid) );

semantic_analyser::semantic_analyser()
{
	caster.reset( new caster_t() );

	// Initialize global state
	lang = salviar::lang_none;
	is_global_scope = true;
	declaration_tid = -1;
	member_counter = -1;
	label_list = NULL;
}

#define SASL_VISITOR_TYPE_NAME semantic_analyser

template <typename NodeT>
shared_ptr<NodeT> semantic_analyser::visit_child( shared_ptr<NodeT> const& child )
{
	node_ptr old_generated_node = generated_node;
	generated_node.reset();

	child->accept(this, NULL);
	
	shared_ptr<NodeT> ret = generated_node->as_handle<NodeT>();
	generated_node = old_generated_node;
	
	return ret;
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
	EFLIB_UNREF_PARAM(data);

	shared_ptr<unary_expression> dup_expr = duplicate( v.as_handle() )->as_handle<unary_expression>();
	dup_expr->expr = visit_child(v.expr);

	node_semantic* inner_tisi = get_node_semantic(dup_expr->expr);
	if( v.op == operators::prefix_incr || v.op == operators::postfix_incr || v.op == operators::prefix_decr || v.op == operators::postfix_decr ){
		if( !is_integer( inner_tisi->ty_proto()->tycode ) ){
			// REPORT ERROR
			EFLIB_ASSERT_UNIMPLEMENTED();
			return;
		}
	}

	node_semantic* ssi = get_or_create_semantic(dup_expr);
	ssi->tid( inner_tisi->tid() );

	generated_node = dup_expr;
}

SASL_VISIT_DEF( cast_expression ){
	EFLIB_UNREF_PARAM(data);

	shared_ptr<cast_expression> dup_cexpr = duplicate(v.as_handle())->as_handle<cast_expression>();

	dup_cexpr->casted_type = visit_child(v.casted_type);
	dup_cexpr->expr = visit_child(v.expr);

	node_semantic* src_tsi = module_semantic_->get_semantic(dup_cexpr->expr);
	node_semantic* casted_tsi = module_semantic_->get_semantic(dup_cexpr->casted_type);

	if( src_tsi->tid() != casted_tsi->tid() ){
		if( caster->try_cast( casted_tsi->tid(), src_tsi->tid() ) == caster_t::nocast ){
			// Here is code error. Compiler should report it.
			EFLIB_ASSERT_UNIMPLEMENTED();
			return;
		}
	}

	node_semantic* ssi = get_or_create_semantic(dup_cexpr);
	ssi->tid( casted_tsi->tid() );

	generated_node = dup_cexpr;
}

SASL_VISIT_DEF( binary_expression )
{
	EFLIB_UNREF_PARAM(data);

	shared_ptr<binary_expression> dup_expr = duplicate( v.as_handle() )->as_handle<binary_expression>();
	generated_node = dup_expr;

	dup_expr->left_expr = visit_child(v.left_expr);
	dup_expr->right_expr = visit_child(v.right_expr);

	std::string opname = operator_name( v.op );
	vector<expression*> exprs;
	exprs += dup_expr->left_expr.get(), dup_expr->right_expr.get();
	
	if(		get_node_semantic(dup_expr->left_expr) == NULL 
		||  get_node_semantic(dup_expr->right_expr) == NULL )
	{
		return;
	}

	vector<symbol*> overloads;

	if( is_assign(v.op) || is_arith_assign(v.op) ){
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
		diags->report( operator_param_unmatched )->token_range( *v.token_begin(), *v.token_end() )->p( atr.str() );
	}
	else if ( overloads.size() > 1 )
	{
		diags->report( operator_multi_overloads )->token_range( *v.token_begin(), *v.token_end() )->p( overloads.size() );
	}
	else
	{
		tid_t result_tid = get_node_semantic( overloads[0]->associated_node() )->tid();
		get_or_create_semantic(dup_expr)->tid( result_tid );
	}
}

SASL_VISIT_DEF_UNIMPL( expression_list );

SASL_VISIT_DEF( cond_expression ){
	EFLIB_UNREF_PARAM(data);

	shared_ptr<cond_expression> dup_expr
		= duplicate( v.as_handle() )->as_handle<cond_expression>();

	dup_expr->cond_expr = visit_child(v.cond_expr);
	dup_expr->yes_expr = visit_child(v.yes_expr);
	dup_expr->no_expr = visit_child(v.no_expr);
	
	// SEMANTIC_TODO Test conversation between type of yes expression and no expression.
	node_semantic* cond_tisi= get_node_semantic(dup_expr->cond_expr);
	node_semantic* yes_tisi	= get_node_semantic(dup_expr->yes_expr);
	node_semantic* no_tisi	= get_node_semantic(dup_expr->no_expr);
 
	assert( cond_tisi && yes_tisi && no_tisi );

	if( !(cond_tisi && yes_tisi && no_tisi) ) {
		return;
	}

	tid_t bool_tid = module_semantic_->pety()->get( builtin_types::_boolean );
	tid_t cond_tid = cond_tisi->tid();

	if( cond_tid != bool_tid && !caster->try_implicit( cond_tisi->tid(), bool_tid ) ){
		diags->report( cannot_convert_type_from )
			->token_range( *dup_expr->cond_expr->token_begin(), *dup_expr->cond_expr->token_end() )
			->p("?")->p(type_repr(cond_tisi->ty_proto()).str())->p("bool");
	}
	
	node_semantic* ssi = get_or_create_semantic(dup_expr);

	tid_t yes_tid = yes_tisi->tid();
	tid_t no_tid = no_tisi->tid();

	if( yes_tid == no_tid ){
		ssi->tid( yes_tid );
	} else if( caster->try_implicit(yes_tid, no_tid) ){
		ssi->tid( yes_tid );
	} else if( caster->try_implicit(no_tid, yes_tid) ){
		ssi->tid( no_tid );
	} else {
		diags->report(cannot_convert_type_from)
			->token_range( *dup_expr->yes_expr->token_begin(), *dup_expr->no_expr->token_end() )
			->p(":")->p(type_repr(no_tisi->ty_proto()).str())->p(type_repr(yes_tisi->ty_proto()).str());
	}

	generated_node = dup_expr;
}

SASL_VISIT_DEF( index_expression )
{
	EFLIB_UNREF_PARAM(data);
	
	shared_ptr<index_expression> dup_idxexpr = duplicate(v.as_handle())->as_handle<index_expression>();
	generated_node = dup_idxexpr;
	dup_idxexpr->expr = visit_child(v.expr);	
	
	node_semantic* agg_ssi = get_node_semantic(dup_idxexpr->expr);
	if( !agg_ssi ){ return; }
	tynode* agg_tyn = agg_ssi->ty_proto();
	builtin_types agg_tycode = agg_tyn->tycode;
	if( !( agg_tyn->is_array() || is_vector(agg_tycode) || is_matrix(agg_tycode) ) )
	{	
		diags->report(not_an_acceptable_operator)
			->token_range( *v.token_begin(), *v.token_end() )
			->p("[")->p( type_repr(agg_tyn).str() );
		agg_ssi = NULL;
	}

	dup_idxexpr->index_expr = visit_child(v.index_expr);
	node_semantic* index_tisi = get_node_semantic(dup_idxexpr->index_expr);

	if( !index_tisi ){ return; }
	tynode* idx_tyn = index_tisi->ty_proto();
	builtin_types idx_tycode = idx_tyn->tycode;

	if( !is_integer(idx_tycode) )
	{
		diags->report( subscript_not_integral )
			->token_range( *v.token_begin(), *v.token_end() );
		index_tisi = NULL;
	}
	
	if( !(agg_ssi && index_tisi) ) { return; }

	node_semantic* ssi = get_or_create_semantic(dup_idxexpr);
	
	if( agg_tyn->is_array() )
	{
		array_type_ptr array_tyn = agg_tyn->as_handle<array_type>();
		tid_t elem_tid  = get_node_semantic(array_tyn->elem_type)->tid();
		tid_t inner_tid = module_semantic_->pety()->get_array(elem_tid, array_tyn->array_lens.size()-1);
		ssi->tid(inner_tid);
	}
	else if( is_vector(agg_tycode) )
	{
		ssi->tid( module_semantic_->pety()->get( scalar_of(agg_tycode) ) );
	}
	else if( is_matrix(agg_tycode) )
	{
		builtin_types vector_tycode = row_vector_of(agg_tycode);
		ssi->tid( module_semantic_->pety()->get(vector_tycode) );
	}
}

SASL_VISIT_DEF( call_expression )
{
	EFLIB_UNREF_PARAM(data);

	shared_ptr<call_expression> dup_callexpr = duplicate(v.as_handle())->as_handle<call_expression>();
	generated_node = dup_callexpr;

	dup_callexpr->expr = visit_child(v.expr);
	
	dup_callexpr->args.clear();
	BOOST_FOREACH( shared_ptr<expression> arg_expr, v.args )
	{
		dup_callexpr->args.push_back( visit_child(arg_expr) );
	}

	node_semantic* expr_si = get_node_semantic(dup_callexpr->expr);
	node_semantic* fnsi = expr_si;

	if( expr_si == NULL ){ return; }
	
	if( expr_si->is_function_pointer() ) {
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
				node_semantic* arg_tisi = get_node_semantic(dup_callexpr->args[i]);
				if( arg_tisi )
				{
					atr.arg( arg_tisi->ty_proto() );
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
			node_semantic* ssi = get_node_semantic( func_sym->associated_node() );
			node_semantic* csi = get_or_create_semantic(dup_callexpr);

			csi->tid( ssi->tid() );
			csi->is_function_pointer(false);
			csi->overloaded_function(func_sym);
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
	EFLIB_UNREF_PARAM(data);

	shared_ptr<member_expression> dup_expr = duplicate( v.as_handle() )->as_handle<member_expression>();
	generated_node = dup_expr;

	dup_expr->expr = visit_child( v.expr );
	
	node_semantic* arg_tisi = get_node_semantic(dup_expr->expr);
	if( !arg_tisi )
	{
		diags->report( member_left_must_have_struct )->token_range( *v.member, *v.member )->p( v.member->str )->p( "<unknown>" );
		return;
	}

	tynode* agg_type = arg_tisi->ty_proto();
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
		// Aggregated class is scalar or vector
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

	node_semantic* ssi = get_or_create_semantic(dup_expr);

	ssi->tid( mem_typeid );
	ssi->swizzle( swizzle_code );
}

SASL_VISIT_DEF( constant_expression )
{
	EFLIB_UNREF_PARAM(data);

	shared_ptr<constant_expression> dup_cexpr = duplicate( v.as_handle() )->as_handle<constant_expression>();
	node_semantic* vsi = get_or_create_semantic(dup_cexpr);
	vsi->const_value( v.value_tok->str, v.ctype );

	generated_node = dup_cexpr;	
}

SASL_VISIT_DEF( variable_expression ){
	EFLIB_UNREF_PARAM(data);

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
			diags->report( illegal_use_type_as_expr )
				->token_range( *v.token_begin(), *v.token_end() )
				->p( name );
		}
		else if ( decl_node || param_node )
		{
			node_semantic* vexpr_sem = module_semantic_->get_or_create_semantic(dup_vexpr);
			*vexpr_sem = *get_node_semantic(node);
			vexpr_sem->associated_symbol(vdecl);
		}
		else
		{
			diags->report( unknown_semantic_error )->token_range(*v.token_begin(), *v.token_end())->p(__FILE__)->p(__LINE__);
		}
	} else {
		// Function
		bool is_function = !current_symbol->find_overloads(name).empty();
		if( is_function ){
			node_semantic* fvsi = get_or_create_semantic(dup_vexpr);
			fvsi->associated_symbol(current_symbol);
			fvsi->function_name(name);
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
	EFLIB_UNREF_PARAM(data);

	shared_ptr<expression_initializer> dup_exprinit = duplicate( v.as_handle() )->as_handle<expression_initializer>();
	generated_node = dup_exprinit->as_handle();

	dup_exprinit->init_expr = visit_child(v.init_expr);

	node_semantic* init_expr_tisi = get_node_semantic(dup_exprinit->init_expr);
	if( !init_expr_tisi ) { return; }

	node_semantic* var_tsi = get_node_semantic(variable_to_initialized);
	if( !var_tsi || var_tsi->tid() == -1 ) { return; }

	if ( var_tsi->tid() != init_expr_tisi->tid() ){
		if( !caster->try_implicit( var_tsi->tid(), init_expr_tisi->tid() ) ){
			diags->report( cannot_convert_type_from )
				->token_range( *dup_exprinit->init_expr->token_begin(), *dup_exprinit->init_expr->token_end() )
				->p( type_repr(init_expr_tisi->ty_proto()).str() )->p( type_repr(var_tsi->ty_proto()).str() );
		}
	}

	node_semantic* ssi = get_or_create_semantic(dup_exprinit);
	ssi->tid( init_expr_tisi->tid() );
}

SASL_VISIT_DEF_UNIMPL( member_initializer );
SASL_VISIT_DEF_UNIMPL( declaration );
SASL_VISIT_DEF( declarator ){
	EFLIB_UNREF_PARAM(data);

	shared_ptr<declarator> dup_decl = duplicate( v.as_handle() )->as_handle<declarator>();

	node_semantic* ssi = get_or_create_semantic(dup_decl);
	ssi->tid(declaration_tid);

	if( member_counter >= 0 )
	{
		ssi->member_index(member_counter++);
	}

	if ( v.init ){
		VARIABLE_TO_INIT_SCOPE(dup_decl);
		dup_decl->init = visit_child(v.init);
	}

	if( declaration_tid != -1 )
	{
		symbol* nodesym = current_symbol->add_named_child( v.name->str, dup_decl.get() );
		parse_semantic( v.semantic, v.semantic_index, ssi );
		if(is_global_scope)
		{
			module_semantic_->global_vars().push_back(nodesym);
		}
	}

	generated_node = dup_decl;
}

SASL_VISIT_DEF( variable_declaration )
{
	EFLIB_UNREF_PARAM(data);

	shared_ptr<variable_declaration> dup_vdecl = duplicate( v.as_handle() )->as_handle<variable_declaration>();
	generated_node = dup_vdecl;

	dup_vdecl->type_info = visit_child(v.type_info);
	
	node_semantic* decl_tisi = get_node_semantic(dup_vdecl->type_info);

	int decl_tid = decl_tisi ? decl_tisi->tid() : -1;
	
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
	EFLIB_UNREF_PARAM(data);

	// create type information on current symbol.
	// for e.g. create type info onto a variable node.
	node_semantic* tsi = module_semantic_->get_or_create_semantic(&v);
	tsi->ty_proto(&v, current_symbol);
	generated_node = tsi->ty_proto()->as_handle();
}

SASL_VISIT_DEF(array_type)
{
	EFLIB_UNREF_PARAM(data);

	tid_t array_tid = module_semantic_->pety()->get( &v, current_symbol );
	assert(array_tid != -1);

	array_type_ptr dup_array = module_semantic_->pety()->get_proto(array_tid)->as_handle<array_type>();
	if( !get_node_semantic(dup_array->elem_type) )
	{
		dup_array->elem_type = visit_child(v.elem_type);
	}
	generated_node = dup_array;
}

SASL_VISIT_DEF( struct_type ){
	EFLIB_UNREF_PARAM(data);

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

	shared_ptr<struct_type> dup_struct
		= module_semantic_->pety()->get_proto( dup_struct_id )->as_handle<struct_type>();
	generated_node = dup_struct;

	node_semantic* tisi = get_node_semantic(dup_struct);
	dup_struct = tisi->ty_proto()->as_handle<struct_type>();

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
	EFLIB_UNREF_PARAM(data);

	tid_t dup_struct_id = -1;
	if( v.alias->str == "sampler" ){
		dup_struct_id = module_semantic_->pety()->get(builtin_types::_sampler);
	} else {
		dup_struct_id = module_semantic_->pety()->get(&v, current_symbol);
	}
	
	if( dup_struct_id == -1 )
	{
		diags->report( undeclared_identifier )->token_range( *v.alias, *v.alias )->p(v.alias->str);
	}

	generated_node = module_semantic_->pety()->get_proto(dup_struct_id)->as_handle();
}

SASL_VISIT_DEF( parameter )
{
	EFLIB_UNREF_PARAM(data);

	shared_ptr<parameter> dup_par = duplicate( v.as_handle() )->as_handle<parameter>();
	generated_node = dup_par;

	if( v.name )
	{
		current_symbol->add_named_child( v.name->str, dup_par.get() );
	}
	else
	{
		current_symbol->add_child( dup_par.get() );
	}

	dup_par->param_type = visit_child(v.param_type);

	if ( v.init ){ dup_par->init = visit_child(v.init); }

	node_semantic* par_tisi = get_node_semantic(dup_par->param_type);
	tid_t tid = par_tisi ? par_tisi->tid() : -1;
	
	if( tid == -1 ) { return; }

	node_semantic* ssi = get_or_create_semantic(dup_par);
	ssi->tid( tid );
	// SEMANTIC_TODO: Nonsupports reference yet.
	ssi->is_reference(false);
	parse_semantic( v.semantic, v.semantic_index, ssi );
}

SASL_VISIT_DEF( function_type )
{
	EFLIB_UNREF_PARAM(data);

	// Copy node
	shared_ptr<node> dup_node = duplicate( v.as_handle() );
	EFLIB_ASSERT_AND_IF( dup_node, "Node swallow duplicated error !"){
		return;
	}
	shared_ptr<function_type> dup_fn = dup_node->as_handle<function_type>();
	generated_node = dup_fn;
	dup_fn->params.clear();

	symbol* sym = current_symbol->add_function_begin( dup_fn.get() );

	{
		SYMBOL_SCOPE(sym);
		FUNCTION_SCOPE(dup_fn);

		dup_fn->retval_type = visit_child(v.retval_type);

		if( !get_node_semantic(dup_fn->retval_type) ) { return; }

		bool successful = true;
		for( vector< shared_ptr<parameter> >::iterator it = v.params.begin();
			it != v.params.end(); ++it )
		{
			shared_ptr<parameter> fn_param = visit_child(*it);
			dup_fn->params.push_back(fn_param);
			node_semantic* param_tisi = get_node_semantic(fn_param);
			if( !param_tisi || param_tisi->tid() == -1 ){ successful = false; }
		}
		if( !successful ){ return; }
	}

	current_symbol->add_function_end(sym);

	node_semantic* ssi = get_or_create_semantic(dup_fn);
	tid_t ret_tid = get_node_semantic(dup_fn->retval_type)->tid();
	ssi->tid( ret_tid );

	// SEMANTIC_TODO judge the true abi.
	ssi->msc_compatible( true );

	parse_semantic( v.semantic, v.semantic_index, ssi );

	if ( v.body )
	{
		SYMBOL_SCOPE(sym);
		FUNCTION_SCOPE(dup_fn);
		GLOBAL_FLAG_SCOPE(false);

		dup_fn->body = visit_child(v.body);
		module_semantic_->functions().push_back(sym);
	}
}

// statement
SASL_VISIT_DEF_UNIMPL( statement );

SASL_VISIT_DEF( declaration_statement )
{
	EFLIB_UNREF_PARAM(data);

	shared_ptr<declaration_statement> dup_declstmt = duplicate( v.as_handle() )->as_handle<declaration_statement>();

	dup_declstmt->decls.clear();
	BOOST_FOREACH( shared_ptr<declaration> const& decl, v.decls )
	{
		shared_ptr<declaration> dup_decl = visit_child(decl);
		if( dup_decl ){ dup_declstmt->decls.push_back(dup_decl); }
	}
	
	get_or_create_semantic(dup_declstmt);

	generated_node = dup_declstmt;
}

SASL_VISIT_DEF( if_statement )
{
	EFLIB_UNREF_PARAM(data);

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
	EFLIB_UNREF_PARAM(data);

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
	EFLIB_UNREF_PARAM(data);

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
	EFLIB_UNREF_PARAM(data);

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
	EFLIB_UNREF_PARAM(data);

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
			node_semantic* expr_tisi = get_node_semantic(dup_case->expr);
			if( !expr_tisi ){ return; }

			builtin_types expr_bt = expr_tisi->ty_proto()->tycode;
			if( !is_integer( expr_bt ) && expr_bt != builtin_types::_boolean )
			{
				diags->report( illegal_type_for_case_expr )
					->token_range( *v.expr->token_begin(), *v.expr->token_end() )
					->p( type_repr(expr_tisi->ty_proto()).str() );
			}
		}
	}

	generated_node = dup_case;
}

SASL_VISIT_DEF( ident_label ){
	EFLIB_UNREF_PARAM(v);
	EFLIB_UNREF_PARAM(data);
	EFLIB_ASSERT_UNIMPLEMENTED();
}

SASL_VISIT_DEF( switch_statement ){
	EFLIB_UNREF_PARAM(data);

	shared_ptr<switch_statement> dup_switch = duplicate( v.as_handle() )->as_handle<switch_statement>();

	dup_switch->cond = visit_child(v.cond);
	node_semantic* cond_tsi = get_node_semantic(dup_switch->cond);
	assert( cond_tsi );
	tid_t int_tid = module_semantic_->pety()->get( builtin_types::_sint32 );
	builtin_types cond_bt = cond_tsi->ty_proto()->tycode;
	assert( is_integer( cond_bt ) || caster->try_implicit( int_tid, cond_tsi->tid() ) );

	node_semantic* ssi = get_or_create_semantic(dup_switch);
	
	LABEL_LIST_SCOPE( &ssi->labeled_statements() );
	dup_switch->stmts = visit_child(v.stmts);
	
	generated_node = dup_switch;
}

SASL_VISIT_DEF( compound_statement )
{
	EFLIB_UNREF_PARAM(data);

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

	get_or_create_semantic(dup_stmt);
	generated_node = dup_stmt;
}

SASL_VISIT_DEF( expression_statement ){
	EFLIB_UNREF_PARAM(data);

	shared_ptr<expression_statement> dup_exprstmt = duplicate( v.as_handle() )->as_handle<expression_statement>();
	dup_exprstmt->expr = visit_child(v.expr);
	get_or_create_semantic(dup_exprstmt);
	generated_node = dup_exprstmt->as_handle();
}

SASL_VISIT_DEF( jump_statement )
{
	EFLIB_UNREF_PARAM(data);

	shared_ptr<jump_statement> dup_jump = duplicate(v.as_handle())->as_handle<jump_statement>();
	generated_node = dup_jump;

	if (v.code == jump_mode::_return){
		if( v.jump_expr ){
			dup_jump->jump_expr = visit_child(v.jump_expr);
		}

		node_semantic* expr_tisi = get_node_semantic(dup_jump->jump_expr);
		node_semantic* fret_tisi = get_node_semantic(current_function->retval_type);

		if( !expr_tisi || !fret_tisi ){ return; }

		tid_t expr_tid = expr_tisi->tid();
		tid_t fret_tid = fret_tisi->tid();

		if( expr_tid == -1 || fret_tid == -1 ){ return; }

		if( expr_tid != fret_tid && !caster->try_implicit(fret_tid, expr_tid) )
		{
			diags->report( cannot_convert_type_from )
				->token_range( *dup_jump->jump_expr->token_begin(), *dup_jump->jump_expr->token_end() )
				->p("return")
				->p( type_repr(expr_tisi->ty_proto()).str() )
				->p( type_repr(fret_tisi->ty_proto()).str() );
		}
	}
}

SASL_VISIT_DEF( for_statement ){
	EFLIB_UNREF_PARAM(data);

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
	EFLIB_UNREF_PARAM(data);

	// create semantic info
	module_semantic_ = module_semantic::create();
	diags = diag_chat::create();

	SYMBOL_SCOPE( module_semantic_->root_symbol() );
	
	register_builtin_types();
	add_cast();
	caster->set_function_get_tynode( boost::bind( &pety_t::get_proto, module_semantic_->pety(), _1) );
	register_builtin_functions();

	shared_ptr<program> dup_prog = duplicate( v.as_handle() )->as_handle<program>();
	prog_ = dup_prog.get();
	dup_prog->decls.clear();

	// analysis declarations.
	for( vector< shared_ptr<declaration> >::iterator it = v.decls.begin(); it != v.decls.end(); ++it ){
		shared_ptr<declaration> node_gen = visit_child(*it);
		dup_prog->decls.push_back( node_gen );
	}

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

void semantic_analyser::add_cast(){
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

void semantic_analyser::register_builtin_functions(){
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

	shared_ptr<builtin_type> fvec_ts[5];
	for( int i = 1; i <= 4; ++i )
	{
		fvec_ts[i] = storage_bttbl[ vector_of( builtin_types::_float, i ) ];
	}

	shared_ptr<builtin_type> fmat_ts[5][5];
	for( int vec_size = 1; vec_size < 5; ++vec_size ){
		for( int n_vec = 1; n_vec < 5; ++n_vec ){
			fmat_ts[vec_size][n_vec] = storage_bttbl[
				matrix_of( builtin_types::_float, vec_size, n_vec )
			];
		}
	}

	// operators
	{
		vector<std::string> op_tbl;
		const vector<operators>& oplist = list_of_operators();

		for( size_t i_op = 0; i_op < oplist.size(); ++i_op ){
			operators op = oplist[i_op];
			std::string op_name( operator_name(op) );

			if ( is_arithmetic(op) )
			{
				for( bt_table_t::iterator it_type = storage_bttbl.begin(); it_type != storage_bttbl.end(); ++it_type )
				{
					builtin_types tycode = it_type->first;
					builtin_types scalar_tycode = scalar_of(tycode);

					if( scalar_tycode == builtin_types::_boolean ) { continue; }

					shared_ptr<builtin_type> ty = it_type->second;
					register_function(op_name) % ty % ty >> ty;

					if(    ( is_vector(tycode) && vector_size(tycode) > 1 )
						|| ( is_matrix(tycode) && (vector_size(tycode) * vector_count(tycode) > 1) )
						)
					{
						shared_ptr<builtin_type> scalar_ty = storage_bttbl[ scalar_of(tycode) ];
						register_function(op_name) % ty % scalar_ty >> ty;
						register_function(op_name) % scalar_ty % ty >> ty;
					}
				}
			}

			if( is_arith_assign(op) )
			{
				for( bt_table_t::iterator it_type = storage_bttbl.begin(); it_type != storage_bttbl.end(); ++it_type )
				{
					builtin_types tycode = it_type->first;
					builtin_types scalar_tycode = scalar_of(tycode);

					if( scalar_tycode == builtin_types::_boolean ) { continue; }

					shared_ptr<builtin_type> ty = it_type->second;
					register_function( op_name ) % ty % ty >> ty;

					if(    ( is_vector(tycode) && vector_size(tycode) > 1 )
						|| ( is_matrix(tycode) && (vector_size(tycode) * vector_count(tycode) > 1) )
						)
					{
						shared_ptr<builtin_type> scalar_ty = storage_bttbl[ scalar_of(tycode) ];
						register_function( op_name ) % ty % scalar_ty >> ty;
					}
				}
			}

			if( is_relationship(op) ){
				for( bt_table_t::iterator it_type = storage_bttbl.begin(); it_type != storage_bttbl.end(); ++it_type )
				{
					builtin_types tycode = it_type->first;
					shared_ptr<builtin_type> ty = it_type->second;
					register_function(op_name) % ty % ty >> storage_bttbl[replace_scalar(tycode, builtin_types::_boolean)];
				}
			}

			if( is_bit(op) || is_bit_assign(op) || is_shift(op) || is_shift_assign(op) )
			{
				for( bt_table_t::iterator it_type = storage_bttbl.begin(); it_type != storage_bttbl.end(); ++it_type )
				{
					builtin_types scalar_tycode = scalar_of(it_type->first);

					if ( is_integer(scalar_tycode) )
					{
						shared_ptr<builtin_type> ty = it_type->second;
						register_function( op_name ) % ty % ty >> ty;
					}
				}
			}

			if( is_bool_arith(op) ){
				register_function( op_name ) % bt_bool % bt_bool >> bt_bool;

				builtin_types tycode( builtin_types::none );
				shared_ptr<builtin_type> ty;
				for( size_t vsize = 1; vsize <= 4; ++vsize )
				{
					tycode = vector_of(builtin_types::_boolean, vsize);
					ty = storage_bttbl[tycode];
					register_function( op_name ) % ty % ty >> ty;
					for( size_t vcnt = 1; vcnt <= 4; ++vcnt )
					{
						tycode = matrix_of(builtin_types::_boolean, vsize, vcnt);
						ty = storage_bttbl[tycode];
						register_function( op_name ) % ty % ty >> ty;
					}
				}
			}

			if( is_prefix(op) || is_postfix(op) || op == operators::positive ){
				for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
					if ( is_integer(it_type->first) )
					{
						shared_ptr<builtin_type> ty = it_type->second;
						register_function( op_name ) % ty >> ty;
					}
				}
			}

			if( op == operators::bit_not ){
				for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
					if ( is_integer(it_type->first) ){
						shared_ptr<builtin_type> ty = it_type->second;
						register_function( op_name ) % ty >> ty;
					}
				}
			}

			if( op == operators::logic_not ){
				register_function( op_name ) % bt_bool >> bt_bool;
			}

			if( op == operators::negative ){
				for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
					if ( it_type->first != builtin_types::_uint64 ){
						shared_ptr<builtin_type> ty = it_type->second;
						register_function( op_name ) % ty >> ty;
					}
				}
			}

			if ( op == operators::assign ){
				for( bt_table_t::iterator it_type = storage_bttbl.begin(); it_type != storage_bttbl.end(); ++it_type )
				{
					shared_ptr<builtin_type> ty = it_type->second;
					register_function( op_name ) % ty % ty >> ty;
				}
			}
		}
	}

	// all, any, ddx, ddy
	{
		for( bt_table_t::iterator it_type = storage_bttbl.begin(); it_type != storage_bttbl.end(); ++it_type )
		{
			builtin_types tycode = it_type->first;
			shared_ptr<builtin_type> ty = it_type->second;

			if( is_scalar(tycode) || is_vector(tycode) || is_matrix(tycode) )
			{
				register_intrinsic( "all" ) % ty >> bt_bool;
				register_intrinsic( "any" ) % ty >> bt_bool;

				if( lang == salviar::lang_pixel_shader )
				{
					register_intrinsic( "ddx" ) % ty >> ty;
					register_intrinsic( "ddy" ) % ty >> ty;
				}
			}
		}
	}

	// abs
	{
		for( bt_table_t::iterator it_type = storage_bttbl.begin(); it_type != storage_bttbl.end(); ++it_type )
		{
			builtin_types tycode = it_type->first;
			shared_ptr<builtin_type> ty = it_type->second;

			if( is_scalar(tycode) || is_vector(tycode) || is_matrix(tycode) )
			{
				builtin_types scalar_tycode = scalar_of(tycode);
				if( is_real(scalar_tycode) || (is_integer(scalar_tycode) && is_signed(scalar_tycode)) )
				{
					register_intrinsic( "abs" ) % ty >> ty;
				}
			}
		}
	}

	// degrees, radians, sqrt, fmod, lerp
	{
		for( bt_table_t::iterator it_type = storage_bttbl.begin(); it_type != storage_bttbl.end(); ++it_type )
		{
			builtin_types tycode = it_type->first;
			shared_ptr<builtin_type> ty = it_type->second;

			if( is_scalar(tycode) || is_vector(tycode) || is_matrix(tycode) )
			{
				if( scalar_of(tycode) == builtin_types::_float )
				{
					register_intrinsic( "degrees"	) % ty			>> ty;
					register_intrinsic( "radians"	) % ty			>> ty;
					register_intrinsic( "sqrt"		) % ty			>> ty;
					register_intrinsic( "exp"		) % ty			>> ty;
					register_intrinsic( "exp2"		) % ty			>> ty;
					register_intrinsic( "sin"		) % ty			>> ty;
					register_intrinsic( "cos"		) % ty			>> ty;
					register_intrinsic( "tan"		) % ty			>> ty;
					register_intrinsic( "asin"		) % ty			>> ty;
					register_intrinsic( "acos"		) % ty			>> ty;
					register_intrinsic( "atan"		) % ty			>> ty;
					register_intrinsic( "ceil"		) % ty			>> ty;
					register_intrinsic( "floor"	) % ty			>> ty;
					register_intrinsic( "log"		) % ty			>> ty;
					register_intrinsic( "log2"		) % ty			>> ty;
					register_intrinsic( "log10"	) % ty			>> ty;
					register_intrinsic( "rsqrt"	) % ty			>> ty;
					register_intrinsic( "fmod"		) % ty % ty		>> ty;
					register_intrinsic( "ldexp"	) % ty % ty		>> ty;
					register_intrinsic( "lerp"		) % ty % ty % ty>> ty;
				}
			}
		}
	}

	// distance, dst, length, dot
	{
		for( size_t i = 1; i <= 4; ++i )
		{
			register_intrinsic("length") % fvec_ts[i] >> BUILTIN_TYPE(_float);
			register_intrinsic("distance") % fvec_ts[i] % fvec_ts[i] >> BUILTIN_TYPE(_float);
			register_intrinsic("dot") % fvec_ts[i] % fvec_ts[i] >> BUILTIN_TYPE(_float);
		}
		register_intrinsic("dst") % fvec_ts[4] % fvec_ts[4] >> fvec_ts[4];
	}

	// Sampling, cross
	{
		shared_ptr<builtin_type> sampler_ty = create_builtin_type( builtin_types::_sampler );

		// External and Intrinsic are Same signatures
		
		{
			if( lang == salviar::lang_pixel_shader || lang == salviar::lang_vertex_shader )
			{
				register_intrinsic( "tex2Dlod", lang == salviar::lang_pixel_shader )
					% sampler_ty % fvec_ts[4]
				>> fvec_ts[4];
				register_intrinsic( "texCUBElod", lang == salviar::lang_pixel_shader )
					% sampler_ty % fvec_ts[4]
				>> fvec_ts[4];
			}
			
			if( lang == salviar::lang_pixel_shader )
			{
				register_intrinsic( "tex2D", true )
					% sampler_ty % fvec_ts[2] 
				>> fvec_ts[4];

				register_intrinsic( "tex2Dbias", true )
					% sampler_ty % fvec_ts[4] /*coord with bias*/
				>> fvec_ts[4];

				register_intrinsic( "tex2Dproj", true )
					% sampler_ty % fvec_ts[4] /*coord with proj*/
				>> fvec_ts[4];

				register_intrinsic( "tex2Dgrad", true ) 
					% sampler_ty % fvec_ts[2] /*coord*/
				% fvec_ts[2] % fvec_ts[2] /*ddx, ddy*/
				>> fvec_ts[4];
			}
		}

		for( size_t vec_size = 1; vec_size <= 4; ++vec_size){
			for( size_t n_vec = 1; n_vec <= 4; ++n_vec ){
				register_intrinsic("mul")
					% fvec_ts[n_vec] % fmat_ts[vec_size][n_vec]
				>> fvec_ts[vec_size];

				register_intrinsic("mul")
					% fmat_ts[vec_size][n_vec] % fvec_ts[vec_size]
				>> fvec_ts[n_vec];
			}
		}

		register_intrinsic( "cross" ) % fvec_ts[3] % fvec_ts[3] >> fvec_ts[3];
	}

	// asfloat, asint, asuint
	{
		vector< shared_ptr<builtin_type> > int_tys;
		vector< shared_ptr<builtin_type> > uint_tys;
		vector< shared_ptr<builtin_type> > float_tys;

		int_tys.push_back	( BUILTIN_TYPE(_sint32) );
		uint_tys.push_back	( BUILTIN_TYPE(_uint32) );
		float_tys.push_back	( BUILTIN_TYPE(_float) );

		for( size_t vsize = 1; vsize <= 4; ++vsize )
		{
			int_tys.push_back	( storage_bttbl[vector_of(builtin_types::_sint32, vsize)] );
			uint_tys.push_back	( storage_bttbl[vector_of(builtin_types::_uint32, vsize)] );
			float_tys.push_back	( storage_bttbl[vector_of(builtin_types::_float,  vsize)] );

			for( size_t vcnt = 1; vcnt <= 4; ++vcnt )
			{
				int_tys.push_back	( storage_bttbl[matrix_of(builtin_types::_sint32, vsize, vcnt)] );
				uint_tys.push_back	( storage_bttbl[matrix_of(builtin_types::_uint32, vsize, vcnt)] );
				float_tys.push_back	( storage_bttbl[matrix_of(builtin_types::_float,  vsize, vcnt)] );
			}
		}

		for( size_t i_ty = 0; i_ty < int_tys.size(); ++i_ty )
		{
			register_intrinsic( "asint" ) % uint_tys[i_ty]  >> int_tys[i_ty];
			register_intrinsic( "asint" ) % float_tys[i_ty] >> int_tys[i_ty];

			register_intrinsic( "asuint" ) % int_tys[i_ty]   >> uint_tys[i_ty];
			register_intrinsic( "asuint" ) % float_tys[i_ty] >> uint_tys[i_ty];

			register_intrinsic( "asfloat" ) % uint_tys[i_ty] >> float_tys[i_ty];
			register_intrinsic( "asfloat" ) % int_tys[i_ty]  >> float_tys[i_ty];
		}
	}

	// constructors.
	{
		unordered_map<builtin_types, size_t> vec_indexes;
		vector< shared_ptr<builtin_type> > bts;

		for( bt_table_t::iterator it_type = storage_bttbl.begin(); it_type != storage_bttbl.end(); ++it_type )
		{
			builtin_types tycode = it_type->first;
			if( is_scalar(tycode) )
			{
				vec_indexes[tycode] = bts.size();
				bts.push_back( storage_bttbl[tycode] );
				bts.push_back( storage_bttbl[tycode] );
				for( int i = 2; i <= 4; ++i ){
					bts.push_back( storage_bttbl[vector_of(tycode, i)] );
				}
			}
		}

#define TY_ADDRESS(tyname) boost::addressof( bts[vec_indexes[builtin_types::tyname]] )

		typedef pair<char const*, shared_ptr<builtin_type>* > name_ty_pair_t;
		vector< name_ty_pair_t > scalar_name_tys;
		scalar_name_tys.push_back( make_pair( "bool" , TY_ADDRESS(_boolean)) );
		scalar_name_tys.push_back( make_pair( "int"	 , TY_ADDRESS(_sint32 )) );
		scalar_name_tys.push_back( make_pair( "float", TY_ADDRESS(_float  )) );

		BOOST_FOREACH( name_ty_pair_t const& name_ty, scalar_name_tys ){
			for( int i = 2; i <= 4; ++i ){
				std::string name( name_ty.first );
				char index_to_str[] = "0";
				index_to_str[0] += (char)i;
				name.append( index_to_str );
				
				register_constructor( name, name_ty.second, i );
			}
		}
	}
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

semantic_analyser::function_register semantic_analyser::register_function(std::string const& name)
{
	shared_ptr<function_type> fn = create_node<function_type>( token_t::null(), token_t::null() );
	fn->name = token_t::from_string( name );

	function_register ret(*this, fn, false, false, false);

	return ret;
}

semantic_analyser::function_register semantic_analyser::register_intrinsic(std::string const& name, /*bool external, */bool partial_exec )
{
	shared_ptr<function_type> fn = create_node<function_type>( token_t::null(), token_t::null() );
	fn->name = token_t::from_string( name );

	function_register ret(*this, fn, true, /*external*/false, partial_exec);

	return ret;
}

void semantic_analyser::mark_intrin_invoked_recursive(symbol* sym)
{
	node_semantic* ssi = get_node_semantic( sym->associated_node() );
	
	assert( ssi ); // Function must be analyzed.
	
	if( ssi->is_invoked() ){ return; }
		
	ssi->is_invoked( true );
}

void semantic_analyser::register_constructor( std::string const& name, builtin_type_ptr* tys, int total )
{
	vector< shared_ptr<builtin_type> > param_tys;
	register_constructor_impl( name, tys, total, 0, param_tys );
}

void semantic_analyser::register_constructor_impl(
	string const& name, shared_ptr<builtin_type>* tys,
	int total, int param_scalar_counts, vector< shared_ptr<builtin_type> >& param_tys )
{
	if( param_scalar_counts == total )
	{
		function_register constructor_reg = register_intrinsic( name );
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
			register_constructor_impl( name, tys, total, param_scalar_counts+i, param_tys );
			param_tys.pop_back();
		}
	}
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

void semantic_analyser::hold_node(node_ptr const& v)
{
	module_semantic_->hold_node(v);
}

// function_register
semantic_analyser::function_register::function_register(
	semantic_analyser& owner,
	shared_ptr<function_type> const& fn,
	bool is_intrinsic,
	bool is_external,
	bool exec_partial
	) :owner(owner), fn(fn), is_intrinsic(is_intrinsic), is_external(is_external), is_partial_exec(exec_partial), is_constr(false)
{
	assert( fn );
}

semantic_analyser::function_register& semantic_analyser::function_register::operator%(
	semantic_analyser::function_register::typenode_ptr const& par_type
	)
{
	return p(par_type);
}

void semantic_analyser::function_register::operator >> (
	semantic_analyser::function_register::typenode_ptr const& ret_type
	)
{
	r(ret_type);
}

semantic_analyser::function_register& semantic_analyser::function_register::p( semantic_analyser::function_register::typenode_ptr const& par_type )
{
	assert( par_type && fn );
	
	shared_ptr<parameter> par = create_node<parameter>( token_t::null(), token_t::null() );
	par->param_type = par_type;
	fn->params.push_back( par );

	return *this;
}

void semantic_analyser::function_register::r(
	semantic_analyser::function_register::typenode_ptr const& ret_type 
	)
{
	assert( ret_type && fn );

	fn->retval_type = ret_type;

	shared_ptr<node> new_node = owner.visit_child(fn);
	owner.hold_node(new_node);

	node_semantic* fn_ssi = owner.get_node_semantic(new_node);

	fn_ssi->is_intrinsic(is_intrinsic);
	fn_ssi->msc_compatible(!is_intrinsic);
	fn_ssi->is_external(is_external);
	fn_ssi->partial_execution(is_partial_exec);
	fn_ssi->is_constructor(is_constr);

	if( is_intrinsic ){
		symbol* new_sym = owner.get_symbol(new_node);
		assert( new_sym );

		owner.get_module_semantic()->intrinsics().push_back( new_sym );
	}
}

semantic_analyser::function_register::function_register( function_register const& rhs)
	: fn(rhs.fn), owner(rhs.owner), is_intrinsic(rhs.is_intrinsic), is_external(rhs.is_external), is_partial_exec(rhs.is_partial_exec), is_constr(rhs.is_constr)
{
}

semantic_analyser::function_register& semantic_analyser::function_register::as_constructor()
{
	is_constr = true;
	return *this;
}

END_NS_SASL_SEMANTIC();
