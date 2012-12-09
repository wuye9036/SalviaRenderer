#ifndef SASL_CODEGEN_CGS_SIMD_H
#define SASL_CODEGEN_CGS_SIMD_H

#include <sasl/include/codegen/forward.h>

#include <sasl/include/codegen/cgs.h>

BEGIN_NS_SASL_CODEGEN();

class cgs_simd: public cg_service
{
public:
	virtual void store( multi_value& lhs, multi_value const& rhs );

	multi_value emit_and(multi_value const& lhs, multi_value const& rhs);
	multi_value emit_or (multi_value const& lhs, multi_value const& rhs);

	virtual multi_value cast_ints( multi_value const& v, cg_type* dest_tyi );
	virtual multi_value cast_i2f ( multi_value const& v, cg_type* dest_tyi );
	virtual multi_value cast_f2i ( multi_value const& v, cg_type* dest_tyi );
	virtual multi_value cast_f2f ( multi_value const& v, cg_type* dest_tyi );
	virtual multi_value cast_i2b ( multi_value const& v );
	virtual multi_value cast_f2b ( multi_value const& v );

	void emit_return();
	void emit_return( multi_value const&, abis::id abi );
	
	virtual multi_value emit_ddx( multi_value const& v );
	virtual multi_value emit_ddy( multi_value const& v );
	
	virtual multi_value create_vector( std::vector<multi_value> const& scalars, abis::id abi );

	bool prefer_externals() const{ return false; }
	bool prefer_scalar_code() const{ return false; }

	virtual void function_body_beg();
	virtual void function_body_end();

	virtual void for_init_beg();
	virtual void for_init_end();
	virtual void for_cond_beg();
	virtual void for_cond_end( multi_value const& );
	virtual void for_body_beg();
	virtual void for_body_end();
	virtual void for_iter_beg();
	virtual void for_iter_end();

	virtual multi_value any_mask_true();

	virtual void if_beg();
	virtual void if_end();
	virtual void if_cond_beg();
	virtual void if_cond_end( multi_value const& );
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
	virtual void while_cond_end( multi_value const& );
	virtual void while_body_beg();
	virtual void while_body_end();

	virtual void do_beg();
	virtual void do_end();
	virtual void do_body_beg();
	virtual void do_body_end();
	virtual void do_cond_beg();
	virtual void do_cond_end( multi_value const& );

	virtual void break_();
	virtual void continue_();

private:
	llvm::Value*	load_loop_execution_mask();
	void			save_loop_execution_mask( llvm::Value* );
	virtual void	enter_loop();
	virtual void	exit_loop();
	virtual void	apply_loop_condition( multi_value const& );
	virtual void	save_next_iteration_exec_mask();

	llvm::Value*	all_one_mask();
	llvm::Value*	all_zero_mask();
	llvm::Value*	expanded_mask( uint32_t expanded_times );

	enum slice_layout_mode{
		slm_horizontal,
		slm_vertical
	};

	multi_value derivation( multi_value const& v, slice_layout_mode slm );

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

END_NS_SASL_CODEGEN();
#endif