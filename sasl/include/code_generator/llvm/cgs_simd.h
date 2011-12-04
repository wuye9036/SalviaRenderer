#ifndef SASL_CODE_GENERATOR_CGLLVM_CGS_SIMD_H
#define SASL_CODE_GENERATOR_CGLLVM_CGS_SIMD_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/include/code_generator/llvm/cgllvm_service.h>

BEGIN_NS_SASL_CODE_GENERATOR();

class cgs_simd: public cg_service
{
	virtual llvm::Value* load( value_t const& );
	virtual llvm::Value* load( value_t const& , abis abi );
	virtual llvm::Value* load_ref( value_t const& );
	virtual void store( value_t& lhs, value_t const& rhs );

	virtual value_t cast_ints( value_t const& v, value_tyinfo* dest_tyi );
	virtual value_t cast_i2f( value_t const& v, value_tyinfo* dest_tyi );
	virtual value_t cast_f2i( value_t const& v, value_tyinfo* dest_tyi );
	virtual value_t cast_f2f( value_t const& v, value_tyinfo* dest_tyi );
	virtual value_t cast_i2b( value_t const& v );
	virtual value_t cast_f2b( value_t const& v );

	virtual value_t create_vector( std::vector<value_t> const& scalars, abis abi );
};

END_NS_SASL_CODE_GENERATOR();
#endif