#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_VS_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_VS_H

#include <sasl/include/code_generator/llvm/cgllvm_sisd.h>

BEGIN_NS_SASL_CODE_GENERATOR();

class cgllvm_vs: public cgllvm_sisd
{
public:
	cgllvm_vs();
	bool generate( sasl::semantic::module_si* mod, sasl::semantic::abi_info const* abii );

	// expressions
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
	SASL_VISIT_DCL( identifier );

	// declaration & type specifier
	SASL_VISIT_DCL( initializer );
	SASL_VISIT_DCL( expression_initializer );
	SASL_VISIT_DCL( member_initializer );
	SASL_VISIT_DCL( declaration );
	SASL_VISIT_DCL( declarator );
	SASL_VISIT_DCL( variable_declaration );
	SASL_VISIT_DCL( type_definition );
	SASL_VISIT_DCL( type_specifier );
	SASL_VISIT_DCL( builtin_type );
	SASL_VISIT_DCL( array_type );
	SASL_VISIT_DCL( struct_type );
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

	// program
	SASL_VISIT_DCL( program );

private:
	void create_entry();
	void create_entry_param();
};

END_NS_SASL_CODE_GENERATOR();

#endif