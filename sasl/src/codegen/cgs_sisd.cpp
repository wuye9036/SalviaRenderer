#include <sasl/include/codegen/cgs_sisd.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/codegen/cg_contexts.h>
#include <sasl/enums/enums_utility.h>
#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/platform/cpuinfo.h>
#include <eflib/include/utility/unref_declarator.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/IRBuilder.h>
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/Intrinsics.h>
#include <llvm/TypeBuilder.h>
#include <llvm/Support/CFG.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <eflib/include/platform/boost_end.h>


using sasl::syntax_tree::node;
using sasl::syntax_tree::function_type;
using sasl::syntax_tree::parameter;
using sasl::syntax_tree::tynode;
using sasl::syntax_tree::declaration;
using sasl::syntax_tree::variable_declaration;
using sasl::syntax_tree::struct_type;

using sasl::semantic::node_semantic;
using sasl::semantic::node_semantic;

using eflib::support_feature;
using eflib::cpu_sse2;

using namespace sasl::utility;

using llvm::APInt;
using llvm::Argument;
using llvm::LLVMContext;
using llvm::Function;
using llvm::FunctionType;
using llvm::IntegerType;
using llvm::Type;
using llvm::PointerType;
using llvm::Value;
using llvm::BasicBlock;
using llvm::Constant;
using llvm::ConstantInt;
using llvm::ConstantVector;
using llvm::StructType;
using llvm::VectorType;
using llvm::UndefValue;
using llvm::StoreInst;
using llvm::TypeBuilder;
using llvm::AttrListPtr;
using llvm::SwitchInst;
using llvm::CmpInst;
using llvm::PHINode;
using llvm::DefaultIRBuilder;

namespace Intrinsic = llvm::Intrinsic;

using boost::any;
using boost::shared_ptr;
using boost::enable_if;
using boost::is_integral;
using boost::unordered_map;
using boost::lexical_cast;

using std::vector;
using std::string;

BEGIN_NS_SASL_CODEGEN();

namespace {
	template <typename T> APInt apint( T v )
	{
		return APInt( sizeof(v) << 3, static_cast<uint64_t>(v), boost::is_signed<T>::value );
	}

	void mask_to_indexes( char indexes[4], uint32_t mask ){
		for( int i = 0; i < 4; ++i ){
			// XYZW is 1,2,3,4 but LLVM used 0,1,2,3
			char comp_index = static_cast<char>( (mask >> i*8) & 0xFF );
			if( comp_index == 0 ){
				indexes[i] = -1;
				break;
			}
			indexes[i] = comp_index - 1;
		}
	}

	uint32_t indexes_to_mask( char indexes[4] ){
		uint32_t mask = 0;
		for( int i = 0; i < 4; ++i ){
			mask += (uint32_t)( (indexes[i] + 1) << (i*8) );
		}
		return mask;
	}

	uint32_t indexes_to_mask( char idx0, char idx1, char idx2, char idx3 ){
		char indexes[4] = { idx0, idx1, idx2, idx3 };
		return indexes_to_mask( indexes );
	}

	void dbg_print_blocks( Function* fn ){
#ifdef _DEBUG
		/*printf( "Function: 0x%X\n", fn );
		for( Function::BasicBlockListType::iterator it = fn->getBasicBlockList().begin(); it != fn->getBasicBlockList().end(); ++it ){
		printf( "  Block: 0x%X\n", &(*it) );
		}*/
		fn = fn;
#else
		fn = fn;
#endif
	}
}

void cgs_sisd::store( cg_value& lhs, cg_value const& rhs ){
	Value* src = NULL;
	Value* address = NULL;
	value_kinds::id kind = lhs.kind();

	if( kind == value_kinds::reference ){	
		src = rhs.load( lhs.abi() );
		address = lhs.raw();
	} else if ( kind == value_kinds::elements ){
		char indexes[4] = {-1, -1, -1, -1};
		cg_value const* root = NULL;
		bool is_swizzle = merge_swizzle(root, indexes, lhs);

		if( is_swizzle && is_vector( root->hint()) ){
			assert( lhs.parent()->storable() );
			
			cg_value rhs_rvalue = rhs.to_rvalue();
			cg_value ret_v = root->to_rvalue();
			for(size_t i = 0; i < vector_size(rhs.hint()); ++i)
			{
				ret_v = emit_insert_val( ret_v, indexes[i], emit_extract_val(rhs_rvalue, i) );
			}

			src = ret_v.load( lhs.abi() );
			address = root->load_ref();
		} else {
			src = rhs.load( lhs.abi() );
			address = lhs.load_ref();
		}
	}

	assert( src && address );
	builder().CreateStore( src, address );
}

cg_value cgs_sisd::cast_ints( cg_value const& v, cg_type* dest_tyi )
{
	builtin_types hint_src = v.hint();
	builtin_types hint_dst = dest_tyi->hint();

	builtin_types scalar_hint_src = scalar_of(hint_src);

	Type* dest_ty = dest_tyi->ty(v.abi());
	Type* elem_ty = type_( scalar_of(hint_dst), abis::llvm );

	cast_ops::id op = is_signed(scalar_hint_src) ? cast_ops::i2i_signed : cast_ops::i2i_unsigned;
	unary_intrin_functor cast_sv_fn = ext_->bind_cast_sv(elem_ty, op);
	Value* val = ext_->call_unary_intrin(dest_ty, v.load(), cast_sv_fn);

	return create_value( dest_tyi, builtin_types::none, val, value_kinds::value, v.abi() );
}

