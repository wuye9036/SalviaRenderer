#include <sasl/include/semantic/semantic_analyser.h>

#include <sasl/enums/operators.h>

#include <sasl/include/common/compiler_info_manager.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/semantic_error.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol_scope.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/semantic/type_converter.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/make_tree.h>
#include <sasl/include/syntax_tree/program.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/utility.h>

#include <softart/include/enums.h>

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

using ::sasl::common::compiler_info_manager;
using ::sasl::common::token_t;

using ::sasl::syntax_tree::alias_type;
using ::sasl::syntax_tree::binary_expression;
using ::sasl::syntax_tree::builtin_type;
using ::sasl::syntax_tree::cast_expression;
using ::sasl::syntax_tree::create_builtin_type;
using ::sasl::syntax_tree::create_node;
using ::sasl::syntax_tree::compound_statement;
using ::sasl::syntax_tree::constant_expression;
using ::sasl::syntax_tree::declaration;
using ::sasl::syntax_tree::declarator;
using ::sasl::syntax_tree::declaration_statement;
using ::sasl::syntax_tree::expression;
using ::sasl::syntax_tree::expression_initializer;
using ::sasl::syntax_tree::expression_statement;
using ::sasl::syntax_tree::function_type;
using ::sasl::syntax_tree::if_statement;
using ::sasl::syntax_tree::jump_statement;
using ::sasl::syntax_tree::member_expression;
using ::sasl::syntax_tree::node;
using ::sasl::syntax_tree::parameter;
using ::sasl::syntax_tree::program;
using ::sasl::syntax_tree::statement;
using ::sasl::syntax_tree::struct_type;
using ::sasl::syntax_tree::type_specifier;
using ::sasl::syntax_tree::variable_declaration;
using ::sasl::syntax_tree::variable_expression;
using ::sasl::syntax_tree::dfunction_combinator;

using namespace boost::assign;

using boost::any;
using boost::any_cast;
using boost::format;
using boost::shared_ptr;
using boost::unordered_map;

using std::vector;
using std::string;

#define SASL_GET_OR_CREATE_SI( si_type, si_var, node ) shared_ptr<si_type> si_var = get_or_create_semantic_info<si_type>(node);
#define SASL_GET_OR_CREATE_SI_P( si_type, si_var, node, param ) shared_ptr<si_type> si_var = get_or_create_semantic_info<si_type>( node, param );

#define SASL_EXTRACT_SI( si_type, si_var, node ) shared_ptr<si_type> si_var = extract_semantic_info<si_type>(node);

// semantic analysis context
struct sacontext{
	sacontext(): declarator_type_id(-1), is_global(true), member_index(-1){
	}

	shared_ptr<symbol> parent_sym;

	shared_ptr<node> generated_node;

	shared_ptr<node> variable_to_fill; // for initializer only.
	type_entry::id_t declarator_type_id;

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

shared_ptr<type_specifier> type_info_of( shared_ptr<node> n ){
	shared_ptr<type_info_si> typesi = extract_semantic_info<type_info_si>( n );
	if ( typesi ){
		return typesi->type_info();
	}
	return shared_ptr<type_specifier>();
}

semantic_analyser::semantic_analyser()
{
	typeconv.reset( new type_converter() );
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
		generated_node = ctxt_ptr(child_ctxt)->generated_node->typed_handle<NodeT>();
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
		softart::semantic sem( softart::SV_None );
		string const& semstr = sem_tok->str;

		if( semstr == "SV_Position" ){
			sem = softart::SV_Position;
		} else if ( semstr == "SV_RPosition" ){
			sem = softart::SV_RPosition;
		} else if( semstr == "SV_Texcoord" ){
			int index = 0;
			if( sem_idx_tok ){
				index = boost::lexical_cast<int16_t>(sem_idx_tok->str);
			}
			sem = pack_semantic( softart::SV_Texcoord, index );
		} else {
			EFLIB_ASSERT_UNIMPLEMENTED();
		}

		ssi->set_semantic( sem );
	}
}

