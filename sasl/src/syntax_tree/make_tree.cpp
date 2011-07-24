#include <sasl/include/syntax_tree/make_tree.h>

#include <sasl/enums/builtin_types.h>
#include <sasl/enums/enums_utility.h>

#include <sasl/include/common/token.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>

#include <eflib/include/platform/disable_warnings.h>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_same.hpp>
#include <eflib/include/platform/enable_warnings.h>

#include <vector>

#define DEFAULT_STATE_SCOPE() state_scope ss(this, e_other);

BEGIN_NS_SASL_SYNTAX_TREE();

using ::sasl::common::token_t;
using ::std::vector;

using namespace sasl::utility;

struct lct_list{
	static lct_list instance_;
	vector<literal_classifications> lst;

	lct_list(){

		literal_classifications::force_initialize();
		lst.resize(11, literal_classifications::none);

		lst[0] = literal_classifications::boolean;
		lst[1] = literal_classifications::integer;
		lst[2] = literal_classifications::integer;
		lst[3] = literal_classifications::integer;
		lst[4] = literal_classifications::integer;
		lst[5] = literal_classifications::integer;
		lst[6] = literal_classifications::integer;
		lst[7] = literal_classifications::integer;
		lst[8] = literal_classifications::integer;

		lst[9] = literal_classifications::real;
		lst[10] = literal_classifications::real;

	}
};

lct_list lct_list::instance_;

const literal_classifications* const typecode_map::type_codes(){
	const literal_classifications* ret_ptr =  &(lct_list::instance_.lst[0]);
	return ret_ptr;
}

SASL_TYPED_NODE_ACCESSORS_IMPL( tree_combinator, node );

tree_combinator& tree_combinator::do_end()
{
	// parent may release it, so reserve what we need for using later.
	tree_combinator* ret_p = parent;

	if( ret_p ){
		ret_p->child_ended();
		return *ret_p;
	}

	return *this;
}

tree_combinator::~tree_combinator()
{
	assert( is_state(e_none) );
}

void tree_combinator::syntax_error()
{
	assert( !"Fuck!" );
}

void tree_combinator::enter( state_t s )
{
	assert( s != e_none );
	assert( e_state == e_none );
	e_state = s;
}

/////////////////////////////////
// program combinator
SASL_TYPED_NODE_ACCESSORS_IMPL( dprog_combinator, program );

dprog_combinator::dprog_combinator( const std::string& prog_name ):
	tree_combinator(NULL)
{
	typed_node( create_node<program>( prog_name ) );
}

tree_combinator& dprog_combinator::dvar()
{
	enter_child( e_vardecl, var_comb );
	return *var_comb;
}

tree_combinator& dprog_combinator::dstruct( const std::string& struct_name )
{
	enter_child( e_struct, struct_comb );
	struct_comb->typed_node()->name = token_t::from_string( struct_name );
	return *struct_comb;
}

tree_combinator& dprog_combinator::dfunction( const std::string& func_name ){
	enter_child( e_function, func_comb );
	func_comb->dname( func_name );
	return *func_comb;
}

tree_combinator& dprog_combinator::dtypedef(){
	return enter_child( e_typedef, typedef_comb );
}

void dprog_combinator::child_ended()
{
	switch ( leave() ){
		case e_vardecl:
			typed_node()->decls.push_back( move_node2<declaration>(var_comb) );
			return;
		case e_struct:
			typed_node()->decls.push_back( move_node2<declaration>(struct_comb) );
			return;
		case e_function:
			typed_node()->decls.push_back( move_node2<declaration>(func_comb) );
			return;
		case e_typedef:
			typed_node()->decls.push_back( move_node2<declaration>(typedef_comb) );
			break;
		default:
			assert(!"invalid state.");
			return;
	}
}

/////////////////////////////////
// type combinator
SASL_TYPED_NODE_ACCESSORS_IMPL( dtype_combinator, tynode );

dtype_combinator::dtype_combinator( tree_combinator* parent )
: tree_combinator( parent )
{
}

tree_combinator& dtype_combinator::dbuiltin( builtin_types btc )
{
	DEFAULT_STATE_SCOPE();

	if( cur_node ){
		return default_proc();
	}

	typed_node( create_node<builtin_type>(token_t::null()) );
	typed_node()->value_typecode = btc;
	return *this;
}

