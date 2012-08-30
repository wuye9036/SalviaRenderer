#include <sasl/include/codegen/cg_sisd.h>

#include <sasl/include/codegen/cg_intrins.h>
#include <sasl/include/codegen/cg_impl.imp.h>
#include <sasl/include/codegen/cg_module_impl.h>
#include <sasl/include/codegen/cg_caster.h>
#include <sasl/include/codegen/utility.h>
#include <sasl/include/codegen/cgs.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/caster.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/enums/enums_utility.h>
#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/utility/unref_declarator.h>
#include <eflib/include/utility/scoped_value.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/BasicBlock.h>
#include <llvm/Constants.h>
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/IRBuilder.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/scope_exit.hpp>
#include <boost/bind.hpp>
#include <eflib/include/platform/boost_end.h>

using namespace llvm;
using namespace sasl::syntax_tree;
using namespace boost::assign;
using namespace sasl::utility;

using sasl::semantic::symbol;
using sasl::semantic::node_semantic;
using sasl::semantic::operator_name;
using sasl::semantic::tid_t;
using sasl::semantic::caster_t;

using eflib::scoped_value;

using boost::addressof;
using boost::any_cast;
using boost::bind;
using boost::weak_ptr;

using std::vector;
using std::string;
using std::pair;
using std::make_pair;

#define SASL_VISITOR_TYPE_NAME cg_sisd

BEGIN_NS_SASL_CODEGEN();

cg_sisd::~cg_sisd(){
}

cg_value cg_sisd::emit_logic_op(operators op, shared_ptr<node> const& left, shared_ptr<node> const& right )
{
	visit_child(left);
	visit_child(right);

	if( op == operators::logic_or )
	{
		return service()->emit_or( node_ctxt(left)->node_value, node_ctxt(right)->node_value );
	}
	else
	{
		return service()->emit_and( node_ctxt(left)->node_value, node_ctxt(right)->node_value );
	}
}

cg_value cg_sisd::emit_short_cond(shared_ptr<node> const& cond, shared_ptr<node> const& yes, shared_ptr<node> const& no )
{
	// NOTE
	//  If 'yes' and 'no' expression are all reference/variable,
	//  and left is as same abi as right, it will return a reference,
	//  otherwise we will return a value.
	visit_child(cond);
	cg_value cond_value = node_ctxt(cond)->node_value.to_rvalue();
	insert_point_t cond_ip = service()->insert_point();

	insert_point_t yes_ip_beg = service()->new_block( "yes_expr", true );
	if( cond != yes ){
		visit_child(yes);
	}
	cg_value yes_value = node_ctxt(yes)->node_value;
	Value* yes_v = yes_value.load();
	Value* yes_ref = yes_value.load_ref();
	insert_point_t yes_ip_end = service()->insert_point();

	insert_point_t no_ip_beg = service()->new_block( "no_expr", true );
	if( cond != no ){
		visit_child(no);
	}
	cg_value no_value = node_ctxt( no, false )->node_value;
	Value* no_ref = ( no_value.abi() == yes_value.abi() ) ? no_value.load_ref() : NULL;
	Value* no_v = no_value.load( yes_value.abi() );
	insert_point_t no_ip_end = service()->insert_point();

	service()->set_insert_point(cond_ip);
	service()->jump_cond( cond_value, yes_ip_beg, no_ip_beg );

	insert_point_t merge_ip =service()->new_block( "cond_merge", false );
	service()->set_insert_point( yes_ip_end );
	service()->jump_to( merge_ip );
	service()->set_insert_point( no_ip_end );
	service()->jump_to( merge_ip );

	service()->set_insert_point(merge_ip);
	cg_value result_value;
	Value*		merged = service()->phi_( yes_ip_end.block, yes_v, no_ip_end.block, no_v );
	value_kinds	vkind = (yes_ref && no_ref) ? vkind_ref : vkind_value;
	result_value = service()->create_value( yes_value.tyinfo(), yes_value.hint(), merged, vkind, yes_value.abi() );

	return result_value;
}