SASL_VISIT_DEF_UNIMPL( unary_expression );
SASL_VISIT_DEF( cast_expression ){
	any child_ctxt_init = *data;
	
	shared_ptr<cast_expression> dup_cexpr = duplicate(v.handle())->typed_handle<cast_expression>();
	
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.casted_type, dup_cexpr->casted_type );
	visit_child( child_ctxt, child_ctxt_init, v.expr, dup_cexpr->expr );

	shared_ptr<type_info_si> src_tsi = extract_semantic_info<type_info_si>( dup_cexpr->expr );
	shared_ptr<type_info_si> casted_tsi = extract_semantic_info<type_info_si>( dup_cexpr->casted_type );

	if( src_tsi->entry_id() != casted_tsi->entry_id() ){
		if( typeconv->convertible( casted_tsi->entry_id(), src_tsi->entry_id() ) == type_converter::cannot_conv ){
			// Here is code error. Compiler should report it.
			EFLIB_ASSERT_UNIMPLEMENTED();
			return;
		}
	}

	SASL_GET_OR_CREATE_SI_P( storage_si, ssi, dup_cexpr, msi->type_manager() );
	ssi->entry_id( casted_tsi->entry_id() );

	data_cptr()->generated_node = dup_cexpr->handle();
}

SASL_VISIT_DEF( binary_expression )
{
	any child_ctxt_init = *data;
	ctxt_ptr(child_ctxt_init)->generated_node.reset();

	shared_ptr<binary_expression> dup_expr = duplicate( v.handle() )->typed_handle<binary_expression>();

	any child_ctxt;
	visit_child( child_ctxt, child_ctxt_init, v.left_expr, dup_expr->left_expr );
	visit_child( child_ctxt, child_ctxt_init, v.right_expr, dup_expr->right_expr );

	// TODO: look up operator prototype.
	std::string opname = operator_name( v.op );
	vector< shared_ptr<expression> > exprs;
	exprs += dup_expr->left_expr, dup_expr->right_expr;
	vector< shared_ptr<symbol> > overloads = data_cptr()->parent_sym->find_overloads( opname, typeconv, exprs );

	EFLIB_ASSERT_AND_IF( !overloads.empty(), "Need to report a compiler error. No overloading." ){
		return;
	}
	EFLIB_ASSERT_AND_IF( overloads.size() == 1,	( format(
		"Need to report a compiler error. Ambigous overloading. \r\n"
		"operator is %1%, left expression type is %2%, right expression type is %3%. \r\n" 
		) % v.op.name() % type_info_of( dup_expr->left_expr )->value_typecode.name()
		% type_info_of( dup_expr->right_expr )->value_typecode.name() ).str().c_str()
		)
	{
		return;
	}

	// update semantic information of binary expression
	type_entry::id_t result_tid = extract_semantic_info<type_info_si>( overloads[0]->node() )->entry_id();
	get_or_create_semantic_info<storage_si>( dup_expr, msi->type_manager() )->entry_id( result_tid );

	data_cptr()->generated_node = dup_expr->handle();
}

SASL_VISIT_DEF_UNIMPL( expression_list );
SASL_VISIT_DEF_UNIMPL( cond_expression );
SASL_VISIT_DEF_UNIMPL( index_expression );
SASL_VISIT_DEF_UNIMPL( call_expression );

int check_swizzle( builtin_type_code btc, std::string const& mask, int32_t& swizzle_code ){
	swizzle_code = 0;

	if( mask.length() > 4 ){
		return 0;
	}

	size_t agg_size = 0;
	if( sasl_ehelper::is_scalar(btc) ){
		agg_size = 1;
	} else if( sasl_ehelper::is_vector(btc) ){
		agg_size = sasl_ehelper::len_0( btc );
	} else if( sasl_ehelper::is_matrix(btc) ){
		agg_size = sasl_ehelper::len_1( btc );
	}

	if( agg_size == 0 ){ return 0; }

	int min_src_size = 0;
	int dest_size = 0;
	swizzle_code = encode_swizzle( dest_size, min_src_size, mask.c_str() );
	
	assert( min_src_size <= agg_size );

	return dest_size;
}