tree_combinator& dtype_combinator::dvec( builtin_types comp_btc, size_t size )
{
	DEFAULT_STATE_SCOPE();

	if ( cur_node ){
		return default_proc();
	}

	typed_node( create_node<builtin_type>(token_t::null()) );
	typed_node()->value_typecode = vector_of( comp_btc, size );
	return *this;
}

tree_combinator& dtype_combinator::dmat( builtin_types comp_btc, size_t s0, size_t s1 )
{
	DEFAULT_STATE_SCOPE();

	if( cur_node ){
		return default_proc();
	}
	typed_node( create_node<builtin_type>(token_t::null() ) );
	typed_node()->value_typecode = matrix_of(comp_btc, s0, s1);
	return *this;
}

tree_combinator& dtype_combinator::dalias( const std::string& alias )
{
	DEFAULT_STATE_SCOPE();

	if( cur_node ){
		return default_proc();
	}
	typed_node( create_node<alias_type>( token_t::null() ) );
	typed_node2<alias_type>()->alias = token_t::from_string(alias);
	return *this;
}

tree_combinator& dtype_combinator::dtypequal( type_qualifiers qual )
{
	DEFAULT_STATE_SCOPE();

	if( !cur_node || typed_node()->qual != type_qualifiers::none )
	{
		return default_proc();
	}
	typed_node()->qual = qual;
	return *this;
}

tree_combinator& dtype_combinator::darray()
{
	if ( !cur_node ) { return default_proc(); }
	return enter_child( e_array, expr_comb );
}

void dtype_combinator::child_ended()
{
	boost::shared_ptr<array_type> outter_type;
	switch ( leave() ){
		case e_array:
			assert ( typed_node() );
			if ( typed_node()->node_class() != node_ids::array_type ){
				outter_type = create_node<array_type>( token_t::null() );
				outter_type->elem_type = typed_node();
				typed_node( outter_type );
			} else {
				outter_type = typed_node2<array_type>();
			}
			outter_type->array_lens.push_back( move_node(expr_comb) );
			return;
		default:
			assert( !"invalid state." );
			return;
	}
}
/////////////////////////////////////
// declarator combinator
SASL_TYPED_NODE_ACCESSORS_IMPL( ddeclarator_combinator, declarator );

ddeclarator_combinator::ddeclarator_combinator( tree_combinator* parent )
: tree_combinator( parent )
{
	typed_node( create_node<declarator>( token_t::null() ) );
}

tree_combinator& ddeclarator_combinator::dname( const std::string& name )
{
	DEFAULT_STATE_SCOPE();

	typed_node()->name = token_t::from_string(name);
	return *this;
}

tree_combinator& ddeclarator_combinator::dinit_expr()
{
	return enter_child( e_initexpr, exprinit_comb );
}

tree_combinator& ddeclarator_combinator::dinit_list()
{
	return enter_child( e_initlist, listinit_comb );
}

void ddeclarator_combinator::child_ended()
{
	switch( leave() )
	{
	case e_initexpr:
		typed_node()->init = move_node2<initializer>( exprinit_comb );
		break;
	case e_initlist:
		typed_node()->init = move_node2<initializer>( listinit_comb );
		break;
	default:
		default_proc();
		break;
	}
}

/////////////////////////////////////
// variable combinator
SASL_TYPED_NODE_ACCESSORS_IMPL( dvar_combinator, variable_declaration );

dvar_combinator::dvar_combinator( tree_combinator* parent )
: tree_combinator( parent )
{
	typed_node( create_node<variable_declaration>( token_t::null() ) );
}

tree_combinator& dvar_combinator::dname( std::string const& str ){
	return enter_child( e_declarator, declarator_comb ).dname(str);
}

tree_combinator& dvar_combinator::dtype()
{
	return enter_child( e_type, type_comb );
}

void dvar_combinator::child_ended()
{
	switch( leave() )
	{
	case e_type:
		typed_node()->type_info = move_node( type_comb );
		break;
	case e_declarator:
		typed_node()->declarators.push_back( move_node2<declarator>( declarator_comb ) );
		break;
	default:
		default_proc();
		break;
	}
}

