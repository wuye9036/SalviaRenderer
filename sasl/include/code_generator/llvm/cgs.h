#ifndef SASL_CODE_GENERATOR_LLVM_CGS_H
#define SASL_CODE_GENERATOR_LLVM_CGS_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/include/code_generator/llvm/cgs_objects.h>
#include <sasl/include/code_generator/llvm/cgllvm_intrins.h>
#include <sasl/enums/builtin_types.h>

#include <eflib/include/metaprog/enable_if.h>
#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/metaprog/util.h>

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

BEGIN_NS_SASL_CODE_GENERATOR();

class value_t;
struct node_context;
class llvm_module_impl;
class cg_service;

class cg_service
{
	friend class value_t;

public:
	virtual bool initialize(
		llvm_module_impl* mod, module_context* ctxt,
		sasl::semantic::module_semantic* sem
		);
	virtual bool register_external_intrinsic();

	/// @name Service States
	/// @{
	virtual abis intrinsic_abi() const = 0;
	/// @}

	/// @name Value Operators
	/// @{
	virtual llvm::Value* load( value_t const& );
	virtual llvm::Value* load( value_t const&, abis abi );
	virtual llvm::Value* load_ref( value_t const& );
	virtual llvm::Value* load_ref( value_t const& v, abis abi );
	virtual void store( value_t& lhs, value_t const& rhs ) = 0;
	/// @}