SASL_VISIT_DEF( member_expression ){
	shared_ptr<member_expression> dup_expr =
		duplicate( v.handle() )->typed_handle<member_expression>();

	any child_ctxt = *data;
	visit_child( child_ctxt, v.expr );
	dup_expr->expr = ctxt_ptr(child_ctxt)->generated_node->typed_handle<expression>();

	SASL_EXTRACT_SI( type_info_si, arg_tisi, dup_expr->expr );
	assert( arg_tisi );

	shared_ptr<type_specifier> agg_type = arg_tisi->type_info();
	type_entry::id_t mem_typeid = -1;

	
	int32_t swizzle_code = 0;
	if( agg_type->node_class() == syntax_node_types::struct_type ){
		// Aggeragated is struct
		shared_ptr<symbol> struct_sym = agg_type->typed_handle<struct_type>()->symbol();
		shared_ptr<declarator> mem_declr
			= struct_sym->find_this( v.member->str )->node()->typed_handle<declarator>();
		// TODO if mem_declr isn't found, it means the name of member is wrong.
		// Need to report that.
		assert( mem_declr );
		SASL_EXTRACT_SI( type_info_si, mem_si, mem_declr );
		mem_typeid = mem_si->entry_id();
		assert( mem_typeid != -1 );
	} else if( agg_type->is_builtin() ){
		// Aggregated class is vector & matrix
		builtin_type_code agg_btc = agg_type->value_typecode;
		int field_count = check_swizzle( agg_btc, v.member->str, swizzle_code );
		if( field_count > 0 ){
			builtin_type_code elem_btc = sasl_ehelper::scalar_of( agg_btc );
			builtin_type_code swizzled_btc = builtin_type_code::none;

			if( sasl_ehelper::is_scalar(agg_btc)
				|| sasl_ehelper::is_vector(agg_btc) )
			{
				swizzled_btc = sasl_ehelper::vector_of(
					elem_btc,
					static_cast<size_t>( field_count )
					);
			} else {
				// matrix only
				swizzled_btc = sasl_ehelper::matrix_of(
					elem_btc,
					sasl_ehelper::len_0( agg_btc ),
					static_cast<size_t>( field_count )
					);
			}

			mem_typeid = msi->type_manager()->get( swizzled_btc );
		} else {
			// TODO swizzle fields are some errors.
			EFLIB_ASSERT_UNIMPLEMENTED();
			return;
		}
	} else {
		// TODO:
		//	If type is not a struct or builtin type, it could not support member operation.
		//	Error on compiling.
		EFLIB_ASSERT_UNIMPLEMENTED();
		return;
	}

	SASL_GET_OR_CREATE_SI_P( storage_si, ssi, dup_expr, msi->type_manager() );
	ssi->entry_id( mem_typeid );
	ssi->swizzle( swizzle_code );

	data_cptr()->generated_node = dup_expr;
}

SASL_VISIT_DEF( constant_expression )
{
	shared_ptr<constant_expression> dup_cexpr = duplicate( v.handle() )->typed_handle<constant_expression>();
	
	SASL_GET_OR_CREATE_SI_P( const_value_si, vsi, dup_cexpr, msi->type_manager() );
	vsi->set_literal( v.value_tok->str, v.ctype );

	data_cptr()->generated_node = dup_cexpr->handle();	
}

SASL_VISIT_DEF( variable_expression ){

	shared_ptr<symbol> vdecl = data_cptr()->parent_sym->find( v.var_name->str );
	shared_ptr<variable_expression> dup_vexpr = duplicate( v.handle() )->typed_handle<variable_expression>();
	
	dup_vexpr->semantic_info( vdecl->node()->semantic_info() );
	data_cptr()->generated_node = dup_vexpr->handle();
}

// declaration & type specifier
SASL_VISIT_DEF_UNIMPL( initializer );
SASL_VISIT_DEF( expression_initializer )
{
	shared_ptr<expression_initializer> dup_exprinit = duplicate( v.handle() )->typed_handle<expression_initializer>();

	any child_ctxt_init = *data;
	ctxt_ptr(child_ctxt_init)->generated_node.reset();

	any child_ctxt;
	visit_child( child_ctxt, child_ctxt_init, v.init_expr, dup_exprinit->init_expr );

	SASL_GET_OR_CREATE_SI_P( storage_si, ssi, dup_exprinit, msi->type_manager() );
	ssi->entry_id( extract_semantic_info<type_info_si>(dup_exprinit->init_expr)->entry_id() );

	shared_ptr<type_info_si> var_tsi = extract_semantic_info<type_info_si>( data_cptr()->variable_to_fill );
	if ( var_tsi->entry_id() != ssi->entry_id() ){
		assert( typeconv->implicit_convertible( var_tsi->entry_id(), ssi->entry_id() ) );
	}

	data_cptr()->generated_node = dup_exprinit->handle();
}