/////////////////////////////////////////////////////////////////
// expression combinator

SASL_TYPED_NODE_ACCESSORS_IMPL( dexpr_combinator, expression );

dexpr_combinator::dexpr_combinator( tree_combinator* parent )
: tree_combinator( parent )
{
}

tree_combinator& dexpr_combinator::dconstant( literal_classifications lct, const std::string& v )
{
	DEFAULT_STATE_SCOPE();

	boost::shared_ptr< constant_expression > ret
		= create_node<constant_expression>( token_t::null() );
	ret->value_tok = token_t::from_string( v );
	ret->ctype = lct;

	typed_node( ret );
	return *this;
}

tree_combinator& dexpr_combinator::dvarexpr( const std::string& v)
{
	DEFAULT_STATE_SCOPE();

	boost::shared_ptr< variable_expression > ret = create_node<variable_expression>( token_t::null() );
	ret->var_name = token_t::from_string( v );

	typed_node( ret );
	return *this;
}

tree_combinator& dexpr_combinator::dunary( operators op )
{
	assert( operators_helper::instance().is_unary(op) );
	assert( !typed_node() );

	enter(e_unary);

	boost::shared_ptr< unary_expression > ret = create_node<unary_expression>( token_t::null() );
	ret->op = op;
	typed_node( ret );
	expr_comb = boost::make_shared<dexpr_combinator>( this );

	return *expr_comb;
}

tree_combinator& dexpr_combinator::dcast()
{
	return enter_child( e_cast, cast_comb );
}

tree_combinator& dexpr_combinator::dbinary()
{
	return enter_child( e_binexpr, binexpr_comb );
}

tree_combinator& dexpr_combinator::dbranchexpr()
{
	return enter_child( e_branchexpr, branch_comb );
}

tree_combinator& dexpr_combinator::dmember( std::string const& m )
{
	DEFAULT_STATE_SCOPE();

	assert( typed_node() );
	boost::shared_ptr<member_expression> mexpr = create_node<member_expression>( token_t::null() );
	mexpr->expr = typed_node();
	mexpr->member = token_t::from_string(m);
	typed_node(mexpr);

	return *this;
}

tree_combinator& dexpr_combinator::dcall()
{
	return enter_child( e_callexpr, call_comb );
}

tree_combinator& dexpr_combinator::dindex()
{
	assert ( typed_node() );
	boost::shared_ptr<index_expression> indexexpr = create_node<index_expression>( token_t::null() );
	indexexpr->expr = typed_node();
	typed_node( indexexpr );
	enter( e_indexexpr );
	expr_comb = boost::make_shared<dexpr_combinator>(this);
	return *expr_comb;
}

void dexpr_combinator::child_ended()
{
	switch( leave() ){
		case e_unary:
			typed_node2<unary_expression>()->expr = move_node( expr_comb );
			return;

		case e_cast:
			assert( !typed_node() );
			typed_node( move_node( cast_comb ) );
			return;

		case e_binexpr:
			assert( !typed_node() );
			typed_node( move_node( binexpr_comb ) );
			return;

		case e_branchexpr:
			assert( !typed_node() );
			typed_node( move_node( branch_comb ) );
			return;

		case e_callexpr:
			assert( !typed_node() );
			typed_node( move_node( call_comb ) );
			return;

		case e_indexexpr:
			assert( typed_node() );
			typed_node2<index_expression>()->index_expr = move_node( expr_comb );
			return;

		default:
			assert( !"unknown state." );
			return;
	}
}

//////////////////////////////////////////////////////////////////////////
// cast combinator

SASL_TYPED_NODE_ACCESSORS_IMPL( dcast_combinator, cast_expression );

dcast_combinator::dcast_combinator( tree_combinator* parent )
: tree_combinator( parent )
{
	typed_node( create_node<cast_expression>( token_t::null() ) );
}

tree_combinator& dcast_combinator::dtype()
{
	return enter_child( e_type, type_comb );
}

tree_combinator& dcast_combinator::dexpr()
{
	return enter_child( e_expr, expr_comb );
}

void dcast_combinator::child_ended()
{
	switch ( leave() ){
		case e_type:
			typed_node()->casted_type = move_node( type_comb );
			return;
		case e_expr:
			typed_node()->expr = move_node( expr_comb );
			return;
		default:
			assert( !"invalid state." );
			return;
	}
}