SASL_VISIT_DEF( member_expression ){
	EFLIB_UNREF_DECLARATOR(data);

	visit_child(v.expr);

	node_context* agg_ctxt = node_ctxt( v.expr );
	assert( agg_ctxt );

	// Aggregated value
	node_semantic* tisi = sem_->get_semantic(v.expr);
	node_context* ctxt = node_ctxt(v, true);

	if( tisi->ty_proto()->is_builtin() ){
		// Swizzle or write mask
		uint32_t masks = sem_->get_semantic(&v)->swizzle();
		cg_value agg_value = agg_ctxt->node_value;
		if( is_scalar( tisi->value_builtin_type() ) ){
			agg_value = service()->cast_s2v(agg_value);
		}
		ctxt->node_value = service()->emit_extract_elem_mask( agg_value, masks );
	} else {
		// Member
		symbol* struct_sym = sem_->get_symbol( tisi->ty_proto() );
		symbol* mem_sym = struct_sym->find_this( v.member->str );

		assert( mem_sym );
		node_context* mem_ctxt = node_ctxt( mem_sym->associated_node(), true );
		ctxt->node_value = mem_ctxt->node_value;
		ctxt->node_value.parent( agg_ctxt->node_value );
		ctxt->node_value.abi( agg_ctxt->node_value.abi() );
	}
}

SASL_VISIT_DEF( cond_expression ){
	EFLIB_UNREF_DECLARATOR(data);
	node_ctxt(v, true)->node_value
		= emit_short_cond(v.cond_expr, v.yes_expr, v.no_expr);
}

SASL_VISIT_DEF( unary_expression ){
	EFLIB_UNREF_DECLARATOR(data);

	visit_child(v.expr);
	
	cg_value inner_value = node_ctxt(v.expr)->node_value;

	cg_type* one_tyinfo = service()->create_ty( sem_->get_semantic(&v)->ty_proto() );
	builtin_types hint = inner_value.hint();

	node_context* ctxt = node_ctxt(v, true);

	if( v.op == operators::negative ){
		cg_value zero_value = service()->null_value( one_tyinfo->hint(), inner_value.abi() );
		ctxt->node_value = service()->emit_sub(zero_value, inner_value);
	} else if( v.op == operators::positive ){
		ctxt->node_value = inner_value;
	} else if( v.op == operators::logic_not ) {
		ctxt->node_value = service()->emit_not(inner_value);
	} else if( v.op == operators::bit_not ) {
		cg_value all_one_value = service()->create_constant_int( NULL, hint, inner_value.abi(), 0xFFFFFFFFFFFFFFFF );
		ctxt->node_value = service()->emit_bit_xor( all_one_value, inner_value );
	} else {

		cg_value one_value = service()->create_constant_int( one_tyinfo, builtin_types::none, inner_value.abi(), 1 ) ;

		if( v.op == operators::prefix_incr ){
			cg_value inc_v = service()->emit_add( inner_value, one_value );
			inner_value.store( inc_v );
			ctxt->node_value = inner_value;
		} else if( v.op == operators::prefix_decr ){
			cg_value dec_v = service()->emit_sub( inner_value, one_value );
			inner_value.store( dec_v );
			ctxt->node_value = inner_value;
		} else if( v.op == operators::postfix_incr ){
			ctxt->node_value = inner_value.to_rvalue();
			inner_value.store( service()->emit_add( inner_value, one_value ) );
		} else if( v.op == operators::postfix_decr ){
			ctxt->node_value = inner_value.to_rvalue();
			inner_value.store( service()->emit_sub( inner_value, one_value ) );
		}
	}
	
	ctxt->ty = one_tyinfo;
}

SASL_VISIT_DEF_UNIMPL( statement );

SASL_VISIT_DEF( compound_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	SYMBOL_SCOPE( sem_->get_symbol(&v) );
	for ( std::vector< boost::shared_ptr<statement> >::iterator it = v.stmts.begin();
		it != v.stmts.end(); ++it)
	{
		visit_child(*it);
	}
}

SASL_VISIT_DEF( if_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	service()->if_cond_beg();
	visit_child(v.cond);
	tid_t cond_tid = sem_->get_semantic(v.cond)->tid();
	tid_t bool_tid = sem_->pety()->get( builtin_types::_boolean );
	if( cond_tid != bool_tid ){
		if( caster->cast(sem_->pety()->get_proto(bool_tid), v.cond.get()) == caster_t::nocast ){
			assert(false);
		}
	}
	cg_value cond_value = node_ctxt(v.cond, false)->node_value;
	service()->if_cond_end( cond_value );

	insert_point_t ip_cond = service()->insert_point();

	insert_point_t ip_yes_beg =service()->new_block( "if.yes", true );
	service()->then_beg();
	visit_child( v.yes_stmt );
	service()->then_end();
	insert_point_t ip_yes_end = service()->insert_point();

	insert_point_t ip_no_beg, ip_no_end;
	if( v.no_stmt ){
		ip_no_beg =service()->new_block( "if.no", true );
		service()->else_beg();
		visit_child( v.no_stmt );
		service()->else_end();
		ip_no_end = service()->insert_point();
	}

	insert_point_t ip_merge =service()->new_block( "if.end", false );

	// Fill back.
	service()->set_insert_point( ip_cond );
	service()->jump_cond( cond_value, ip_yes_beg, ip_no_beg ? ip_no_beg : ip_merge );

	service()->set_insert_point( ip_yes_end );
	service()->jump_to( ip_merge );

	if( ip_no_end ){
		service()->set_insert_point( ip_no_end );
		service()->jump_to(ip_merge);
	}

	service()->set_insert_point( ip_merge );
}

