#ifndef SASL_CODE_GENERATOR_LLVM_CG_EXTENSION_H
#define SASL_CODE_GENERATOR_LLVM_CG_EXTENSION_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/include/code_generator/llvm/cgllvm_intrins.h>
#include <sasl/enums/builtin_types.h>

#include <eflib/include/utility/enable_if.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/TypeBuilder.h>
#include <llvm/Constants.h>
#include <llvm/ADT/APInt.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/function.hpp>
#include <boost/type_traits.hpp>
#include <eflib/include/platform/boost_end.h>

namespace llvm
{
	class Constant;
	class LLVMContext;
	class Module;
	class Type;
	class Value;
	class Argument;
	class Function;
	class BasicBlock;
	class ConstantInt;
	class ConstantVector;
	class AllocaInst;
	class Function;
	class APInt;
	class PHINode;

	template <bool preserveNames> class IRBuilderDefaultInserter;
	template< bool preserveNames, typename T, typename Inserter
	> class IRBuilder;
	class ConstantFolder;
	typedef IRBuilder<true, ConstantFolder, IRBuilderDefaultInserter<true> >
		DefaultIRBuilder;
	template <typename T> class ArrayRef;
}

BEGIN_NS_SASL_CODE_GENERATOR();

namespace externals
{
	enum id
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
		round_f32,
		trunc_f32,
		log_f32,
		log2_f32,
		log10_f32,
		rsqrt_f32,
		ldexp_f32,
		sinh_f32,
		cosh_f32,
		tanh_f32,
		mod_f32,
		pow_f32,
		countbits_u32,
		firstbithigh_u32,
		firstbitlow_u32,
		reversebits_u32,
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
		count
	};
};

namespace cast_ops
{
	enum id
	{
		unknown,
		f2u,
		f2i,
		u2f,
		i2f,
		bitcast,
		i2i_signed,
		i2i_unsigned
	};
}

typedef boost::function<llvm::Value* (llvm::Value*, llvm::Value*)>	binary_intrin_functor;
typedef boost::function<llvm::Value* (llvm::Value*, llvm::Type*)>	cast_intrin_functor;
typedef boost::function<llvm::Value* (llvm::Value*)>				unary_intrin_functor;

class cg_extension
{
public:
	cg_extension( llvm::DefaultIRBuilder* builder, llvm::LLVMContext& context, llvm::Module* module );

	// Type of lhs and type of rhs are same.
	// Scalar, vector and aggregated type could be used as type of lhs and rhs.
	llvm::Value* call_binary_intrin(
		llvm::Type* ret_ty,
		llvm::Value* lhs, llvm::Value* rhs,
		binary_intrin_functor sv_fn, unary_intrin_functor cast_result_sv_fn );
		
	// Scalar, vector and aggregated type could be used as type of lhs and rhs.
	llvm::Value* call_unary_intrin( llvm::Type* ret_ty, llvm::Value* v, unary_intrin_functor sv_fn );

	// Bind functions and externals to functor.
	unary_intrin_functor	bind_to_unary(llvm::Function* fn);
	binary_intrin_functor	bind_to_binary(llvm::Function* fn);
	unary_intrin_functor	bind_external_to_unary ( llvm::Function* fn );
	binary_intrin_functor	bind_external_to_binary( llvm::Function* fn );
	binary_intrin_functor	bind_external_to_binary( externals::id id );
	unary_intrin_functor	bind_cast_sv(llvm::Type* elem_ty, cast_ops::id op);

	// Promote scalar, vector And SIMD intrinsics to scalar-vector intrinsic.
	unary_intrin_functor	promote_to_unary_sv( unary_intrin_functor sfn, unary_intrin_functor vfn, unary_intrin_functor simd_fn );
	binary_intrin_functor	promote_to_binary_sv( binary_intrin_functor sfn, binary_intrin_functor vfn, binary_intrin_functor simd_fn );

	// Extended IR intrinsics.
	llvm::Function* vm_intrin(int intrin_id);
	llvm::Function* vm_intrin(int intrin_id, llvm::FunctionType* ty);
	template <typename FunctionT>
	llvm::Function* vm_intrin( int intrin_id )
	{
		return vm_intrin( intrin_id, llvm::TypeBuilder<FunctionT, false>::get(context_) );
	}