//////////////////////////////////////////////////////////////////////////
// binary expression combinator
SASL_TYPED_NODE_ACCESSORS_IMPL( dbinexpr_combinator, binary_expression );

dbinexpr_combinator::dbinexpr_combinator( tree_combinator* parent )
: tree_combinator( parent )
{
	typed_node( create_node<binary_expression>( token_t::null() ) );
}

tree_combinator& dbinexpr_combinator::dlexpr()
{
	return enter_child( e_lexpr, lexpr_comb );
}

tree_combinator& dbinexpr_combinator::dop( operators op )
{
	DEFAULT_STATE_SCOPE();

	assert( typed_node()->op == operators::none );
	assert( operators_helper::instance().is_binary(op) );
	typed_node()->op = op;

	return *this;
}

tree_combinator& dbinexpr_combinator::drexpr()
{
	return enter_child( e_rexpr, rexpr_comb );
}

void dbinexpr_combinator::child_ended()
{
	switch( leave() ){
		case e_lexpr:
			typed_node()->left_expr = move_node( lexpr_comb );
			return;
		case e_rexpr:
			assert( rexpr_comb->typed_node() );
			typed_node()->right_expr = move_node( rexpr_comb );
			return;
		default:
			assert( !"invalid state" );
			return;
	}
}

//////////////////////////////////////////////////////////////////////////
//  conditional expression

SASL_TYPED_NODE_ACCESSORS_IMPL( dbranchexpr_combinator, cond_expression );

dbranchexpr_combinator::dbranchexpr_combinator( tree_combinator* parent )
: tree_combinator( parent )
{
	typed_node( create_node<cond_expression>( token_t::null() ) );
}

tree_combinator& dbranchexpr_combinator::dcond()
{
	return enter_child( e_cond, cond_comb );
}

tree_combinator& dbranchexpr_combinator::dyes()
{
	return enter_child( e_yes, yes_comb );
}

tree_combinator& dbranchexpr_combinator::dno()
{
	return enter_child( e_no, no_comb );
}

void dbranchexpr_combinator::child_ended()
{
	switch( leave() ){
		case e_cond:
			typed_node()->cond_expr = move_node( cond_comb );
			return;
		case e_yes:
			assert( yes_comb->typed_node() );
			typed_node()->yes_expr = yes_comb->typed_node();
			return;
		case e_no:
			assert( no_comb->typed_node() );
			typed_node()->no_expr = no_comb->typed_node();
			return;
		default:
			assert(!"invalid state.");
			return;
	}
}

//////////////////////////////////////////////////////////////////////////
// call expression combinator

SASL_TYPED_NODE_ACCESSORS_IMPL( dcallexpr_combinator, call_expression );

dcallexpr_combinator::dcallexpr_combinator( tree_combinator* parent )
: tree_combinator( parent )
{
	typed_node( create_node<call_expression>( token_t::null() ) );

	assert( parent->typed_node() );
	parent->get_node( typed_node()->expr );
	parent->typed_node( boost::shared_ptr<node>() );
}

tree_combinator& dcallexpr_combinator::dargument()
{
	return enter_child( e_argument, argexpr );
}

void dcallexpr_combinator::child_ended()
{
	switch ( leave() ){
		case e_argument:
			typed_node()->args.push_back( move_node2<expression>(argexpr) );
			return;
		default:
			assert( !"invalid state." );
			return;
	}
}

//////////////////////////////////////////////////////////////////////////
//  struct combinator

SASL_TYPED_NODE_ACCESSORS_IMPL( dstruct_combinator, struct_type );

dstruct_combinator::dstruct_combinator( tree_combinator* parent )
: tree_combinator( parent )
{
	typed_node( create_node<struct_type>( token_t::null() ) );
}

tree_combinator& dstruct_combinator::dname( const std::string& name )
{
	assert( !typed_node()->name );
	typed_node()->name = token_t::from_string( name );
	return *this;
}

tree_combinator& dstruct_combinator::dmember()
{
	enter_child(e_vardecl, var_comb);
	return *var_comb;
}

