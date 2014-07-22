#ifndef SASL_CODEGEN_CG_SIMD_H
#define SASL_CODEGEN_CG_SIMD_H

#include <sasl/include/codegen/cg_impl.h>

#include <sasl/include/codegen/cgs_simd.h>
#include <sasl/enums/builtin_types.h>
#include <salviar/include/shader_reflection.h>

namespace sasl{
	namespace semantic{
		class module_semantic;
		class reflection_impl;
	}
}

namespace llvm{
	class StructType;
	class Type;
}

BEGIN_NS_SASL_CODEGEN();

// Code generation for SIMD( Single Instruction Multiple Data )
class cg_simd: public cg_impl
{

public:
	typedef cg_impl parent_class;

	cg_simd();
	~cg_simd();

	// expression
	SASL_VISIT_DCL( unary_expression );
	SASL_VISIT_DCL( cast_expression );
	SASL_VISIT_DCL( expression_list );
	SASL_VISIT_DCL( cond_expression );
	SASL_VISIT_DCL( index_expression );
	SASL_VISIT_DCL( member_expression );
	SASL_VISIT_DCL( variable_expression );

	// declaration & type specifier
	SASL_VISIT_DCL( initializer );
	SASL_VISIT_DCL( member_initializer );
	SASL_VISIT_DCL( declaration );
	SASL_VISIT_DCL( type_definition );
	SASL_VISIT_DCL( tynode );
	SASL_VISIT_DCL( array_type );
	SASL_VISIT_DCL( alias_type );

	// statement
	SASL_VISIT_DCL( statement );
	SASL_VISIT_DCL( if_statement );
	SASL_VISIT_DCL( while_statement );
	SASL_VISIT_DCL( dowhile_statement );
	SASL_VISIT_DCL( for_statement );
	SASL_VISIT_DCL( case_label );
	SASL_VISIT_DCL( ident_label );
	SASL_VISIT_DCL( switch_statement );
	SASL_VISIT_DCL( compound_statement );
	SASL_VISIT_DCL( labeled_statement );

protected:
	cgs_simd*	service() const;
	abis	local_abi( bool is_c_compatible ) const;

	multi_value	layout_to_value( salviar::sv_layout* svl );

	SASL_SPECIFIC_VISIT_DCL( before_decls_visit, program );

	SASL_SPECIFIC_VISIT_DCL( create_fnsig,			function_def );
	SASL_SPECIFIC_VISIT_DCL( create_fnargs,			function_def );
	SASL_SPECIFIC_VISIT_DCL( create_virtual_args,	function_def );

	SASL_SPECIFIC_VISIT_DCL( visit_return	, jump_statement );
	SASL_SPECIFIC_VISIT_DCL( visit_continue	, jump_statement );
	SASL_SPECIFIC_VISIT_DCL( visit_break	, jump_statement );

	SASL_SPECIFIC_VISIT_DCL( bin_logic, binary_expression );

	llvm::Function*	entry_fn;
	multi_value		entry_values[salviar::sv_usage_count];
};

END_NS_SASL_CODEGEN();

#endif
