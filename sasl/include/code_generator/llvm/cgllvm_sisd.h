#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_SISD_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_SISD_H

#include <sasl/include/code_generator/llvm/cgllvm_impl.h>

#include <sasl/include/code_generator/llvm/cgllvm_service.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/any.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <eflib/include/platform/typedefs.h>

#include <vector>

namespace sasl{
	namespace semantic{
		class caster_t;
	}
	namespace syntax_tree{
		struct tynode;
		struct node;	
	}
}

namespace llvm{
	class BasicBlock;
	class Constant;
	class ConstantInt;
	class ConstantFP;
	class Function;
	class Value;
}

BEGIN_NS_SASL_CODE_GENERATOR();

struct cgllvm_sctxt_data;
struct cgllvm_sctxt_env;

// Code generation for SISD( Single Instruction Single Data )
class cgllvm_sisd: public cgllvm_impl, public cgs_sisd{

public:
	~cgllvm_sisd();

	SASL_VISIT_DCL( binary_expression );
	SASL_VISIT_DCL( member_expression );
	SASL_VISIT_DCL( variable_expression );
	SASL_VISIT_DCL( constant_expression );
	SASL_VISIT_DCL( cond_expression );
	SASL_VISIT_DCL( unary_expression );
	SASL_VISIT_DCL( call_expression );

	SASL_VISIT_DCL( expression_initializer );

	SASL_VISIT_DCL( builtin_type );
	SASL_VISIT_DCL( function_type );
	SASL_VISIT_DCL( struct_type );

	SASL_VISIT_DCL( declarator );
	SASL_VISIT_DCL( variable_declaration );
	SASL_VISIT_DCL( parameter );

	SASL_VISIT_DCL( statement );
	SASL_VISIT_DCL( declaration_statement );
	SASL_VISIT_DCL( compound_statement );
	SASL_VISIT_DCL( jump_statement );
	SASL_VISIT_DCL( expression_statement );
	SASL_VISIT_DCL( for_statement );
	SASL_VISIT_DCL( if_statement );
	SASL_VISIT_DCL( while_statement );
	SASL_VISIT_DCL( dowhile_statement );
	SASL_VISIT_DCL( case_label );
	SASL_VISIT_DCL( ident_label );
	SASL_VISIT_DCL( switch_statement );
	SASL_VISIT_DCL( labeled_statement );
protected:
	cg_service* service() const;

	SASL_SPECIFIC_VISIT_DCL( process_intrinsics, program );

	// It is called in program visitor BEFORE declaration was visited.
	// If any additional initialization you want to add before visit, override it.
	// DONT FORGET call parent function before your code.
	SASL_SPECIFIC_VISIT_DCL( before_decls_visit, program );

	// Called by function_type visitor.
	SASL_SPECIFIC_VISIT_DCL( create_fnsig, function_type );
	SASL_SPECIFIC_VISIT_DCL( create_fnargs, function_type );
	SASL_SPECIFIC_VISIT_DCL( create_fnbody, function_type );

	SASL_SPECIFIC_VISIT_DCL( return_statement, jump_statement );

	SASL_SPECIFIC_VISIT_DCL( visit_member_declarator, declarator );
	SASL_SPECIFIC_VISIT_DCL( visit_global_declarator, declarator );
	SASL_SPECIFIC_VISIT_DCL( visit_local_declarator, declarator );

	SASL_SPECIFIC_VISIT_DCL( bin_assign, binary_expression );
	SASL_SPECIFIC_VISIT_DCL( bin_logic, binary_expression );

	/// Create short-circuit evaluation for condition expression. And logic operators will use it.
	value_t emit_short_cond( 
		boost::any const& ctxt_init,
		boost::shared_ptr<sasl::syntax_tree::node> const& cond,
		boost::shared_ptr<sasl::syntax_tree::node> const& yes,
		boost::shared_ptr<sasl::syntax_tree::node> const& no
		);

	// Override node_ctxt of cgllvm_impl
	template <typename NodeT >
	cgllvm_sctxt* node_ctxt( boost::shared_ptr<NodeT> const& v, bool create_if_need = false ){
		return cgllvm_impl::node_ctxt<NodeT, cgllvm_sctxt>(v, create_if_need);
	}
	//cgllvm_sctxt* node_ctxt( boost::shared_ptr<sasl::syntax_tree::node> const& v, bool create_if_need = false ){
	//	return cgllvm_impl::node_ctxt<cgllvm_sctxt>(v, create_if_need);
	//}
	cgllvm_sctxt* node_ctxt( sasl::syntax_tree::node&, bool create_if_need = false );

	void mask_to_indexes( char index[4], uint32_t mask );

	llvm_module_impl* mod_ptr();
};

cgllvm_sctxt const * sc_ptr( const boost::any& any_val  );
cgllvm_sctxt* sc_ptr( boost::any& any_val );

cgllvm_sctxt const * sc_ptr( const boost::any* any_val  );
cgllvm_sctxt* sc_ptr( boost::any* any_val );

cgllvm_sctxt_data* sc_data_ptr( boost::any* any_val );
cgllvm_sctxt_data const* sc_data_ptr( boost::any const* any_val );

cgllvm_sctxt_env* sc_env_ptr( boost::any* any_val );
cgllvm_sctxt_env const* sc_env_ptr( boost::any const* any_val );

END_NS_SASL_CODE_GENERATOR();

#endif