	/** @name Emit expressions
	Some simple overload-able operators such as '+' '-' '*' '/'
	will be implemented in 'cgv_*' classes in operator overload form.
	@{ */
	virtual value_t emit_add( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_sub( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_div( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_mod( value_t const& lhs, value_t const& rhs );

	virtual value_t emit_mul_comp  ( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_mul_intrin( value_t const& lhs, value_t const& rhs );
	
	virtual value_t emit_lshift ( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_rshift ( value_t const& lhs, value_t const& rhs );

	virtual value_t emit_bit_and( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_bit_or ( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_bit_xor( value_t const& lhs, value_t const& rhs );

	virtual value_t emit_cmp_lt ( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_cmp_le ( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_cmp_eq ( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_cmp_ne ( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_cmp_ge ( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_cmp_gt ( value_t const& lhs, value_t const& rhs );

	virtual value_t emit_and( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_or ( value_t const& lhs, value_t const& rhs );

	virtual value_t emit_call( function_t const& fn, std::vector<value_t> const& args );
	virtual value_t emit_call( function_t const& fn, std::vector<value_t> const& args, value_t const& exec_mask );
	/// @}

	/// @name Emit element extraction
	/// @{
	virtual value_t emit_insert_val( value_t const& lhs, value_t const& idx, value_t const& elem_value );
	virtual value_t emit_insert_val( value_t const& lhs, int index, value_t const& elem_value );

	virtual value_t emit_extract_val( value_t const& lhs, int idx );
	virtual value_t emit_extract_val( value_t const& lhs, value_t const& idx );
	virtual value_t emit_extract_ref( value_t const& lhs, int idx );
	virtual value_t emit_extract_ref( value_t const& lhs, value_t const& idx );
	virtual value_t emit_extract_elem_mask( value_t const& vec, uint32_t mask );
	value_t emit_extract_col( value_t const& lhs, size_t index );

	template <typename IndexT>
	value_t emit_extract_elem( value_t const& vec, IndexT const& idx ){
		if( vec.storable() ){
			return emit_extract_ref( vec, idx );
		} else {
			return emit_extract_val( vec, idx );
		}
	}
	/// @}

	/// @name Intrinsics
	/// @{
	virtual value_t emit_dot( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_abs( value_t const& lhs );
	virtual value_t emit_exp( value_t const& lhs );
	virtual value_t emit_sqrt( value_t const& lhs );
	virtual value_t emit_cross( value_t const& lhs, value_t const& rhs );
	virtual value_t emit_ddx( value_t const& v ) = 0;
	virtual value_t emit_ddy( value_t const& v ) = 0;
	virtual value_t emit_any( value_t const& v );
	virtual value_t emit_all( value_t const& v );
	
	virtual value_t emit_tex2Dlod	( value_t const& samp, value_t const& coord );
	virtual value_t emit_tex2Dgrad	( value_t const& samp, value_t const& coord, value_t const& ddx, value_t const& ddy );
	virtual value_t emit_tex2Dbias	( value_t const& samp, value_t const& coord );
	virtual value_t emit_tex2Dproj	( value_t const& samp, value_t const& coord );

	virtual value_t emit_texCUBElod	( value_t const& samp, value_t const& coord );
	virtual value_t emit_texCUBEgrad( value_t const& samp, value_t const& coord, value_t const& ddx, value_t const& ddy );
	virtual value_t emit_texCUBEbias( value_t const& samp, value_t const& coord );
	virtual value_t emit_texCUBEproj( value_t const& samp, value_t const& coord );

	/// @}

	/// @name Emit type casts
	/// @{
	/// Cast between integer types.
	virtual value_t cast_ints( value_t const& v, cg_type* dest_tyi ) = 0;
	/// Cast integer to float.
	virtual value_t cast_i2f ( value_t const& v, cg_type* dest_tyi ) = 0;
	/// Cast float to integer.
	virtual value_t cast_f2i ( value_t const& v, cg_type* dest_tyi ) = 0;
	/// Cast between float types.
	virtual value_t cast_f2f ( value_t const& v, cg_type* dest_tyi ) = 0;
	/// Cast integer to bool
	virtual value_t cast_i2b ( value_t const& v ) = 0;
	/// Cast float to bool
	virtual value_t cast_f2b ( value_t const& v ) = 0;
	/// Cast scalar to vector
	virtual value_t cast_s2v ( value_t const& v );
	/// Cast vector to scalar
	virtual value_t cast_v2s ( value_t const& v );
	/// Bit casts
	virtual value_t cast_bits(value_t const& v, cg_type* dest_tyi);
	/// @}

	/// @name Emit statement
	/// @{
	virtual void emit_return() = 0;
	virtual void emit_return( value_t const&, abis abi ) = 0;
	/// @}

	/// @name Context switch
	/// @{
	virtual void function_beg(){}
	virtual void function_end(){}

	virtual void for_init_beg(){}
	virtual void for_init_end(){}
	virtual void for_cond_beg(){}
	virtual void for_cond_end( value_t const& ){}
	virtual void for_body_beg(){}
	virtual void for_body_end(){}
	virtual void for_iter_beg(){}
	virtual void for_iter_end(){}

	virtual value_t joinable(){ return value_t(); }

	virtual void if_beg(){}
	virtual void if_end(){}
	virtual void if_cond_beg(){}
	virtual void if_cond_end( value_t const& ){}
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
	virtual void while_cond_end( value_t const& ){}
	virtual void while_body_beg(){}
	virtual void while_body_end(){}

	virtual void do_beg(){}
	virtual void do_end(){}
	virtual void do_body_beg(){}
	virtual void do_body_end(){}
	virtual void do_cond_beg(){}
	virtual void do_cond_end( value_t const& ){}

	virtual void break_(){}
	virtual void continue_(){}

	virtual void push_fn(function_t* fn);
	virtual void pop_fn();
	virtual void set_insert_point( insert_point_t const& ip );
	virtual insert_point_t insert_point() const;
	/// @}

	/// @name Context queries
	/// @{
	bool in_function() const;
	function_t& fn();
	/// Get Packed Mask Which is a uint16_t.
	virtual value_t packed_mask() = 0;
	/// @}

	/// @name Emit value and variables
	/// @{
	value_t extend_to_vm( value_t const&, builtin_types hint );

	function_t* fetch_function(sasl::syntax_tree::function_type* fn_node);
	
	template <typename T>
	value_t create_constant_scalar( T const& v, cg_type* tyinfo, builtin_types hint, EFLIB_ENABLE_IF_COND( boost::is_integral<T> ) ){
		Value* ll_val = ConstantInt::get( IntegerType::get( context(), sizeof(T) * 8 ), uint64_t(v), boost::is_signed<T>::value );
		return create_scalar( ll_val, tyinfo, hint );
	}

	template <typename T>
	value_t create_constant_scalar( T const& v, cg_type* tyinfo, builtin_types hint, EFLIB_ENABLE_IF_COND( boost::is_floating_point<T> ) ){
		Value* ll_val = ConstantFP::get( Type::getFloatTy( context() ), v );
		return create_scalar( ll_val, tyinfo, hint );
	}
	virtual value_t create_scalar( llvm::Value* val, cg_type* tyinfo, builtin_types hint ) = 0;

	value_t null_value( cg_type* tyinfo, abis abi );
	value_t null_value( builtin_types bt, abis abi );
	value_t undef_value( builtin_types bt, abis abi );

	value_t create_constant_int( cg_type* tyinfo, builtin_types bt, abis abi, uint64_t v );

	value_t create_value( cg_type* tyinfo, llvm::Value* val, value_kinds k, abis abi );
	value_t create_value( builtin_types hint, llvm::Value* val, value_kinds k, abis abi );
	value_t create_value( cg_type* tyinfo, builtin_types hint, llvm::Value* val, value_kinds k, abis abi );

	value_t create_variable( cg_type const*, abis abi, std::string const& name );
	value_t create_variable( builtin_types bt, abis abi, std::string const& name );

	virtual value_t create_vector( std::vector<value_t> const& scalars, abis abi ) = 0;
	virtual value_t create_value_by_scalar( value_t const& scalar, cg_type* tyinfo, builtin_types hint );
	/// @}

	/// @name Utilities
	/// @{
	/// Create a new block at the last of function
	insert_point_t new_block( std::string const& hint, bool set_insert_point );
	/// Jump to the specified block by condition.
	void jump_cond( value_t const& cond_v, insert_point_t const & true_ip, insert_point_t const& false_ip );
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
	llvm::Type* type_( builtin_types bt, abis abi );
	llvm::Type* type_( cg_type const*, abis abi );
	template <typename T>
	llvm::ConstantInt* int_( T v ){
		return llvm::ConstantInt::get( context(), apint(v) );
	}
	template <typename T>
	llvm::Constant* vector_( T const* vals, size_t length, EFLIB_ENABLE_IF_PRED1(is_integral, T) )
	{
		assert( vals && length > 0 );

		std::vector<llvm::Constant*> elems(length);
		for( size_t i = 0; i < length; ++i ){
			elems[i] = int_(vals[i]);
		}

		return llvm::cast<llvm::Constant>( llvm::ConstantVector::get( elems ) );
	}

	template <typename U, typename T>
	llvm::Constant* vector_( T const* vals, size_t length, EFLIB_ENABLE_IF_PRED1(is_integral, T) )
	{
		assert( vals && length > 0 );

		std::vector<llvm::Constant*> elems(length);
		for( size_t i = 0; i < length; ++i ){
			elems[i] = int_( U(vals[i]) );
		}

		return llvm::cast<llvm::Constant>( llvm::ConstantVector::get( elems ) );
	}

	llvm::Value* integer_value_( llvm::Type* ty, llvm::APInt const& );

	llvm::Value* load_as( value_t const& v, abis abi );
	/// @}

	llvm::Module*			module () const;
	llvm::LLVMContext&		context() const;
	llvm::DefaultIRBuilder& builder() const;

	virtual bool			prefer_externals() const	= 0;
	virtual bool			prefer_scalar_code() const	= 0;

	virtual abis			param_abi( bool is_c_compatible ) const = 0;
			abis			promote_abi( abis abi0, abis abi1 );
			abis			promote_abi( abis abi0, abis abi1, abis abi2 );

	node_context*			get_node_context( sasl::syntax_tree::node* );
	node_context*			get_or_create_node_context( sasl::syntax_tree::node* );
	sasl::semantic::node_semantic*
							get_node_semantic( sasl::syntax_tree::node* );
protected:
	enum intrin_ids
	{
		exp_f32,
		exp2_f32,
		sin_f32,
		cos_f32,
		tan_f32,
		asin_f32,
		acos_f32,
		atan_f32,
		ceil_f32,
		floor_f32,
		log_f32,
		log2_f32,
		log10_f32,
		rsqrt_f32,
		ldexp_f32,
		mod_f32,
		tex2dlod_vs,
		tex2dlod_ps,
		tex2dgrad_ps,
		tex2dbias_ps,
		tex2dproj_ps,
		texCUBElod_vs,
		texCUBElod_ps,
		texCUBEgrad_ps,
		texCUBEbias_ps,
		texCUBEproj_ps,
		intrins_count
	};

	sasl::semantic::module_semantic*	sem_;
	llvm_module_impl*					llvm_mod_;
	module_context*						ctxt_;
	
	std::vector<function_t*>			fn_ctxts;
	llvm_intrin_cache					intrins;
	llvm::Function*						external_intrins[intrins_count];
	value_t								exec_mask;
	
	value_t emit_cmp(
		value_t const& lhs, value_t const& rhs,
		uint32_t pred_signed, uint32_t pred_unsigned, uint32_t pred_float
		);
	
	typedef boost::function<llvm::Value* (llvm::Value*, llvm::Value*)>	bin_fn_t;
	typedef boost::function<llvm::Value* (llvm::Value*, llvm::Type*)>	cast_fn;
	typedef boost::function<llvm::Value* (llvm::Value*)>				unary_fn_t;

	llvm::Value* call_external1_		( llvm::Function* f, llvm::Value* v );
	llvm::Value* call_external2_		( llvm::Function* f, llvm::Value* v0, llvm::Value* v1 );

	unary_fn_t	bind_unary_call_		( llvm::Function* fn );
	bin_fn_t	bind_binary_call_		( llvm::Function* fn );
	
	unary_fn_t	bind_unary_external_	( llvm::Function* fn );
	bin_fn_t	bind_binary_external_	( llvm::Function* fn );

	// LLVM have some instructions/intrinsics to support unary and binary operations.
	// But even simple instruction 'add', there are two overloads to support 'iadd' and 'fadd' separately.
	// Additional, some intrinsics are only support scalar or SIMD vector as argument.
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

	llvm::Value* bin_op_ps_ts_sva_		(
		llvm::Type* ret_ty, llvm::Value* lhs, llvm::Value* rhs,
		bin_fn_t sfn, bin_fn_t vfn, bin_fn_t simd_fn, bin_fn_t sv_fn,
		unary_fn_t cast_result_sv_fn
		);

	llvm::Value* unary_op_ps_ts_sva_	(
		llvm::Type* ret_ty, llvm::Value* v,
		unary_fn_t sfn, unary_fn_t vfn, unary_fn_t simd_fn, unary_fn_t sv_fn
		);

public:
	value_t emit_bin_ps_ta_sva(
		value_t const& lhs, value_t const& rhs,
		bin_fn_t signed_sv_fn, bin_fn_t unsigned_sv_fn, bin_fn_t float_sv_fn
		);
	value_t emit_bin_ps_ta_sva(
		std::string const& scalar_external_intrin_name,
		value_t const& v0, value_t const& v1
		);
	value_t emit_bin_es_ta_sva(
		std::string const& scalar_external_intrin_name,
		value_t const& lhs, value_t const& rhs
		);
	value_t emit_bin_es_ta_sva(
		value_t const& lhs, value_t const& rhs,
		bin_fn_t signed_sv_fn, bin_fn_t unsigned_sv_fn, bin_fn_t float_sv_fn
		);
	value_t emit_unary_ps( std::string const& scalar_external_intrin_name, value_t const& v );

protected:
	value_t emit_bin_mm(
		value_t const& lhs, value_t const& rhs,
		bin_fn_t signed_fn, bin_fn_t unsigned_fn, bin_fn_t float_fn
		);
	value_t emit_dot_vv( value_t const& lhs, value_t const& rhs );

	value_t emit_mul_sv( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_sm( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_vm( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_mv( value_t const& lhs, value_t const& rhs );
	value_t emit_mul_mm( value_t const& lhs, value_t const& rhs );

	virtual value_t emit_tex_lod_impl(
		value_t const& samp, value_t const& coord,
		intrin_ids vs_intrin, intrin_ids ps_intrin );
	virtual value_t emit_tex_grad_impl(
		value_t const& samp, value_t const& coord,
		value_t const& ddx, value_t const& ddy,
		intrin_ids ps_intrin );
	virtual value_t emit_tex_bias_impl(
		value_t const& samp, value_t const& coord,
		intrin_ids ps_intrin );
	virtual value_t emit_tex_proj_impl(
		value_t const& samp, value_t const& coord,
		intrin_ids ps_intrin );

	void merge_swizzle( value_t const*& root, char indexes[], value_t const& v );

	llvm::AllocaInst* alloca_(llvm::Type* ty, std::string const& name);

	llvm::Function* intrin_( int );
	template <typename FunctionT>
	llvm::Function* intrin_( int );
	
	llvm::Value* shrink_( llvm::Value* vec, size_t vsize );
	llvm::Value* extract_elements_( llvm::Value* src, size_t start_pos, size_t length );
	llvm::Value* insert_elements_ ( llvm::Value* dst, llvm::Value* src, size_t start_pos );
	llvm::Value* i8toi1_( llvm::Value* );
	llvm::Value* i1toi8_( llvm::Value* );

	enum cast_ops
	{
		cast_op_unknown,
		cast_op_f2u,
		cast_op_f2i,
		cast_op_u2f,
		cast_op_i2f,
		cast_op_bitcast,
		cast_op_i2i_signed,
		cast_op_i2i_unsigned
	};

	llvm::Value* cast_sv_( llvm::Value*, llvm::Type* elem_ty, cast_ops op );
	unary_fn_t	 bind_cast_sv_(llvm::Type* elem_ty, cast_ops op);
	llvm::Value* safe_idiv_imod_sv_( llvm::Value*, llvm::Value*, bin_fn_t div_or_mod_fn );
private:
	llvm::Value* load_as_llvm_c			( value_t const& v, abis abi );
	llvm::Value* load_c_as_package		( value_t const& v );
	llvm::Value* load_llvm_as_vec		( value_t const& v );
	llvm::Value* load_vec_as_llvm		( value_t const& v );
	llvm::Value* load_vec_as_package	( value_t const& v );
};

END_NS_SASL_CODE_GENERATOR();

#endif
