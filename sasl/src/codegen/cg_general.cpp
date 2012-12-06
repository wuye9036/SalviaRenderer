#include <sasl/include/codegen/cg_general.h>

#include <sasl/enums/enums_utility.h>
#include <sasl/include/codegen/cg_contexts.h>
#include <sasl/include/codegen/cg_impl.imp.h>
#include <sasl/include/codegen/cg_module_impl.h>
#include <sasl/include/codegen/cg_caster.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/abi_analyser.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/semantic/caster.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/statement.h>
#include <sasl/include/syntax_tree/program.h>

#include <salviar/include/enums.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/utility/unref_declarator.h>
#include <eflib/include/platform/boost_begin.h>
#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

BEGIN_NS_SASL_CODEGEN();

using namespace syntax_tree;
using namespace semantic;
using namespace boost::assign;
using namespace llvm;
using namespace sasl::utility;

using semantic::abi_info;
using semantic::node_semantic;
using semantic::module_semantic;
using semantic::operator_name;
using semantic::symbol;
using semantic::caster_t;
using semantic::pety_item_t;
using semantic::type_equal;
using semantic::node_semantic;

using boost::shared_ptr;
using boost::any;
using boost::any_cast;

using std::vector;

#define is_node_class( handle_of_node, typecode ) ( (handle_of_node)->node_class() == node_ids::typecode )

//////////////////////////////////////////////////////////////////////////
//
#define SASL_VISITOR_TYPE_NAME cg_general

cg_general::cg_general()
{
	service_ = new cgs_sisd();
}

SASL_SPECIFIC_VISIT_DEF( before_decls_visit, program )
{
	parent_class::before_decls_visit(v, data);
}

SASL_VISIT_DEF_UNIMPL( expression_list );

SASL_VISIT_DEF_UNIMPL( identifier );

// declaration & type specifier
SASL_VISIT_DEF_UNIMPL( initializer );
SASL_VISIT_DEF_UNIMPL( member_initializer );
SASL_VISIT_DEF_UNIMPL( type_definition );
SASL_VISIT_DEF_UNIMPL( tynode );

SASL_VISIT_DEF_UNIMPL( alias_type );

SASL_SPECIFIC_VISIT_DEF(bin_logic, binary_expression)
{
	EFLIB_UNREF_DECLARATOR(data);

	multi_value ret_value;
	builtin_types bt = sem_->get_semantic(v.left_expr)->value_builtin_type();
	if( is_scalar(bt) )
	{
		if( v.op == operators::logic_or ){
			// return left ? left : right;
			ret_value = emit_short_cond(v.left_expr, v.left_expr, v.right_expr) ;
		} else {
			// return left ? right : left;
			ret_value = emit_short_cond(v.left_expr, v.right_expr, v.left_expr) ;
		}
	}
	else
	{
		ret_value = emit_logic_op(v.op, v.left_expr, v.right_expr);
	}

	node_ctxt(v,true)->node_value = ret_value.to_rvalue();
}

cg_module_impl* cg_general::mod_ptr(){
	return llvm_mod_.get();
}

END_NS_SASL_CODEGEN();
