#include <sasl/include/code_generator/llvm/cgllvm_general.h>

#include <sasl/enums/enums_utility.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>
#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/code_generator/llvm/cgllvm_type_converters.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/abi_analyser.h>
#include <sasl/include/semantic/semantic_infos.imp.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/semantic/tecov.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/program.h>

#include <salviar/include/enums.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/metaprog/util.h>
#include <eflib/include/platform/boost_begin.h>
#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

BEGIN_NS_SASL_CODE_GENERATOR();

using namespace syntax_tree;
using namespace semantic;
using namespace boost::assign;
using namespace llvm;
using namespace sasl::utility;

using semantic::abi_info;
using semantic::const_value_si;
using semantic::extract_semantic_info;
using semantic::module_si;
using semantic::storage_si;
using semantic::operator_name;
using semantic::statement_si;
using semantic::symbol;
using semantic::tecov_t;
using semantic::pety_item_t;
using semantic::type_equal;
using semantic::type_info_si;

using boost::shared_ptr;
using boost::any;
using boost::any_cast;

using std::vector;

typedef cgllvm_sctxt* sctxt_handle;

#define is_node_class( handle_of_node, typecode ) ( (handle_of_node)->node_class() == node_ids::typecode )

//////////////////////////////////////////////////////////////////////////
//
#define SASL_VISITOR_TYPE_NAME cgllvm_general

cgllvm_general::cgllvm_general()
{}

SASL_VISIT_DEF_UNIMPL( unary_expression );
SASL_VISIT_DEF( cast_expression ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	//any child_ctxt_init = *data;
	//any child_ctxt;

	//visit_child( child_ctxt, child_ctxt_init, v.casted_type );
	//visit_child( child_ctxt, child_ctxt_init, v.expr );

	//shared_ptr<type_info_si> src_tsi = extract_semantic_info<type_info_si>( v.expr );
	//shared_ptr<type_info_si> casted_tsi = extract_semantic_info<type_info_si>( v.casted_type );

	//if( src_tsi->entry_id() != casted_tsi->entry_id() ){
	//	if( typeconv->convertible( casted_tsi->entry_id(), src_tsi->entry_id() ) == tecov_t::cannot_conv ){
	//		// Here is code error. Compiler should report it.
	//		EFLIB_ASSERT_UNIMPLEMENTED();
	//	}
	//	node_ctxt(v, true)->data().val_type = node_ctxt(v.casted_type)->data().val_type;
	//	typeconv->convert( v.as_handle(), v.expr );
	//}

	//cgllvm_sctxt* vctxt = node_ctxt(v, false);
	//sc_data_ptr(data)->val_type = vctxt->data().val_type;
	//sc_data_ptr(data)->val = load( vctxt );
}

SASL_VISIT_DEF_UNIMPL( expression_list );
SASL_VISIT_DEF_UNIMPL( cond_expression );
SASL_VISIT_DEF_UNIMPL( index_expression );

SASL_VISIT_DEF_UNIMPL( identifier );

// declaration & type specifier
SASL_VISIT_DEF_UNIMPL( initializer );
SASL_VISIT_DEF( expression_initializer ){
	any child_ctxt_init = *data;
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.init_expr );

	shared_ptr<type_info_si> init_tsi = extract_semantic_info<type_info_si>(v.as_handle());
	shared_ptr<type_info_si> var_tsi = extract_semantic_info<type_info_si>(sc_env_ptr(data)->variable_to_fill.lock());

	if( init_tsi->entry_id() != var_tsi->entry_id() ){
		typeconv->convert( var_tsi->type_info(), v.init_expr );
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	//	sc_ptr(data)->storage_and_type( sc_ptr(child_ctxt) );
	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF_UNIMPL( member_initializer );
SASL_VISIT_DEF_UNIMPL( type_definition );
SASL_VISIT_DEF_UNIMPL( tynode );


SASL_VISIT_DEF_UNIMPL( array_type );
SASL_VISIT_DEF_UNIMPL( alias_type );
SASL_VISIT_DEF( parameter ){
	parent_class::visit(v, data);
}

SASL_VISIT_DEF( function_type ){
	parent_class::visit(v, data);
}

// statement
SASL_VISIT_DEF_UNIMPL( statement );

SASL_VISIT_DEF( if_statement ){
	any child_ctxt_init = *data;
	any child_ctxt;

	visit_child( child_ctxt, child_ctxt_init, v.cond );
	tid_t cond_tid = extract_semantic_info<type_info_si>(v.cond)->entry_id();
	tid_t bool_tid = msi->pety()->get( builtin_types::_boolean );
	if( cond_tid != bool_tid ){
		typeconv->convert( msi->pety()->get(bool_tid), v.cond );
	}
	
	insert_point_t ip_cond = insert_point();

	insert_point_t ip_yes = new_block( v.yes_stmt->symbol()->mangled_name(), true );
	visit_child( child_ctxt, child_ctxt_init, v.yes_stmt );

	insert_point_t ip_no;
	if( v.no_stmt ){
		ip_no = new_block( v.no_stmt->symbol()->mangled_name(), true );
		visit_child( child_ctxt, child_ctxt_init, v.no_stmt );
		
	}

	insert_point_t ip_merge = new_block( extract_semantic_info<statement_si>(v)->exit_point(), false );

	// Fill back.
	set_insert_point( ip_cond );
	value_t cond_value = node_ctxt( v.cond, false )->get_value();
	jump_cond( cond_value, ip_yes, ip_no ? ip_no : ip_merge );

	set_insert_point( ip_yes );
	jump_to( ip_merge );

	if( ip_no ){
		set_insert_point( ip_no );
		jump_to(ip_merge);
	}

	set_insert_point( ip_merge );

	node_ctxt(v, true)->copy( sc_ptr(data) );
}

SASL_VISIT_DEF_UNIMPL( while_statement );
SASL_VISIT_DEF_UNIMPL( dowhile_statement );
SASL_VISIT_DEF_UNIMPL( case_label );
SASL_VISIT_DEF_UNIMPL( switch_statement );

SASL_VISIT_DEF_UNIMPL( ident_label );

SASL_VISIT_DEF_UNIMPL( for_statement );

cgllvm_modimpl* cgllvm_general::mod_ptr(){
	assert( dynamic_cast<cgllvm_modimpl*>( mod.get() ) );
	return static_cast<cgllvm_modimpl*>( mod.get() );
}

bool cgllvm_general::create_mod( sasl::syntax_tree::program& v ){
	if ( mod ){ return false; }
	mod = create_codegen_context<cgllvm_modimpl>( v.as_handle() );
	return true;
}

END_NS_SASL_CODE_GENERATOR();
