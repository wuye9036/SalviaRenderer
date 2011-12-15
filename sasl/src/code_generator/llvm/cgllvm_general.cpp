#include <sasl/include/code_generator/llvm/cgllvm_general.h>

#include <sasl/enums/enums_utility.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/code_generator/llvm/cgllvm_impl.imp.h>
#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/code_generator/llvm/cgllvm_caster.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/abi_analyser.h>
#include <sasl/include/semantic/semantic_infos.imp.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/semantic/caster.h>
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
using semantic::caster_t;
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

SASL_SPECIFIC_VISIT_DEF( before_decls_visit, program )
{
}

SASL_VISIT_DEF( cast_expression ){
	EFLIB_ASSERT_UNIMPLEMENTED();
	//any child_ctxt_init = *data;
	//any child_ctxt;

	//visit_child( child_ctxt, child_ctxt_init, v.casted_type );
	//visit_child( child_ctxt, child_ctxt_init, v.expr );

	//shared_ptr<type_info_si> src_tsi = extract_semantic_info<type_info_si>( v.expr );
	//shared_ptr<type_info_si> casted_tsi = extract_semantic_info<type_info_si>( v.casted_type );

	//if( src_tsi->entry_id() != casted_tsi->entry_id() ){
	//	if( caster->try_cast( casted_tsi->entry_id(), src_tsi->entry_id() ) == caster_t::nocast ){
	//		// Here is code error. Compiler should report it.
	//		EFLIB_ASSERT_UNIMPLEMENTED();
	//	}
	//	node_ctxt(v, true)->data().val_type = node_ctxt(v.casted_type)->data().val_type;
	//	caster->convert( v.as_handle(), v.expr );
	//}

	//cgllvm_sctxt* vctxt = node_ctxt(v, false);
	//sc_data_ptr(data)->val_type = vctxt->data().val_type;
	//sc_data_ptr(data)->val = load( vctxt );
}

SASL_VISIT_DEF_UNIMPL( expression_list );
SASL_VISIT_DEF_UNIMPL( index_expression );

SASL_VISIT_DEF_UNIMPL( identifier );

// declaration & type specifier
SASL_VISIT_DEF_UNIMPL( initializer );
SASL_VISIT_DEF_UNIMPL( member_initializer );
SASL_VISIT_DEF_UNIMPL( type_definition );
SASL_VISIT_DEF_UNIMPL( tynode );

SASL_VISIT_DEF_UNIMPL( array_type );
SASL_VISIT_DEF_UNIMPL( alias_type );

llvm_module_impl* cgllvm_general::mod_ptr(){
	assert( dynamic_cast<llvm_module_impl*>( mod.get() ) );
	return static_cast<llvm_module_impl*>( mod.get() );
}

END_NS_SASL_CODE_GENERATOR();