SASL_VISIT_DEF_UNIMPL( member_initializer );
SASL_VISIT_DEF_UNIMPL( declaration );
SASL_VISIT_DEF( declarator ){

	any child_ctxt_init = *data;
	ctxt_ptr(child_ctxt_init)->generated_node.reset();
	any child_ctxt;

	shared_ptr<declarator> dup_decl = duplicate( v.handle() )->typed_handle<declarator>();

	SASL_GET_OR_CREATE_SI_P( storage_si, ssi, dup_decl, msi->type_manager() );
	ssi->entry_id( data_cptr()->declarator_type_id );

	if( data_cptr()->member_index >= 0 ){
		 ssi->mem_index( data_cptr()->member_index++ );
	}

	ctxt_ptr(child_ctxt_init)->variable_to_fill = dup_decl;
	if ( v.init ){
		visit_child( child_ctxt, child_ctxt_init, v.init, dup_decl->init );
	}

	shared_ptr<symbol> nodesym = data_cptr()->parent_sym->add_child( v.name->str, dup_decl );

	parse_semantic( v.semantic, v.semantic_index, ssi );

	if(	data_cptr()->is_global )
	{
		msi->globals().push_back( nodesym );
	}

	data_cptr()->generated_node = dup_decl->handle();
}

SASL_VISIT_DEF( variable_declaration )
{
	any child_ctxt_init = *data;
	ctxt_ptr(child_ctxt_init)->generated_node.reset();
	any child_ctxt;

	shared_ptr<variable_declaration> dup_vdecl = duplicate( v.handle() )->typed_handle<variable_declaration>();

	visit_child( child_ctxt, child_ctxt_init, v.type_info, dup_vdecl->type_info );
	
	ctxt_ptr(child_ctxt_init)->declarator_type_id
		= extract_semantic_info<type_info_si>( dup_vdecl->type_info )->entry_id();
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

	data_cptr()->generated_node = dup_vdecl->handle();
}

SASL_VISIT_DEF_UNIMPL( type_definition );
SASL_VISIT_DEF_UNIMPL( type_specifier );
SASL_VISIT_DEF( builtin_type ){
	// create type information on current symbol.
	// for e.g. create type info onto a variable node.
	SASL_GET_OR_CREATE_SI_P( type_si, tsi, v, msi->type_manager() );
	tsi->type_info( v.typed_handle<type_specifier>(), data_cptr()->parent_sym );

	data_cptr()->generated_node = tsi->type_info()->handle();
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
	type_entry::id_t dup_struct_id
		= msi->type_manager()->get( v.typed_handle<type_specifier>(), data_cptr()->parent_sym );

	assert( dup_struct_id != -1 );

	shared_ptr<struct_type> dup_struct
		= msi->type_manager()->get( dup_struct_id )->typed_handle<struct_type>();
	
	SASL_EXTRACT_SI( type_info_si, tisi, dup_struct );
	dup_struct = tisi->type_info()->typed_handle<struct_type>();
	if( !dup_struct->decls.empty() && !v.decls.empty() && v.decls[0] != dup_struct->decls[0] ){
		// TODO: struct redefinition
		EFLIB_ASSERT_UNIMPLEMENTED();
	} else {
		dup_struct->decls.clear();
	}

	shared_ptr<symbol> sym = dup_struct->symbol();

	any child_ctxt;
	any child_ctxt_init = *data;
	ctxt_ptr(child_ctxt_init)->parent_sym = sym;
	ctxt_ptr(child_ctxt_init)->member_index = 0;

	BOOST_FOREACH( shared_ptr<declaration> const& decl, v.decls ){
		visit_child( child_ctxt, child_ctxt_init, decl );
		dup_struct->decls.push_back(
			ctxt_ptr( child_ctxt )->generated_node->typed_handle<declaration>()
			);

		// Update member index
		ctxt_ptr(child_ctxt_init)->member_index = ctxt_ptr(child_ctxt)->member_index;
	}

	data_cptr()->generated_node = dup_struct;
}

SASL_VISIT_DEF( alias_type ){
	type_entry::id_t dup_struct_id
		= msi->type_manager()->get( v.typed_handle<type_specifier>(), data_cptr()->parent_sym );
	// TODO: If struct id not found, it means the type name is wrong.
	// Compiler will report that.
	assert( dup_struct_id != -1 );

	data_cptr()->generated_node = msi->type_manager()->get(dup_struct_id);
}

