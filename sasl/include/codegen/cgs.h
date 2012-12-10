#ifndef SASL_CODEGEN_CGS_H
#define SASL_CODEGEN_CGS_H

#include <sasl/include/codegen/forward.h>

#include <sasl/include/codegen/cgs_objects.h>
#include <sasl/include/codegen/cg_intrins.h>
#include <sasl/include/codegen/cg_extension.h>

#include <sasl/enums/builtin_types.h>

#include <eflib/include/utility/enable_if.h>
#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/type_traits.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

namespace sasl
{
	namespace semantic
	{
		class module_semantic;
		class node_semantic;
	}
}

BEGIN_NS_SASL_CODEGEN();

class  multi_value;
class  cg_module_impl;
struct node_context;

class cg_service
{
	friend class multi_value;

public:
	cg_service(size_t parallel_factor);

	virtual bool initialize(
		cg_module_impl* mod, module_context* ctxt,
		sasl::semantic::module_semantic* sem
		);

	/// @name Value Operators
	/// @{
	virtual value_array	 load(multi_value const&);
	virtual value_array	 load(multi_value const&, abis::id abi);
	virtual value_array	 load_ref(multi_value const&);
	virtual value_array	 load_ref(multi_value const& v, abis::id abi);

	/*virtual llvm::Value* load(multi_value const&, size_t index);
	virtual llvm::Value* load(multi_value const&, size_t index, abis::id abi);
	virtual llvm::Value* load_ref(multi_value const&, size_t index);
	virtual llvm::Value* load_ref(multi_value const& v, size_t index, abis::id abi);*/

	virtual void store( multi_value& lhs, multi_value const& rhs ) = 0;
	/// @}

	/** @name Emit expressions
	Some simple overload-able operators such as '+' '-' '*' '/'
	will be implemented in 'cgv_*' classes in operator overload form.
	@{ */
	virtual multi_value emit_add( multi_value const& lhs, multi_value const& rhs );
	virtual multi_value emit_sub( multi_value const& lhs, multi_value const& rhs );
	virtual multi_value emit_div( multi_value const& lhs, multi_value const& rhs );
	virtual multi_value emit_mod( multi_value const& lhs, multi_value const& rhs );

	virtual multi_value emit_mul_comp  ( multi_value const& lhs, multi_value const& rhs );
	virtual multi_value emit_mul_intrin( multi_value const& lhs, multi_value const& rhs );
	
	virtual multi_value emit_lshift ( multi_value const& lhs, multi_value const& rhs );
	virtual multi_value emit_rshift ( multi_value const& lhs, multi_value const& rhs );

	virtual multi_value emit_bit_and( multi_value const& lhs, multi_value const& rhs );
	virtual multi_value emit_bit_or ( multi_value const& lhs, multi_value const& rhs );
	virtual multi_value emit_bit_xor( multi_value const& lhs, multi_value const& rhs );

	virtual multi_value emit_cmp_lt ( multi_value const& lhs, multi_value const& rhs );
	virtual multi_value emit_cmp_le ( multi_value const& lhs, multi_value const& rhs );
	virtual multi_value emit_cmp_eq ( multi_value const& lhs, multi_value const& rhs );
	virtual multi_value emit_cmp_ne ( multi_value const& lhs, multi_value const& rhs );
	virtual multi_value emit_cmp_ge ( multi_value const& lhs, multi_value const& rhs );
	virtual multi_value emit_cmp_gt ( multi_value const& lhs, multi_value const& rhs );

	virtual multi_value emit_not( multi_value const& v );
	virtual multi_value emit_and( multi_value const& lhs, multi_value const& rhs );
	virtual multi_value emit_or ( multi_value const& lhs, multi_value const& rhs );

	virtual multi_value emit_call( cg_function const& fn, std::vector<multi_value> const& args );
	virtual multi_value emit_call( cg_function const& fn, std::vector<multi_value> const& args, llvm::Value* exec_mask );
	/// @}

