#ifndef SASL_CODEGEN_CG_SISD_H
#define SASL_CODEGEN_CG_SISD_H

#include <sasl/codegen/cg_impl.h>

#include <sasl/codegen/cgs.h>
#include <sasl/enums/operators.h>

#include <eflib/platform/typedefs.h>

#include <memory>
#include <any>
#include <functional>
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

namespace sasl::codegen {

// Code generation for SISD( Single Instruction Single Data )
class cg_sisd: public cg_impl
{
public:
	~cg_sisd();

	SASL_VISIT_DCL( member_expression );
	SASL_VISIT_DCL( cond_expression );
	SASL_VISIT_DCL( unary_expression );

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
	virtual cgs_sisd*	service() const;
	abis	local_abi( bool is_c_compatible ) const;

	// Called by function_full_def visitor.
	SASL_SPECIFIC_VISIT_DCL( visit_continue	, jump_statement );
	SASL_SPECIFIC_VISIT_DCL( visit_break	, jump_statement );
	SASL_SPECIFIC_VISIT_DCL( bin_logic	, binary_expression ) = 0;
	
	multi_value emit_logic_op(
		operators op,
		std::shared_ptr<sasl::syntax_tree::node> const& left,
		std::shared_ptr<sasl::syntax_tree::node> const& right
		);

	/// Create short-circuit evaluation for condition expression. And logic operators will use it.
	multi_value emit_short_cond(
		std::shared_ptr<sasl::syntax_tree::node> const& cond,
		std::shared_ptr<sasl::syntax_tree::node> const& yes,
		std::shared_ptr<sasl::syntax_tree::node> const& no
		);

	// Override node_ctxt of cg_impl
	
	void	mask_to_indexes( char index[4], uint32_t mask );
	module_vmcode_impl* mod_ptr();
};

}

#endif