SASL_VISIT_DEF( parameter )
{
	shared_ptr<parameter> dup_par = duplicate( v.handle() )->typed_handle<parameter>();
	data_cptr()->parent_sym->add_child( v.name ? v.name->str : std::string(), dup_par );

	any child_ctxt;
	visit_child( child_ctxt, *data, v.param_type, dup_par->param_type );

	if ( v.init ){
		visit_child( child_ctxt, *data, v.init, dup_par->init );
	}

	type_entry::id_t tid = extract_semantic_info<type_info_si>(dup_par->param_type)->entry_id();
	shared_ptr<storage_si> ssi = get_or_create_semantic_info<storage_si>( dup_par, msi->type_manager() );
	ssi->entry_id( tid );

	parse_semantic( v.semantic, v.semantic_index, ssi );

	data_cptr()->generated_node = dup_par->handle();
}

SASL_VISIT_DEF( function_type )
{
	// Copy node
	shared_ptr<node> dup_node = duplicate( v.handle() );
	EFLIB_ASSERT_AND_IF( dup_node, "Node swallow duplicated error !"){
		return;
	}
	shared_ptr<function_type> dup_fn = dup_node->typed_handle<function_type>();
	dup_fn->params.clear();

	shared_ptr<symbol> sym = data_cptr()->parent_sym->add_function_begin( dup_fn );

	any child_ctxt_init = *data;
	ctxt_ptr(child_ctxt_init)->parent_sym = sym;

	any child_ctxt;

	dup_fn->retval_type
		= ctxt_ptr( visit_child( child_ctxt, child_ctxt_init, v.retval_type ) )->generated_node->typed_handle<type_specifier>();

	for( vector< shared_ptr<parameter> >::iterator it = v.params.begin();
		it != v.params.end(); ++it )
	{
		dup_fn->params.push_back( 
			ctxt_ptr( visit_child(child_ctxt, child_ctxt_init, *it) )->generated_node->typed_handle<parameter>()
			);
	}

	data_cptr()->parent_sym->add_function_end( sym );
	
	type_entry::id_t ret_tid = extract_semantic_info<type_info_si>( dup_fn->retval_type )->entry_id();

	shared_ptr<storage_si> ssi = get_or_create_semantic_info<storage_si>( dup_fn, msi->type_manager() );
	ssi->entry_id( ret_tid );
	parse_semantic( v.semantic, v.semantic_index, ssi );

	ctxt_ptr(child_ctxt_init)->is_global = false;

	if ( v.body ){
		visit_child( child_ctxt, child_ctxt_init, v.body, dup_fn->body );
		msi->functions().push_back( sym );
	}

	data_cptr()->generated_node = dup_fn;
}

// statement
SASL_VISIT_DEF_UNIMPL( statement );

SASL_VISIT_DEF( declaration_statement )
{
	any child_ctxt_init = *data;
	ctxt_ptr(child_ctxt_init)->generated_node.reset();
	any child_ctxt;

	shared_ptr<declaration_statement> dup_declstmt = duplicate( v.handle() )->typed_handle<declaration_statement>();

	visit_child( child_ctxt, child_ctxt_init, v.decl, dup_declstmt->decl );

	get_or_create_semantic_info<statement_si>(dup_declstmt);

	data_cptr()->generated_node = dup_declstmt;
}

SASL_VISIT_DEF( if_statement )
{
	any child_ctxt_init = *data;
	any child_ctxt;

	shared_ptr<if_statement> dup_ifstmt = duplicate(v.handle())->typed_handle<if_statement>();

	visit_child( child_ctxt, child_ctxt_init, v.cond, dup_ifstmt->cond );
	shared_ptr<type_info_si> cond_tsi = extract_semantic_info<type_info_si>(dup_ifstmt->cond);
	assert( cond_tsi );
	type_entry::id_t bool_tid = msi->type_manager()->get( builtin_type_code::_boolean );
	assert( cond_tsi->entry_id() == bool_tid || typeconv->implicit_convertible( bool_tid, cond_tsi->entry_id() ) );

	visit_child( child_ctxt, child_ctxt_init, v.yes_stmt, dup_ifstmt->yes_stmt );
	visit_child( child_ctxt, child_ctxt_init, v.no_stmt, dup_ifstmt->no_stmt );

	extract_semantic_info<statement_si>(dup_ifstmt->yes_stmt)->parent_block( dup_ifstmt );
	extract_semantic_info<statement_si>(dup_ifstmt->no_stmt)->parent_block( dup_ifstmt );

	if( !dup_ifstmt->yes_stmt->symbol() ){
		data_cptr()->parent_sym->add_anonymous_child( dup_ifstmt->yes_stmt );
	}

	if( !dup_ifstmt->no_stmt->symbol() ){
		data_cptr()->parent_sym->add_anonymous_child( dup_ifstmt->yes_stmt );
	}

	SASL_GET_OR_CREATE_SI( statement_si, si, dup_ifstmt);
	si->exit_point( symbol::unique_name() );

	data_cptr()->generated_node = dup_ifstmt;
}