	/// @name Emit element extraction
	/// @{
	virtual multi_value emit_insert_val( multi_value const& lhs, multi_value const& idx, multi_value const& elem_value );
	virtual multi_value emit_insert_val( multi_value const& lhs, int index, multi_value const& elem_value );

	virtual multi_value emit_extract_val( multi_value const& lhs, int idx );
	virtual multi_value emit_extract_val( multi_value const& lhs, multi_value const& idx );
	virtual multi_value emit_extract_ref( multi_value const& lhs, int idx );
	virtual multi_value emit_extract_ref( multi_value const& lhs, multi_value const& idx );
	virtual multi_value emit_extract_elem_mask( multi_value const& vec, uint32_t mask );
	multi_value emit_extract_col( multi_value const& lhs, size_t index );

	template <typename IndexT>
	multi_value emit_extract_elem( multi_value const& vec, IndexT const& idx ){
		if( vec.storable() ){
			return emit_extract_ref( vec, idx );
		} else {
			return emit_extract_val( vec, idx );
		}
	}
	/// @}

	/// @name Intrinsics
	/// @{
	virtual multi_value emit_dot( multi_value const& lhs, multi_value const& rhs );
	virtual multi_value emit_abs( multi_value const& lhs );
	virtual multi_value emit_sqrt( multi_value const& lhs );
	virtual multi_value emit_cross( multi_value const& lhs, multi_value const& rhs );
	virtual multi_value emit_ddx( multi_value const& v ) = 0;
	virtual multi_value emit_ddy( multi_value const& v ) = 0;
	virtual multi_value emit_any( multi_value const& v );
	virtual multi_value emit_all( multi_value const& v );
	virtual multi_value emit_select( multi_value const& flag, multi_value const& v0, multi_value const& v1 );
	virtual multi_value emit_isinf( multi_value const& v );
	virtual multi_value emit_isfinite( multi_value const& v );
	virtual multi_value emit_isnan( multi_value const& v );
	virtual multi_value emit_sign(multi_value const& v);
	virtual multi_value emit_clamp(multi_value const& v, multi_value const& min_v, multi_value const& max_v);
	virtual multi_value emit_saturate(multi_value const& v);

	virtual multi_value emit_tex2Dlod	( multi_value const& samp, multi_value const& coord );
	virtual multi_value emit_tex2Dgrad	( multi_value const& samp, multi_value const& coord, multi_value const& ddx, multi_value const& ddy );
	virtual multi_value emit_tex2Dbias	( multi_value const& samp, multi_value const& coord );
	virtual multi_value emit_tex2Dproj	( multi_value const& samp, multi_value const& coord );

	virtual multi_value emit_texCUBElod	( multi_value const& samp, multi_value const& coord );
	virtual multi_value emit_texCUBEgrad( multi_value const& samp, multi_value const& coord, multi_value const& ddx, multi_value const& ddy );
	virtual multi_value emit_texCUBEbias( multi_value const& samp, multi_value const& coord );
	virtual multi_value emit_texCUBEproj( multi_value const& samp, multi_value const& coord );

	/// @}

	/// @name Emit type casts
	/// @{
	/// Cast between integer types.
	virtual multi_value cast_ints( multi_value const& v, cg_type* dest_tyi ) = 0;
	/// Cast integer to float.
	virtual multi_value cast_i2f ( multi_value const& v, cg_type* dest_tyi ) = 0;
	/// Cast float to integer.
	virtual multi_value cast_f2i ( multi_value const& v, cg_type* dest_tyi ) = 0;
	/// Cast between float types.
	virtual multi_value cast_f2f ( multi_value const& v, cg_type* dest_tyi ) = 0;
	/// Cast integer to bool
	virtual multi_value cast_i2b ( multi_value const& v ) = 0;
	/// Cast float to bool
	virtual multi_value cast_f2b ( multi_value const& v ) = 0;
	/// Cast scalar to vector
	virtual multi_value cast_s2v ( multi_value const& v );
	/// Cast vector to scalar
	virtual multi_value cast_v2s ( multi_value const& v );
	/// Bit casts
	virtual multi_value cast_bits(multi_value const& v, cg_type* dest_tyi);
	/// @}

