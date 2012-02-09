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
	
	value_t emit_cmp_lt( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_le( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_eq( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_ne( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_ge( value_t const& lhs, value_t const& rhs );
	value_t emit_cmp_gt( value_t const& lhs, value_t const& rhs );

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
	bool prefer_externals() const{ return false; }
	bool prefer_scalar_code() const{ return false; }

	virtual void function_beg();
	virtual void function_end();

	virtual void for_init_beg(){}
	virtual void for_init_end(){}
	virtual void for_cond_beg(){}
	virtual void for_cond_end(){}
	virtual void for_body_beg(){}
	virtual void for_body_end(){}
	virtual void for_iter_beg(){}
	virtual void for_iter_end(){}

	virtual void if_cond_beg(){}
	virtual void if_cond_end(){}
	virtual void then_beg(){}
	virtual void then_end(){}
	virtual void else_beg(){}
	virtual void else_end(){}

	virtual void switch_cond_beg(){}
	virtual void switch_cond_end(){}
	virtual void switch_expr_beg(){}
	virtual void switch_expr_end(){}

	virtual void while_cond_beg(){}
	virtual void while_cond_end(){}
	virtual void while_body_beg(){}
	virtual void while_body_end(){}

	virtual void do_body_beg(){}
	virtual void do_body_end(){}
	virtual void do_cond_beg(){}
	virtual void do_cond_end(){}

	virtual void break_(){}
	virtual void continue_(){}

private:
	value_t			all_one_mask();
	llvm::Value*	expanded_mask( uint32_t expanded_times );

	std::vector<value_t> exec_masks;
};

END_NS_SASL_CODE_GENERATOR();
#endif