SASL_VISIT_DEF( while_statement ){
	v.cond->accept( this, data );
	v.body->accept( this, data );
}

SASL_VISIT_DEF_UNIMPL( dowhile_statement );
SASL_VISIT_DEF_UNIMPL( case_label );
SASL_VISIT_DEF_UNIMPL( ident_label );
SASL_VISIT_DEF_UNIMPL( switch_statement );

SASL_VISIT_DEF( compound_statement )
{
	shared_ptr<compound_statement> dup_stmt = duplicate(v.handle())->typed_handle<compound_statement>();
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
		dup_stmt->stmts.push_back(child_gen);
	}

	get_or_create_semantic_info<statement_si>(dup_stmt);
	data_cptr()->generated_node = dup_stmt->handle();
}

SASL_VISIT_DEF( expression_statement ){
	
	shared_ptr<expression_statement> dup_exprstmt = duplicate( v.handle() )->typed_handle<expression_statement>();
	
	any child_ctxt_init = *data;
	ctxt_ptr(child_ctxt_init)->generated_node.reset();

	any child_ctxt;
	visit_child( child_ctxt, child_ctxt_init, v.expr, dup_exprstmt->expr );

	get_or_create_semantic_info<statement_si>(dup_exprstmt);

	data_cptr()->generated_node = dup_exprstmt->handle();

}

SASL_VISIT_DEF( jump_statement )
{
	shared_ptr<jump_statement> dup_jump = duplicate(v.handle())->typed_handle<jump_statement>();

	any child_ctxt_init = *data;
	ctxt_ptr( child_ctxt_init )->generated_node.reset();

	if (v.code == jump_mode::_return){
		if( v.jump_expr ){
			any child_ctxt;
			visit_child( child_ctxt, child_ctxt_init, v.jump_expr, dup_jump->jump_expr );
		}
	}

	data_cptr()->generated_node = dup_jump;
}

// program
SASL_VISIT_DEF( program ){
	data = data;

	// create semantic info
	msi.reset( new module_si() );

	any child_ctxt_init = sacontext();
	ctxt_ptr(child_ctxt_init)->parent_sym = msi->root();

	any child_ctxt = child_ctxt_init;

	register_builtin_types();
	register_type_converter( child_ctxt_init );
	register_builtin_functions( child_ctxt_init );

	shared_ptr<program> dup_prog = duplicate( v.handle() )->typed_handle<program>();
	dup_prog->decls.clear();

	// analysis decalarations.
	for( vector< shared_ptr<declaration> >::iterator it = v.decls.begin(); it != v.decls.end(); ++it ){
		shared_ptr<declaration> node_gen;
		visit_child( child_ctxt, child_ctxt_init, (*it), node_gen );
		dup_prog->decls.push_back( node_gen );
	}

	msi->root()->relink( dup_prog->handle() );
}

SASL_VISIT_DEF_UNIMPL( for_statement );

void semantic_analyser::builtin_type_convert( shared_ptr<node> lhs, shared_ptr<node> rhs ){
	// do nothing
}