	/// @name Emit statement
	/// @{
	virtual void emit_return() = 0;
	virtual void emit_return( multi_value const&, abis::id abi ) = 0;
	/// @}

	/// @name Context switch hooks
	/// @{
	virtual void function_body_beg();
	virtual void function_body_end();

	virtual void for_init_beg(){}
	virtual void for_init_end(){}
	virtual void for_cond_beg(){}
	virtual void for_cond_end( multi_value const& ){}
	virtual void for_body_beg(){}
	virtual void for_body_end(){}
	virtual void for_iter_beg(){}
	virtual void for_iter_end(){}

	virtual multi_value any_mask_true(){ return multi_value(parallel_factor_); }

	virtual void if_beg(){}
	virtual void if_end(){}
	virtual void if_cond_beg(){}
	virtual void if_cond_end( multi_value const& ){}
	virtual void then_beg(){}
	virtual void then_end(){}
	virtual void else_beg(){}
	virtual void else_end(){}

	virtual void switch_cond_beg(){}
	virtual void switch_cond_end(){}
	virtual void switch_expr_beg(){}
	virtual void switch_expr_end(){}

	virtual void while_beg(){}
	virtual void while_end(){}
	virtual void while_cond_beg(){}
	virtual void while_cond_end( multi_value const& ){}
	virtual void while_body_beg(){}
	virtual void while_body_end(){}

	virtual void do_beg(){}
	virtual void do_end(){}
	virtual void do_body_beg(){}
	virtual void do_body_end(){}
	virtual void do_cond_beg(){}
	virtual void do_cond_end( multi_value const& ){}

	virtual void break_(){}
	virtual void continue_(){}

	virtual void push_fn(cg_function* fn);
	virtual void pop_fn();
	virtual void set_insert_point( insert_point_t const& ip );
	virtual insert_point_t insert_point() const;
	/// @}

	/// @name Context queries
	/// @{
	bool in_function() const;
	cg_function& fn();
	/// @}

	/// @name Emit value and variables
	/// @{
	multi_value extend_to_vm( multi_value const&, builtin_types hint );

	cg_function* fetch_function(sasl::syntax_tree::function_type* fn_node);
	
	template <typename T>
	multi_value create_constant_scalar( T const& v, cg_type* tyinfo, builtin_types hint, EFLIB_ENABLE_IF_COND( boost::is_integral<T> ) ){
		Value* ll_val = ConstantInt::get( IntegerType::get( context(), sizeof(T) * 8 ), uint64_t(v), boost::is_signed<T>::value );
		return create_scalar( ll_val, tyinfo, hint );
	}

	template <typename T>
	multi_value create_constant_scalar( T const& v, cg_type* tyinfo, builtin_types hint, EFLIB_ENABLE_IF_COND( boost::is_floating_point<T> ) ){
		Value* ll_val = ConstantFP::get( Type::getFloatTy( context() ), v );
		return create_scalar( ll_val, tyinfo, hint );
	}
	virtual multi_value create_scalar(llvm::Value* val, cg_type* tyinfo, builtin_types hint);

	multi_value  null_value		(cg_type* tyinfo, abis::id abi);
	multi_value  null_value		(builtin_types bt, abis::id abi);
	multi_value  undef_value	(builtin_types bt, abis::id abi);
	multi_value  one_value		(multi_value const& proto);
	multi_value  numeric_value	(multi_value const& proto, double fp, uint64_t ui);
	llvm::Value* get_mask_flag	(llvm::Value* mask, size_t index);
	void		 set_mask_flag	(llvm::Value* mask, size_t index, llvm::Value* flag);
	llvm::Value* combine_flags	(value_array const& flags);
	value_array  split_mask		(llvm::Value* mask);
	