cg_value cgs_sisd::cast_i2f( cg_value const& v, cg_type* dest_tyi )
{
	builtin_types hint_i = v.hint();
	builtin_types hint_f = dest_tyi->hint();

	builtin_types scalar_hint_i = scalar_of(hint_i);

	Type* dest_ty = dest_tyi->ty(v.abi());
	Type* elem_ty = type_( scalar_of(hint_f), abis::llvm );

	cast_ops::id op = is_signed(hint_i) ? cast_ops::i2f : cast_ops::u2f;
	unary_intrin_functor cast_sv_fn = ext_->bind_cast_sv(elem_ty, op);

	Value* val = ext_->call_unary_intrin(dest_ty, v.load(), cast_sv_fn);

	return create_value( dest_tyi, builtin_types::none, val, value_kinds::value, v.abi() );
}

cg_value cgs_sisd::cast_f2i( cg_value const& v, cg_type* dest_tyi )
{
	builtin_types hint_i = dest_tyi->hint();
	builtin_types hint_f = v.hint();

	builtin_types scalar_hint_i = scalar_of(hint_i);

	Type* dest_ty = dest_tyi->ty(v.abi());
	Type* elem_ty = type_( scalar_of(hint_i), abis::llvm );

	cast_ops::id op = is_signed(hint_i) ? cast_ops::f2i : cast_ops::f2u;
	unary_intrin_functor cast_sv_fn = ext_->bind_cast_sv(elem_ty, op);

	Value* val = ext_->call_unary_intrin(dest_ty, v.load(), cast_sv_fn);

	return create_value( dest_tyi, builtin_types::none, val, value_kinds::value, v.abi() );
}

cg_value cgs_sisd::cast_f2f( cg_value const& v, cg_type* dest_tyi )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return cg_value();
}

cg_value cgs_sisd::create_vector( std::vector<cg_value> const& scalars, abis::id abi ){
	builtin_types scalar_hint = scalars[0].hint();
	builtin_types hint = vector_of(scalar_hint, scalars.size());

	cg_value ret = undef_value(hint, abi);
	for( size_t i = 0; i < scalars.size(); ++i )
	{
		ret = emit_insert_val( ret, (int)i, scalars[i] );
	}
	return ret;
}

void cgs_sisd::emit_return(){
	builder().CreateRetVoid();
}

void cgs_sisd::emit_return( cg_value const& ret_v, abis::id abi ){
	if( abi == abis::unknown ){ abi = fn().abi(); }

	if( fn().first_arg_is_return_address() ){
		builder().CreateStore( ret_v.load(abi), fn().return_address() );
		builder().CreateRetVoid();
	} else {
		builder().CreateRet( ret_v.load(abi) );
	}
}

cg_value cgs_sisd::create_scalar( Value* val, cg_type* tyinfo, builtin_types hint ){
	assert( is_scalar(hint) );
	return create_value( tyinfo, hint, val, value_kinds::value, abis::llvm );
}

Value* cgs_sisd::select_( Value* cond, Value* yes, Value* no )
{
	return builder().CreateSelect( cond, yes, no );
}

bool cgs_sisd::prefer_externals() const
{
	return false;
}

bool cgs_sisd::prefer_scalar_code() const
{
	return false;
}

cg_value cgs_sisd::emit_swizzle( cg_value const& lhs, uint32_t mask )
{
	EFLIB_UNREF_DECLARATOR(lhs);
	EFLIB_UNREF_DECLARATOR(mask);

	EFLIB_ASSERT_UNIMPLEMENTED();
	return cg_value();
}

cg_value cgs_sisd::emit_write_mask( cg_value const& vec, uint32_t mask )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return cg_value();
}

void cgs_sisd::switch_to( cg_value const& cond, std::vector< std::pair<cg_value, insert_point_t> > const& cases, insert_point_t const& default_branch )
{
	Value* v = cond.load();
	SwitchInst* inst = builder().CreateSwitch( v, default_branch.block, static_cast<unsigned>(cases.size()) );
	for( size_t i_case = 0; i_case < cases.size(); ++i_case ){
		inst->addCase( llvm::cast<ConstantInt>( cases[i_case].first.load() ), cases[i_case].second.block );
	}
}

cg_value cgs_sisd::cast_i2b( cg_value const& v )
{
	assert( is_integer(v.hint()) );
	return emit_cmp_ne( v, null_value( v.hint(), v.abi() ) );
}

cg_value cgs_sisd::cast_f2b( cg_value const& v )
{
	assert( is_real(v.hint()) );
	return emit_cmp_ne( v, null_value( v.hint(), v.abi() ) );
}

abis::id cgs_sisd::intrinsic_abi() const
{
	return abis::llvm;
}

abis::id cgs_sisd::param_abi( bool c_compatible ) const
{
	return c_compatible ? abis::c : abis::llvm;
}

cg_value cgs_sisd::emit_ddx( cg_value const& v )
{
	// It is not available in SISD mode.
	EFLIB_ASSERT_UNIMPLEMENTED();
	return v;
}

cg_value cgs_sisd::emit_ddy( cg_value const& v )
{
	// It is not available in SISD mode.
	EFLIB_ASSERT_UNIMPLEMENTED();
	return v;
}

cg_value cgs_sisd::packed_mask()
{
	assert(false);
	return cg_value();
}

Value* cgs_sisd::phi_( BasicBlock* b0, Value* v0, BasicBlock* b1, Value* v1 )
{
	PHINode* phi = builder().CreatePHI( v0->getType(), 2 );
	phi->addIncoming(v0, b0);
	phi->addIncoming(v1, b1);
	return phi;
}

END_NS_SASL_CODEGEN();
