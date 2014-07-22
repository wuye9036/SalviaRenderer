#include <sasl/include/codegen/cg_extension.h>

#include <sasl/include/codegen/ty_cache.h>
#include <sasl/enums/enums_utility.h>
#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/TypeBuilder.h>
#include <llvm/Support/CFG.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Intrinsics.h>
#include <eflib/include/platform/enable_warnings.h>

#include <vector>

using sasl::utility::vector_of;
using llvm::APInt;
using llvm::Argument;
using llvm::ArrayRef;
using llvm::AllocaInst;
using llvm::BasicBlock;
using llvm::cast;
using llvm::CallInst;
using llvm::Constant;
using llvm::DefaultIRBuilder;
using llvm::Function;
using llvm::FunctionType;
using llvm::IntegerType;
using llvm::Instruction;
using llvm::LLVMContext;
using llvm::Module;
using llvm::PHINode;
using llvm::Type;
using llvm::Twine;
using llvm::UndefValue;
using llvm::Value;
using llvm::VectorType;
using std::vector;

static int const SASL_SIMD_ELEMENT_COUNT = 4;

BEGIN_NS_SASL_CODEGEN();

cg_extension::cg_extension( DefaultIRBuilder* builder, LLVMContext& context, Module* module, size_t parallel_factor )
	: builder_(builder), context_(context), module_(module), alloc_point_(NULL)
{
	initialize_external_intrinsics(parallel_factor);
}

value_array cg_extension::call_binary_intrin(
	Type* ret_ty,
	value_array const& lhs, value_array const& rhs,
	binary_intrin_functor sv_fn, unary_intrin_functor cast_result_sv_fn
	)
{
	assert( valid_all(lhs) );
	assert( valid_all(rhs) );
	assert( lhs.size() == rhs.size() );
	assert( sv_fn );

	size_t parallel_factor = lhs.size();
	value_array ret(parallel_factor, NULL);
	for(size_t value_index = 0; value_index < parallel_factor; ++value_index)
	{
		ret[value_index] = call_binary_intrin_mono(
			ret_ty, lhs[value_index], rhs[value_index], sv_fn, cast_result_sv_fn
			);
	}
	return ret;
}