	multi_value create_constant_int( cg_type* tyinfo, builtin_types bt, abis::id abi, uint64_t v );

	value_array invalid_value_array();

	multi_value create_value( cg_type* tyinfo
		, value_array const& v, value_kinds::id k, abis::id abi );
	multi_value create_value( builtin_types hint
		, value_array const& v, value_kinds::id k, abis::id abi );
	multi_value create_value( cg_type* tyinfo, builtin_types hint
		, value_array const& v, value_kinds::id k, abis::id abi );

	multi_value create_variable( cg_type const*, abis::id abi, std::string const& name );
	multi_value create_variable( builtin_types bt, abis::id abi, std::string const& name );

	virtual multi_value create_vector( std::vector<multi_value> const& scalars, abis::id abi ) = 0;
	virtual multi_value create_value_by_scalar( multi_value const& scalar, cg_type* tyinfo, builtin_types hint );
	/// @}

	/// @name Utilities
	/// @{
	/// Create a new block at the last of function
	insert_point_t new_block( std::string const& hint, bool set_insert_point );
	/// Jump to the specified block by condition.
	void jump_cond( multi_value const& cond_v, insert_point_t const & true_ip, insert_point_t const& false_ip );
	/// Jump to
	void jump_to( insert_point_t const& ip );
	void clean_empty_blocks();
	/// @}

	/// @name Type emitters
	/// @{
	/// Create tyinfo.
	cg_type* create_ty( sasl::syntax_tree::tynode* tyn );
	/// Get member type information is type is aggregated.
	cg_type* member_tyinfo( cg_type const* agg, size_t index ) const;
	/// @}
	
	/// @name Bridges
	/// @{
	llvm::Type*	 type_(builtin_types bt, abis::id abi);
	llvm::Type*  type_(cg_type const*, abis::id abi);

	value_array  load_as( multi_value const& v, abis::id abi );
	llvm::Value* restore(llvm::Value* v);
	value_array  restore(value_array const& v);
	/// @}

	llvm::Module*			module () const;
	llvm::LLVMContext&		context() const;
	llvm::DefaultIRBuilder& builder() const;

	virtual llvm::Value*	current_execution_mask() const = 0;
	virtual bool			prefer_externals() const	= 0;
	virtual bool			prefer_scalar_code() const	= 0;
	virtual size_t			parallel_factor() const;

	virtual abis::id		param_abi( bool is_c_compatible ) const;
			abis::id		promote_abi( abis::id abi0, abis::id abi1 );
			abis::id		promote_abi( abis::id abi0, abis::id abi1, abis::id abi2 );

	node_context*			get_node_context( sasl::syntax_tree::node* );
	node_context*			get_or_create_node_context( sasl::syntax_tree::node* );
	sasl::semantic::node_semantic*
							get_node_semantic( sasl::syntax_tree::node* );
protected:
	sasl::semantic::module_semantic*	sem_;
	cg_module_impl*						llvm_mod_;
	module_context*						ctxt_;
	
	std::vector<cg_function*>			fn_ctxts;
	size_t								parallel_factor_;
	multi_value							exec_mask;
	
	multi_value emit_cmp(
		multi_value const& lhs, multi_value const& rhs,
		uint32_t pred_signed, uint32_t pred_unsigned, uint32_t pred_float
		);
	
	// LLVM have some instructions/intrinsics to support unary and binary operations.
	// But even simple instruction 'add', there are two overloads to support 'iadd' and 'fadd' separately.
	// Additional, some intrinsics only support scalar or SIMD vector as argument.
	// In fact, For conventionally if binary operation is per-scalar, we need a function
	// which support all aggregation and all scalar types.
	// So, we could use techniques looks like high-order function to combine native intrinsics to
	// a full-scalar-typed and full-aggregation supported function.
	// But, sometimes we cannot combine them at once. Some intermediate functions/functors are generated.
	// For instance, we combine some scalar intrinsic to scalar-vector supported intrinsic
	// and later to all aggregation supported function. Also we may need some function as binder.
	// For distinguish these side-effect functors, we built a postfix system, which decorated function names to
	// indicate what the function can do.

