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

	value_t emit_and(value_t const& lhs, value_t const& rhs);
	value_t emit_or (value_t const& lhs, value_t const& rhs);

	virtual value_t cast_ints( value_t const& v, value_tyinfo* dest_tyi );
	virtual value_t cast_i2f ( value_t const& v, value_tyinfo* dest_tyi );
	virtual value_t cast_f2i ( value_t const& v, value_tyinfo* dest_tyi );
	virtual value_t cast_f2f ( value_t const& v, value_tyinfo* dest_tyi );
	virtual value_t cast_i2b ( value_t const& v );
	virtual value_t cast_f2b ( value_t const& v );

	void emit_return();
	void emit_return( value_t const&, abis abi );
	
	virtual value_t emit_ddx( value_t const& v );
	virtual value_t emit_ddy( value_t const& v );
	
	value_t create_scalar( llvm::Value* val, value_tyinfo* tyinfo, builtin_types hint );
	virtual value_t create_vector( std::vector<value_t> const& scalars, abis abi );

	abis param_abi( bool c_compatible ) const;
	bool prefer_externals() const{ return false; }
	bool prefer_scalar_code() const{ return false; }

	virtual void function_beg();
	virtual void function_end();

	virtual void for_init_beg();
	virtual void for_init_end();
	virtual void for_cond_beg();
	virtual void for_cond_end( value_t const& );
	virtual void for_body_beg();
	virtual void for_body_end();
	virtual void for_iter_beg();
	virtual void for_iter_end();

	virtual value_t joinable();

	virtual void if_beg();
	virtual void if_end();
	virtual void if_cond_beg();
	virtual void if_cond_end( value_t const& );
	virtual void then_beg();
	virtual void then_end();
	virtual void else_beg();
	virtual void else_end();

	virtual void switch_cond_beg(){}
	virtual void switch_cond_end(){}
	virtual void switch_expr_beg(){}
	virtual void switch_expr_end(){}

	virtual void while_beg();
	virtual void while_end();
	virtual void while_cond_beg();
	virtual void while_cond_end( value_t const& );
	virtual void while_body_beg();
	virtual void while_body_end();

	virtual void do_beg();
	virtual void do_end();
	virtual void do_body_beg();
	virtual void do_body_end();
	virtual void do_cond_beg();
	virtual void do_cond_end( value_t const& );

	virtual void break_();
	virtual void continue_();

private:
	llvm::Value*	load_loop_execution_mask();
	void			save_loop_execution_mask( llvm::Value* );
	virtual void	enter_loop();
	virtual void	exit_loop();
	virtual void	apply_loop_condition( value_t const& );
	virtual void	save_next_iteration_exec_mask();

	value_t			packed_mask();
	llvm::Value*	all_one_mask();
	llvm::Value*	all_zero_mask();
	llvm::Value*	expanded_mask( uint32_t expanded_times );

	enum slice_layout_mode{
		slm_horizontal,
		slm_vertical
	};

	value_t derivation( value_t const& v, slice_layout_mode slm );

	llvm::Value* pack_slices(
		llvm::Value** slices,
		int slice_count,
		int slice_size,
		int slice_stride,
		int elem_stride,
		int elem_width
		);

	void unpack_slices(
		llvm::Value* pkg,
		int slice_count,
		int slice_size,
		int slice_stride,
		int elem_stride,
		int elem_width,
		llvm::Value** out_slices
		);

	// Apply break mask and continue mask to top of exec mask stack.
	//  It affects the following execution.
	void apply_break_and_continue();
	void apply_break();
	void apply_continue();

	// Masks
	std::vector<llvm::Value*>	cond_exec_masks;
	std::vector<llvm::Value*>	mask_vars;
	std::vector<llvm::Value*>	break_masks;
	std::vector<llvm::Value*>	continue_masks;
	std::vector<llvm::Value*>	exec_masks;
};

END_NS_SASL_CODE_GENERATOR();
#endif