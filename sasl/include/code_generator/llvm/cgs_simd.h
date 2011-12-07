#ifndef SASL_CODE_GENERATOR_CGLLVM_CGS_SIMD_H
#define SASL_CODE_GENERATOR_CGLLVM_CGS_SIMD_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/include/code_generator/llvm/cgllvm_service.h>

BEGIN_NS_SASL_CODE_GENERATOR();

class cgs_simd: public cg_service
{
protected:
	abis intrinsic_abi() const;

	virtual llvm::Value* load( value_t const& );
	virtual llvm::Value* load( value_t const& , abis abi );
	virtual llvm::Value* load_ref( value_t const& );
	virtual void store( value_t& lhs, value_t const& rhs );

	value_t emit_add( value_t const& lhs, value_t const& rhs );
	value_t emit_sub( value_t const& lhs, value_t const& rhs );
	value_t emit_mul( value_t const& lhs, value_t const& rhs );
	
	value_t emit_dot( value_t const& lhs, value_t const& rhs );
	value_t emit_sqrt( value_t const& lhs );
	value_t emit_cross( value_t const& lhs, value_t const& rhs );

	virtual value_t cast_ints( value_t const& v, value_tyinfo* dest_tyi );
	virtual value_t cast_i2f( value_t const& v, value_tyinfo* dest_tyi );
	virtual value_t cast_f2i( value_t const& v, value_tyinfo* dest_tyi );
	virtual value_t cast_f2f( value_t const& v, value_tyinfo* dest_tyi );
	virtual value_t cast_i2b( value_t const& v );
	virtual value_t cast_f2b( value_t const& v );

	void push_fn( function_t const& fn );
	void pop_fn();

	void set_insert_point( insert_point_t const& ip );
	insert_point_t insert_point() const;

	void emit_return();
	void emit_return( value_t const&, abis abi );
	
	virtual value_t create_vector( std::vector<value_t> const& scalars, abis abi );

	abis param_abi( bool c_compatible ) const;
};

END_NS_SASL_CODE_GENERATOR();
#endif