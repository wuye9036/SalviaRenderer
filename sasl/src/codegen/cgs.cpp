#include <sasl/include/codegen/cgs.h>

#include <sasl/include/codegen/utility.h>
#include <sasl/include/codegen/ty_cache.h>
#include <sasl/include/codegen/cg_module_impl.h>
#include <sasl/include/codegen/cg_contexts.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/enums/enums_utility.h>

#include <salviar/include/shader_abi.h>

#include <eflib/include/math/math.h>
#include <eflib/include/utility/polymorphic_cast.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/IRBuilder.h>
#include <llvm/TypeBuilder.h>
#include <llvm/Support/CFG.h>
#include <llvm/Intrinsics.h>
#include <llvm/Constants.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <eflib/include/platform/boost_end.h>

using llvm::Module;
using llvm::LLVMContext;
using llvm::DefaultIRBuilder;
using llvm::Value;
using llvm::Type;
using llvm::VectorType;
using llvm::TypeBuilder;
using llvm::BasicBlock;
using llvm::Instruction;

namespace Intrinsic = llvm::Intrinsic;

using boost::shared_ptr;

using namespace sasl::syntax_tree;
using namespace sasl::semantic;

using sasl::utility::is_integer;
using sasl::utility::is_real;
using sasl::utility::is_signed;
using sasl::utility::is_scalar;
using sasl::utility::is_vector;
using sasl::utility::is_matrix;
using sasl::utility::is_sampler;
using sasl::utility::scalar_of;
using sasl::utility::vector_of;
using sasl::utility::matrix_of;
using sasl::utility::vector_size;
using sasl::utility::vector_count;
using sasl::utility::storage_size;
using sasl::utility::replace_scalar;
using sasl::utility::row_vector_of;

using salviar::PACKAGE_ELEMENT_COUNT;
using salviar::SIMD_ELEMENT_COUNT;

using namespace eflib;

BEGIN_NS_SASL_CODEGEN();

/// @}

bool cg_service::initialize( cg_module_impl* mod, module_context* ctxt, module_semantic* sem )
{
	assert(mod);
	assert(ctxt);
	assert(sem);

	llvm_mod_ = mod;
	ctxt_ = ctxt;
	sem_ = sem;

	initialize_cache( context() );
	ext_.reset( new cg_extension( llvm_mod_->builder(), llvm_mod_->llvm_context(), llvm_mod_->llvm_module() ) );
	return true;
}

Module* cg_service::module() const{
	return llvm_mod_->llvm_module();
}

LLVMContext& cg_service::context() const{
	return llvm_mod_->llvm_context();
}

DefaultIRBuilder& cg_service::builder() const{
	return *( llvm_mod_->builder() );
}

cg_function* cg_service::fetch_function(function_type* fn_node){
	node_context* fn_ctxt = ctxt_->get_node_context(fn_node);
	if(fn_ctxt->function_scope)
	{
		return fn_ctxt->function_scope;
	}

	cg_function* ret = ctxt_->create_cg_function();

	ret->fnty = fn_node;
	ret->c_compatible		= sem_->get_semantic(fn_node)->msc_compatible();
	ret->external			= sem_->get_semantic(fn_node)->is_external();
	ret->partial_execution	= sem_->get_semantic(fn_node)->partial_execution();

	abis::id abi = param_abi( ret->c_compatible );

	vector<Type*> par_tys;

	Type* ret_ty = ctxt_->get_node_context( fn_node->retval_type.get() )->ty->ty(abi);

	ret->ret_void = true;
	if( abi == abis::c || ret->external ){
		if( fn_node->retval_type->tycode != builtin_types::_void ){
			// If function need C compatible and return value is not void, The first parameter is set to point to return value, and parameters moves right.
			Type* ret_ptr = PointerType::getUnqual( ret_ty );
			par_tys.push_back( ret_ptr );
			ret->ret_void = false;
		}

		ret_ty = Type::getVoidTy( context() );
	}

	if( abi == abis::package && ret->partial_execution ){
		Type* mask_ty = Type::getInt16Ty( context() );
		par_tys.push_back(mask_ty);
	}

	// Create function type.
	BOOST_FOREACH( shared_ptr<parameter> const& par, fn_node->params )
	{
		node_context* par_ctxt = ctxt_->get_node_context( par.get() );
		cg_type* par_ty = par_ctxt->ty;
		assert(par_ty);

		Type* par_llty = par_ty->ty( abi );
		
		bool as_ref = false;
		builtin_types par_hint = par_ty->hint();
		if( ret->c_compatible || ret->external ){
			if( is_sampler( par_hint ) ){
				as_ref = false;
			} else if ( is_scalar(par_hint) && ( promote_abi( param_abi(false), abis::llvm ) == abis::llvm ) ){
				as_ref = false;
			} else {
				as_ref = true;
			}
		} else {
			as_ref = false;
		}

		if( as_ref )
		{
			par_tys.push_back( PointerType::getUnqual( par_llty ) );
		}
		else
		{
			par_tys.push_back( par_llty );
		}
	}

	FunctionType* fty = FunctionType::get( ret_ty, par_tys, false );

	// Create function
	ret->fn = Function::Create( fty, Function::ExternalLinkage, sem_->get_symbol(fn_node)->mangled_name(), module() );
	ret->cg = this;
	return ret;
}

cg_value cg_service::null_value( cg_type* tyinfo, abis::id abi )
{
	assert( tyinfo && abi != abis::unknown );
	Type* value_type = tyinfo->ty(abi);
	assert( value_type );
	return create_value( tyinfo, Constant::getNullValue(value_type), value_kinds::value, abi );
}

cg_value cg_service::null_value( builtin_types bt, abis::id abi )
{
	assert( bt != builtin_types::none );
	Type* valty = type_( bt, abi );
	cg_value val = create_value( bt, Constant::getNullValue( valty ), value_kinds::value, abi );
	return val;
}

cg_value cg_service::create_value( cg_type* tyinfo, Value* val, value_kinds::id k, abis::id abi ){
	return cg_value( tyinfo, val, k, abi, this );
}

cg_value cg_service::create_value( builtin_types hint, Value* val, value_kinds::id k, abis::id abi )
{
	return cg_value( hint, val, k, abi, this );
}

cg_value cg_service::create_value( cg_type* tyinfo, builtin_types hint, Value* val, value_kinds::id k, abis::id abi )
{
	if( tyinfo ){
		return create_value( tyinfo, val, k, abi );
	} else {
		return create_value( hint, val, k ,abi );
	}
}

cg_type* cg_service::create_ty(tynode* tyn)
{
	node_context* ctxt = ctxt_->get_or_create_node_context(tyn);
	
	if( ctxt->ty ) { return ctxt->ty; }

	cg_type* ret= ctxt_->create_cg_type();
	ret->tyn		= tyn;

	if( tyn->is_builtin() ){
		ret->tys[abis::c]			= type_(tyn->tycode, abis::c);
		ret->tys[abis::llvm]		= type_(tyn->tycode, abis::llvm);
		ret->tys[abis::vectorize]	= type_(tyn->tycode, abis::vectorize);
		ret->tys[abis::package]	= type_(tyn->tycode, abis::package);
	} else {
		if( tyn->is_struct() )
		{
			shared_ptr<struct_type> struct_tyn = tyn->as_handle<struct_type>();

			vector<Type*> c_member_types;
			vector<Type*> llvm_member_types;
			vector<Type*> vectorize_member_types;
			vector<Type*> package_member_types;

			BOOST_FOREACH( shared_ptr<declaration> const& decl, struct_tyn->decls){
				if( decl->node_class() == node_ids::variable_declaration ){
					shared_ptr<variable_declaration> decl_tyn = decl->as_handle<variable_declaration>();
					cg_type* decl_cgty = create_ty( sem_->get_semantic(decl_tyn->type_info)->ty_proto() );
					size_t declarator_count = decl_tyn->declarators.size();
					c_member_types.insert( c_member_types.end(), declarator_count, decl_cgty->ty(abis::c) );
					llvm_member_types.insert( llvm_member_types.end(), declarator_count, decl_cgty->ty(abis::llvm) );
					vectorize_member_types.insert( vectorize_member_types.end(), declarator_count, decl_cgty->ty(abis::vectorize) );
					package_member_types.insert( package_member_types.end(), declarator_count, decl_cgty->ty(abis::package) );
				}
			}

			StructType* ty_c	= StructType::create( c_member_types,			struct_tyn->name->str + ".abi.c" );
			StructType* ty_llvm	= StructType::create( llvm_member_types,		struct_tyn->name->str + ".abi.llvm" );
			StructType* ty_vec	= NULL;
			if( vectorize_member_types[0] != NULL ){
				ty_vec = StructType::create( vectorize_member_types,struct_tyn->name->str + ".abi.vec" );
			}
			StructType* ty_pkg	= NULL;
			if( package_member_types[0] != NULL )
			{
				ty_pkg = StructType::create( package_member_types,	struct_tyn->name->str + ".abi.pkg" );
			}

			ret->tys[abis::c]			= ty_c;
			ret->tys[abis::llvm]		= ty_llvm;
			ret->tys[abis::vectorize]	= ty_vec;
			ret->tys[abis::package]	= ty_pkg;
		}
		else if( tyn->is_array() )
		{
			array_type*	array_tyn	= polymorphic_cast<array_type*>(tyn);
			cg_type*	elem_ti		= create_ty( array_tyn->elem_type.get() );

			ret->tys[abis::c]			= PointerType::getUnqual( elem_ti->ty(abis::c) );
			ret->tys[abis::llvm]		= PointerType::getUnqual( elem_ti->ty(abis::llvm) );
			ret->tys[abis::vectorize]	= PointerType::getUnqual( elem_ti->ty(abis::vectorize) );
			ret->tys[abis::package]	= PointerType::getUnqual( elem_ti->ty(abis::package) );
		}
		else
		{
			assert(false);
		}
	}

	return ret;
}

