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

namespace salviar{
	struct sv_layout;
}

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

	SASL_VISIT_DCL( member_expression );
	SASL_VISIT_DCL( cond_expression );
	SASL_VISIT_DCL( unary_expression );
	SASL_VISIT_DCL( call_expression );

	SASL_VISIT_DCL( expression_initializer );

	SASL_VISIT_DCL( statement );
	SASL_VISIT_DCL( compound_statement );
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
	abis		local_abi( bool is_c_compatible ) const;

	// Called by function_type visitor.
	SASL_SPECIFIC_VISIT_DCL( visit_continue	, jump_statement );
	SASL_SPECIFIC_VISIT_DCL( visit_break	, jump_statement );

	SASL_SPECIFIC_VISIT_DCL( bin_logic	, binary_expression );

	/// Create short-circuit evaluation for condition expression. And logic operators will use it.
	value_t emit_short_cond( 
		boost::any const& ctxt_init,
		boost::shared_ptr<sasl::syntax_tree::node> const& cond,
		boost::shared_ptr<sasl::syntax_tree::node> const& yes,
		boost::shared_ptr<sasl::syntax_tree::node> const& no
		);

	// Override node_ctxt of cgllvm_impl
	
	void	mask_to_indexes( char index[4], uint32_t mask );
	value_t	layout_to_value( salviar::sv_layout* svl );

	cgllvm_sctxt* node_ctxt( sasl::syntax_tree::node&, bool create_if_need = false );
	template <typename NodeT >
	cgllvm_sctxt* node_ctxt( boost::shared_ptr<NodeT> const& v, bool create_if_need = false ){
		return cgllvm_impl::node_ctxt<NodeT>(v, create_if_need);
	}
	llvm_module_impl* mod_ptr();
};

END_NS_SASL_CODE_GENERATOR();

#endif