void dstruct_combinator::child_ended()
{
	switch( leave() )
	{
	case e_vardecl:
		typed_node()->decls.push_back( move_node2<declaration>(var_comb) );
		return;
	default:
		assert("invalid state.");
		return;
	}
}

//////////////////////////////////////////////////////////////////////////
// compound statements

SASL_TYPED_NODE_ACCESSORS_IMPL( dstatements_combinator, compound_statement );

dstatements_combinator::dstatements_combinator( tree_combinator* parent )
: tree_combinator( parent )
{
	typed_node( create_node<compound_statement>( token_t::null() ) );
}


tree_combinator& dstatements_combinator::dlabel( const std::string& lbl_str )
{
	boost::shared_ptr<ident_label> lbl = create_node<ident_label>( token_t::null() );
	lbl->label_tok = token_t::from_string( lbl_str );
	push_label(lbl);

	return *this;
}

tree_combinator& dstatements_combinator::dvarstmt()
{
	return enter_child( e_varstmt, var_comb );
}

tree_combinator& dstatements_combinator::dexprstmt()
{
	return enter_child( e_exprstmt, expr_comb );
}

tree_combinator& dstatements_combinator::dif()
{
	return enter_child( e_if, if_comb );
}

tree_combinator& dstatements_combinator::ddowhile()
{
	return enter_child( e_dowhile, dowhile_comb );
}

tree_combinator& dstatements_combinator::dwhiledo()
{
	return enter_child( e_whiledo, whiledo_comb );
}

tree_combinator& dstatements_combinator::dswitch()
{
	return enter_child( e_switch, switch_comb );
}

tree_combinator& dstatements_combinator::dfor()
{
	return enter_child( e_for, for_comb );
}

tree_combinator& dstatements_combinator::dbreak()
{
	DEFAULT_STATE_SCOPE();

	boost::shared_ptr<jump_statement> jumpstmt = create_node<jump_statement>( token_t::null() );
	jumpstmt->code = jump_mode::_break;
	typed_node()->stmts.push_back( boost::shared_polymorphic_downcast<statement>(jumpstmt) );

	return *this;
}

tree_combinator& dstatements_combinator::dcontinue()
{
	DEFAULT_STATE_SCOPE();

	boost::shared_ptr<jump_statement> jumpstmt = create_node<jump_statement>( token_t::null() );
	jumpstmt->code = jump_mode::_continue;
	typed_node()->stmts.push_back( boost::shared_polymorphic_downcast<statement>(jumpstmt) );

	return *this;
}

tree_combinator& dstatements_combinator::dreturn_expr()
{
	return enter_child( e_return, ret_comb );
}

tree_combinator& dstatements_combinator::dreturn_void()
{
	return dreturn_expr().dnode( boost::shared_ptr<node>() ).end();
}

tree_combinator& dstatements_combinator::dstmts(){
	return enter_child( e_compound, compound_comb );
}

void dstatements_combinator::child_ended()
{
	switch( leave() ){
		case e_varstmt:
			typed_node()->stmts.push_back(
				move_node2<statement>( var_comb ) );
			break;
		case e_exprstmt:
			typed_node()->stmts.push_back(
				move_node2<statement>( expr_comb ) );
			break;
		case e_if:
			typed_node()->stmts.push_back( move_node2<statement>( if_comb ) );
			break;
		case e_dowhile:
			typed_node()->stmts.push_back( move_node2<statement>(dowhile_comb) );
			break;
		case e_whiledo:
			typed_node()->stmts.push_back( move_node2<statement>(whiledo_comb) );
			break;
		case e_switch:
			typed_node()->stmts.push_back( move_node2<statement>(switch_comb) );
			break;
		case e_for:
			typed_node()->stmts.push_back( move_node2<statement>(for_comb) );
			break;
		case e_return:
			typed_node()->stmts.push_back( move_node2<statement>(ret_comb) );
			break;
		case e_compound:
			typed_node()->stmts.push_back( move_node2<statement>( compound_comb ) );
			break;
		default:
			assert(!"invalid state.");
			return;
	}

	typed_node()->stmts.back()->labels = lbls;
	lbls.clear();
}

//////////////////////////////////////////////////////////////////////////
// declaration statement combinator

dvarstmt_combinator::dvarstmt_combinator( tree_combinator* parent )
: dvar_combinator( parent ){
}