SASL_VISIT_DEF( while_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	insert_point_t cond_beg = service()->new_block( "while.cond", true );
	visit_child( v.cond );
	tid_t cond_tid = sem_->get_semantic(v.cond)->tid();
	tid_t bool_tid = sem_->pety()->get( builtin_types::_boolean );
	if( cond_tid != bool_tid ){
		caster->cast( sem_->pety()->get_proto(bool_tid), v.cond.get() );
	}
	insert_point_t cond_end = service()->insert_point();

	insert_point_t break_beg = service()->new_block( "while.break", true );
	insert_point_t break_end = service()->insert_point();

	insert_point_t body_beg =service()->new_block( "while.body", true );

	{
		CONTINUE_TO_SCOPE(cond_beg);
		BREAK_TO_SCOPE(break_beg);
		visit_child( v.body );
	}
	
	insert_point_t body_end = service()->insert_point();
	
	insert_point_t while_end =service()->new_block( "while.end", true );
	
	// Fill back
	service()->set_insert_point( cond_end );
	service()->jump_cond( node_ctxt( v.cond )->node_value, body_beg, break_beg );

	service()->set_insert_point( break_end );
	service()->jump_to( while_end );

	service()->set_insert_point( body_end );
	service()->jump_to( cond_beg );

	service()->set_insert_point( while_end );
}

SASL_VISIT_DEF( dowhile_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	insert_point_t do_beg = service()->new_block( "do.to_body", true );
	insert_point_t do_end = service()->insert_point();

	insert_point_t cond_beg =service()->new_block( "while.cond", true );
	visit_child( v.cond );
	tid_t cond_tid = sem_->get_semantic(v.cond)->tid();
	tid_t bool_tid = sem_->pety()->get( builtin_types::_boolean );
	if( cond_tid != bool_tid ){
		if ( caster->cast( sem_->pety()->get_proto(bool_tid), v.cond.get() ) == caster_t::nocast ){
			assert( false );
		}
	}
	insert_point_t cond_end = service()->insert_point();

	insert_point_t break_beg =service()->new_block( "while.break", true );
	insert_point_t break_end = service()->insert_point();

	insert_point_t body_beg =service()->new_block( "while.body", true );

	{
		CONTINUE_TO_SCOPE(cond_beg);
		BREAK_TO_SCOPE(break_beg);
		visit_child( v.body );
	}

	insert_point_t body_end = service()->insert_point();
	
	insert_point_t while_end =service()->new_block( "while.end", true );
	
	// Fill back
	service()->set_insert_point( do_end );
	service()->jump_to( body_beg );

	service()->set_insert_point( cond_end );
	service()->jump_cond( node_ctxt( v.cond )->node_value, body_beg, break_beg );

	service()->set_insert_point( break_end );
	service()->jump_to( while_end );

	service()->set_insert_point( body_end );
	service()->jump_to( cond_beg );

	service()->set_insert_point( while_end );
}

SASL_VISIT_DEF( case_label ){
	EFLIB_UNREF_DECLARATOR(data);

	if( v.expr ){
		visit_child( v.expr );
	}
}

SASL_VISIT_DEF_UNIMPL( ident_label );