Value* cg_extension::call_binary_intrin_mono(
	Type* ret_ty,
	Value* lhs, Value* rhs,
	binary_intrin_functor sv_fn, unary_intrin_functor cast_result_sv_fn
	)
{
	Type* ty = lhs->getType();
	if( !ret_ty ) ret_ty = ty;

	if( !ty->isAggregateType() )
	{
		Value* ret_v = sv_fn(lhs, rhs);
		assert(ret_v);
		if( cast_result_sv_fn ){ ret_v = cast_result_sv_fn(ret_v); }
		return ret_v;
	}

	if( ty->isStructTy() )
	{
		Value* ret = UndefValue::get(ret_ty);
		size_t elem_count = ty->getStructNumElements();
		unsigned int elem_index[1] = {0};
		for( unsigned int i = 0;i < elem_count; ++i )
		{
			elem_index[0] = i;
			Value* lhs_elem = builder_->CreateExtractValue(lhs, elem_index);
			Value* rhs_elem = builder_->CreateExtractValue(rhs, elem_index);
			Type* ret_elem_ty = ret_ty->getStructElementType(i);
			Value* ret_elem = call_binary_intrin_mono(ret_elem_ty, lhs_elem, rhs_elem, sv_fn, cast_result_sv_fn);
			ret = builder_->CreateInsertValue(ret, ret_elem, elem_index);
		}
		return ret;
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

value_array cg_extension::call_unary_intrin( Type* ret_ty, value_array const& v, unary_intrin_functor sv_fn )
{
	assert( valid_all(v) );
	assert( sv_fn );
	size_t parallel_factor = v.size();
	value_array ret(parallel_factor, NULL);
	for(size_t value_index = 0; value_index < parallel_factor; ++value_index)
	{
		ret[value_index] = call_unary_intrin(ret_ty, v[value_index], sv_fn);
	}
	return ret;
}

Value* cg_extension::call_unary_intrin( Type* ret_ty, Value* v, unary_intrin_functor sv_fn )
{
	Type* ty = v->getType();
	ret_ty = ret_ty ? ret_ty : ty;

	if( !ty->isAggregateType() )
	{
		return sv_fn(v);
	}

	if( ty->isStructTy() )
	{
		Value* ret = UndefValue::get(ret_ty);
		size_t elem_count = ty->getStructNumElements();
		unsigned int elem_index[1] = {0};
		for( unsigned int i = 0;i < elem_count; ++i )
		{
			elem_index[0] = i;
			Value* v_elem = builder_->CreateExtractValue(v, elem_index);
			Value* ret_elem = call_unary_intrin(ret_ty->getStructElementType(i), v_elem, sv_fn);
			ret = builder_->CreateInsertValue( ret, ret_elem, elem_index );
		}
		return ret;
	}

	assert(false);
	return NULL;
}

unary_intrin_functor cg_extension::bind_to_unary( Function* fn )
{ 
	return [this, fn](Value* v) -> CallInst* { return builder_->CreateCall(fn, v); };
}

binary_intrin_functor cg_extension::bind_to_binary( Function* fn )
{
	return [this, fn](Value* v0, Value* v1) -> CallInst* { return builder_->CreateCall2(fn, v0, v1); };
}

unary_intrin_functor cg_extension::bind_external_to_unary( Function* fn )
{
	return [this, fn](Value* v) -> Value* { return call_external_1(fn, v); };
}

binary_intrin_functor cg_extension::bind_external_to_binary( Function* fn )
{
	return [this, fn](Value* v0, Value* v1) -> Value* { return call_external_2(fn, v0, v1); };
}

binary_intrin_functor cg_extension::bind_external_to_binary( externals::id id )
{
	return bind_external_to_binary(externals_[id]);
}

unary_intrin_functor cg_extension::bind_cast_sv( Type* elem_ty, cast_ops::id op )
{
	return [this, elem_ty, op] (Value* v) -> Value* { return cast_sv(v, elem_ty, op); };
}

unary_intrin_functor cg_extension::promote_to_unary_sv( unary_intrin_functor sfn, unary_intrin_functor vfn, unary_intrin_functor simd_fn )
{
	return [this, sfn, vfn, simd_fn] (Value* v) -> Value* { return promote_to_unary_sv_impl(v, sfn, vfn, simd_fn); };
}

binary_intrin_functor cg_extension::promote_to_binary_sv( binary_intrin_functor sfn, binary_intrin_functor vfn, binary_intrin_functor simd_fn )
{
	return [this, sfn, vfn, simd_fn] (Value* v0, Value* v1) -> Value* { return promote_to_binary_sv_impl(v0, v1, sfn, vfn, simd_fn); };
}

Function* cg_extension::vm_intrin( int intrin_id )
{
	return intrins_cache_.get( llvm::Intrinsic::ID(intrin_id), module_ );
}

Function* cg_extension::vm_intrin(int intrin_id, FunctionType* ty)
{
	return intrins_cache_.get(llvm::Intrinsic::ID(intrin_id), module_, ty);
}

Function* cg_extension::external(externals::id id)
{
	return externals_[id];
}

Value* cg_extension::safe_idiv_imod_sv( Value* lhs, Value* rhs, binary_intrin_functor div_or_mod_sv )
{
	Type* rhs_ty = rhs->getType();
	Type* rhs_scalar_ty = rhs_ty->getScalarType();
	assert( rhs_scalar_ty->isIntegerTy() );

	Value* zero = Constant::getNullValue( rhs_ty );
	Value* is_zero = builder_->CreateICmpEQ( rhs, zero );
	Value* one_value = Constant::getIntegerValue( rhs_ty, APInt(rhs_scalar_ty->getIntegerBitWidth(), 1) );
	Value* non_zero_rhs = builder_->CreateSelect( is_zero, one_value, rhs );

	return div_or_mod_sv( lhs, non_zero_rhs );
}

Value* cg_extension::abs_sv( Value* v )
{
	Type* ty = v->getType();
	assert( !ty->isAggregateType() );

	Type*    elem_ty   = NULL;
	unsigned elem_size = 0;

	if( ty->isVectorTy() )
	{
		elem_ty   = ty->getVectorElementType();
		elem_size = ty->getVectorNumElements();
	}
	else
	{
		elem_ty   = ty;
		elem_size = 1;
	}
	
	if( ty->isFPOrFPVectorTy() )
	{
		Type* elem_int_ty = NULL;
		uint64_t mask = 0;
		if( elem_ty->isFloatTy() )
		{
			elem_int_ty = Type::getInt32Ty( context_ );
			mask = (1ULL << 31) - 1;
		}
		else if ( elem_ty->isDoubleTy() )
		{
			elem_int_ty = Type::getInt32Ty( context_ );
			mask = (1ULL << 63) - 1;
		}
		else
		{
			EFLIB_ASSERT_UNIMPLEMENTED();
		}

		Type* int_ty = ty->isVectorTy() ? VectorType::get(elem_int_ty, elem_size) : elem_int_ty;
		Value* i = builder_->CreateBitCast( v, int_ty );
		i = builder_->CreateAnd(i, mask);
		return builder_->CreateBitCast(i, ty);
	}
	else
	{
		Value* sign = builder_->CreateICmpSGT( v, Constant::getNullValue( v->getType() ) );
		Value* neg = builder_->CreateNeg( v );
		return builder_->CreateSelect(sign, v, neg);
	}
}

Value* cg_extension::shrink( Value* vec, size_t vsize )
{
	return extract_elements(vec, 0, vsize);
}

Value* cg_extension::extract_elements( Value* src, size_t start_pos, size_t length )
{
	VectorType* vty = llvm::cast<VectorType>(src->getType());
	uint32_t elem_count = vty->getNumElements();
	if( start_pos == 0 && length == elem_count ){
		return src;
	}

	vector<int> indexes;
	indexes.resize( length, 0 );
	for ( size_t i_elem = 0; i_elem < length; ++i_elem ){
		indexes[i_elem] = static_cast<int>(i_elem + start_pos);
	}
	Value* mask = get_vector<int>( ArrayRef<int>(indexes) );
	return builder_->CreateShuffleVector( src, UndefValue::get( src->getType() ), mask );
}

Value* cg_extension::insert_elements( Value* dst, Value* src, size_t start_pos )
{
	if( src->getType() == dst->getType() ){
		return src;
	}

	VectorType* src_ty = llvm::cast<VectorType>(src->getType());
	uint32_t count = src_ty->getNumElements();

	// Expand source to dest size
	Value* ret = dst;
	for( size_t i_elem = 0; i_elem < count; ++i_elem ){
		Value* src_elem = builder_->CreateExtractElement( src, get_int(i_elem) );
		ret = builder_->CreateInsertElement( ret, src_elem, get_int(i_elem+start_pos) );
	}
	
	return ret;
}

Value* cg_extension::i8toi1_sv( Value* v )
{
	Type* ty = IntegerType::get( v->getContext(), 1 );
	if( v->getType()->isVectorTy() )
	{
		ty = VectorType::get( ty, llvm::cast<VectorType>(v->getType())->getNumElements() );
	}

	return builder_->CreateTruncOrBitCast(v, ty);
}

value_array cg_extension::i8toi1_sv(value_array const& v)
{
	assert( valid_all(v) );
	value_array ret(v.size(), NULL);
	for(size_t value_index = 0; value_index < v.size(); ++value_index)
	{
		ret[value_index] = i8toi1_sv(v[value_index]);
	}
	return ret;
}

Value* cg_extension::i1toi8_sv( Value* v )
{
	Type* ty = IntegerType::get( v->getContext(), 8 );
	if( v->getType()->isVectorTy() )
	{
		ty = VectorType::get( ty, llvm::cast<VectorType>(v->getType())->getNumElements() );
	}

	return builder_->CreateZExtOrBitCast(v, ty);
}

Value* cg_extension::call_external_1(Function* f, Value* v)
{
	Argument* ret_arg = f->getArgumentList().begin();
	Type* ret_ty = ret_arg->getType()->getPointerElementType();
	Value* tmp = stack_alloc(ret_ty, "tmp");
	builder_->CreateCall2( f, tmp, v );
	return builder_->CreateLoad( tmp );
}

Value* cg_extension::call_external_2( Function* f, Value* v0, Value* v1 )
{
	Argument* ret_arg = f->getArgumentList().begin();
	Type* ret_ty = ret_arg->getType()->getPointerElementType();
	Value* tmp = stack_alloc(ret_ty, "tmp");
	builder_->CreateCall3( f, tmp, v0, v1 );
	return builder_->CreateLoad( tmp );
}

Value* cg_extension::cast_sv( Value* v, Type* ty, cast_ops::id op )
{
	Type* elem_ty = ty->isVectorTy() ? ty->getVectorElementType() : ty;
	assert( !v->getType()->isAggregateType() );
	Type* ret_ty
		= v->getType()->isVectorTy()
		? VectorType::get( elem_ty, v->getType()->getVectorNumElements() )
		: elem_ty;

	Instruction::CastOps llvm_op = Instruction::BitCast;
	switch ( op )
	{
	case cast_ops::f2u:
		llvm_op = Instruction::FPToUI;
		break;
	case cast_ops::f2i:
		llvm_op = Instruction::FPToSI;
		break;
	case cast_ops::u2f:
		llvm_op = Instruction::UIToFP;
		break;
	case cast_ops::i2f:
		llvm_op = Instruction::SIToFP;
		break;
	case cast_ops::bitcast:
		llvm_op = Instruction::BitCast;
		break;
	case cast_ops::i2i_signed:
		return builder_->CreateIntCast( v, ret_ty, true );
	case cast_ops::i2i_unsigned:
		return builder_->CreateIntCast( v, ret_ty, false);
	default:
		assert(false);
	}

	return builder_->CreateCast( llvm_op, v, ret_ty );
}

Value* cg_extension::select( Value* flag, Value* v0, Value* v1 )
{
	Type* flag_ty = flag->getType();

	if( !flag_ty->isAggregateType() )
	{
		Value* flag_i1 = flag;
		if( !flag_ty->isIntegerTy(1) ) { flag_i1 = i8toi1_sv(flag); }
		return builder_->CreateSelect(flag_i1, v0, v1);
	}
	else if( flag_ty->isStructTy() )
	{
		Value* ret = UndefValue::get( v0->getType() );
		size_t elem_count = flag_ty->getStructNumElements();
		unsigned int elem_index[1] = {0};
		for( unsigned int i = 0; i < elem_count; ++i )
		{
			elem_index[0] = i;
			Value* v0_elem   = builder_->CreateExtractValue(v0, elem_index);
			Value* v1_elem   = builder_->CreateExtractValue(v1, elem_index);
			Value* flag_elem = builder_->CreateExtractValue(flag, elem_index);
			Value* ret_elem = select(flag_elem, v0_elem, v1_elem);
			ret = builder_->CreateInsertValue( ret, ret_elem, elem_index );
		}
		return ret;
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

value_array cg_extension::select(
	value_array const& flag, value_array const& v0, value_array const& v1
	)
{
	assert( valid_all(flag) );
	assert( valid_all(v0) );
	assert( valid_all(v1) );
	assert( flag.size() == v0.size() );
	assert( v0.size() == v1.size() );

	value_array ret(flag.size(), NULL);
	for(size_t value_index = 0; value_index < flag.size(); ++value_index)
	{
		ret[value_index] = select(flag[value_index], v0[value_index], v1[value_index]);
	}
	return ret;
}

value_array cg_extension::select(
	Value* mask, value_array const& v0, value_array const& v1
	)
{
	assert( mask != NULL );
	assert( valid_all(v0) );
	assert( valid_all(v1) );
	assert( v0.size() == v1.size() );

	value_array ret(v0.size(), NULL);
	for(size_t value_index = 0; value_index < v0.size(); ++value_index)
	{
		uint32_t flag_mask = (1U << value_index);
		Value* flag = builder_->CreateICmpNE( builder_->CreateAnd(mask, flag_mask), get_int(0) );
		ret[value_index] = builder_->CreateSelect(flag, v0[value_index], v1[value_index]);
	}
	return ret;
}

Type* cg_extension::extract_scalar_type( Type* ty )
{
	if( ty->isVectorTy() )
	{
		return ty->getVectorElementType();
	}
	else if( ty->isAggregateType() )
	{
		if( ty->isStructTy() )
		{
			assert( ty->getStructNumElements() > 0 );
			return extract_scalar_type( ty->getStructElementType(0) );
		}
		else
		{
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	}

	return ty;
}

PHINode* cg_extension::phi( BasicBlock* b0, Value* v0, BasicBlock* b1, Value* v1 )
{
	PHINode* phi = builder_->CreatePHI( v0->getType(), 2 );
	phi->addIncoming(v0, b0);
	phi->addIncoming(v1, b1);
	return phi;
}

Value* cg_extension::get_constant_by_scalar( Type* ty, Value* scalar )
{
	Type* scalar_ty = scalar->getType();
	assert( !scalar_ty->isAggregateType() && !scalar_ty->isVectorTy() );
	if ( ty->isVectorTy() )
	{
		// Vector
		unsigned vector_size = ty->getVectorNumElements();
		assert( ty->getVectorElementType() == scalar_ty );

		vector<Value*> scalar_values(vector_size, scalar);
		return get_vector( ArrayRef<Value*>(scalar_values) );
	}
	else if ( !ty->isAggregateType() )
	{
		assert(ty == scalar_ty);
		return scalar;
	}
	else if ( ty->isStructTy() )
	{
		// Struct
		vector<Value*> elem_values;
		for(unsigned i = 0; i < ty->getStructNumElements(); ++i)
		{
			elem_values.push_back(
				get_constant_by_scalar(ty->getStructElementType(i), scalar)
				);
		}
		return get_struct( ty, ArrayRef<Value*>(elem_values) );
	}
	else
	{
		EFLIB_ASSERT_UNIMPLEMENTED();
		return NULL;
	}
}

Value* cg_extension::get_vector( ArrayRef<Value*> const& elements )
{
	assert( !elements.empty() );
	Type* vector_ty = VectorType::get(
		elements.front()->getType(), static_cast<uint32_t>( elements.size() )
		);
	Value* ret = UndefValue::get(vector_ty);

	for(unsigned i = 0; i < elements.size(); ++i)
	{
		ret = builder_->CreateInsertElement( ret, elements[i], get_int(i) );
	}

	return ret;
}

Value* cg_extension::get_struct( Type* ty, ArrayRef<Value*> const& elements )
{
	assert( !elements.empty() );
	Value* ret = UndefValue::get(ty);

	for(unsigned i = 0; i < elements.size(); ++i)
	{
		unsigned indexes[] = {i};
		ret = builder_->CreateInsertValue(ret, elements[i], indexes);
	}

	return ret;
}

Value* cg_extension::get_array (ArrayRef<Value*> const& elements)
{
	ArrayType* arr_type = ArrayType::get( elements[0]->getType(), elements.size() );
	Value* return_val = UndefValue::get(arr_type);
	uint32_t indexes[1] = {0};
	for(; indexes[0] < elements.size(); ++indexes[0])
	{
		return_val = builder_->CreateInsertValue(return_val, elements[ indexes[0] ], indexes);
	}
	return return_val;
}

value_array cg_extension::split_array(llvm::Value* v)
{
	Type* ty = v->getType();
	assert( ty->isArrayTy() );
	size_t parallel_factor = ty->getArrayNumElements();
	value_array ret(parallel_factor, NULL);
	unsigned value_index[] = {0};
	for(; value_index[0] < parallel_factor; ++value_index[0])
	{
		ret[ value_index[0] ] = builder_->CreateExtractValue(v, value_index);
	}
	return ret;
}

value_array cg_extension::split_array_ref(Value* v)
{
	return split_array( builder_->CreateLoad(v) );
}

Value* cg_extension::promote_to_binary_sv_impl(Value* lhs, Value* rhs,
		binary_intrin_functor sfn, binary_intrin_functor vfn, binary_intrin_functor simd_fn )
{
	Type* ty = lhs->getType();

	if( !ty->isAggregateType() && !ty->isVectorTy() )
	{
		assert(sfn);
		return sfn(lhs, rhs);
	}

	if( ty->isVectorTy() )
	{
		if(vfn) { return vfn(lhs, rhs); }

		unsigned elem_count = ty->getVectorNumElements();

		// SIMD
		if( simd_fn && elem_count % SASL_SIMD_ELEMENT_COUNT == 0 )
		{
			int batch_count = elem_count / SASL_SIMD_ELEMENT_COUNT;

			Value* ret = NULL;
			for( int i_batch = 0; i_batch < batch_count; ++i_batch ){
				Value* lhs_simd_elem = extract_elements( lhs, i_batch*SASL_SIMD_ELEMENT_COUNT, SASL_SIMD_ELEMENT_COUNT );
				Value* rhs_simd_elem = extract_elements( rhs, i_batch*SASL_SIMD_ELEMENT_COUNT, SASL_SIMD_ELEMENT_COUNT );
				Value* ret_simd_elem = simd_fn( lhs_simd_elem, rhs_simd_elem );
				if(!ret)
				{
					Type* result_element_ty = ret_simd_elem->getType()->getVectorElementType();
					unsigned result_elements_count = lhs->getType()->getVectorNumElements();
					Type* result_ty = VectorType::get(result_element_ty, result_elements_count);
					ret = UndefValue::get(result_ty);
				}
				ret = insert_elements( ret, ret_simd_elem, i_batch*SASL_SIMD_ELEMENT_COUNT );
			}

			return ret;
		}

		// Scalar
		assert( sfn );
		Value* ret = NULL;
		for( unsigned i = 0; i < elem_count; ++i )
		{
			Value* lhs_elem = builder_->CreateExtractElement( lhs, get_int(i) );
			Value* rhs_elem = builder_->CreateExtractElement( rhs, get_int(i) );
			Value* ret_elem = sfn(lhs_elem, rhs_elem);
			if(!ret)
			{
				unsigned result_elements_count = lhs->getType()->getVectorNumElements();
				Type* result_ty = VectorType::get(ret_elem->getType(), result_elements_count);
				ret = UndefValue::get(result_ty);
			}
			ret = builder_->CreateInsertElement( ret, ret_elem, get_int(i) );
		}

		return ret;
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

Value* cg_extension::promote_to_unary_sv_impl(Value* v,
		unary_intrin_functor sfn, unary_intrin_functor vfn, unary_intrin_functor simd_fn )
{
	Type* ty = v->getType();

	if( !ty->isAggregateType() && !ty->isVectorTy() )
	{
		assert(sfn);
		return sfn(v);
	}

	if( ty->isVectorTy() )
	{
		if(vfn) { return vfn(v); }

		unsigned elem_count = ty->getVectorNumElements();

		// SIMD
		if( simd_fn && elem_count % SASL_SIMD_ELEMENT_COUNT == 0 )
		{
			int batch_count = elem_count / SASL_SIMD_ELEMENT_COUNT;

			Value* ret = NULL;
			for( int i_batch = 0; i_batch < batch_count; ++i_batch ){
				Value* v_simd_elem = extract_elements( v, i_batch*SASL_SIMD_ELEMENT_COUNT, SASL_SIMD_ELEMENT_COUNT );
				Value* ret_simd_elem = simd_fn(v_simd_elem);
				if(!ret)
				{
					Type* result_element_ty = ret_simd_elem->getType()->getVectorElementType();
					unsigned result_elements_count = v->getType()->getVectorNumElements();
					Type* result_ty = VectorType::get(result_element_ty, result_elements_count);
					ret = UndefValue::get(result_ty);
				}
				ret = insert_elements( ret, ret_simd_elem, i_batch*SASL_SIMD_ELEMENT_COUNT );
			}

			return ret;
		}

		// Scalar
		assert( sfn );
		Value* ret = NULL;
		for( unsigned i = 0; i < elem_count; ++i )
		{
			Value* v_elem = builder_->CreateExtractElement( v, get_int(i) );
			Value* ret_elem = sfn(v_elem);
			if(!ret)
			{
				unsigned result_elements_count = v->getType()->getVectorNumElements();
				Type* result_ty = VectorType::get(ret_elem->getType(), result_elements_count);
				ret = UndefValue::get(result_ty);
			}
			ret = builder_->CreateInsertElement( ret, ret_elem, get_int(i) );
		}

		return ret;
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
}

void cg_extension::set_stack_alloc_point(BasicBlock* alloc_point)
{
	alloc_point_ = alloc_point;
}

AllocaInst* cg_extension::stack_alloc(Type* ty, Twine const& name)
{
	assert(alloc_point_);
	return new AllocaInst(ty, NULL, name, alloc_point_);
}

value_array cg_extension::stack_alloc(Type* ty, size_t parallel_factor, llvm::Twine const& name)
{
	value_array ret(parallel_factor, NULL);
	for(size_t value_index = 0; value_index < parallel_factor; ++value_index)
	{
		ret[value_index] = new AllocaInst(ty, NULL, name, alloc_point_);
	}
	return ret;
}

value_array cg_extension::extract_element(value_array const& agg, value_array const& index)
{
	assert( agg.size() == index.size() );
	assert( valid_all(agg) );
	assert( valid_all(index) );
	value_array ret(agg.size(), NULL);

	for(size_t value_index = 0; value_index < agg.size(); ++value_index)
	{
		ret[value_index] = builder_->CreateExtractElement(agg[value_index], index[value_index]);
	}
	return ret;
}

value_array cg_extension::extract_value(value_array const& agg, uint32_t index)
{
	assert( valid_all(agg) );
	value_array ret(agg.size(), NULL);
	for(size_t value_index = 0; value_index < agg.size(); ++value_index)
	{
		ret[value_index] = builder_->CreateExtractValue(agg[value_index], index);
	}
	return ret;
}

value_array cg_extension::gep(value_array const& agg_addr, value_array const& index)
{
	assert( agg_addr.size() == index.size() );
	assert( valid_all(agg_addr) );
	assert( valid_all(index) );
	
	value_array ret(agg_addr.size(), NULL);
	for(size_t value_index = 0; value_index < agg_addr.size(); ++value_index)
	{
		ret[value_index] = builder_->CreateGEP(agg_addr[value_index], index[value_index] );
	}
	return ret;
}

value_array cg_extension::struct_gep(value_array const& agg, uint32_t index)
{
	assert( valid_all(agg) );
	
	value_array ret(agg.size(), NULL);
	for(size_t value_index = 0; value_index < agg.size(); ++value_index)
	{
		ret[value_index] = builder_->CreateStructGEP(agg[value_index], index);
	}
	return ret;
}

value_array cg_extension::load(value_array const& addr)
{
	assert( valid_all(addr) );
	value_array ret(addr.size(), NULL);
	for(size_t value_index = 0; value_index < addr.size(); ++value_index)
	{
		ret[value_index] = builder_->CreateLoad(addr[value_index]);
	}
	return ret;
}

value_array cg_extension::store(value_array const& values, value_array const& addr)
{
	assert( values.size() == addr.size() );
	assert( valid_all(addr) );
	assert( valid_all(values) );
	
	value_array ret(values.size(), NULL);
	for(size_t value_index = 0; value_index < values.size(); ++value_index)
	{
		ret[value_index] = builder_->CreateStore(values[value_index], addr[value_index]);
	}
	return ret;
}

value_array cg_extension::call(
	value_array const& fn, ArrayRef<value_array> const& args
	)
{	
	value_array ret(fn.size(), NULL);
	vector<Value*> arg_values(args.size(), NULL);
	for(size_t value_index = 0; value_index < fn.size(); ++value_index)
	{
		for(size_t i_arg = 0; i_arg < args.size(); ++i_arg)
		{
			arg_values[i_arg] = args[i_arg][value_index];
		}
		ret[value_index] = builder_->CreateCall(fn[value_index], arg_values);
	}
	return ret;
}

value_array cg_extension::call(
	value_array const& fn, ArrayRef<value_array const*> const& args
	)
{	
	value_array ret(fn.size(), NULL);
	vector<Value*> arg_values(args.size(), NULL);
	for(size_t value_index = 0; value_index < fn.size(); ++value_index)
	{
		for(size_t i_arg = 0; i_arg < args.size(); ++i_arg)
		{
			arg_values[i_arg] = (*args[i_arg])[value_index];
		}
		ret[value_index] = builder_->CreateCall(fn[value_index], arg_values);
	}
	return ret;
}

value_array cg_extension::shuffle_vector(value_array const& v1, value_array const& v2, value_array const& mask)
{
	assert( valid_all(v1) );
	assert( valid_all(v2) );
	assert( valid_all(mask) );
	assert( v1.size() == v2.size() );
	assert( v1.size() == mask.size() );
	size_t parallel_factor = v1.size();

	value_array ret(parallel_factor, NULL);
	for(size_t value_index = 0; value_index < parallel_factor; ++value_index)
	{
		ret[value_index] = builder_->CreateShuffleVector(v1[value_index], v2[value_index], mask[value_index]);
	}
	return ret;
}

using namespace externals;

bool cg_extension::initialize_external_intrinsics(size_t /*parallel_factor*/)
{
	// Get types used in external intrinsic registration.
	builtin_types v4f32_hint = vector_of( builtin_types::_float, 4 );
	builtin_types v3f32_hint = vector_of( builtin_types::_float, 3 );
	builtin_types v2f32_hint = vector_of( builtin_types::_float, 2 );

	Type* void_ty		= Type::getVoidTy( context_ );
	Type* u32_ty		= Type::getInt32Ty( context_ );
	Type* f32_ty		= Type::getFloatTy( context_ );
	Type* samp_ty		= Type::getInt8PtrTy( context_ );
	Type* f32ptr_ty		= Type::getFloatPtrTy( context_ );
	Type* u32ptr_ty		= Type::getInt32PtrTy( context_ );
	
	Type* v4f32_ty		= get_llvm_type(context_, v4f32_hint, abis::llvm);
	Type* v3f32_ty		= get_llvm_type(context_, v3f32_hint, abis::llvm);
	Type* v2f32_ty		= get_llvm_type(context_, v2f32_hint, abis::llvm);

	FunctionType* f_f = NULL;
	{
		Type* arg_tys[2] = { f32ptr_ty, f32_ty };
		f_f = FunctionType::get( void_ty, arg_tys, false );
	}

	FunctionType* f_ff = NULL;
	{
		Type* arg_tys[3] = { f32ptr_ty, f32_ty, f32_ty };
		f_ff = FunctionType::get( void_ty, arg_tys, false );
	}

	FunctionType* vs_texlod_ty = NULL;
	{
		Type* arg_tys[3] =
		{
			PointerType::getUnqual( v4f32_ty ),	/*Pixel*/
			samp_ty,							/*Sampler*/
			PointerType::getUnqual( v4f32_ty )	/*Coords(x, y, _, lod)*/
		};
		vs_texlod_ty = FunctionType::get( void_ty, arg_tys, false );
	}

	FunctionType* ps_texlod_ty = NULL;
	{
		Type* arg_tys[4] =
		{
			PointerType::getUnqual( v4f32_ty ),	/*Pixels*/
			u32_ty,								/*Mask*/
			samp_ty,							/*Sampler*/
			PointerType::getUnqual( v4f32_ty )	/*Coords(x, y, _, lod)*/
		};
		ps_texlod_ty = FunctionType::get( void_ty, arg_tys, false );
	}

	FunctionType* ps_tex2dgrad_ty = NULL;
	{
		Type* arg_tys[6] =
		{
			PointerType::getUnqual( v4f32_ty ),	/*Pixels*/
			u32_ty,								/*Mask*/
			samp_ty,							/*Sampler*/
			PointerType::getUnqual( v2f32_ty ),	/*Coords(x, y)*/
			PointerType::getUnqual( v2f32_ty ),	/*ddx*/
			PointerType::getUnqual( v2f32_ty ),	/*ddy*/
		};
		ps_tex2dgrad_ty = FunctionType::get( void_ty, arg_tys, false );
	}

	FunctionType* ps_texCUBEgrad_ty = NULL;
	{
		Type* arg_tys[6] =
		{
			PointerType::getUnqual( v4f32_ty ),	/*Pixels*/
			u32_ty,								/*Mask*/
			samp_ty,							/*Sampler*/
			PointerType::getUnqual( v3f32_ty ),	/*Coords(x, y)*/
			PointerType::getUnqual( v3f32_ty ),	/*ddx*/
			PointerType::getUnqual( v3f32_ty ),	/*ddy*/
		};
		ps_texCUBEgrad_ty = FunctionType::get( void_ty, arg_tys, false );
	}

	FunctionType* ps_tex2dbias_ty = NULL;
	{
		Type* arg_tys[6] =
		{
			PointerType::getUnqual( v4f32_ty ),	/*Pixels*/
			u32_ty,								/*Mask*/
			samp_ty,							/*Sampler*/
			PointerType::getUnqual( v4f32_ty ),	/*Coords(x, y, _, bias)*/
			PointerType::getUnqual( v2f32_ty ),	/*ddx*/
			PointerType::getUnqual( v2f32_ty ),	/*ddy*/
		};
		ps_tex2dbias_ty = FunctionType::get( void_ty, arg_tys, false );
	}

	FunctionType* ps_texCUBEbias_ty = NULL;
	{
		Type* arg_tys[6] =
		{
			PointerType::getUnqual( v4f32_ty ),	/*Pixels*/
			u32_ty,								/*Mask*/
			samp_ty,							/*Sampler*/
			PointerType::getUnqual( v4f32_ty ),	/*Coords(x, y, _, bias)*/
			PointerType::getUnqual( v3f32_ty ),	/*ddx*/
			PointerType::getUnqual( v3f32_ty ),	/*ddy*/
		};
		ps_texCUBEbias_ty = FunctionType::get( void_ty, arg_tys, false );
	}

	FunctionType* ps_texproj_ty = NULL;
	{
		Type* arg_tys[6] =
		{
			PointerType::getUnqual( v4f32_ty ),	/*Pixels*/
			u32_ty,								/*Mask*/
			samp_ty,							/*Sampler*/
			PointerType::getUnqual( v4f32_ty ),	/*Coords(x, y, _, proj)*/
			PointerType::getUnqual( v4f32_ty ),	/*ddx*/
			PointerType::getUnqual( v4f32_ty ),	/*ddy*/
		};
		ps_texproj_ty = FunctionType::get( void_ty, arg_tys, false );
	}

	FunctionType* u32_u32_ty = NULL;
	{
		Type* arg_tys[2] = { u32ptr_ty, u32_ty };
		u32_u32_ty = FunctionType::get(void_ty, arg_tys, false);
	}

	externals_[exp_f32]		= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.exp.f32", module_ );
	externals_[exp2_f32]	= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.exp2.f32", module_ );
	externals_[sin_f32]		= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.sin.f32", module_ );
	externals_[cos_f32]		= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.cos.f32", module_ );
	externals_[tan_f32]		= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.tan.f32", module_ );
	externals_[asin_f32]	= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.asin.f32", module_ );
	externals_[acos_f32]	= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.acos.f32", module_ );
	externals_[atan_f32]	= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.atan.f32", module_ );
	externals_[ceil_f32]	= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.ceil.f32", module_ );
	externals_[floor_f32]	= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.floor.f32", module_ );
	externals_[round_f32]	= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.round.f32", module_ );
	externals_[trunc_f32]	= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.trunc.f32", module_ );
	externals_[log_f32]		= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.log.f32", module_ );
	externals_[log2_f32]	= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.log2.f32", module_ );
	externals_[log10_f32]	= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.log10.f32", module_ );
	externals_[rsqrt_f32]	= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.rsqrt.f32", module_ );
	externals_[mod_f32]		= Function::Create(f_ff, GlobalValue::ExternalLinkage, "sasl.mod.f32", module_ );
	externals_[ldexp_f32]	= Function::Create(f_ff, GlobalValue::ExternalLinkage, "sasl.ldexp.f32", module_ );
	externals_[pow_f32]		= Function::Create(f_ff, GlobalValue::ExternalLinkage, "sasl.pow.f32", module_ );
	externals_[sinh_f32]	= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.sinh.f32", module_ );
	externals_[cosh_f32]	= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.cosh.f32", module_ );
	externals_[tanh_f32]	= Function::Create(f_f , GlobalValue::ExternalLinkage, "sasl.tanh.f32", module_ );

	externals_[countbits_u32]   = Function::Create(u32_u32_ty, GlobalValue::ExternalLinkage, "sasl.countbits.u32", module_ );
	externals_[firstbithigh_u32]= Function::Create(u32_u32_ty, GlobalValue::ExternalLinkage, "sasl.firstbithigh.u32", module_ );
	externals_[firstbitlow_u32] = Function::Create(u32_u32_ty, GlobalValue::ExternalLinkage, "sasl.firstbitlow.u32", module_ );
	externals_[reversebits_u32] = Function::Create(u32_u32_ty, GlobalValue::ExternalLinkage, "sasl.reversebits.u32", module_ );

	externals_[tex2dlod_vs]	= Function::Create(vs_texlod_ty,	GlobalValue::ExternalLinkage, "sasl.vs.tex2d.lod", module_ );
	externals_[tex2dlod_ps]	= Function::Create(ps_texlod_ty,	GlobalValue::ExternalLinkage, "sasl.ps.tex2d.lod", module_ );
	externals_[tex2dgrad_ps]= Function::Create(ps_tex2dgrad_ty,	GlobalValue::ExternalLinkage, "sasl.ps.tex2d.grad", module_ );
	externals_[tex2dbias_ps]= Function::Create(ps_tex2dbias_ty,	GlobalValue::ExternalLinkage, "sasl.ps.tex2d.bias", module_ );
	externals_[tex2dproj_ps]= Function::Create(ps_texproj_ty,	GlobalValue::ExternalLinkage, "sasl.ps.tex2d.proj", module_ );

	externals_[texCUBElod_vs]	= Function::Create(vs_texlod_ty,		GlobalValue::ExternalLinkage, "sasl.vs.texCUBE.lod", module_ );
	externals_[texCUBElod_ps]	= Function::Create(ps_texlod_ty,		GlobalValue::ExternalLinkage, "sasl.ps.texCUBE.lod", module_ );
	externals_[texCUBEgrad_ps]	= Function::Create(ps_texCUBEgrad_ty,	GlobalValue::ExternalLinkage, "sasl.ps.texCUBE.grad", module_ );
	externals_[texCUBEbias_ps]	= Function::Create(ps_texCUBEbias_ty,	GlobalValue::ExternalLinkage, "sasl.ps.texCUBE.bias", module_ );
	externals_[texCUBEproj_ps]	= Function::Create(ps_texproj_ty,		GlobalValue::ExternalLinkage, "sasl.ps.texCUBE.proj", module_ );

	return true;
}
END_NS_SASL_CODEGEN();