cg_type* cg_service::member_tyinfo( cg_type const* agg, size_t index ) const
{
	if( !agg ){
		return NULL;
	} else if ( agg->tyn_ptr()->is_struct() ){
		shared_ptr<struct_type> struct_sty = agg->tyn_ptr()->as_handle<struct_type>();

		size_t var_index = 0;
		BOOST_FOREACH( shared_ptr<declaration> const& child, struct_sty->decls ){
			if( child->node_class() == node_ids::variable_declaration ){
				shared_ptr<variable_declaration> vardecl = child->as_handle<variable_declaration>();
				var_index += vardecl->declarators.size();
				if( index < var_index ){
					return const_cast<cg_service*>(this)->ctxt_->get_node_context( vardecl.get() )->ty;
				}
			}
		}

		assert(!"Out of struct bound.");
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	return NULL;
}

cg_value cg_service::create_variable( builtin_types bt, abis::id abi, std::string const& name )
{
	Type* vty = type_( bt, abi );
	return create_value( bt, ext_->stack_alloc(vty, name), value_kinds::reference, abi );
}

cg_value cg_service::create_variable( cg_type const* ty, abis::id abi, std::string const& name )
{
	Type* vty = type_( ty, abi );
	return create_value( const_cast<cg_type*>(ty), ext_->stack_alloc(vty, name), value_kinds::reference, abi );
}

insert_point_t cg_service::new_block( std::string const& hint, bool set_as_current )
{
	assert( in_function() );
	insert_point_t ret;
	ret.block = BasicBlock::Create( context(), hint, fn().fn );
	if( set_as_current ){ set_insert_point( ret ); }
	return ret;
}

void cg_service::clean_empty_blocks()
{
	assert( in_function() );

	typedef Function::BasicBlockListType::iterator block_iterator_t;
	block_iterator_t beg = fn().fn->getBasicBlockList().begin();
	block_iterator_t end = fn().fn->getBasicBlockList().end();

	// Remove useless blocks
	vector<BasicBlock*> useless_blocks;

	for( block_iterator_t it = beg; it != end; ++it )
	{
		// If block has terminator, that's a well-formed block.
		if( it->getTerminator() ) continue;

		// Add no-pred & empty block to remove list.
		if( llvm::pred_begin(it) == llvm::pred_end(it) && it->empty() ){
			useless_blocks.push_back( it );
			continue;
		}
	}

	BOOST_FOREACH( BasicBlock* bb, useless_blocks ){
		bb->removeFromParent();
	}

	// Relink unlinked blocks
	beg = fn().fn->getBasicBlockList().begin();
	end = fn().fn->getBasicBlockList().end();
	for( block_iterator_t it = beg; it != end; ++it ){
		if( it->getTerminator() ) continue;

		// Link block to next.
		block_iterator_t next_it = it;
		++next_it;
		builder().SetInsertPoint( it );
		if( next_it != fn().fn->getBasicBlockList().end() ){
			builder().CreateBr( next_it );
		} else {
			if( !fn().fn->getReturnType()->isVoidTy() ){
				builder().CreateRet( Constant::getNullValue( fn().fn->getReturnType() ) );
			} else {
				emit_return();
			}
		}
	}
}

bool cg_service::in_function() const{
	return !fn_ctxts.empty();
}

cg_function& cg_service::fn(){
	return *fn_ctxts.back();
}

void cg_service::push_fn(cg_function* fn){
	if( !fn_ctxts.empty() )
	{
		assert( fn->fn != this->fn().fn );
	}
	fn_ctxts.push_back(fn);
}

void cg_service::pop_fn(){
	fn_ctxts.pop_back();
}

void cg_service::set_insert_point( insert_point_t const& ip ){
	builder().SetInsertPoint(ip.block);
}

insert_point_t cg_service::insert_point() const
{
	insert_point_t ret;
	ret.block = builder().GetInsertBlock();
	return ret;
}

Type* cg_service::type_( builtin_types bt, abis::id abi )
{
	assert( abi != abis::unknown );
	return get_llvm_type( context(), bt, abi );
}

Type* cg_service::type_( cg_type const* ty, abis::id abi )
{
	assert( ty->ty(abi) );
	return ty->ty(abi);
}

llvm::Value* cg_service::load( cg_value const& v )
{
	value_kinds::id kind = v.kind();
	Value* raw = v.raw();

	uint32_t masks = v.masks();

	assert( kind != value_kinds::unknown && kind != value_kinds::ty_only );

	Value* ref_val = NULL;
	if( kind == value_kinds::reference || kind == value_kinds::value ){
		ref_val = raw;
	} else if( ( kind & (~value_kinds::reference) ) == value_kinds::elements ){
		if( masks > 0 )
		{
			// Decompose indexes.
			char indexes[4] = {-1, -1, -1, -1};
			mask_to_indexes(indexes, masks);
			vector<int> index_values;
			index_values.reserve(4);
			for( int i = 0; i < 4 && indexes[i] != -1; ++i ){
				index_values.push_back( indexes[i] );
			}
			assert( !index_values.empty() );

			// Swizzle and write mask
			if( index_values.size() == 1 && is_scalar(v.hint()) ){
				// Only one member we could extract reference.
				ref_val = emit_extract_val( v.parent()->to_rvalue(), index_values[0] ).load();
			} else {
				// Multi-members must be swizzle/writemask.
				assert( (kind & value_kinds::reference) == 0 );
				cg_value ret_val = emit_extract_elem_mask( v.parent()->to_rvalue(), masks );
				return ret_val.load( v.abi() );
			}
		}
		else
		{
			assert( v.index() );
			ref_val = emit_extract_val( v.parent()->to_rvalue(), *v.index() ).load();
		}
	} else {
		assert(false);
	}

	if( kind & value_kinds::reference ){
		return builder().CreateLoad( ref_val );
	} else {
		return ref_val;
	}
}

llvm::Value* cg_service::load( cg_value const& v, abis::id abi )
{
	return load_as( v, abi );
}

llvm::Value* cg_service::load_ref( cg_value const& v )
{
	value_kinds::id kind = v.kind();

	if( kind == value_kinds::reference ){
		return v.raw();
	} else if( kind == (value_kinds::elements|value_kinds::reference) ){
		cg_value non_ref( v );
		non_ref.kind( value_kinds::elements );
		return non_ref.load();
	} if( kind == value_kinds::elements ){
		assert( v.masks() );
		return emit_extract_elem_mask( *v.parent(), v.masks() ).load_ref();
	}
	return NULL;
}

Value* cg_service::load_ref( cg_value const& v, abis::id abi )
{
	if( v.abi() == abi || v.hint() == builtin_types::_sampler ){
		return load_ref(v);
	} else {
		return NULL;
	}
}

Value* cg_service::load_as( cg_value const& v, abis::id abi )
{
	assert( abi != abis::unknown );

	if( v.abi() == abi ){ return v.load(); }

	switch( v.abi() )
	{
	case abis::c:
		if( abi == abis::llvm ){
			return load_as_llvm_c(v, abi);
		} else if ( abi == abis::vectorize ){
			EFLIB_ASSERT_UNIMPLEMENTED();
			return NULL;
		} else if ( abi == abis::package ) {
			return load_c_as_package( v );
		} else {
			assert(false);
			return NULL;
		}
	case abis::llvm:
		if( abi == abis::c ){
			return load_as_llvm_c(v, abi);
		} else if ( abi == abis::vectorize ){
			EFLIB_ASSERT_UNIMPLEMENTED();
			return NULL;
		} else if ( abi == abis::package ) {
			EFLIB_ASSERT_UNIMPLEMENTED();
			return NULL;
		} else {
			assert(false);
			return NULL;
		}
	case abis::vectorize:
		if( abi == abis::c ){
			EFLIB_ASSERT_UNIMPLEMENTED();
			return NULL;
		} else if ( abi == abis::llvm ){
			EFLIB_ASSERT_UNIMPLEMENTED();
			return NULL;
		} else if ( abi == abis::package ) {
			return load_vec_as_package( v );
		} else {
			assert(false);
			return NULL;
		}
	case abis::package:
		if( abi == abis::c ){
			EFLIB_ASSERT_UNIMPLEMENTED();
			return NULL;
		} else if ( abi == abis::llvm ){
			EFLIB_ASSERT_UNIMPLEMENTED();
			return NULL;
		} else if ( abi == abis::vectorize ) {
			EFLIB_ASSERT_UNIMPLEMENTED();
			return NULL;
		} else {
			assert(false);
			return NULL;
		}
	}

	assert(false);
	return NULL;
}

Value* cg_service::load_as_llvm_c( cg_value const& v, abis::id abi )
{
	builtin_types hint = v.hint();

	if( is_scalar(hint) || is_sampler(hint) ){
		return v.load();
	} else if( is_vector( hint ) ){
		cg_value ret_value = undef_value( hint, abi );

		size_t vec_size = vector_size( hint );
		for( size_t i = 0; i < vec_size; ++i ){
			ret_value = emit_insert_val( ret_value, (int)i, emit_extract_elem(v, i) );
		}

		return ret_value.load();
	} else if( is_matrix( hint ) ){
		cg_value ret_value = null_value( hint, abi );
		size_t vec_count = vector_count( hint );
		for( size_t i = 0; i < vec_count; ++i ){
			cg_value org_vec = emit_extract_val(v, (int)i);
			ret_value = emit_insert_val( ret_value, (int)i, org_vec );
		}

		return ret_value.load();
	} else {
		// NOTE: We assume that, if tyinfo is null and hint is none, it is only the entry of vs/ps. Otherwise, tyinfo must be not NULL.
		if( !v.ty() && hint == builtin_types::none ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		} else {
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	}
	return NULL;
}

cg_value cg_service::emit_insert_val( cg_value const& lhs, cg_value const& idx, cg_value const& elem_value )
{
	Value* indexes[1] = { idx.load() };
	Value* agg = lhs.load();
	Value* new_value = NULL;
	if( agg->getType()->isStructTy() ){
		assert(false);
	} else if ( agg->getType()->isVectorTy() ){
		if( lhs.abi() == abis::vectorize || lhs.abi() == abis::package ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
		new_value = builder().CreateInsertElement( agg, elem_value.load(), indexes[0] );
	}
	assert(new_value);
	
	return create_value( lhs.ty(), lhs.hint(), new_value, value_kinds::value, lhs.abi() );
}

cg_value cg_service::emit_insert_val( cg_value const& lhs, int index, cg_value const& elem_value )
{
	assert(index >= 0);

	Value* agg = lhs.load();
	Value* new_value = NULL;
	
	if( agg->getType()->isStructTy() ){
		if( lhs.abi() == abis::vectorize || lhs.abi() == abis::package ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
		new_value = builder().CreateInsertValue( agg, elem_value.load(lhs.abi()), (unsigned)index );
	} else if ( agg->getType()->isVectorTy() ){
		if( lhs.abi() == abis::vectorize || lhs.abi() == abis::package ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
		cg_value index_value = create_value( builtin_types::_sint32, ext_->get_int(index), value_kinds::value, abis::llvm );
		return emit_insert_val( lhs, index_value, elem_value );
	}
	assert(new_value);

	return create_value( lhs.ty(), lhs.hint(), new_value, value_kinds::value, lhs.abi() );
}

Value* cg_service::load_vec_as_package( cg_value const& v )
{
	builtin_types hint = v.hint();

	if( is_scalar(hint) || is_vector(hint) )
	{
		Value* vec_v = v.load();
		vector<Constant*> indexes;
		indexes.reserve( PACKAGE_ELEMENT_COUNT );
		for( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
			indexes.push_back( ext_->get_int( static_cast<int>(i) % SIMD_ELEMENT_COUNT() ) );
		}
		return builder().CreateShuffleVector( vec_v, vec_v, ConstantVector::get( indexes ) );
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	return NULL;
}

Value* cg_service::load_c_as_package( cg_value const& v )
{
	if( v.hint() == builtin_types::_sampler ){
		return v.load();
	} else {
		
		cg_value llvm_v = create_value( v.ty(), v.hint(), v.load(abis::llvm), value_kinds::value, abis::llvm );

		if( is_scalar( v.hint() ) || is_vector( v.hint() ) ){

			// Vectorize value if scalar.
			Value* vec_val = NULL;
			if( is_scalar(v.hint()) ){
				vec_val = cast_s2v( llvm_v ).load();
			} else {
				vec_val = v.load(abis::llvm);
			}

			// Shuffle llvm value to package value.
			size_t vec_size = vector_size(v.hint());
			size_t vec_stride = ceil_to_pow2(vec_size);

			int package_scalar_count = static_cast<int>( vec_stride * PACKAGE_ELEMENT_COUNT );
			vector<size_t> shuffle_indexes(package_scalar_count);
			
			for ( size_t i_elem = 0; i_elem < PACKAGE_ELEMENT_COUNT; ++i_elem ){
				for ( size_t i_scalar = 0 ; i_scalar < vec_stride; ++i_scalar ){
					shuffle_indexes[i_elem*vec_stride+i_scalar] = i_scalar;
				}
			}

			Value* shuffle_mask = ext_->get_vector<int>( ArrayRef<size_t>(shuffle_indexes) );
			return builder().CreateShuffleVector( vec_val, UndefValue::get(vec_val->getType()), shuffle_mask );
		} else {
			EFLIB_ASSERT_UNIMPLEMENTED();
			return NULL;
		}
	}
}

abis::id cg_service::promote_abi( abis::id abi0, abis::id abi1 )
{
	if( abi0 == abis::c ){ return abi1; }
	if( abi1 == abis::c ){ return abi0; }
	if( abi0 == abis::llvm ){ return abi1; }
	if( abi1 == abis::llvm ){ return abi0; }
	if( abi0 == abis::vectorize ){ return abi1; }
	if( abi1 == abis::vectorize ){ return abi0; }
	return abi0;
}

abis::id cg_service::promote_abi( abis::id abi0, abis::id abi1, abis::id abi2 )
{
	return promote_abi( promote_abi( abi0, abi1 ), abi2 );
}

cg_value cg_service::emit_mul_comp( cg_value const& lhs, cg_value const& rhs )
{
	builtin_types lhint = lhs.hint();
	builtin_types rhint = rhs.hint();

	assert( lhint != builtin_types::none );
	assert( rhint != builtin_types::none );

	cg_value lv = lhs;
	cg_value rv = rhs;

	if( lhint != rhint )
	{
		if( is_scalar(lhint) ) { lv = extend_to_vm( lhs, rhint ); }
		else if( is_scalar(rhint) ) { rv = extend_to_vm( rhs, lhint ); }
		else { assert(false); }
	}

	binary_intrin_functor f_mul = boost::bind( &DefaultIRBuilder::CreateFMul, builder(), _1, _2, "", (llvm::MDNode*)(NULL) );
	binary_intrin_functor i_mul = boost::bind( &DefaultIRBuilder::CreateMul,  builder(), _1, _2, "", false, false );
	return emit_bin_ps_ta_sva( lv, rv, i_mul, i_mul, f_mul );
}

bool xor(bool l, bool r)
{
	return (l && !r) || (!l && r);
}

cg_value cg_service::emit_add( cg_value const& lhs, cg_value const& rhs )
{
	binary_intrin_functor f_add = boost::bind( &DefaultIRBuilder::CreateFAdd, builder(), _1, _2, "", (llvm::MDNode*)(NULL) );
	binary_intrin_functor i_add = boost::bind( &DefaultIRBuilder::CreateAdd,  builder(), _1, _2, "", false, false );

	return emit_bin_es_ta_sva( lhs, rhs, i_add, i_add, f_add );
}

cg_value cg_service::emit_sub( cg_value const& lhs, cg_value const& rhs )
{
	binary_intrin_functor f_sub = boost::bind( &DefaultIRBuilder::CreateFSub, builder(), _1, _2, "", (llvm::MDNode*)(NULL) );
	binary_intrin_functor i_sub = boost::bind( &DefaultIRBuilder::CreateSub,  builder(), _1, _2, "", false, false );
	
	return emit_bin_es_ta_sva( lhs, rhs, i_sub, i_sub, f_sub );
}

cg_value cg_service::emit_mul_intrin( cg_value const& lhs, cg_value const& rhs )
{
	builtin_types lhint = lhs.hint();
	builtin_types rhint = rhs.hint();

	if( is_scalar(lhint) ){
		if( is_scalar(rhint) ){
			return emit_mul_comp( lhs, rhs );
		} else if ( is_vector(rhint) ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		} else if ( is_matrix(rhint) ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	} else if ( is_vector(lhint) ){
		if( is_scalar(rhint) ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		} else if ( is_vector(rhint) ){
			return emit_dot_vv( lhs, rhs );
		} else if ( is_matrix(rhint) ){
			return emit_mul_vm( lhs, rhs );
		}
	} else if ( is_matrix(lhint) ){
		if( is_scalar(rhint) ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		} else if( is_vector(rhint) ){
			return emit_mul_mv( lhs, rhs );
		} else if( is_matrix(rhint) ){
			return emit_mul_mm( lhs, rhs );
		}
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return cg_value();
}

cg_value cg_service::emit_div( cg_value const& lhs, cg_value const& rhs )
{
	binary_intrin_functor f_div = boost::bind( &DefaultIRBuilder::CreateFDiv, builder(), _1, _2, "", (llvm::MDNode*)(NULL) );
	binary_intrin_functor i_div = boost::bind( &DefaultIRBuilder::CreateSDiv, builder(), _1, _2, "", false );
	binary_intrin_functor u_div = boost::bind( &DefaultIRBuilder::CreateUDiv, builder(), _1, _2, "", false );
	binary_intrin_functor i_safe_div = boost::bind( &cg_extension::safe_idiv_imod_sv, ext_.get(), _1, _2, i_div );
	binary_intrin_functor u_safe_div = boost::bind( &cg_extension::safe_idiv_imod_sv, ext_.get(), _1, _2, u_div );

	return emit_bin_es_ta_sva( lhs, rhs, i_safe_div, u_safe_div, f_div );
}

cg_value cg_service::emit_mod( cg_value const& lhs, cg_value const& rhs )
{	
	binary_intrin_functor i_mod = boost::bind( &DefaultIRBuilder::CreateSRem, builder(), _1, _2, "" );
	binary_intrin_functor u_mod = boost::bind( &DefaultIRBuilder::CreateURem, builder(), _1, _2, "" );
		
	binary_intrin_functor i_safe_mod = boost::bind( &cg_extension::safe_idiv_imod_sv, ext_.get(), _1, _2, i_mod );
	binary_intrin_functor u_safe_mod = boost::bind( &cg_extension::safe_idiv_imod_sv, ext_.get(), _1, _2, u_mod );

	binary_intrin_functor intrin_mod_f32 = ext_->bind_external_to_binary(externals::mod_f32);
	binary_intrin_functor f_mod_sv =
		ext_->promote_to_binary_sv(intrin_mod_f32, null_binary, null_binary);
	binary_intrin_functor f_mod =
		boost::bind(&cg_extension::call_binary_intrin, ext_.get(), (Type*)NULL, _1, _2, f_mod_sv, null_unary);

	return emit_bin_es_ta_sva( lhs, rhs, i_safe_mod, u_safe_mod, f_mod );
}

cg_value cg_service::emit_lshift( cg_value const& lhs, cg_value const& rhs )
{
	binary_intrin_functor shl = boost::bind( &DefaultIRBuilder::CreateBinOp, builder(), Instruction::Shl, _1, _2, "" );
	return emit_bin_es_ta_sva( lhs, rhs, shl, shl, shl );
}

cg_value cg_service::emit_rshift( cg_value const& lhs, cg_value const& rhs )
{
	binary_intrin_functor shr = boost::bind( &DefaultIRBuilder::CreateBinOp, builder(), Instruction::LShr, _1, _2, "" );
	return emit_bin_es_ta_sva( lhs, rhs, shr, shr, shr );
}

cg_value cg_service::emit_bit_and( cg_value const& lhs, cg_value const& rhs )
{
	binary_intrin_functor bit_and = boost::bind( &DefaultIRBuilder::CreateBinOp, builder(), Instruction::And, _1, _2, "" );
	return emit_bin_es_ta_sva( lhs, rhs, bit_and, bit_and, bit_and );
}

cg_value cg_service::emit_bit_or( cg_value const& lhs, cg_value const& rhs )
{
	binary_intrin_functor bit_or = boost::bind( &DefaultIRBuilder::CreateBinOp, builder(), Instruction::Or, _1, _2, "" );
	return emit_bin_es_ta_sva( lhs, rhs, bit_or, bit_or, bit_or );
}

cg_value cg_service::emit_bit_xor( cg_value const& lhs, cg_value const& rhs )
{
	binary_intrin_functor bit_xor = boost::bind( &DefaultIRBuilder::CreateBinOp, builder(), Instruction::Xor, _1, _2, "" );
	return emit_bin_es_ta_sva( lhs, rhs, bit_xor, bit_xor, bit_xor );
}

cg_value cg_service::emit_dot( cg_value const& lhs, cg_value const& rhs )
{
	return emit_dot_vv(lhs, rhs);
}

cg_value cg_service::emit_cross( cg_value const& lhs, cg_value const& rhs )
{
	assert( lhs.hint() == vector_of( builtin_types::_float, 3 ) );
	assert( rhs.hint() == lhs.hint() );

	uint32_t swz_va = indexes_to_mask( 1, 2, 0, -1 );
	uint32_t swz_vb = indexes_to_mask( 2, 0, 1, -1 );

	cg_value lvec_a = emit_extract_elem_mask( lhs, swz_va );
	cg_value lvec_b = emit_extract_elem_mask( lhs, swz_vb );
	cg_value rvec_a = emit_extract_elem_mask( rhs, swz_va );
	cg_value rvec_b = emit_extract_elem_mask( rhs, swz_vb );

	return emit_sub( emit_mul_comp(lvec_a, rvec_b), emit_mul_comp(lvec_b, rvec_a) );
}

cg_value cg_service::emit_extract_ref( cg_value const& lhs, int idx )
{
	assert( lhs.storable() );

	builtin_types agg_hint = lhs.hint();

	if( is_vector(agg_hint) ){
		char indexes[4] = { (char)idx, -1, -1, -1 };
		uint32_t mask = indexes_to_mask( indexes );
		return cg_value::slice( lhs, mask );
	} else if( is_matrix(agg_hint) ){
		EFLIB_ASSERT_UNIMPLEMENTED();
		return cg_value();
	} else if ( agg_hint == builtin_types::none ){
		Value* agg_address = lhs.load_ref();
		Value* elem_address = builder().CreateStructGEP( agg_address, (unsigned)idx );
		cg_type* tyinfo = NULL;
		if( lhs.ty() ){
			tyinfo = member_tyinfo( lhs.ty(), (size_t)idx );
		}
		return create_value( tyinfo, elem_address, value_kinds::reference, lhs.abi() );
	}
	EFLIB_ASSERT_UNIMPLEMENTED();
	return cg_value();
}

cg_value cg_service::emit_extract_ref( cg_value const& lhs, cg_value const& idx )
{
	assert( lhs.storable() );
	
	abis::id promoted_abi = promote_abi( lhs.abi(), idx.abi() );
	builtin_types agg_hint = lhs.hint();

	if( is_vector(agg_hint) )
	{
		return cg_value::slice( lhs, idx );
	}
	else if( is_matrix(agg_hint) )
	{
		Value* addr = lhs.load_ref();
		switch (promoted_abi)
		{
		case abis::c:
		case abis::llvm:
			{
				Type*  value_ty = addr->getType()->getPointerElementType();
				Type*  element_ty = value_ty->getStructElementType(0);
				Value* first_elem_ptr = builder().CreateBitCast( addr, PointerType::getUnqual(element_ty) );
				Value* indexes[] = { idx.load() };
				Value* elem_ptr = builder().CreateGEP(first_elem_ptr, indexes);
				return create_value(NULL, row_vector_of(lhs.hint()), elem_ptr, value_kinds::reference, lhs.abi() );
			}
		case abis::package:
			EFLIB_ASSERT_UNIMPLEMENTED();
		default:
			assert(false);
		}
		return cg_value();
	}
	else if ( agg_hint == builtin_types::none )
	{
		// Array only
		Value* addr = lhs.load_ref();
		assert(addr);
		array_type_ptr array_tyn = lhs.ty()->tyn_ptr()->as_handle<array_type>();

		// Support one-dimension array only.
		if( array_tyn->array_lens.size() > 1 ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		}

		switch(promoted_abi)
		{
		case abis::c:
		case abis::llvm:
			{
				Value* elem_addr = builder().CreateGEP(addr, idx.load() );
				cg_type* elem_tyinfo = ctxt_->get_node_context( array_tyn->elem_type.get() )->ty;
				return create_value( elem_tyinfo, elem_addr, value_kinds::reference, lhs.abi() );
			}
		default:
			assert(false);
		}
	}
	return cg_value();
}

cg_value cg_service::emit_extract_val( cg_value const& lhs, int idx )
{
	builtin_types agg_hint = lhs.hint();

	Value* val = lhs.load();
	Value* elem_val = NULL;
	abis::id abi = abis::unknown;

	builtin_types elem_hint = builtin_types::none;
	cg_type* elem_tyi = NULL;

	if( agg_hint == builtin_types::none ){
		elem_val = builder().CreateExtractValue(val, static_cast<unsigned>(idx));
		abi = lhs.abi();
		elem_tyi = member_tyinfo( lhs.ty(), (size_t)idx );
	} else if( is_scalar(agg_hint) ){
		assert( idx == 0 );
		elem_val = val;
		elem_hint = agg_hint;
	} else if( is_vector(agg_hint) ){
		switch( lhs.abi() ){
		case abis::c:
			elem_val = builder().CreateExtractValue(val, static_cast<unsigned>(idx));
			break;
		case abis::llvm:
			elem_val = builder().CreateExtractElement(val, ext_->get_int(idx) );
			break;
		case abis::vectorize:
			EFLIB_ASSERT_UNIMPLEMENTED();
			break;
		case abis::package:
			{
				char indexes[4] = { char(idx), -1, -1, -1};
				elem_val = emit_extract_elem_mask( lhs, indexes_to_mask(indexes) ).load();
				break;
			}
		default:
			assert(!"Unknown ABI");
			break;
		}
		abi = promote_abi( abis::llvm, lhs.abi() );
		elem_hint = scalar_of(agg_hint);
	} else if( is_matrix(agg_hint) ){
		assert( promote_abi(lhs.abi(), abis::llvm) == abis::llvm );
		elem_val = builder().CreateExtractValue(val, static_cast<unsigned>(idx));
		abi = lhs.abi();
		elem_hint = vector_of( scalar_of(agg_hint), vector_size(agg_hint) );
	}

	return create_value( elem_tyi, elem_hint, elem_val, value_kinds::value, abi );
}

cg_value cg_service::emit_extract_val( cg_value const& lhs, cg_value const& idx )
{
	builtin_types agg_hint = lhs.hint();

	Value* elem_val = NULL;
	abis::id abi = promote_abi(lhs.abi(), idx.abi());

	builtin_types elem_hint = builtin_types::none;
	cg_type* elem_tyi = NULL;

	if( agg_hint == builtin_types::none ){
		// Array only
		Value* addr = lhs.load_ref();
		assert(addr);
		array_type_ptr array_tyn = lhs.ty()->tyn_ptr()->as_handle<array_type>();

		// Support one-dimension array only.
		if( array_tyn->array_lens.size() > 1 ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		}

		switch(abi)
		{
		case abis::c:
		case abis::llvm:
			{
				Value* elem_addr = builder().CreateGEP(addr, idx.load() );
				cg_type* elem_tyinfo = ctxt_->get_node_context( array_tyn->elem_type.get() )->ty;
				return create_value( elem_tyinfo, builtin_types::none, elem_addr, value_kinds::reference, lhs.abi() );
			}
		default:
			assert(false);
		}
		
	} else if( is_scalar(agg_hint) ){
		elem_val	= lhs.load();
		elem_hint	= agg_hint;
	} else if( is_vector(agg_hint) ){
		switch( abi ){
		case abis::c:
		case abis::llvm:
			elem_val = builder().CreateExtractElement( lhs.load(abis::llvm), idx.load() );
			break;
		case abis::vectorize:
			EFLIB_ASSERT_UNIMPLEMENTED();
			break;
		case abis::package:
			EFLIB_ASSERT_UNIMPLEMENTED();
			break;
		default:
			assert(!"Unknown ABI");
			break;
		}
		elem_hint = scalar_of(agg_hint);
	} else if( is_matrix(agg_hint) ){
		EFLIB_ASSERT_UNIMPLEMENTED();
		//assert( promote_abi(lhs.abi(), abis::llvm) == abis::llvm );
		//elem_val = builder().CreateExtractValue(val, static_cast<unsigned>(idx));
		//abi = lhs.abi();
		//elem_hint = vector_of( scalar_of(agg_hint), vector_size(agg_hint) );
	}

	return create_value( elem_tyi, elem_hint, elem_val, value_kinds::value, abi );
}

cg_value cg_service::emit_extract_elem_mask( cg_value const& vec, uint32_t mask )
{
	char indexes[4] = {-1, -1, -1, -1};
	mask_to_indexes( indexes, mask );
	uint32_t idx_len = indexes_length(indexes);

	assert( idx_len > 0 );
	if( vec.hint() == builtin_types::none && idx_len == 1 ){
		// struct, array or not-package, return extract elem.
		// Else do extract mask.
		if( vec.abi() != abis::package || vec.hint() == builtin_types::none ){
			return emit_extract_elem( vec, indexes[0] );
		}
	}
	// Get the hint.
	assert( vec.hint() != builtin_types::none );

	builtin_types swz_hint = scalar_of( vec.hint() );
	if( is_vector(vec.hint()) ){
		swz_hint = vector_of( scalar_of(vec.hint()), idx_len );
	} else if ( is_matrix(vec.hint()) ){
		EFLIB_ASSERT_UNIMPLEMENTED();
	} else {
		assert(false);
	}

	if( vec.storable() ){
		cg_value swz_proxy = create_value( NULL, swz_hint, NULL, value_kinds::elements, vec.abi() );
		swz_proxy.parent( vec );
		swz_proxy.masks( mask );
		return swz_proxy;
	} else {
		if( is_scalar( vec.hint() ) ) {
			EFLIB_ASSERT_UNIMPLEMENTED();
		} else if( is_vector( vec.hint() ) ) {
			Value* vec_v = vec.load( promote_abi(abis::llvm, vec.abi()) );
			switch( vec.abi() ){
			case abis::c:
			case abis::llvm:
				{
					Value* v = builder().CreateShuffleVector( vec_v, vec_v,
						ext_->get_vector<int>( ArrayRef<char>(indexes, idx_len) )
						);
					return create_value( NULL, swz_hint, v, value_kinds::value, abis::llvm );
				}
			case abis::vectorize:
				{
					vector<char> vectorize_idx( SIMD_ELEMENT_COUNT(), -1 );
					assert( idx_len < static_cast<uint32_t>(SIMD_ELEMENT_COUNT()) );
					copy( &indexes[0], &indexes[idx_len], vectorize_idx.begin() );
					fill( vectorize_idx.begin() + idx_len, vectorize_idx.end(), vector_size(vec.hint()) );

					Value* v = builder().CreateShuffleVector(
						vec_v, UndefValue::get(vec_v->getType()),
						ext_->get_vector<int>( ArrayRef<char>(vectorize_idx) )
						);
					return create_value( NULL, swz_hint, v, value_kinds::value, abis::vectorize );
				}
			case abis::package:
				{
					int src_element_pitch = ceil_to_pow2( static_cast<int>(vector_size(vec.hint())) );
					int swz_element_pitch = ceil_to_pow2( static_cast<int>(idx_len) );
						
					int src_scalar_count = src_element_pitch * PACKAGE_ELEMENT_COUNT;

					vector<char> indexes_per_value( src_scalar_count, src_scalar_count );
					copy( &indexes[0], &indexes[idx_len], indexes_per_value.begin() );

					vector<char> package_idx( PACKAGE_ELEMENT_COUNT * swz_element_pitch, -1 );
					assert( (int)idx_len <= SIMD_ELEMENT_COUNT() );

					for ( size_t i = 0; i < PACKAGE_ELEMENT_COUNT; ++i ){
						for( size_t j = 0; j < (size_t)swz_element_pitch; ++j ){
							package_idx[i*swz_element_pitch+j] = (char)(indexes_per_value[j]+i*src_element_pitch);
						}
					}

					Value* v = builder().CreateShuffleVector(
						vec_v, UndefValue::get(vec_v->getType()),
						ext_->get_vector<int>( ArrayRef<char>(package_idx) )
						);
					return create_value( NULL, swz_hint, v, value_kinds::value, abis::package );
				}
			default:
				assert(false);
			}
		} else if( is_matrix(vec.hint()) ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	}

	return cg_value();
}

cg_value cg_service::emit_extract_col( cg_value const& lhs, size_t index )
{
	assert( promote_abi(lhs.abi(), abis::llvm) == abis::llvm );

	cg_value val = lhs.to_rvalue();
	builtin_types mat_hint( lhs.hint() );
	assert( is_matrix(mat_hint) );

	size_t row_count = vector_count( mat_hint );

	builtin_types out_hint = vector_of( scalar_of(mat_hint), row_count );

	cg_value out_value = null_value( out_hint, lhs.abi() );
	for( size_t irow = 0; irow < row_count; ++irow ){
		cg_value row = emit_extract_val( val, (int)irow );
		cg_value cell = emit_extract_val( row, (int)index );
		out_value = emit_insert_val( out_value, (int)irow, cell );
	}

	return out_value;
}

cg_value cg_service::emit_dot_vv( cg_value const& lhs, cg_value const& rhs )
{
	abis::id promoted_abi = promote_abi(lhs.abi(), rhs.abi(), abis::llvm);
	// assert( promoted_abi == abis::llvm );
	
	size_t vec_size = vector_size( lhs.hint() );
	cg_value total = null_value( scalar_of( lhs.hint() ), promoted_abi );
	cg_value prod = emit_mul_comp( lhs, rhs );
	for( size_t i = 0; i < vec_size; ++i ){
		cg_value prod_elem = emit_extract_elem( prod, i );
		total.emplace( emit_add( total, prod_elem ).to_rvalue() );
	}

	return total;
}

cg_value cg_service::emit_mul_mv( cg_value const& lhs, cg_value const& rhs )
{
	assert( promote_abi(lhs.abi(), rhs.abi(), abis::llvm) == abis::llvm );

	builtin_types mhint = lhs.hint();
	builtin_types vhint = rhs.hint();

	size_t row_count = vector_count(mhint);

	builtin_types ret_hint = vector_of( scalar_of(vhint), row_count );

	cg_value ret_v = null_value( ret_hint, lhs.abi() );
	for( size_t irow = 0; irow < row_count; ++irow ){
		cg_value row_vec = emit_extract_val( lhs, irow );
		ret_v = emit_insert_val( ret_v, irow, emit_dot_vv(row_vec, rhs) );
	}

	return ret_v;
}

cg_value cg_service::emit_mul_vm( cg_value const& lhs, cg_value const& rhs )
{
	assert( promote_abi(lhs.abi(), rhs.abi(), abis::llvm) == abis::llvm );

	size_t out_v = vector_size( rhs.hint() );

	cg_value lrv = lhs.to_rvalue();
	cg_value rrv = rhs.to_rvalue();

	cg_value ret = null_value( vector_of( scalar_of(lhs.hint()), out_v ), lhs.abi() );
	for( size_t idx = 0; idx < out_v; ++idx ){
		ret = emit_insert_val( ret, (int)idx, emit_dot_vv( lrv, emit_extract_col(rrv, idx) ) );
	}

	return ret;
}

cg_value cg_service::emit_mul_mm( cg_value const& lhs, cg_value const& rhs )
{
	assert( promote_abi(lhs.abi(), rhs.abi(), abis::llvm) == abis::llvm );

	builtin_types lhint = lhs.hint();
	builtin_types rhint = rhs.hint();

	size_t out_v = vector_size( lhint );
	size_t out_r = vector_count( rhint );

	assert( vector_count(lhint) == vector_size(rhint) );

	builtin_types out_row_hint = vector_of( scalar_of(lhint), out_v );
	builtin_types out_hint = matrix_of( scalar_of(lhint), out_v, out_r );
	abis::id out_abi = lhs.abi();

	vector<cg_value> out_cells(out_v*out_r);
	out_cells.resize( out_v*out_r );

	// Calculate matrix cells.
	for( size_t icol = 0; icol < out_v; ++icol){
		cg_value col = emit_extract_col( rhs, icol );
		for( size_t irow = 0; irow < out_r; ++irow )
		{
			cg_value row = emit_extract_col( rhs, icol );
			out_cells[irow*out_v+icol] = emit_dot_vv( col, row );
		}
	}

	// Compose cells to matrix
	cg_value ret_value = null_value( out_hint, out_abi );
	for( size_t irow = 0; irow < out_r; ++irow ){
		cg_value row_vec = null_value( out_row_hint, out_abi );
		for( size_t icol = 0; icol < out_v; ++icol ){
			row_vec = emit_insert_val( row_vec, (int)icol, out_cells[irow*out_v+icol] );
		}
		ret_value = emit_insert_val( ret_value, (int)irow, row_vec );
	}

	return ret_value;
}

cg_value cg_service::emit_abs( cg_value const& arg_value )
{
	builtin_types hint = arg_value.hint();
	builtin_types scalar_hint = scalar_of( arg_value.hint() );
	abis::id arg_abi = arg_value.abi();

	Value* v = arg_value.load(arg_abi);

	Value* ret_v = ext_->call_unary_intrin( v->getType(), v, boost::bind(&cg_extension::abs_sv, ext_.get(), _1) );
	return create_value(arg_value.ty(), hint, ret_v, value_kinds::value, arg_abi);
}

cg_value cg_service::emit_sqrt( cg_value const& arg_value )
{
	builtin_types hint = arg_value.hint();
	builtin_types scalar_hint = scalar_of( arg_value.hint() );
	abis::id arg_abi = arg_value.abi();

	Value* v = arg_value.load(arg_abi);

	if( scalar_hint == builtin_types::_float )
	{
		unary_intrin_functor sqrt_sv = ext_->promote_to_unary_sv(
			ext_->bind_to_unary( ext_->vm_intrin<float(float)>(Intrinsic::sqrt) ),
			null_unary,
			ext_->bind_to_unary( ext_->vm_intrin(Intrinsic::x86_sse_sqrt_ps) )
			);
		Value* ret_v = ext_->call_unary_intrin(NULL, v, sqrt_sv);
		return create_value( arg_value.ty(), arg_value.hint(), ret_v, value_kinds::value, arg_abi );
	}
	else
	{
		EFLIB_ASSERT_UNIMPLEMENTED();
		return cg_value();
	}
}

cg_value cg_service::undef_value( builtin_types bt, abis::id abi )
{
	assert( bt != builtin_types::none );
	Type* valty = type_( bt, abi );
	cg_value val = create_value( bt, UndefValue::get(valty), value_kinds::value, abi );
	return val;
}

cg_value cg_service::emit_call( cg_function const& fn, vector<cg_value> const& args )
{
	return emit_call( fn, args, cg_value() );
}

cg_value cg_service::emit_call( cg_function const& fn, vector<cg_value> const& args, cg_value const& exec_mask )
{
	abis::id promoted_abi = abis::llvm;
	BOOST_FOREACH( cg_value const& arg, args )
	{
		promoted_abi = promote_abi( arg.abi(), promoted_abi );
	}

	vector<Value*> arg_values;
	cg_value var;

	if ( fn.first_arg_is_return_address() ){
		var = create_variable( fn.get_return_ty(), fn.abi(), ".tmp" );
		arg_values.push_back( var.load_ref() );
	}

	abis::id arg_abi = fn.c_compatible ? abis::c : promoted_abi;
	if( arg_abi == abis::package && fn.partial_execution ){
		if( exec_mask.abi() == abis::unknown ){
			arg_values.push_back( packed_mask().load( abis::llvm ) );
		} else {
			arg_values.push_back( exec_mask.load( abis::llvm ) );
		}
	}

	if( fn.c_compatible || fn.external ){
		BOOST_FOREACH( cg_value const& arg, args ){
			builtin_types hint = arg.hint();
			if( ( is_scalar(hint) && (arg_abi == abis::c || arg_abi == abis::llvm) ) || is_sampler(hint) ){
				arg_values.push_back( arg.load( arg_abi ) );
			} else {
				Value* ref_arg = load_ref( arg, arg_abi );
				if( !ref_arg ){
					Value* arg_val = load( arg, arg_abi );
					ref_arg = builder().CreateAlloca( arg_val->getType() );
					builder().CreateStore( arg_val, ref_arg );
				}
				arg_values.push_back( ref_arg );
			}
		}
	} else {
		BOOST_FOREACH( cg_value const& arg, args ){
			arg_values.push_back( arg.load( promoted_abi ) );
		}
	}

	Value* ret_val = builder().CreateCall( fn.fn, arg_values );

	if( fn.first_arg_is_return_address() ){
		return var;
	}

	abis::id ret_abi = fn.c_compatible ? abis::c : promoted_abi;
	return create_value( fn.get_return_ty(), ret_val, value_kinds::value, ret_abi );
}

cg_value cg_service::cast_s2v( cg_value const& v )
{
	builtin_types hint = v.hint();
	assert( is_scalar(hint) );
	builtin_types vhint = vector_of(hint, 1);

	// vector1 and scalar are same LLVM vector type when abi is Vectorize and Package 
	if( v.abi() == abis::vectorize || v.abi() == abis::package )
	{
		return create_value( NULL, vhint, v.load(), value_kinds::value, v.abi() );
	}

	// Otherwise return a new vector
	cg_value ret = null_value( vhint, v.abi() );
	return emit_insert_val( ret, 0, v );
}

cg_value cg_service::cast_v2s( cg_value const& v )
{
	assert( is_vector(v.hint()) );

	// vector1 and scalar are same LLVM vector type when abi is Vectorize and Package 
	if( v.abi() == abis::vectorize || v.abi() == abis::package )
	{
		return create_value( NULL, scalar_of(v.hint()), v.load(), value_kinds::value, v.abi() );
	}
	return emit_extract_val( v, 0 );
}

cg_value cg_service::cast_bits( cg_value const& v, cg_type* dest_tyi )
{
	abis::id abi = promote_abi(v.abi(), abis::llvm);

	Type* ty = dest_tyi->ty(abi);
	builtin_types dest_scalar_hint = scalar_of( dest_tyi->hint() );
	Type* dest_scalar_ty = type_( dest_scalar_hint, abis::llvm );
	unary_intrin_functor bitcast_sv = ext_->bind_cast_sv( dest_scalar_ty, cast_ops::bitcast );
	Value* ret = ext_->call_unary_intrin( ty, v.load(abi), bitcast_sv );
	return create_value( dest_tyi, ret, value_kinds::value, abi );
}

void cg_service::jump_to( insert_point_t const& ip )
{
	assert( ip );
	if( !insert_point().block->getTerminator() ){
		builder().CreateBr( ip.block );
	}
}

void cg_service::jump_cond( cg_value const& cond_v, insert_point_t const & true_ip, insert_point_t const& false_ip )
{
	Value* cond = cond_v.load_i1();
	builder().CreateCondBr( cond, true_ip.block, false_ip.block );
}

bool cg_service::merge_swizzle( cg_value const*& root, char indexes[], cg_value const& v )
{
	bool is_swizzle = false;

	// Find root of swizzle.
	root = &v;
	vector<uint32_t> masks;
	while( root->masks() != 0 && root->parent()->hint() != builtin_types::none ){
		is_swizzle = true;
		masks.push_back(root->masks());
		root = root->parent();
	}

	if(!is_swizzle){ return false; }

	// Merge swizzles
	for( vector<uint32_t>::reverse_iterator it = masks.rbegin();
		it != masks.rend(); ++it )
	{
		if( it == masks.rbegin() ){
			mask_to_indexes(indexes, *it);
			continue;
		}

		char current_indexes[]	= {-1, -1, -1, -1};
		char tmp_indexes[]		= {-1, -1, -1, -1};

		mask_to_indexes(current_indexes, *it);

		for(int i = 0; i < 4; ++i)
		{
			if( current_indexes[i] == -1 ) break;
			tmp_indexes[i] = indexes[ current_indexes[i] ];
		}

		std::copy(tmp_indexes, tmp_indexes+4, indexes);
	}

	return true;
}

cg_value cg_service::create_value_by_scalar( cg_value const& scalar, cg_type* tyinfo, builtin_types hint )
{
	builtin_types src_hint = scalar.hint();
	assert( is_scalar(src_hint) );
	builtin_types dst_hint = tyinfo ? tyinfo->hint() : hint;
	builtin_types dst_scalar = scalar_of(dst_hint);
	assert( dst_scalar == src_hint );

	if( is_scalar(dst_hint) )
	{
		return scalar;
	}
	else if( is_vector(dst_hint) )
	{
		size_t vsize = vector_size(dst_hint);
		vector<cg_value> scalars;
		scalars.insert(scalars.end(), vsize, scalar);
		return create_vector(scalars, scalar.abi());
	}
	else
	{
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	return cg_value();
}

cg_value cg_service::emit_any( cg_value const& v )
{
	builtin_types hint = v.hint();
	builtin_types scalar_hint = scalar_of(v.hint());
	if( is_scalar(hint) )
	{
		return emit_cmp_ne( v, null_value(hint, v.abi()) );
	}
	else if( is_vector(hint) )
	{
		cg_value elem_v = emit_extract_val(v, 0);
		cg_value ret = emit_cmp_ne( elem_v, null_value(scalar_hint, v.abi()) );
		for( size_t i = 1; i < vector_size(hint); ++i )
		{
			elem_v = emit_extract_val(v, i);
			ret = emit_or( ret, emit_cmp_ne( elem_v, null_value(scalar_hint, v.abi()) ) );
		}
		return ret;
	}
	else if( is_matrix(hint) )
	{
		EFLIB_ASSERT_UNIMPLEMENTED();	
	}
	else
	{
		assert(false);
	}
	return cg_value();
}

cg_value cg_service::emit_all( cg_value const& v )
{
	builtin_types hint = v.hint();
	builtin_types scalar_hint = scalar_of(v.hint());
	if( is_scalar(hint) )
	{
		return emit_cmp_ne( v, null_value(hint, v.abi()) );
	}
	else if( is_vector(hint) )
	{
		cg_value elem_v = emit_extract_val(v, 0);
		cg_value ret = emit_cmp_ne( elem_v, null_value(scalar_hint, v.abi()) );
		for( size_t i = 1; i < vector_size(hint); ++i )
		{
			elem_v = emit_extract_val(v, i);
			ret = emit_and( ret, emit_cmp_ne( elem_v, null_value(scalar_hint, v.abi()) ) );
		}
		return ret;
	}
	else if( is_matrix(hint) )
	{
		EFLIB_ASSERT_UNIMPLEMENTED();	
	}
	else
	{
		assert(false);
	}
	return cg_value();
}

cg_value cg_service::emit_cmp( cg_value const& lhs, cg_value const& rhs, uint32_t pred_signed, uint32_t pred_unsigned, uint32_t pred_float )
{
	builtin_types hint = lhs.hint();
	builtin_types scalar_hint = scalar_of(hint);
	builtin_types ret_hint = replace_scalar(hint, builtin_types::_boolean);

	assert( hint == rhs.hint() );
	assert( is_scalar(scalar_hint) );

	abis::id promoted_abi = promote_abi(lhs.abi(), rhs.abi(), abis::llvm);

	Value* lhs_v = lhs.load(promoted_abi);
	Value* rhs_v = rhs.load(promoted_abi);
	Type* ret_ty = type_(ret_hint, promoted_abi);

	binary_intrin_functor cmp_fn;
	if( is_real(scalar_hint) )
	{
		cmp_fn = boost::bind( &DefaultIRBuilder::CreateFCmp, builder(), (CmpInst::Predicate)pred_float, _1, _2, "" );
	}
	else if ( is_integer(scalar_hint) )
	{
		int pred = is_signed(scalar_hint) ? pred_signed : pred_unsigned;
		cmp_fn = boost::bind( &DefaultIRBuilder::CreateICmp, builder(), (CmpInst::Predicate)pred, _1, _2, "" );
	}

	unary_intrin_functor cast_fn = boost::bind( &cg_extension::i1toi8_sv, ext_.get(), _1 );
	Value* ret = ext_->call_binary_intrin( ret_ty, lhs_v, rhs_v, cmp_fn, cast_fn );

	return create_value( ret_hint, ret, value_kinds::value, promoted_abi );
}

cg_value cg_service::emit_cmp_eq( cg_value const& lhs, cg_value const& rhs )
{
	return emit_cmp( lhs, rhs, CmpInst::ICMP_EQ, CmpInst::ICMP_EQ, CmpInst::FCMP_OEQ );
}

cg_value cg_service::emit_cmp_lt( cg_value const& lhs, cg_value const& rhs )
{
	return emit_cmp( lhs, rhs, CmpInst::ICMP_SLT, CmpInst::ICMP_ULT, CmpInst::FCMP_ULT );
}

cg_value cg_service::emit_cmp_le( cg_value const& lhs, cg_value const& rhs )
{
	return emit_cmp( lhs, rhs, CmpInst::ICMP_SLE, CmpInst::ICMP_ULE, CmpInst::FCMP_ULE );
}

cg_value cg_service::emit_cmp_ne( cg_value const& lhs, cg_value const& rhs )
{
	return emit_cmp( lhs, rhs, CmpInst::ICMP_NE, CmpInst::ICMP_NE, CmpInst::FCMP_UNE );
}

cg_value cg_service::emit_cmp_ge( cg_value const& lhs, cg_value const& rhs )
{
	return emit_cmp( lhs, rhs, CmpInst::ICMP_SGE, CmpInst::ICMP_UGE, CmpInst::FCMP_UGE );
}

cg_value cg_service::emit_cmp_gt( cg_value const& lhs, cg_value const& rhs )
{
	return emit_cmp( lhs, rhs, CmpInst::ICMP_SGT, CmpInst::ICMP_UGT, CmpInst::FCMP_UGT );
}

cg_value cg_service::emit_bin_ps_ta_sva( cg_value const& lhs, cg_value const& rhs, binary_intrin_functor signed_sv_fn, binary_intrin_functor unsigned_sv_fn, binary_intrin_functor float_sv_fn )
{
	builtin_types hint( lhs.hint() );
	assert( hint == rhs.hint() );

	Value* ret = NULL;

	builtin_types scalar_hint = is_scalar(hint) ? hint : scalar_of(hint);
	abis::id promoted_abi = promote_abi( rhs.abi(), lhs.abi() );
	abis::id internal_abi = promote_abi( promoted_abi, abis::llvm );

	Value* lhs_v = lhs.load(internal_abi);
	Value* rhs_v = rhs.load(internal_abi);
	
	Type* ret_ty = lhs_v->getType();

	if( is_real(scalar_hint) )
	{
		ret = ext_->call_binary_intrin(ret_ty, lhs_v, rhs_v, float_sv_fn, null_unary);
	}
	else if( is_integer(scalar_hint) ) 
	{
		if( is_signed(scalar_hint) )
		{
			ret = ext_->call_binary_intrin(ret_ty, lhs_v, rhs_v, signed_sv_fn, null_unary);
		}
		else
		{
			ret = ext_->call_binary_intrin(ret_ty, lhs_v, rhs_v, unsigned_sv_fn, null_unary);
		}
	}
	else if( scalar_hint == builtin_types::_boolean )
	{
		ret = ext_->call_binary_intrin(ret_ty, lhs_v, rhs_v, unsigned_sv_fn, null_unary);
	}
	else
	{
		assert(false);
	}

	cg_value retval = create_value( hint, ret, value_kinds::value, internal_abi );
	abis::id ret_abi = is_scalar(hint) ? internal_abi : promoted_abi;
	return create_value( hint, retval.load(ret_abi), value_kinds::value, ret_abi );
}

cg_value cg_service::emit_and( cg_value const& lhs, cg_value const& rhs )
{
	assert( scalar_of( lhs.hint() ) == builtin_types::_boolean );
	assert( lhs.hint() == rhs.hint() );

	return emit_bit_and( lhs, rhs );
}

cg_value cg_service::emit_or( cg_value const& lhs, cg_value const& rhs )
{
	assert( scalar_of( lhs.hint() ) == builtin_types::_boolean );
	assert( lhs.hint() == rhs.hint() );

	return emit_bit_or( lhs, rhs );
}

cg_value cg_service::emit_tex2Dlod( cg_value const& samp, cg_value const& coord )
{
	return emit_tex_lod_impl(samp, coord, externals::tex2dlod_vs, externals::tex2dlod_ps);
}

cg_value cg_service::emit_tex2Dgrad( cg_value const& samp, cg_value const& coord, cg_value const& ddx, cg_value const& ddy )
{
	return emit_tex_grad_impl(samp, coord, ddx, ddy, externals::tex2dgrad_ps);
}

cg_value cg_service::emit_tex2Dbias( cg_value const& samp, cg_value const& coord )
{
	return emit_tex_bias_impl(samp, coord, externals::tex2dbias_ps);
}

cg_value cg_service::emit_tex2Dproj( cg_value const& samp, cg_value const& coord )
{
	return emit_tex_proj_impl(samp, coord, externals::tex2dproj_ps);
}

cg_value cg_service::create_constant_int( cg_type* tyinfo, builtin_types bt, abis::id abi, uint64_t v )
{
	builtin_types hint = tyinfo ? tyinfo->hint() : bt;
	builtin_types scalar_hint = scalar_of(hint);
	assert( is_integer(scalar_hint) || scalar_hint == builtin_types::_boolean );
	uint32_t bits = static_cast<uint32_t>( storage_size(scalar_hint) ) << 3; 

	Type* ret_ty = type_( hint, abi );
	Value* ret = ext_->get_int( ret_ty, APInt( bits, v, is_signed(scalar_hint) ) );
	return create_value(tyinfo, bt, ret, value_kinds::value, abi);
}

cg_value cg_service::emit_unary_ps( std::string const& scalar_external_intrin_name, cg_value const& v )
{
	Function* scalar_intrin = module()->getFunction(scalar_external_intrin_name);
	assert( scalar_intrin );

	unary_intrin_functor intrin_sv = ext_->promote_to_unary_sv(
		ext_->bind_external_to_unary(scalar_intrin), null_unary, null_unary
		);

	Value* ret_v = ext_->call_unary_intrin(NULL, v.load(), intrin_sv);
	return create_value( v.ty(), v.hint(), ret_v, value_kinds::value, v.abi() );
}

cg_value cg_service::emit_bin_ps_ta_sva( std::string const& scalar_external_intrin_name, cg_value const& v0, cg_value const& v1 )
{
	Function* scalar_intrin = module()->getFunction( scalar_external_intrin_name );
	assert( scalar_intrin );

	builtin_types hint = v0.hint();
	assert( hint == v1.hint() );
	abis::id abi = promote_abi( v0.abi(), v1.abi() );

	binary_intrin_functor intrin_sv = ext_->promote_to_binary_sv(
		ext_->bind_external_to_binary(scalar_intrin), null_binary, null_binary
		);

	Value* ret_v = ext_->call_binary_intrin( (Type*)NULL, v0.load(abi), v1.load(abi), intrin_sv, null_unary );

	return create_value( v0.ty(), v0.hint(), ret_v, value_kinds::value, abi );
}

cg_value cg_service::extend_to_vm( cg_value const& v, builtin_types complex_hint )
{
	builtin_types hint = v.hint();
	assert( is_scalar(hint) );

	if( is_vector(complex_hint) )
	{
		vector<cg_value> values(vector_size(complex_hint), v);
		return create_vector( values, v.abi() );
	}

	if( is_matrix(complex_hint) )
	{
		vector<cg_value> values(vector_size(complex_hint), v);
		cg_value vec_v = create_vector( values, v.abi() );
		cg_value ret_v = undef_value(complex_hint, v.abi());
		for( int i = 0; i < (int)vector_count(complex_hint); ++i )
		{
			ret_v = emit_insert_val( ret_v, i, vec_v );
		}
		return ret_v;
	}

	assert(false);
	return cg_value();
}

cg_value cg_service::emit_bin_es_ta_sva( std::string const& scalar_external_intrin_name, cg_value const& lhs, cg_value const& rhs )
{
	builtin_types lhint = lhs.hint();
	builtin_types rhint = rhs.hint();

	assert( lhint != builtin_types::none );
	assert( rhint != builtin_types::none );

	cg_value lv = lhs;
	cg_value rv = rhs;

	if( lhint != rhint )
	{
		if( is_scalar(lhint) ) { lv = extend_to_vm( lhs, rhint ); }
		else if( is_scalar(rhint) ) { rv = extend_to_vm( rhs, lhint ); }
		else { assert(false); }
	}

	return emit_bin_ps_ta_sva( scalar_external_intrin_name, lv, rv );
}

cg_value cg_service::emit_bin_es_ta_sva( cg_value const& lhs, cg_value const& rhs, binary_intrin_functor signed_sv_fn, binary_intrin_functor unsigned_sv_fn, binary_intrin_functor float_sv_fn )
{
	builtin_types lhint = lhs.hint();
	builtin_types rhint = rhs.hint();

	assert( lhint != builtin_types::none );
	assert( rhint != builtin_types::none );

	cg_value lv = lhs;
	cg_value rv = rhs;

	if( lhint != rhint )
	{
		if( is_scalar(lhint) ) { lv = extend_to_vm( lhs, rhint ); }
		else if( is_scalar(rhint) ) { rv = extend_to_vm( rhs, lhint ); }
		else { assert(false); }
	}

	return emit_bin_ps_ta_sva( lv, rv, signed_sv_fn, unsigned_sv_fn, float_sv_fn );
}

cg_value cg_service::emit_tex_lod_impl( cg_value const& samp, cg_value const& coord, externals::id vs_intrin, externals::id ps_intrin )
{
	builtin_types v4f32_hint = vector_of( builtin_types::_float, 4 );
	abis::id abi = param_abi(false);
	assert( abi == abis::llvm || abi == abis::package );

	Type* ret_ty = type_( v4f32_hint, abi );
	Value* ret_ptr = ext_->stack_alloc( ret_ty, "ret.tmp" );

	Type* coord_ty = ret_ty;
	Value* coord_ptr = ext_->stack_alloc(coord_ty, "coord.tmp");
	builder().CreateStore( coord.load(abi), coord_ptr );

	if( abi == abis::llvm)
	{
		builder().CreateCall3(ext_->external(vs_intrin), ret_ptr, samp.load(), coord_ptr);
	}
	else
	{
		builder().CreateCall4( ext_->external(ps_intrin), ret_ptr, fn().packed_execution_mask().load(), samp.load(), coord_ptr );
	}

	return create_value(NULL, v4f32_hint, ret_ptr, value_kinds::reference, abi);
}

cg_value cg_service::emit_tex_grad_impl( cg_value const& samp, cg_value const& coord, cg_value const& ddx, cg_value const& ddy, externals::id ps_intrin )
{
	builtin_types coord_hint = coord.hint();
	builtin_types v4f32_hint = vector_of( builtin_types::_float, 4 );

	abis::id abi = param_abi(false);
	assert( abi == abis::package );

	Type* ret_ty = type_( v4f32_hint, abi );
	Value* ret_ptr = ext_->stack_alloc( ret_ty, "ret.tmp" );

	Type* coord_ty = type_(coord_hint, abi);

	Value* coord_ptr = ext_->stack_alloc(coord_ty, "coord.tmp");
	builder().CreateStore( coord.load(abi), coord_ptr );

	Value* ddx_ptr = ext_->stack_alloc(coord_ty, "ddx.tmp");
	builder().CreateStore( ddx.load(abi), ddx_ptr );

	Value* ddy_ptr = ext_->stack_alloc(coord_ty, "ddy.tmp");
	builder().CreateStore( ddy.load(abi), ddy_ptr );

	Value* args[] = 
	{
		ret_ptr, fn().packed_execution_mask().load(), samp.load(), coord_ptr, ddx_ptr, ddy_ptr
	};

	builder().CreateCall( ext_->external(ps_intrin), args );

	return create_value( NULL, v4f32_hint, ret_ptr, value_kinds::reference, abi );
}

cg_value cg_service::emit_tex_bias_impl( cg_value const& /*samp*/, cg_value const& /*coord*/, externals::id /*ps_intrin*/ )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return cg_value();
}

cg_value cg_service::emit_tex_proj_impl( cg_value const& samp, cg_value const& coord, externals::id ps_intrin )
{
	cg_value ddx = emit_ddx( coord );
	cg_value ddy = emit_ddy( coord );

	builtin_types v4f32_hint = vector_of( builtin_types::_float, 4 );

	abis::id abi = param_abi(false);
	assert( abi == abis::package );

	Type* ret_ty = type_( v4f32_hint, abi );
	Value* ret_ptr = ext_->stack_alloc( ret_ty, "ret.tmp" );

	Type* v4f32_ty = type_( v4f32_hint, abi );

	Value* coord_ptr = ext_->stack_alloc(v4f32_ty, "coord.tmp");
	builder().CreateStore( coord.load(abi), coord_ptr );

	Value* ddx_ptr = ext_->stack_alloc(v4f32_ty, "ddx.tmp");
	builder().CreateStore( ddx.load(abi), ddx_ptr );

	Value* ddy_ptr = ext_->stack_alloc(v4f32_ty, "ddy.tmp");
	builder().CreateStore( coord.load(abi), ddy_ptr );

	Value* args[] = 
	{
		ret_ptr, fn().packed_execution_mask().load(), samp.load(), coord_ptr, ddx_ptr, ddy_ptr
	};
	builder().CreateCall(ext_->external(ps_intrin), args );

	return create_value( NULL, v4f32_hint, ret_ptr, value_kinds::reference, abi );
}

cg_value cg_service::emit_texCUBElod( cg_value const& samp, cg_value const& coord )
{
	return emit_tex_lod_impl(samp, coord, externals::texCUBElod_vs, externals::texCUBElod_ps);
}

cg_value cg_service::emit_texCUBEgrad( cg_value const& samp, cg_value const& coord, cg_value const& ddx, cg_value const& ddy )
{
	return emit_tex_grad_impl(samp, coord, ddx, ddy, externals::texCUBEgrad_ps);
}

cg_value cg_service::emit_texCUBEbias( cg_value const& samp, cg_value const& coord )
{
	return emit_tex_bias_impl(samp, coord, externals::texCUBEbias_ps);
}

cg_value cg_service::emit_texCUBEproj( cg_value const& samp, cg_value const& coord )
{
	return emit_tex_proj_impl(samp, coord, externals::texCUBEproj_ps);
}

node_context* cg_service::get_node_context(node* v)
{
	return ctxt_->get_node_context(v);
}

node_semantic* cg_service::get_node_semantic( sasl::syntax_tree::node* v )
{
	return sem_->get_semantic(v);
}

cg_value cg_service::emit_select( cg_value const& flag, cg_value const& v0, cg_value const& v1 )
{
	abis::id promoted_abi = promote_abi(flag.abi(), v0.abi(), v1.abi() );

	Value* flag_v = flag.load(promoted_abi);
	Value* v0_v = v0.load(promoted_abi);
	Value* v1_v = v1.load(promoted_abi);

	return create_value(
		v0.ty(), v0.hint(), 
		ext_->select(flag_v, v0_v, v1_v), value_kinds::value, promoted_abi
		);
}

cg_value cg_service::emit_not( cg_value const& v )
{
	cg_value mask_value = create_constant_int( NULL, v.hint(), v.abi(), 1 );
	return emit_bit_xor( mask_value, v );
}

cg_value cg_service::inf_from_value( cg_value const& v, bool negative )
{
	Value* v_v = v.load();
	builtin_types scalar_of_v = scalar_of( v.hint() );
	Type* scalar_ty = type_(scalar_of_v, abis::llvm);
	Value* scalar_inf = ConstantFP::getInfinity(scalar_ty, negative);
	Value* inf_value = ext_->get_constant_by_scalar(v_v->getType(), scalar_inf);
	return create_value( v.ty(), v.hint(), inf_value, value_kinds::value, v.abi() );
}

cg_value cg_service::emit_isinf(cg_value const& v)
{
	cg_value abs_v = emit_abs(v);
	return emit_cmp(abs_v, inf_from_value(v, false), ICmpInst::ICMP_EQ, ICmpInst::ICMP_EQ, FCmpInst::FCMP_OEQ);
}

cg_value cg_service::emit_isfinite( cg_value const& v )
{
	cg_value is_eq = emit_cmp(v, v, ICmpInst::ICMP_EQ, ICmpInst::ICMP_EQ, FCmpInst::FCMP_OEQ);
	cg_value is_inf = emit_isinf(v);
	return emit_and(emit_not(is_inf), is_eq);
}

cg_value cg_service::emit_isnan( cg_value const& v )
{
	return emit_cmp(v, v, ICmpInst::ICMP_EQ, ICmpInst::ICMP_EQ, FCmpInst::FCMP_UNO);
}

cg_value cg_service::one_value( cg_value const& proto )
{
	return numeric_value(proto, 1.0f, 1);
}

cg_value cg_service::emit_sign( cg_value const& v )
{
	builtin_types ret_btc = replace_scalar(v.hint(), builtin_types::_sint32);
	cg_value zero = null_value( v.hint(), v.abi() );
	cg_value i_zero = null_value( ret_btc, v.abi() );
	cg_value i_one = one_value( i_zero );
	cg_value i_neg_one = emit_sub(i_zero, i_one);

	cg_value v0 = emit_select( emit_cmp_lt(v, zero), i_neg_one, i_zero );
	cg_value v1 = emit_select( emit_cmp_gt(v, zero), i_one, i_zero );
	return emit_add(v0, v1);
}

cg_value cg_service::emit_clamp( cg_value const& v, cg_value const& min_v, cg_value const& max_v )
{
	cg_value ret = emit_select(emit_cmp_ge(v, min_v), v, min_v);
	return emit_select(emit_cmp_le(ret, max_v), ret, max_v);
}

cg_value cg_service::emit_saturate( cg_value const& v )
{
	return emit_clamp(v, null_value( v.hint(), v.abi() ), one_value(v) );
}

cg_value cg_service::numeric_value(cg_value const& proto, double fp, uint64_t ui)
{
	Type* ty = NULL;
	if( proto.ty() ){
		ty = proto.ty()->ty( proto.abi() );
	} else {
		ty = type_( proto.hint(), proto.abi() );
	}

	Type* scalar_ty = ext_->extract_scalar_type(ty);

	Value* scalar_value = NULL;
	if( scalar_ty->isFloatingPointTy() )
	{
		scalar_value = ConstantFP::get(scalar_ty, fp);
	}
	else if( scalar_ty->isIntegerTy() )
	{
		scalar_value = ConstantInt::get(scalar_ty, ui);
	}

	assert(scalar_value);

	Value* ret_value = ext_->get_constant_by_scalar(ty, scalar_value);
	return create_value( proto.ty(), proto.hint(), ret_value, value_kinds::value, proto.abi() );
}

cg_extension* cg_service::extension()
{
	return ext_.get();
}

void cg_service::function_body_beg()
{
	ext_->set_stack_alloc_point( fn().allocation_block().block );
}

void cg_service::function_body_end()
{
	ext_->set_stack_alloc_point(NULL);
}

END_NS_SASL_CODEGEN();