void dvarstmt_combinator::before_end()
{
	if( typed_node2<node>()->node_class() == node_ids::declaration_statement ){
		// this node may be set by dnode() function.
		return;
	}
	assert( typed_node() );
	boost::shared_ptr<declaration_statement> instead_node = create_node<declaration_statement>( token_t::null() );
	instead_node->decl = typed_node();
	typed_node( instead_node );
}

//////////////////////////////////////////////////////////////////////////
// expression statement combinator

dexprstmt_combinator::dexprstmt_combinator( tree_combinator* parent )
: dexpr_combinator( parent ){}

void dexprstmt_combinator::before_end()
{
	if( typed_node2<node>()->node_class() == node_ids::expression_statement ){
		// this node may be set by dnode() function.
		return;
	}
	assert( typed_node() );
	boost::shared_ptr<expression_statement> instead_node = create_node<expression_statement>( token_t::null() );
	instead_node->expr = typed_node();
	typed_node( instead_node );
}

//////////////////////////////////////////////////////////////////////////
// if statement combinator

SASL_TYPED_NODE_ACCESSORS_IMPL( dif_combinator, if_statement );

dif_combinator::dif_combinator( tree_combinator* parent )
: tree_combinator( parent )
{
	typed_node( create_node<if_statement>( token_t::null() ) );
}

tree_combinator& dif_combinator::dcond(){
	return enter_child( e_cond, expr_comb );
}

tree_combinator& dif_combinator::dthen(){
	return enter_child( e_then, then_stmt_comb );
}

tree_combinator& dif_combinator::delse(){
	return enter_child( e_else, else_stmt_comb );
}

void dif_combinator::child_ended()
{
	switch( leave() ){
		case e_cond:
			typed_node()->cond = move_node2<expression>( expr_comb );
			return;
		case e_then:
			typed_node()->yes_stmt = move_node2<statement>( then_stmt_comb );
			return;
		case e_else:
			typed_node()->no_stmt = move_node2<statement>( else_stmt_comb );
			return;
		default:
			assert(!"invalid state.");
			return;
	}
}

//////////////////////////////////////////////////////////////////////////
// do-while statement combinator.

SASL_TYPED_NODE_ACCESSORS_IMPL( ddowhile_combinator, dowhile_statement );

ddowhile_combinator::ddowhile_combinator( tree_combinator* parent )
: tree_combinator( parent )
{
	typed_node( create_node<dowhile_statement>( token_t::null() ) );
}

tree_combinator& ddowhile_combinator::ddo()
{
	return enter_child( e_do, do_comb );
}

tree_combinator& ddowhile_combinator::dwhile()
{
	return enter_child( e_while, cond_comb );
}

void ddowhile_combinator::child_ended()
{
	switch( leave() ){
		case e_do:
			typed_node()->body = move_node2<statement>(do_comb);
			return;
		case e_while:
			typed_node()->cond = move_node2<expression>(cond_comb);
			return;
		default:
			assert( !"invalid state" );
			return;
	}
}

//////////////////////////////////////////////////////////////////////////
// while-do statement combinator

SASL_TYPED_NODE_ACCESSORS_IMPL( dwhiledo_combinator, while_statement );

dwhiledo_combinator::dwhiledo_combinator( tree_combinator* parent )
: tree_combinator( parent )
{
	typed_node( create_node<while_statement>( token_t::null() ) );
}

tree_combinator& dwhiledo_combinator::ddo()
{
	return enter_child( e_do, do_comb );
}

tree_combinator& dwhiledo_combinator::dwhile()
{
	return enter_child( e_while, cond_comb );
}

void dwhiledo_combinator::child_ended()
{
	switch( leave() ){
		case e_do:
			typed_node()->body = move_node2<statement>(do_comb);
			return;
		case e_while:
			typed_node()->cond = move_node2<expression>(cond_comb);
			return;
		default:
			assert( !"invalid state" );
			return;
	}
}

//////////////////////////////////////////////////////////////////////////
// switch combinator
SASL_TYPED_NODE_ACCESSORS_IMPL( dswitch_combinator, switch_statement );

