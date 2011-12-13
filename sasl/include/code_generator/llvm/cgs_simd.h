#ifndef SASL_CODE_GENERATOR_CGLLVM_CGS_SIMD_H
#define SASL_CODE_GENERATOR_CGLLVM_CGS_SIMD_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/include/code_generator/llvm/cgllvm_service.h>

BEGIN_NS_SASL_CODE_GENERATOR();

class cgs_simd: public cg_service
{
protected:
	abis intrinsic_abi() const;

	virtual void store( value_t& lhs, value_t const& rhs );

	value_t emit_add( value_t const& lhs, value_t const& rhs );
	value_t emit_sub( value_t const& lhs, value_t const& rhs );
	value_t emit_mul( value_t const& lhs, value_t const& rhs );
	
	value_t emit_cmp_lt( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_le( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_eq( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_ne( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_ge( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_gt( value_t const& lhs, value_t const& rhs );

	value_t emit_dot( value_t const& lhs, value_t const& rhs );
	value_t emit_sqrt( value_t const& lhs );
	value_t emit_cross( value_t const& lhs, value_t const& rhs );

	value_t emit_extract_ref( value_t const& lhs, int idx );
	value_t emit_extract_ref( value_t const& lhs, value_t const& idx );
	value_t emit_extract_val( value_t const& lhs, int idx );
	value_t emit_extract_val( value_t const& lhs, value_t const& idx );
	value_t emit_extract_elem_mask( value_t const& vec, uint32_t mask );

	virtual value_t cast_ints( value_t const& v, value_tyinfo* dest_tyi );
	virtual value_t cast_i2f( value_t const& v, value_tyinfo* dest_tyi );
	virtual value_t cast_f2i( value_t const& v, value_tyinfo* dest_tyi );
	virtual value_t cast_f2f( value_t const& v, value_tyinfo* dest_tyi );
	virtual value_t cast_i2b( value_t const& v );
	virtual value_t cast_f2b( value_t const& v );

	void emit_return();
	void emit_return( value_t const&, abis abi );
	
	virtual value_t create_scalar( llvm::Value*, value_tyinfo* );
	virtual value_t create_vector( std::vector<value_t> const& scalars, abis abi );

	abis param_abi( bool c_compatible ) const;
};

END_NS_SASL_CODE_GENERATOR();
#endif