	// Postfix of function name described the requirements of arguments of function.
	//   Component operation modes:
	//     PS: Per scalar
	//     ES: Two parameters are available and one is scalar, the other is aggregated, repeat scalar to aggregated and to PS
	//   Scalar Type Compatibility:
	//     TS: Type are Same. Type of component of Value are same as functor supported type.
	//       e.g. if sv_fn only support Singed Int or Vector of Signed Int
	//            lhs and rhs must be value of singed int or <singed int x n>.
	//     TA: Type are All. Type of component of Value are one of functor supported type.
	//       e.g. signed_sv_fn and float_sv_fn are functors to forward,
	//	          lhs and rhs could be value of signed int or <singed int x n> or float or <float x n>
	//   Aggregation Compatibility:
	//     S: Scalar
    //     V: Vector
    //     A: Aggregated (Structure, Array)
	//	   M: SIMD Vector
	//       e.g. SVA means Value could be Scalar, Vector or Aggregated.
public:
	multi_value emit_bin_ps_ta_sva(
		multi_value const& lhs, multi_value const& rhs,
		binary_intrin_functor signed_sv_fn,
		binary_intrin_functor unsigned_sv_fn,
		binary_intrin_functor float_sv_fn
		);
	multi_value emit_bin_ps_ta_sva(
		std::string const& scalar_external_intrin_name,
		multi_value const& v0, multi_value const& v1
		);
	multi_value emit_bin_es_ta_sva(
		std::string const& scalar_external_intrin_name,
		multi_value const& lhs, multi_value const& rhs
		);
	multi_value emit_bin_es_ta_sva(
		multi_value const& lhs, multi_value const& rhs,
		binary_intrin_functor signed_sv_fn,
		binary_intrin_functor unsigned_sv_fn,
		binary_intrin_functor float_sv_fn
		);
	multi_value emit_unary_ps( std::string const& scalar_external_intrin_name, multi_value const& v );

	cg_extension* extension();

protected:
	multi_value emit_bin_mm(
		multi_value const& lhs, multi_value const& rhs,
		binary_intrin_functor signed_fn,
		binary_intrin_functor unsigned_fn,
		binary_intrin_functor float_fn
		);
	multi_value emit_dot_vv( multi_value const& lhs, multi_value const& rhs );

	multi_value emit_mul_sv( multi_value const& lhs, multi_value const& rhs );
	multi_value emit_mul_sm( multi_value const& lhs, multi_value const& rhs );
	multi_value emit_mul_vm( multi_value const& lhs, multi_value const& rhs );
	multi_value emit_mul_mv( multi_value const& lhs, multi_value const& rhs );
	multi_value emit_mul_mm( multi_value const& lhs, multi_value const& rhs );

	virtual multi_value emit_tex_lod_impl(
		multi_value const& samp, multi_value const& coord,
		externals::id vs_intrin, externals::id ps_intrin );
	virtual multi_value emit_tex_grad_impl(
		multi_value const& samp, multi_value const& coord,
		multi_value const& ddx, multi_value const& ddy,
		externals::id ps_intrin );
	virtual multi_value emit_tex_bias_impl(
		multi_value const& samp, multi_value const& coord,
		externals::id ps_intrin );
	virtual multi_value emit_tex_proj_impl(
		multi_value const& samp, multi_value const& coord,
		externals::id ps_intrin );

	bool merge_swizzle( multi_value const*& root, char indexes[], multi_value const& v );

	multi_value inf_from_value(multi_value const& v, bool negative);

protected:
	boost::scoped_ptr<cg_extension> ext_;
	value_array load_as_llvm_c(multi_value const& v, abis::id abi);
};

END_NS_SASL_CODEGEN();

#endif