SASL_VISIT_DEF( switch_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	visit_child( v.cond );
	insert_point_t cond_end = service()->insert_point();

	insert_point_t break_end =service()->new_block( "switch.break", true );

	insert_point_t body_beg =service()->new_block( "switch.body", true );

	{
		BREAK_TO_SCOPE(break_end);
		visit_child( v.stmts );
	}
	
	insert_point_t body_end = service()->insert_point();

	insert_point_t switch_end =service()->new_block( "switch.end", true );

	// Collect Labeled Statement Position
	vector< pair<cg_value,insert_point_t> > cases;
	node_semantic* ssi = sem_->get_semantic(&v);
	assert( ssi );

	insert_point_t default_beg = switch_end;
	BOOST_FOREACH( weak_ptr<labeled_statement> const& weak_lbl_stmt, ssi->labeled_statements() ){
		shared_ptr<labeled_statement> lbl_stmt = weak_lbl_stmt.lock();
		assert( lbl_stmt );

		insert_point_t stmt_ip = node_ctxt(lbl_stmt)->label_position; 
		BOOST_FOREACH( shared_ptr<label> const& lbl, lbl_stmt->labels ){
			assert( lbl->node_class() == node_ids::case_label );
			shared_ptr<case_label> case_lbl = lbl->as_handle<case_label>();
			if( case_lbl->expr ){
				cg_value v = node_ctxt( case_lbl->expr )->node_value;
				cases.push_back( make_pair(v, stmt_ip ) );
			} else {
				default_beg = stmt_ip;
			}
		}
	}
	
	// Fill back jumps
	service()->set_insert_point(cond_end);
	cg_value cond_v = node_ctxt( v.cond )->node_value;
	service()->switch_to( cond_v, cases, default_beg );

	service()->set_insert_point( break_end );
	service()->jump_to( switch_end );

	service()->set_insert_point( body_end );
	service()->jump_to( switch_end );

	service()->set_insert_point( switch_end );
}

SASL_VISIT_DEF( labeled_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	BOOST_FOREACH( shared_ptr<label> const& lbl, v.labels ){
		// Constant expression, no instruction was generated.
		visit_child( lbl );
	}
	insert_point_t stmt_pos =service()->new_block( "switch.case", true );
	visit_child( v.stmt );
	node_ctxt(v, true)->label_position = stmt_pos;
}

SASL_VISIT_DEF( for_statement ){
	EFLIB_UNREF_DECLARATOR(data);

	SYMBOL_SCOPE( sem_->get_symbol(&v) );

	node_context* ctxt = node_ctxt(v, true);
	// For instructions layout:
	//		... before code ...
	//		for initializer
	//		goto for_cond
	//	for_cond:
	//		condition expression
	//		goto for_body
	//	for_iter:
	//		iteration code
	//		goto for_cond
	//	for_break:
	//		goto the for_end
	//	for_body:
	//		...
	//		...
	//		goto for_cond
	// This design is to avoid fill-back that need to store lots of break and continue points.

	visit_child( v.init );
	insert_point_t init_end = service()->insert_point();

	insert_point_t cond_beg =service()->new_block( "for_cond", true );
	if( v.cond ){ visit_child( v.cond ); }
	insert_point_t cond_end = service()->insert_point();

	insert_point_t iter_beg =service()->new_block( "for_iter", true );
	if( v.iter ){ visit_child( v.iter ); }
	insert_point_t iter_end = service()->insert_point();

	insert_point_t for_break =service()->new_block( "for_break", true );

	insert_point_t body_beg =service()->new_block( "for_body", true );
	{
		CONTINUE_TO_SCOPE(iter_beg);
		BREAK_TO_SCOPE(for_break);
		visit_child( v.body );
	}
	insert_point_t body_end = service()->insert_point();

	insert_point_t for_end =service()->new_block( "for_end", true );

	// Fill back jumps
	service()->set_insert_point( init_end );
	service()->jump_to( cond_beg );

	service()->set_insert_point( cond_end );
	if( !v.cond ){
		service()->jump_to( body_beg );
	} else {
		service()->jump_cond( node_ctxt( v.cond, false )->node_value, body_beg, for_end );
	}

	service()->set_insert_point( iter_end );
	service()->jump_to( cond_beg );

	service()->set_insert_point( body_end );
	service()->jump_to( iter_beg );

	service()->set_insert_point( for_break );
	service()->jump_to( for_end );

	// Set correct out block.
	service()->set_insert_point( for_end );
}

SASL_SPECIFIC_VISIT_DEF( visit_continue	, jump_statement )
{
	assert(continue_to_);
	service()->jump_to(continue_to_);
}

SASL_SPECIFIC_VISIT_DEF( visit_break	, jump_statement )
{
	assert(break_to_);
	service()->jump_to(break_to_);
}

cg_module_impl* cg_sisd::mod_ptr(){
	return llvm_mod_.get();
}

cgs_sisd* cg_sisd::service() const
{
	return static_cast<cgs_sisd*>(service_);
}

abis cg_sisd::local_abi( bool is_c_compatible ) const
{
	return is_c_compatible ? abi_c : abi_llvm;
}

END_NS_SASL_CODEGEN();
