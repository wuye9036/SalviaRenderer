#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_VS_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_VS_H

#include <sasl/include/code_generator/llvm/cgllvm_sisd.h>
#include <sasl/include/semantic/abi_info.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/utility/value_init.hpp>
#include <eflib/include/platform/boost_end.h>

namespace sasl{
	namespace semantic{
		class type_converter;
		class module_si;
	}
	namespace syntax_tree{
		struct expression;
		struct type_specifier;
		struct node;
	}
}

namespace llvm{
	class StructType;
}

BEGIN_NS_SASL_CODE_GENERATOR();

class cgllvm_modvs;

class cgllvm_vs: public cgllvm_sisd
{
public:
	typedef cgllvm_sisd parent_class;

	cgllvm_vs();

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

private:
	SASL_SPECIFIC_VISIT_DCL( before_decls_visit, program );
	SASL_SPECIFIC_VISIT_DCL( create_entry, function_type );

	virtual bool create_mod( sasl::syntax_tree::program& v );
	cgllvm_modvs* mod_ptr();

	
	void create_entry_params();

	void fill_llvm_type_from_si( sasl::semantic::storage_types st );

	std::vector< llvm::Type const* > entry_params_types[sasl::semantic::storage_types_count];
	boost::value_initialized<llvm::StructType*> entry_params_structs[sasl::semantic::storage_types_count];
};

END_NS_SASL_CODE_GENERATOR();

#endif