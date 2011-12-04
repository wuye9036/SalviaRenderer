#include <sasl/include/code_generator/llvm/cgllvm_simd.h>

#include <sasl/include/semantic/abi_info.h>
#include <sasl/include/semantic/semantic_infos.h>

#define SASL_VISITOR_TYPE_NAME cgllvm_simd

BEGIN_NS_SASL_CODE_GENERATOR();

cgllvm_simd::~cgllvm_simd(){}

cg_service* cgllvm_simd::service() const{
	return const_cast<cgllvm_simd*>(this);
}

SASL_VISIT_DEF_UNIMPL( unary_expression );
SASL_VISIT_DEF_UNIMPL( cast_expression );
SASL_VISIT_DEF_UNIMPL( binary_expression );
SASL_VISIT_DEF_UNIMPL( expression_list );
SASL_VISIT_DEF_UNIMPL( cond_expression );
SASL_VISIT_DEF_UNIMPL( index_expression );
SASL_VISIT_DEF_UNIMPL( call_expression );
SASL_VISIT_DEF_UNIMPL( member_expression );
SASL_VISIT_DEF_UNIMPL( constant_expression );
SASL_VISIT_DEF_UNIMPL( variable_expression );

// declaration & type specifier
SASL_VISIT_DEF_UNIMPL( initializer );
SASL_VISIT_DEF_UNIMPL( expression_initializer );
SASL_VISIT_DEF_UNIMPL( member_initializer );
SASL_VISIT_DEF_UNIMPL( declaration );
SASL_VISIT_DEF_UNIMPL( declarator );
SASL_VISIT_DEF_UNIMPL( variable_declaration );
SASL_VISIT_DEF_UNIMPL( type_definition );
SASL_VISIT_DEF_UNIMPL( tynode );
SASL_VISIT_DEF_UNIMPL( builtin_type );
SASL_VISIT_DEF_UNIMPL( array_type );
SASL_VISIT_DEF_UNIMPL( struct_type );
SASL_VISIT_DEF_UNIMPL( alias_type );
SASL_VISIT_DEF_UNIMPL( parameter );
SASL_VISIT_DEF_UNIMPL( function_type );

// statement
SASL_VISIT_DEF_UNIMPL( statement );
SASL_VISIT_DEF_UNIMPL( declaration_statement );
SASL_VISIT_DEF_UNIMPL( if_statement );
SASL_VISIT_DEF_UNIMPL( while_statement );
SASL_VISIT_DEF_UNIMPL( dowhile_statement );
SASL_VISIT_DEF_UNIMPL( for_statement );
SASL_VISIT_DEF_UNIMPL( case_label );
SASL_VISIT_DEF_UNIMPL( ident_label );
SASL_VISIT_DEF_UNIMPL( switch_statement );
SASL_VISIT_DEF_UNIMPL( compound_statement );
SASL_VISIT_DEF_UNIMPL( expression_statement );
SASL_VISIT_DEF_UNIMPL( jump_statement );
SASL_VISIT_DEF_UNIMPL( labeled_statement );

SASL_SPECIFIC_VISIT_DEF( before_decls_visit, program )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

END_NS_SASL_CODE_GENERATOR();