dswitch_combinator::dswitch_combinator( tree_combinator* parent )
: tree_combinator( parent ){
	typed_node( create_node<switch_statement>( token_t::null() ) );
}

tree_combinator& dswitch_combinator::dexpr(){
	return enter_child( e_case, expr_comb );
}

tree_combinator& dswitch_combinator::dbody(){
	return enter_child( e_switchbody, body_comb );
}

void dswitch_combinator::child_ended()
{
	switch( leave() ){
		case e_case:
			typed_node()->cond = move_node2<expression>(expr_comb);
			return;
		case e_switchbody:
			typed_node()->stmts = move_node2<compound_statement>(body_comb);
			return;
		default:
			assert( !"invalid state.");
			return;
	}
}
//////////////////////////////////////////////////////////////////////////
// switch body combinator

dswitchbody_combinator::dswitchbody_combinator( tree_combinator* parent )
: dstatements_combinator( parent ){
}

tree_combinator& dswitchbody_combinator::dcase(){
	return enter_child( e_case, case_comb );
}

tree_combinator& dswitchbody_combinator::ddefault()
{
	return dcase().dnode( boost::shared_ptr<node>() ).end();
}

void dswitchbody_combinator::child_ended()
{
	state_t stat = leave();
	switch( stat ){
	case e_case:
		push_label( move_node2<label>( case_comb ) );
		return;
	default:
		// reset the state and dispatch to parent to process.
		enter( stat );
		dstatements_combinator::child_ended();
	}
}
//////////////////////////////////////////////////////////////////////////
// case expression combinator
dcase_combinator::dcase_combinator( tree_combinator* parent )
: dexpr_combinator( parent ){
}

void dcase_combinator::before_end(){
	if ( typed_node2<node>() && typed_node2<node>()->node_class() == node_ids::case_label ){
		return;
	}

	boost::shared_ptr<case_label> instead_node = create_node<case_label>( token_t::null() );
	instead_node->expr = typed_node2<expression>();
	typed_node( instead_node );
}

//////////////////////////////////////////////////////////////////////////
// return combinator
dreturn_combinator::dreturn_combinator( tree_combinator* parent )
: dexpr_combinator( parent )
{
}

void dreturn_combinator::before_end(){
	if ( typed_node2<node>()
		&& typed_node2<node>()->node_class() == node_ids::jump_statement ){
		return;
	}
	boost::shared_ptr<jump_statement> instead_node = create_node<jump_statement>( token_t::null() );
	instead_node->jump_expr = typed_node2<expression>();
	instead_node->code = jump_mode::_return;
	typed_node( instead_node );
}

//////////////////////////////////////////////////////////////////////////
// for combinator

SASL_TYPED_NODE_ACCESSORS_IMPL( dfor_combinator, for_statement );

dfor_combinator::dfor_combinator( tree_combinator* parent )
: tree_combinator( parent ){
	typed_node( create_node<for_statement>( token_t::null() ) );
}

tree_combinator& dfor_combinator::dinit_expr(){
	return enter_child( e_init, initexpr_comb );
}

tree_combinator& dfor_combinator::dinit_var(){
	return enter_child( e_init, initvar_comb );
}

tree_combinator& dfor_combinator::dcond(){
	return enter_child( e_cond, cond_comb );
}

tree_combinator& dfor_combinator::diter(){
	return enter_child( e_iter, iter_comb );
}

tree_combinator& dfor_combinator::dbody(){
	return enter_child( e_body, body_comb );
}

void dfor_combinator::child_ended()
{
	switch( leave() ){
		case e_init:
			if ( initexpr_comb ){
				typed_node()->init = move_node2<statement>( initexpr_comb );
			}
			if ( initvar_comb ){
				typed_node()->init = move_node2<statement>( initvar_comb );
			}
			return;
		case e_cond:
			typed_node()->cond = move_node( cond_comb );
			return;
		case e_iter:
			typed_node()->iter = move_node( iter_comb );
			return;
		case e_body:
			typed_node()->body = move_node( body_comb );
			return;
		default:
			assert( !"invalid state." );
			return;
	}
}

//////////////////////////////////////////////////////////////////////////
// function combinator

SASL_TYPED_NODE_ACCESSORS_IMPL( dfunction_combinator, function_type );

