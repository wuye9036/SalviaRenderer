#include <sasl/include/code_generator/llvm/cg_extension.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/IRBuilder.h>
#include <llvm/TypeBuilder.h>
#include <llvm/Support/CFG.h>
#include <llvm/Intrinsics.h>
#include <llvm/Constants.h>
#include <llvm/Instructions.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/bind.hpp>
#include <eflib/include/platform/boost_end.h>

#include <vector>

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

BEGIN_NS_SASL_CODE_GENERATOR();

cg_extension::cg_extension( DefaultIRBuilder* builder, LLVMContext& context, Module* module )
	: builder_(builder), context_(context), module_(module)
{
}

Value* cg_extension::call_binary_intrin( Type* ret_ty, Value* lhs, Value* rhs, binary_intrin_functor sv_fn, unary_intrin_functor cast_result_sv_fn )
{
	Type* ty = lhs->getType();
	if( !ret_ty ) ret_ty = ty;

	if( !ty->isAggregateType() )
	{
		Value* ret_v = NULL;
		ret_v = sv_fn(lhs, rhs);
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
			Value* ret_elem = call_binary_intrin(ret_elem_ty, lhs_elem, rhs_elem, sv_fn, cast_result_sv_fn);
			ret = builder_->CreateInsertValue(ret, ret_elem, elem_index);
		}
		return ret;
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return NULL;
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
	typedef CallInst* (DefaultIRBuilder::*call_fn)(Value*, Value*, Twine const&);
	return boost::bind( static_cast<call_fn>(&DefaultIRBuilder::CreateCall), builder_, fn, _1, "" );
}

CallInst* irbuilder_create_call2( DefaultIRBuilder* builder, Value* callee, Value* arg0, Value* arg1, Twine const& name = "")
{
	return builder->CreateCall2(callee, arg0, arg1, name);
}

binary_intrin_functor cg_extension::bind_to_binary( Function* fn )
{
	return boost::bind(&irbuilder_create_call2, builder_, fn, _1, _2, "");
}

unary_intrin_functor cg_extension::bind_external_to_unary( Function* fn )
{
	return boost::bind( &cg_extension::call_external_1, this, fn, _1 );
}

binary_intrin_functor cg_extension::bind_external_to_binary( Function* fn )
{
	return boost::bind( &cg_extension::call_external_2, this, fn, _1, _2 );
}

unary_intrin_functor cg_extension::bind_cast_sv( Type* elem_ty, cast_ops op )
{
	return boost::bind( &cg_extension::cast_sv, this, _1, elem_ty, op );
}

unary_intrin_functor cg_extension::promote_to_unary_sv( unary_intrin_functor sfn, unary_intrin_functor vfn, unary_intrin_functor simd_fn )
{
	return boost::bind(&cg_extension::promote_to_unary_sv_impl, this, _1, sfn, vfn, simd_fn);
}

binary_intrin_functor cg_extension::promote_to_binary_sv( binary_intrin_functor sfn, binary_intrin_functor vfn, binary_intrin_functor simd_fn )
{
	return boost::bind(&cg_extension::promote_to_binary_sv_impl, this, _1, _2, sfn, vfn, simd_fn);
}

Function* cg_extension::intrin( int intrin_id )
{
	return intrins_cache_.get( llvm::Intrinsic::ID(intrin_id), module_ );
}

Function* cg_extension::intrin(int intrin_id, FunctionType* ty)
{
	return intrins_cache_.get(llvm::Intrinsic::ID(intrin_id), module_, ty);
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
		indexes[i_elem] = i_elem + start_pos;
	}
	Value* mask = get_vector( ArrayRef<int>(indexes) );
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
		Value* src_elem = builder_->CreateExtractElement( src, get_int<int>(i_elem) );
		ret = builder_->CreateInsertElement( ret, src_elem, get_int<int>(i_elem+start_pos) );
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

Value* cg_extension::cast_sv( Value* v, Type* ty, cast_ops op )
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
	case cast_op_f2u:
		llvm_op = Instruction::FPToUI;
		break;
	case cast_op_f2i:
		llvm_op = Instruction::FPToSI;
		break;
	case cast_op_u2f:
		llvm_op = Instruction::UIToFP;
		break;
	case cast_op_i2f:
		llvm_op = Instruction::SIToFP;
		break;
	case cast_op_bitcast:
		llvm_op = Instruction::BitCast;
		break;
	case cast_op_i2i_signed:
		return builder_->CreateIntCast( v, ret_ty, true );
	case cast_op_i2i_unsigned:
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
	Type* vector_ty = VectorType::get( elements.front()->getType(), elements.size() );
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
	return new AllocaInst(ty, NULL, name, alloc_point_);
}

END_NS_SASL_CODE_GENERATOR();