	llvm::Function* external(externals::id id);

	llvm::Value* safe_idiv_imod_sv( llvm::Value*, llvm::Value*, binary_intrin_functor div_or_mod_sv );
	llvm::Value* abs_sv(llvm::Value* v);
	llvm::Value* shrink( llvm::Value* vec, size_t vsize );
	llvm::Value* extract_elements( llvm::Value* src, size_t start_pos, size_t length );
	llvm::Value* insert_elements( llvm::Value* dst, llvm::Value* src, size_t start_pos );
	llvm::Value* i8toi1_sv( llvm::Value* );
	llvm::Value* i1toi8_sv( llvm::Value* );
	llvm::Value* call_external_1( llvm::Function* f, llvm::Value* v );
	llvm::Value* call_external_2( llvm::Function* f, llvm::Value* v0, llvm::Value* v1 );
	llvm::Value* cast_sv(llvm::Value* v, llvm::Type* ty, cast_ops::id op);
	llvm::Value* select(llvm::Value*, llvm::Value*, llvm::Value*);
	llvm::Type*  extract_scalar_type(llvm::Type* ty);
	llvm::PHINode* phi( llvm::BasicBlock* b0, llvm::Value* v0, llvm::BasicBlock* b1, llvm::Value* v1 );
	
	// Alloca
	void set_stack_alloc_point(llvm::BasicBlock* alloc_point);
	llvm::AllocaInst* stack_alloc(llvm::Type* ty, llvm::Twine const& name);

	// Value generator
	llvm::Value* get_constant_by_scalar(llvm::Type* ty, llvm::Value* scalar);
	llvm::Value* get_vector(llvm::ArrayRef<llvm::Value*> const& elements);

	template <typename T>
	llvm::APInt apint(T v)
	{
		return llvm::APInt( sizeof(v) << 3, static_cast<uint64_t>(v), boost::is_signed<T>::value );
	}
	template <typename T>
	llvm::ConstantInt* get_int( T v, EFLIB_ENABLE_IF_PRED1(is_integral, T) )
	{
		return llvm::ConstantInt::get( context_, apint(v) );
	}
	llvm::Value* get_int(llvm::Type* ty, llvm::APInt const& v)
	{
		return get_constant_by_scalar( ty, llvm::ConstantInt::get(context_, v) );
	}
	
	template <typename U, typename T>
	llvm::ConstantVector* get_vector(llvm::ArrayRef<T> const& elements, EFLIB_ENABLE_IF_PRED1(is_floating_point, T) );

	template <typename U, typename T>
	llvm::Constant* get_vector(
		llvm::ArrayRef<T> const& elements,
		EFLIB_ENABLE_IF_PRED1(is_integral, T) )
	{
		assert( !elements.empty() );
		std::vector<llvm::Constant*> constant_elems( elements.size() );
		for( size_t i = 0; i < elements.size(); ++i ){
			constant_elems[i] = get_int( U(elements[i]) );
		}

		return llvm::ConstantVector::get(constant_elems);
	}

	llvm::Value* get_struct(llvm::Type* ty, llvm::ArrayRef<llvm::Value*> const& elements);

private:
	cg_extension& operator = (cg_extension const&);
	
	llvm::Value* promote_to_unary_sv_impl(
		llvm::Value* v, unary_intrin_functor sfn, unary_intrin_functor vfn, unary_intrin_functor simd_fn );
	llvm::Value* promote_to_binary_sv_impl(
		llvm::Value* lhs, llvm::Value* rhs,
		binary_intrin_functor sfn, binary_intrin_functor vfn, binary_intrin_functor simd_fn );

	bool initialize_external_intrinsics();

	llvm::DefaultIRBuilder* builder_;
	llvm::Module*			module_;
	llvm::LLVMContext&		context_;

	llvm::BasicBlock*		alloc_point_;

	llvm::Function*			externals_[externals::count];
	llvm_intrin_cache		intrins_cache_;
};

END_NS_SASL_CODE_GENERATOR();

#endif