dfunction_combinator::dfunction_combinator( tree_combinator* parent )
: tree_combinator( parent ){
	typed_node( create_node<function_type>( token_t::null() ) );
}

tree_combinator& dfunction_combinator::dname( const std::string& str ){
	typed_node()->name = token_t::from_string(str);
	return *this;
}

tree_combinator& dfunction_combinator::dreturntype(){
	return enter_child( e_type, rettype_comb );
}

tree_combinator& dfunction_combinator::dparam(){
	return enter_child( e_param, par_comb );
}

tree_combinator& dfunction_combinator::dbody(){
	return enter_child( e_body, body_comb );
}

void dfunction_combinator::child_ended(){
	switch( leave() ){
		case e_type:
			typed_node()->retval_type = move_node2<tynode>( rettype_comb );
			break;
		case e_param:
			typed_node()->params.push_back( move_node2<parameter>(par_comb) );
			break;
		case e_body:
			typed_node()->body = move_node2<compound_statement>( body_comb );
			break;
	}
}

//////////////////////////////////////////////////////////////////////////
// parameter combinator
dparameter_combinator::dparameter_combinator( tree_combinator* parent )
: dvar_combinator( parent ){
}

void dparameter_combinator::before_end(){
	if( typed_node2<node>()->node_class() == node_ids::parameter ){
		return;
	}
	assert( typed_node() );
	assert( typed_node()->declarators.size() <= 1 );

	boost::shared_ptr<parameter> instead_node = create_node<parameter>( token_t::null() );
	instead_node->param_type = typed_node()->type_info;
	if( ! typed_node()->declarators.empty() ){
		instead_node->name = typed_node()->declarators[0]->name;
		instead_node->init = typed_node()->declarators[0]->init;
	}

	typed_node( instead_node );
}

//////////////////////////////////////////////////////////////////////////
// typedef combinator

SASL_TYPED_NODE_ACCESSORS_IMPL( dtypedef_combinator, type_definition )

dtypedef_combinator::dtypedef_combinator( tree_combinator* parent )
: tree_combinator( parent ){
	typed_node( create_node<type_definition>( token_t::null() ) );
}

tree_combinator& dtypedef_combinator::dname( const std::string& name ){
	typed_node()->name = token_t::from_string(name);
	return *this;
}

tree_combinator& dtypedef_combinator::dtype(){
	return enter_child( e_type, type_comb );
}

void dtypedef_combinator::child_ended(){
	switch( leave() ){
		case e_type:
			typed_node()->type_info = move_node2<tynode>( type_comb );
			break;
		default:
			assert( !"invalid state." );
			break;
	}
}

//////////////////////////////////////////////////////////////////////////
// expression initializer combinator
dinitexpr_combinator::dinitexpr_combinator( tree_combinator* parent )
: dexpr_combinator( parent ){
}

void dinitexpr_combinator::before_end()
{
	if ( !typed_node2<node>() ) { return; }
	if (typed_node2<node>()->node_class() == node_ids::expression_initializer){
		return;
	}
	boost::shared_ptr<expression_initializer> instead_node
		= create_node<expression_initializer>( token_t::null() );
	instead_node->init_expr = typed_node2<expression>();
	typed_node( instead_node );
}

//////////////////////////////////////////////////////////////////////////
// initilaizer list combinator

SASL_TYPED_NODE_ACCESSORS_IMPL( dinitlist_combinator, member_initializer );

dinitlist_combinator::dinitlist_combinator( tree_combinator* parent )
: tree_combinator( parent ){
	typed_node( create_node<member_initializer>( token_t::null() ) );
}

tree_combinator& dinitlist_combinator::dinit_expr(){
	return enter_child( e_initexpr, expr_comb );
}

tree_combinator& dinitlist_combinator::dinit_list()
{
	return enter_child( e_initlist, list_comb );
}

void dinitlist_combinator::child_ended()
{
	switch ( leave() ){
		case e_initexpr:
			typed_node()->sub_inits.push_back( move_node2<initializer>(expr_comb) );
			break;
		case e_initlist:
			typed_node()->sub_inits.push_back( move_node2<initializer>(list_comb) );
			break;
		default:
			assert(!"invalid state" );
			break;
	}
}

END_NS_SASL_SYNTAX_TREE();
