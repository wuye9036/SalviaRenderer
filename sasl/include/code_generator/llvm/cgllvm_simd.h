#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_SIMD_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_SIMD_H

#include <sasl/include/code_generator/llvm/cgllvm_impl.h>

#include <sasl/include/code_generator/llvm/cgs_simd.h>
#include <sasl/enums/builtin_types.h>
#include <salviar/include/shader_abi.h>

namespace sasl{
	namespace semantic{
		class module_si;
		class abi_info;
	}
}

namespace llvm{
	class StructType;
	class Type;
}

int const SIMD_WIDE = 4;
int const PACKAGE_WIDE = 16;

BEGIN_NS_SASL_CODE_GENERATOR();

// Code generation for SIMD( Single Instruction Multiple Data )
class cgllvm_simd: public cgllvm_impl, public cgs_simd{

public:
	cgllvm_simd();
	~cgllvm_simd();

	// expression
	SASL_VISIT_DCL( unary_expression );
	SASL_VISIT_DCL( cast_expression );
	SASL_VISIT_DCL( binary_expression );
	SASL_VISIT_DCL( expression_list );
	SASL_VISIT_DCL( cond_expression );
	SASL_VISIT_DCL( index_expression );
	SASL_VISIT_DCL( call_expression );
	SASL_VISIT_DCL( member_expression );
	SASL_VISIT_DCL( constant_expression );
	SASL_VISIT_DCL( variable_expression );

	// declaration & type specifier
	SASL_VISIT_DCL( initializer );
	SASL_VISIT_DCL( expression_initializer );
	SASL_VISIT_DCL( member_initializer );
	SASL_VISIT_DCL( declaration );
	SASL_VISIT_DCL( declarator );
	SASL_VISIT_DCL( variable_declaration );
	SASL_VISIT_DCL( type_definition );
	SASL_VISIT_DCL( tynode );
	SASL_VISIT_DCL( builtin_type );
	SASL_VISIT_DCL( array_type );
	SASL_VISIT_DCL( struct_type );
	SASL_VISIT_DCL( alias_type );
	SASL_VISIT_DCL( parameter );
	SASL_VISIT_DCL( function_type );

	// statement
	SASL_VISIT_DCL( statement );
	SASL_VISIT_DCL( declaration_statement );
	SASL_VISIT_DCL( if_statement );
	SASL_VISIT_DCL( while_statement );
	SASL_VISIT_DCL( dowhile_statement );
	SASL_VISIT_DCL( for_statement );
	SASL_VISIT_DCL( case_label );
	SASL_VISIT_DCL( ident_label );
	SASL_VISIT_DCL( switch_statement );
	SASL_VISIT_DCL( compound_statement );
	SASL_VISIT_DCL( expression_statement );
	SASL_VISIT_DCL( jump_statement );
	SASL_VISIT_DCL( labeled_statement );

protected:
	cg_service* service() const;
	
	void create_entries();
	void create_entry_param( salviar::sv_usage usage );

	SASL_SPECIFIC_VISIT_DCL( before_decls_visit, program );

	std::vector<llvm::Type*> entry_tys[salviar::storage_usage_count];
	llvm::StructType* entry_structs[salviar::storage_usage_count];
	std::vector<builtin_types> entry_tyns[salviar::storage_usage_count];
};

END_NS_SASL_CODE_GENERATOR();

#endif