void semantic_analyser::register_type_converter( const boost::any& /*ctxt*/ ){
	// register default type converter
	type_manager* typemgr = msi->type_manager().get();

	type_entry::id_t sint8_ts = typemgr->get( builtin_type_code::_sint8 );
	type_entry::id_t sint16_ts = typemgr->get( builtin_type_code::_sint16 );
	type_entry::id_t sint32_ts = typemgr->get( builtin_type_code::_sint32 );
	type_entry::id_t sint64_ts = typemgr->get( builtin_type_code::_sint64 );

	type_entry::id_t uint8_ts = typemgr->get( builtin_type_code::_uint8 );
	type_entry::id_t uint16_ts = typemgr->get( builtin_type_code::_uint16 );
	type_entry::id_t uint32_ts = typemgr->get( builtin_type_code::_uint32 );
	type_entry::id_t uint64_ts = typemgr->get( builtin_type_code::_uint64 );

	type_entry::id_t float_ts = typemgr->get( builtin_type_code::_float );
	type_entry::id_t double_ts = typemgr->get( builtin_type_code::_double );

	type_entry::id_t bool_ts = typemgr->get( builtin_type_code::_boolean );

	// default conversation will do nothing.
	type_converter::converter_t default_conv = bind(&semantic_analyser::builtin_type_convert, this, _1, _2);

	typeconv->register_converter( type_converter::implicit_conv, sint8_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint8_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint8_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint8_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint8_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint8_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint8_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint8_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint8_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint8_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, sint16_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint16_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint16_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint16_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint16_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint16_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint16_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint16_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint16_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint16_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, sint32_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint32_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint32_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint32_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint32_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint32_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint32_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, sint32_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint32_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint32_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, sint64_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint64_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint64_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint64_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint64_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint64_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, sint64_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, sint64_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, sint64_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, sint64_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, uint8_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint8_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, uint16_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint16_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint16_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint16_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint16_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint16_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint16_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint16_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint16_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint16_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, uint32_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint32_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint32_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::better_conv, uint32_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint32_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint32_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint32_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, uint32_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint32_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint32_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, uint64_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint64_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint64_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint64_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint64_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint64_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, uint64_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, uint64_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, uint64_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, uint64_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, float_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, float_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, float_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, float_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, float_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, float_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, float_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, float_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::implicit_conv, float_ts, double_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, float_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, double_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, double_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, double_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, double_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::warning_conv, double_ts, bool_ts, default_conv );

	typeconv->register_converter( type_converter::explicit_conv, bool_ts, sint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, sint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, sint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, sint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, uint8_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, uint16_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, uint32_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, uint64_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, float_ts, default_conv );
	typeconv->register_converter( type_converter::explicit_conv, bool_ts, double_ts, default_conv );

}

void semantic_analyser::register_builtin_functions( const boost::any& child_ctxt_init ){
	any child_ctxt;

	typedef unordered_map<
		builtin_type_code, shared_ptr<builtin_type>, enum_hasher
		> bt_table_t;
	bt_table_t standard_bttbl;
	bt_table_t storage_bttbl;
	map_of_builtin_type( standard_bttbl, &sasl_ehelper::is_standard );
	map_of_builtin_type( storage_bttbl, &sasl_ehelper::is_storagable );

	shared_ptr<builtin_type> bt_bool = storage_bttbl[ builtin_type_code::_boolean ];
	shared_ptr<builtin_type> bt_i32 = storage_bttbl[ builtin_type_code::_sint32 ];

	shared_ptr<function_type> tmpft;

	// arithmetic operators
	vector<std::string> op_tbl;
	const vector<operators>& oplist = sasl_ehelper::list_of_operators();

	for( size_t i_op = 0; i_op < oplist.size(); ++i_op ){
		operators op = oplist[i_op];
		std::string op_name( operator_name(op) );

		if ( sasl_ehelper::is_arithmetic(op) ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				dfunction_combinator(NULL).dname( op_name )
					.dreturntype().dnode( it_type->second ).end()
					.dparam().dtype().dnode( it_type->second ).end().end()
					.dparam().dtype().dnode( it_type->second ).end().end()
				.end( tmpft );

				if ( tmpft ){ visit_child( child_ctxt, child_ctxt_init, tmpft ); }
			}
		}

		if( sasl_ehelper::is_arith_assign(op) ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				dfunction_combinator(NULL).dname( op_name )
					.dreturntype().dnode( it_type->second ).end()
					.dparam().dtype().dnode( it_type->second ).end().end()
					.dparam().dtype().dnode( it_type->second ).end().end()
					.end( tmpft );
				if ( tmpft ){ visit_child( child_ctxt, child_ctxt_init, tmpft ); }
			}
		}

		if( sasl_ehelper::is_relationship(op) ){

			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				dfunction_combinator(NULL).dname( op_name )
					.dreturntype().dnode( bt_bool ).end()
					.dparam().dtype().dnode( it_type->second ).end().end()
					.dparam().dtype().dnode( it_type->second ).end().end()
					.end( tmpft );
				if ( tmpft ){ visit_child( child_ctxt, child_ctxt_init, tmpft ); }
			}
		}

		if( sasl_ehelper::is_bit(op) || sasl_ehelper::is_bit_assign(op) ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				if ( sasl_ehelper::is_integer(it_type->first) ){
					dfunction_combinator(NULL).dname( op_name )
						.dreturntype().dnode( it_type->second ).end()
						.dparam().dtype().dnode( it_type->second ).end().end()
						.dparam().dtype().dnode( it_type->second ).end().end()
					.end( tmpft );
					if ( tmpft ){ visit_child( child_ctxt, child_ctxt_init, tmpft ); }
				}
			}
		}

		if( sasl_ehelper::is_shift(op) || sasl_ehelper::is_shift_assign(op) ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				if ( sasl_ehelper::is_integer(it_type->first) ){
					dfunction_combinator(NULL).dname( op_name )
						.dreturntype().dnode( it_type->second ).end()
						.dparam().dtype().dnode( it_type->second ).end().end()
						.dparam().dtype().dnode( bt_i32 ).end().end()
						.end( tmpft );
					if ( tmpft ){ visit_child( child_ctxt, child_ctxt_init, tmpft ); }
				}
			}
		}

		if( sasl_ehelper::is_bool_arith(op) ){
			dfunction_combinator(NULL).dname( op_name )
				.dreturntype().dnode( bt_bool ).end()
				.dparam().dtype().dnode( bt_bool ).end().end()
				.dparam().dtype().dnode( bt_bool ).end().end()
			.end( tmpft );
			if ( tmpft ){ visit_child( child_ctxt, child_ctxt_init, tmpft ); }
		}

		if( sasl_ehelper::is_prefix(op) || sasl_ehelper::is_postfix(op) || op == operators::positive ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				if ( sasl_ehelper::is_integer(it_type->first) ){
					dfunction_combinator(NULL).dname( op_name )
						.dreturntype().dnode( it_type->second ).end()
						.dparam().dtype().dnode( it_type->second ).end().end()
						.end( tmpft );

					if ( tmpft ){ visit_child( child_ctxt, child_ctxt_init, tmpft ); }
				}
			}
		}

		if( op == operators::bit_not ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				if ( sasl_ehelper::is_integer(it_type->first) ){
					dfunction_combinator(NULL).dname( op_name )
						.dreturntype().dnode( it_type->second ).end()
						.dparam().dtype().dnode( it_type->second ).end().end()
						.end( tmpft );

					if ( tmpft ){ visit_child( child_ctxt, child_ctxt_init, tmpft ); }
				}
			}
		}

		if( op == operators::logic_not ){
			dfunction_combinator(NULL).dname( op_name )
				.dreturntype().dnode( bt_bool ).end()
				.dparam().dtype().dnode( bt_bool ).end().end()
			.end( tmpft );

			if ( tmpft ){ visit_child( child_ctxt, child_ctxt_init, tmpft ); }
		}

		if( op == operators::negative ){
			for( bt_table_t::iterator it_type = standard_bttbl.begin(); it_type != standard_bttbl.end(); ++it_type ){
				if ( it_type->first != builtin_type_code::_uint64 ){
					dfunction_combinator(NULL).dname( op_name )
						.dreturntype().dnode( it_type->second ).end()
						.dparam().dtype().dnode( it_type->second ).end().end()
					.end( tmpft );

					if ( tmpft ){ visit_child( child_ctxt, child_ctxt_init, tmpft ); }
				}
			}
		}

		if ( op == operators::assign ){
			for( bt_table_t::iterator it_type = storage_bttbl.begin(); it_type != storage_bttbl.end(); ++it_type ){
				dfunction_combinator(NULL).dname( op_name )
					.dreturntype().dnode( it_type->second ).end()
					.dparam().dtype().dnode( it_type->second ).end().end()
					.dparam().dtype().dnode( it_type->second ).end().end()
				.end( tmpft );

				if ( tmpft ){ visit_child( child_ctxt, child_ctxt_init, tmpft ); }
			}
		}
	}
}

void semantic_analyser::register_builtin_types(){
	BOOST_FOREACH( builtin_type_code const & btc, sasl_ehelper::list_of_builtin_type_codes() ){
		EFLIB_ASSERT( msi->type_manager()->get( btc ) > -1, "Register builtin type failed!" );
	}
}

boost::shared_ptr<module_si> const& semantic_analyser::module_semantic_info() const{
	return msi;
}

END_NS_SASL_SEMANTIC();
