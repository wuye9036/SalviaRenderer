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

cg_service::cg_service(size_t parallel_factor)
	: llvm_mod_(NULL), ctxt_(NULL), sem_(NULL)
	, parallel_factor_(parallel_factor)
{
}

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

	if( parallel_factor_ > 1 && ret->partial_execution ){
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

multi_value cg_service::null_value( cg_type* tyinfo, abis::id abi )
{
	assert( tyinfo && abi != abis::unknown );
	Type* value_type = tyinfo->ty(abi);
	assert( value_type );
	vector<Value*> values( parallel_factor_, Constant::getNullValue(value_type) );
	multi_value val = create_value(tyinfo, values, value_kinds::value, abi);
	return val;
}

multi_value cg_service::null_value( builtin_types bt, abis::id abi )
{
	assert( bt != builtin_types::none );
	Type* value_type = type_( bt, abi );
	vector<Value*> values( parallel_factor_, Constant::getNullValue(value_type) );
	multi_value val = create_value(bt, values, value_kinds::value, abi);
	return val;
}

value_array cg_service::invalid_value_array()
{
	return value_array(parallel_factor_, NULL);
}

multi_value cg_service::create_value(
	cg_type* tyinfo, value_array const& values,
	value_kinds::id k, abis::id abi)
{
	return multi_value( tyinfo, values, k, abi, this );
}

multi_value cg_service::create_value(
	builtin_types hint, value_array const& values,
	value_kinds::id k, abis::id abi)
{
	return multi_value( hint, values, k, abi, this );
}

multi_value cg_service::create_value( cg_type* tyinfo, builtin_types hint, value_array const& values, value_kinds::id k, abis::id abi )
{
	if( tyinfo ){
		return create_value(tyinfo, values, k, abi);
	} else {
		return create_value(hint,   values, k ,abi);
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
	} else {
		if( tyn->is_struct() )
		{
			shared_ptr<struct_type> struct_tyn = tyn->as_handle<struct_type>();

			vector<Type*> c_member_types;
			vector<Type*> llvm_member_types;

			BOOST_FOREACH( shared_ptr<declaration> const& decl, struct_tyn->decls){
				if( decl->node_class() == node_ids::variable_declaration ){
					shared_ptr<variable_declaration> decl_tyn = decl->as_handle<variable_declaration>();
					cg_type* decl_cgty = create_ty( sem_->get_semantic(decl_tyn->type_info)->ty_proto() );
					size_t declarator_count = decl_tyn->declarators.size();
					c_member_types.insert( c_member_types.end(), declarator_count, decl_cgty->ty(abis::c) );
					llvm_member_types.insert( llvm_member_types.end(), declarator_count, decl_cgty->ty(abis::llvm) );
				}
			}

			StructType* ty_c	= StructType::create( c_member_types,			struct_tyn->name->str + ".abi.c" );
			StructType* ty_llvm	= StructType::create( llvm_member_types,		struct_tyn->name->str + ".abi.llvm" );

			ret->tys[abis::c]	= ty_c;
			ret->tys[abis::llvm]	= ty_llvm;
		}
		else if( tyn->is_array() )
		{
			array_type*	array_tyn	= polymorphic_cast<array_type*>(tyn);
			cg_type*	elem_ti		= create_ty( array_tyn->elem_type.get() );

			ret->tys[abis::c]		= PointerType::getUnqual( elem_ti->ty(abis::c) );
			ret->tys[abis::llvm]		= PointerType::getUnqual( elem_ti->ty(abis::llvm) );
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

multi_value cg_service::create_variable( builtin_types bt, abis::id abi, std::string const& name )
{
	Type* vty = type_( bt, abi );
	value_array values(parallel_factor_, NULL);
	std::generate(
		values.begin(), values.end(),
		boost::bind(&cg_extension::stack_alloc, ext_.get(), vty, name)
		);
	return create_value( bt, values, value_kinds::reference, abi );
}

multi_value cg_service::create_variable( cg_type const* ty, abis::id abi, std::string const& name )
{
	Type* vty = type_( ty, abi );
	value_array values(parallel_factor_, NULL);
	std::generate(
		values.begin(), values.end(),
		boost::bind(&cg_extension::stack_alloc, ext_.get(), vty, name)
		);
	return create_value( const_cast<cg_type*>(ty), values, value_kinds::reference, abi );
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

Type* cg_service::type_(builtin_types bt, abis::id abi)
{
	assert( abi != abis::unknown );
	return get_llvm_type( context(), bt, abi );
}

Type* cg_service::type_(cg_type const* ty, abis::id abi)
{
	assert( ty->ty(abi) );
	return ty->ty(abi);
}

value_array cg_service::load(multi_value const& v)
{
	value_kinds::id kind = v.kind();
	value_array raw = v.raw();
	uint32_t masks = v.masks();
	value_array ret = value_array(parallel_factor_, NULL);

	assert( kind != value_kinds::unknown && kind != value_kinds::ty_only );

	// Resolve index/swizzle/write mask
	if( kind == value_kinds::reference || kind == value_kinds::value )
	{
		ret = raw;
	}
	else if( ( kind & (~value_kinds::reference) ) == value_kinds::elements )
	{
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
				ret = emit_extract_val( v.parent()->to_rvalue(), index_values[0] ).load();
			} else {
				// Multi-members must be swizzle/writemask.
				assert( (kind & value_kinds::reference) == 0 );
				multi_value resolved = emit_extract_elem_mask( v.parent()->to_rvalue(), masks );
				return resolved.load( v.abi() );
			}
		}
		else
		{
			assert( v.index() );
			ret = emit_extract_val( v.parent()->to_rvalue(), *v.index() ).load();
		}
	} else {
		assert(false);
	}

	// Resolve reference
	if( kind & value_kinds::reference ){
		for(size_t i = 0; i < parallel_factor_; ++i)
		{
			ret[i] = builder().CreateLoad(ret[i]);
		}
	}

	return ret;
}

value_array cg_service::load( multi_value const& v, abis::id abi )
{
	return load_as(v, abi);
}

value_array cg_service::load_ref( multi_value const& v )
{
	value_kinds::id kind = v.kind();

	switch(kind)
	{
	case value_kinds::reference:
		{
			return v.raw();
		}
	case value_kinds::elements|value_kinds::reference:
		{
			multi_value non_ref( v );
			non_ref.kind( value_kinds::elements );
			return non_ref.load();
		}
	case value_kinds::elements:
		{
			assert( v.masks() );
			return emit_extract_elem_mask( *v.parent(), v.masks() ).load_ref();
		}
	}

	return value_array(parallel_factor_, NULL);
}

value_array cg_service::load_ref( multi_value const& v, abis::id abi )
{
	if( v.abi() == abi || v.hint() == builtin_types::_sampler )
	{
		return load_ref(v);
	}
	else
	{
		return value_array(parallel_factor_, NULL);
	}
}

value_array cg_service::load_as( multi_value const& v, abis::id abi )
{
	assert( abi != abis::unknown );

	if( v.abi() == abi ){ return v.load(); }

	switch( v.abi() )
	{
	case abis::c:
		if( abi == abis::llvm ){
			return load_as_llvm_c(v, abi);
		}
		break;
	case abis::llvm:
		if( abi == abis::c ){
			return load_as_llvm_c(v, abi);
		}
		break;
	}

	assert(false);
	return value_array(parallel_factor_, NULL);
}

value_array cg_service::load_as_llvm_c(multi_value const& v, abis::id abi)
{
	builtin_types hint = v.hint();

	if( is_scalar(hint) || is_sampler(hint) ){
		return v.load();
	} else if( is_vector( hint ) ){
		multi_value ret_value = undef_value( hint, abi );

		size_t vec_size = vector_size( hint );
		for( size_t i = 0; i < vec_size; ++i ){
			ret_value = emit_insert_val( ret_value, (int)i, emit_extract_elem(v, i) );
		}

		return ret_value.load();
	} else if( is_matrix( hint ) ){
		multi_value ret_value = null_value( hint, abi );
		size_t vec_count = vector_count( hint );
		for( size_t i = 0; i < vec_count; ++i ){
			multi_value org_vec = emit_extract_val(v, (int)i);
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

	return value_array(parallel_factor_, NULL);
}

multi_value cg_service::emit_insert_val( multi_value const& lhs, multi_value const& idx, multi_value const& elem_value )
{
	value_array indexes = idx.load();
	value_array agg = lhs.load();
	value_array new_value(parallel_factor_, NULL);
	value_array elem = elem_value.load();

	for(int value_index = 0; value_index < parallel_factor_; ++value_index)
	{
		Type* value_type = agg[value_index]->getType();
		if( value_type->isStructTy() )
		{
			assert(false);
		}
		else if ( value_type->isVectorTy() )
		{
			new_value[value_index] = builder().CreateInsertElement(
					agg[value_index], elem[value_index], indexes[value_index]
				);
		}
	}

	assert( valid_all(new_value) );

	return create_value( lhs.ty(), lhs.hint(), new_value, value_kinds::value, lhs.abi() );
}

multi_value cg_service::emit_insert_val(multi_value const& lhs, int index, multi_value const& elem_value)
{
	assert(index >= 0);

	value_array agg = lhs.load();
	value_array new_value = value_array(parallel_factor_, NULL);
	value_array elem_vm_values = elem_value.load(lhs.abi());
	for(int value_index = 0; value_index < parallel_factor_; ++value_index)
	{
		Type* value_type = agg[value_index]->getType();
		if( value_type->isStructTy() )
		{
			new_value[value_index] = builder().CreateInsertValue(
				agg[value_index], elem_vm_values[value_index], static_cast<unsigned>(index)
				);
		}
		else if ( value_type->isVectorTy() )
		{
			Value* vm_index = ext_->get_int(index);
			value_array index_vm_values(parallel_factor_, vm_index);
			multi_value index_value = create_value(
				builtin_types::_sint32, index_vm_values, value_kinds::value, abis::llvm
				);
			return emit_insert_val(lhs, index_value, elem_value);
		}
	}

	assert( valid_all(new_value) );
	return create_value( lhs.ty(), lhs.hint(), new_value, value_kinds::value, lhs.abi() );
}

abis::id cg_service::promote_abi( abis::id abi0, abis::id abi1 )
{
	if( abi0 == abis::c ){ return abi1; }
	if( abi1 == abis::c ){ return abi0; }
	if( abi0 == abis::llvm ){ return abi1; }
	if( abi1 == abis::llvm ){ return abi0; }
	return abi0;
}

abis::id cg_service::promote_abi( abis::id abi0, abis::id abi1, abis::id abi2 )
{
	return promote_abi(promote_abi(abi0, abi1), abi2);
}

multi_value cg_service::emit_mul_comp( multi_value const& lhs, multi_value const& rhs )
{
	builtin_types lhint = lhs.hint();
	builtin_types rhint = rhs.hint();

	assert( lhint != builtin_types::none );
	assert( rhint != builtin_types::none );

	multi_value lv = lhs;
	multi_value rv = rhs;

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

multi_value cg_service::emit_add( multi_value const& lhs, multi_value const& rhs )
{
	binary_intrin_functor f_add = boost::bind( &DefaultIRBuilder::CreateFAdd, builder(), _1, _2, "", (llvm::MDNode*)(NULL) );
	binary_intrin_functor i_add = boost::bind( &DefaultIRBuilder::CreateAdd,  builder(), _1, _2, "", false, false );

	return emit_bin_es_ta_sva( lhs, rhs, i_add, i_add, f_add );
}

multi_value cg_service::emit_sub( multi_value const& lhs, multi_value const& rhs )
{
	binary_intrin_functor f_sub = boost::bind( &DefaultIRBuilder::CreateFSub, builder(), _1, _2, "", (llvm::MDNode*)(NULL) );
	binary_intrin_functor i_sub = boost::bind( &DefaultIRBuilder::CreateSub,  builder(), _1, _2, "", false, false );
	
	return emit_bin_es_ta_sva( lhs, rhs, i_sub, i_sub, f_sub );
}

multi_value cg_service::emit_mul_intrin( multi_value const& lhs, multi_value const& rhs )
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
	return multi_value();
}

multi_value cg_service::emit_div( multi_value const& lhs, multi_value const& rhs )
{
	binary_intrin_functor f_div = boost::bind( &DefaultIRBuilder::CreateFDiv, builder(), _1, _2, "", (llvm::MDNode*)(NULL) );
	binary_intrin_functor i_div = boost::bind( &DefaultIRBuilder::CreateSDiv, builder(), _1, _2, "", false );
	binary_intrin_functor u_div = boost::bind( &DefaultIRBuilder::CreateUDiv, builder(), _1, _2, "", false );
	binary_intrin_functor i_safe_div = boost::bind( &cg_extension::safe_idiv_imod_sv, ext_.get(), _1, _2, i_div );
	binary_intrin_functor u_safe_div = boost::bind( &cg_extension::safe_idiv_imod_sv, ext_.get(), _1, _2, u_div );

	return emit_bin_es_ta_sva( lhs, rhs, i_safe_div, u_safe_div, f_div );
}

multi_value cg_service::emit_mod( multi_value const& lhs, multi_value const& rhs )
{	
	binary_intrin_functor i_mod = boost::bind( &DefaultIRBuilder::CreateSRem, builder(), _1, _2, "" );
	binary_intrin_functor u_mod = boost::bind( &DefaultIRBuilder::CreateURem, builder(), _1, _2, "" );
		
	binary_intrin_functor i_safe_mod = boost::bind( &cg_extension::safe_idiv_imod_sv, ext_.get(), _1, _2, i_mod );
	binary_intrin_functor u_safe_mod = boost::bind( &cg_extension::safe_idiv_imod_sv, ext_.get(), _1, _2, u_mod );

	binary_intrin_functor intrin_mod_f32 = ext_->bind_external_to_binary(externals::mod_f32);
	binary_intrin_functor f_mod_sv =
		ext_->promote_to_binary_sv(intrin_mod_f32, null_binary, null_binary);
	binary_intrin_functor f_mod =
		boost::bind(&cg_extension::call_binary_intrin_mono, ext_.get(), (Type*)NULL, _1, _2, f_mod_sv, null_unary);

	return emit_bin_es_ta_sva( lhs, rhs, i_safe_mod, u_safe_mod, f_mod );
}

multi_value cg_service::emit_lshift( multi_value const& lhs, multi_value const& rhs )
{
	binary_intrin_functor shl = boost::bind( &DefaultIRBuilder::CreateBinOp, builder(), Instruction::Shl, _1, _2, "" );
	return emit_bin_es_ta_sva( lhs, rhs, shl, shl, shl );
}

multi_value cg_service::emit_rshift( multi_value const& lhs, multi_value const& rhs )
{
	binary_intrin_functor shr = boost::bind( &DefaultIRBuilder::CreateBinOp, builder(), Instruction::LShr, _1, _2, "" );
	return emit_bin_es_ta_sva( lhs, rhs, shr, shr, shr );
}

multi_value cg_service::emit_bit_and( multi_value const& lhs, multi_value const& rhs )
{
	binary_intrin_functor bit_and = boost::bind( &DefaultIRBuilder::CreateBinOp, builder(), Instruction::And, _1, _2, "" );
	return emit_bin_es_ta_sva( lhs, rhs, bit_and, bit_and, bit_and );
}

multi_value cg_service::emit_bit_or( multi_value const& lhs, multi_value const& rhs )
{
	binary_intrin_functor bit_or = boost::bind( &DefaultIRBuilder::CreateBinOp, builder(), Instruction::Or, _1, _2, "" );
	return emit_bin_es_ta_sva( lhs, rhs, bit_or, bit_or, bit_or );
}

multi_value cg_service::emit_bit_xor( multi_value const& lhs, multi_value const& rhs )
{
	binary_intrin_functor bit_xor = boost::bind( &DefaultIRBuilder::CreateBinOp, builder(), Instruction::Xor, _1, _2, "" );
	return emit_bin_es_ta_sva( lhs, rhs, bit_xor, bit_xor, bit_xor );
}

multi_value cg_service::emit_dot( multi_value const& lhs, multi_value const& rhs )
{
	return emit_dot_vv(lhs, rhs);
}

multi_value cg_service::emit_cross( multi_value const& lhs, multi_value const& rhs )
{
	assert( lhs.hint() == vector_of( builtin_types::_float, 3 ) );
	assert( rhs.hint() == lhs.hint() );

	uint32_t swz_va = indexes_to_mask( 1, 2, 0, -1 );
	uint32_t swz_vb = indexes_to_mask( 2, 0, 1, -1 );

	multi_value lvec_a = emit_extract_elem_mask( lhs, swz_va );
	multi_value lvec_b = emit_extract_elem_mask( lhs, swz_vb );
	multi_value rvec_a = emit_extract_elem_mask( rhs, swz_va );
	multi_value rvec_b = emit_extract_elem_mask( rhs, swz_vb );

	return emit_sub( emit_mul_comp(lvec_a, rvec_b), emit_mul_comp(lvec_b, rvec_a) );
}

multi_value cg_service::emit_extract_ref( multi_value const& lhs, int idx )
{
	assert( lhs.storable() );

	builtin_types agg_hint = lhs.hint();

	if( is_vector(agg_hint) ){
		char indexes[4] = { (char)idx, -1, -1, -1 };
		uint32_t mask = indexes_to_mask( indexes );
		return multi_value::slice( lhs, mask );
	} else if( is_matrix(agg_hint) ){
		EFLIB_ASSERT_UNIMPLEMENTED();
		return multi_value();
	} else if ( agg_hint == builtin_types::none ){
		value_array agg_address = lhs.load_ref();

		value_array elem_address(parallel_factor_, NULL);
		for(size_t value_index = 0; value_index < parallel_factor_; ++value_index)
		{
			elem_address[value_index] = builder().CreateStructGEP(
				agg_address[value_index], static_cast<unsigned>(idx)
				);
		}

		cg_type* tyinfo = NULL;
		if( lhs.ty() ){
			tyinfo = member_tyinfo( lhs.ty(), (size_t)idx );
		}
		return create_value( tyinfo, elem_address, value_kinds::reference, lhs.abi() );
	}
	EFLIB_ASSERT_UNIMPLEMENTED();
	return multi_value();
}

multi_value cg_service::emit_extract_ref( multi_value const& lhs, multi_value const& idx )
{
	assert( lhs.storable() );
	
	abis::id promoted_abi = promote_abi( lhs.abi(), idx.abi() );
	builtin_types agg_hint = lhs.hint();

	if( is_vector(agg_hint) )
	{
		return multi_value::slice( lhs, idx );
	}
	else if( is_matrix(agg_hint) )
	{
		value_array addr = lhs.load_ref();
		switch (promoted_abi)
		{
		case abis::c:
		case abis::llvm:
			{
				value_array index_values = idx.load();
				value_array elem_ptr(parallel_factor_, NULL);
				for(size_t value_index = 0; value_index < parallel_factor_; ++value_index)
				{
					Type*  value_ty = addr[value_index]->getType()->getPointerElementType();
					Type*  element_ty = value_ty->getStructElementType(0);

					Value* first_elem_ptr = builder().CreateBitCast( addr[value_index], PointerType::getUnqual(element_ty) );
					Value* indexes[] = { index_values[value_index] };
					elem_ptr[value_index] = builder().CreateGEP(first_elem_ptr, indexes);
				}
				return create_value(NULL, row_vector_of(lhs.hint()), elem_ptr, value_kinds::reference, lhs.abi() );
			}
		default:
			assert(false);
		}
		return multi_value();
	}
	else if ( agg_hint == builtin_types::none )
	{
		// Array only
		value_array addr = lhs.load_ref();
		assert( valid_all(addr) );
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
				cg_type* elem_tyinfo = ctxt_->get_node_context( array_tyn->elem_type.get() )->ty;
				value_array index_values = idx.load();
				value_array elem_addr(parallel_factor_, NULL);
				for(size_t value_index = 0; value_index < parallel_factor_; ++value_index)
				{
					elem_addr[value_index] = builder().CreateGEP(addr[value_index], index_values[value_index]);
				}
				return create_value( elem_tyinfo, elem_addr, value_kinds::reference, lhs.abi() );
			}
		default:
			assert(false);
		}
	}
	return multi_value();
}

multi_value cg_service::emit_extract_val( multi_value const& lhs, int idx )
{
	builtin_types agg_hint = lhs.hint();
	value_array val = lhs.load();
	value_array elem_val(parallel_factor_, NULL);
	abis::id abi = abis::unknown;
	builtin_types elem_hint = builtin_types::none;
	cg_type* elem_tyi = NULL;

	if( agg_hint == builtin_types::none ){
		elem_val = ext_->extract_value(val, idx);
		abi = lhs.abi();
		elem_tyi = member_tyinfo( lhs.ty(), static_cast<size_t>(idx) );
	} else if( is_scalar(agg_hint) ){
		assert( idx == 0 );
		elem_val = val;
		elem_hint = agg_hint;
	} else if( is_vector(agg_hint) ){
		switch( lhs.abi() ){
		case abis::c:
			elem_val = ext_->extract_value(val, idx);
			break;
		case abis::llvm:
			{
				Value* vm_index = ext_->get_int(idx);
				for(size_t value_index = 0; value_index < parallel_factor_; ++value_index)
				{
					elem_val[value_index] = builder().CreateExtractElement(
						val[value_index], vm_index
						);
				}
			}
			break;
		default:
			assert(!"Unknown ABI");
			break;
		}
		abi = promote_abi( abis::llvm, lhs.abi() );
		elem_hint = scalar_of(agg_hint);
	} else if( is_matrix(agg_hint) ){
		assert( promote_abi(lhs.abi(), abis::llvm) == abis::llvm );
		elem_val = ext_->extract_value(val, idx);
		abi = lhs.abi();
		elem_hint = vector_of( scalar_of(agg_hint), vector_size(agg_hint) );
	}

	return create_value( elem_tyi, elem_hint, elem_val, value_kinds::value, abi );
}

multi_value cg_service::emit_extract_val( multi_value const& lhs, multi_value const& idx )
{
	builtin_types agg_hint = lhs.hint();

	value_array elem_val(parallel_factor_, NULL);
	abis::id abi = promote_abi(lhs.abi(), idx.abi());

	builtin_types elem_hint = builtin_types::none;
	cg_type* elem_tyi = NULL;

	if( agg_hint == builtin_types::none ){
		// Array only
		value_array addr = lhs.load_ref();
		assert( valid_all(addr) );
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
				value_array elem_addr = ext_->gep( addr, idx.load() );
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
			elem_val = ext_->extract_element( lhs.load(abis::llvm), idx.load() );
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

multi_value cg_service::emit_extract_elem_mask( multi_value const& vec, uint32_t mask )
{
	char indexes[4] = {-1, -1, -1, -1};
	mask_to_indexes( indexes, mask );
	uint32_t idx_len = indexes_length(indexes);

	assert( idx_len > 0 );
	if( vec.hint() == builtin_types::none && idx_len == 1 ){
		// struct, array or not-package, return extract elem.
		// Else do extract mask.
		if(vec.hint() == builtin_types::none){
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
		multi_value swz_proxy = create_value( NULL, swz_hint, invalid_value_array(), value_kinds::elements, vec.abi() );
		swz_proxy.parent(vec);
		swz_proxy.masks(mask);
		return swz_proxy;
	} else {
		if( is_scalar( vec.hint() ) ) {
			EFLIB_ASSERT_UNIMPLEMENTED();
		} else if( is_vector( vec.hint() ) ) {
			value_array vec_v = vec.load( promote_abi(abis::llvm, vec.abi()) );
			switch( vec.abi() ){
			case abis::c:
			case abis::llvm:
				{
					Value* mask_mono = ext_->get_vector<int>( ArrayRef<char>(indexes, idx_len) );
					value_array mask(parallel_factor_, mask_mono);
					value_array v = ext_->shuffle_vector(vec_v, vec_v, mask);
					return create_value(NULL, swz_hint, v, value_kinds::value, abis::llvm);
				}
			default:
				assert(false);
			}
		} else if( is_matrix(vec.hint()) ){
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	}

	return multi_value();
}

multi_value cg_service::emit_extract_col( multi_value const& lhs, size_t index )
{
	assert( promote_abi(lhs.abi(), abis::llvm) == abis::llvm );

	multi_value val = lhs.to_rvalue();
	builtin_types mat_hint( lhs.hint() );
	assert( is_matrix(mat_hint) );

	size_t row_count = vector_count( mat_hint );

	builtin_types out_hint = vector_of( scalar_of(mat_hint), row_count );

	multi_value out_value = null_value( out_hint, lhs.abi() );
	for( size_t irow = 0; irow < row_count; ++irow ){
		multi_value row = emit_extract_val( val, (int)irow );
		multi_value cell = emit_extract_val( row, (int)index );
		out_value = emit_insert_val( out_value, (int)irow, cell );
	}

	return out_value;
}

multi_value cg_service::emit_dot_vv( multi_value const& lhs, multi_value const& rhs )
{
	abis::id promoted_abi = promote_abi(lhs.abi(), rhs.abi(), abis::llvm);
	// assert( promoted_abi == abis::llvm );
	
	size_t vec_size = vector_size( lhs.hint() );
	multi_value total = null_value( scalar_of( lhs.hint() ), promoted_abi );
	multi_value prod = emit_mul_comp( lhs, rhs );
	for( size_t i = 0; i < vec_size; ++i ){
		multi_value prod_elem = emit_extract_elem( prod, i );
		total.emplace( emit_add( total, prod_elem ).to_rvalue() );
	}

	return total;
}

multi_value cg_service::emit_mul_mv( multi_value const& lhs, multi_value const& rhs )
{
	assert( promote_abi(lhs.abi(), rhs.abi(), abis::llvm) == abis::llvm );

	builtin_types mhint = lhs.hint();
	builtin_types vhint = rhs.hint();

	size_t row_count = vector_count(mhint);

	builtin_types ret_hint = vector_of( scalar_of(vhint), row_count );

	multi_value ret_v = null_value( ret_hint, lhs.abi() );
	for( size_t irow = 0; irow < row_count; ++irow ){
		multi_value row_vec = emit_extract_val( lhs, irow );
		ret_v = emit_insert_val( ret_v, irow, emit_dot_vv(row_vec, rhs) );
	}

	return ret_v;
}

multi_value cg_service::emit_mul_vm( multi_value const& lhs, multi_value const& rhs )
{
	assert( promote_abi(lhs.abi(), rhs.abi(), abis::llvm) == abis::llvm );

	size_t out_v = vector_size( rhs.hint() );

	multi_value lrv = lhs.to_rvalue();
	multi_value rrv = rhs.to_rvalue();

	multi_value ret = null_value( vector_of( scalar_of(lhs.hint()), out_v ), lhs.abi() );
	for( size_t idx = 0; idx < out_v; ++idx ){
		ret = emit_insert_val( ret, (int)idx, emit_dot_vv( lrv, emit_extract_col(rrv, idx) ) );
	}

	return ret;
}

multi_value cg_service::emit_mul_mm( multi_value const& lhs, multi_value const& rhs )
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

	vector<multi_value> out_cells(out_v*out_r);
	out_cells.resize( out_v*out_r );

	// Calculate matrix cells.
	for( size_t icol = 0; icol < out_v; ++icol){
		multi_value col = emit_extract_col( rhs, icol );
		for( size_t irow = 0; irow < out_r; ++irow )
		{
			multi_value row = emit_extract_col( rhs, icol );
			out_cells[irow*out_v+icol] = emit_dot_vv( col, row );
		}
	}

	// Compose cells to matrix
	multi_value ret_value = null_value( out_hint, out_abi );
	for( size_t irow = 0; irow < out_r; ++irow ){
		multi_value row_vec = null_value( out_row_hint, out_abi );
		for( size_t icol = 0; icol < out_v; ++icol ){
			row_vec = emit_insert_val( row_vec, (int)icol, out_cells[irow*out_v+icol] );
		}
		ret_value = emit_insert_val( ret_value, (int)irow, row_vec );
	}

	return ret_value;
}

multi_value cg_service::emit_abs( multi_value const& arg_value )
{
	builtin_types hint = arg_value.hint();
	builtin_types scalar_hint = scalar_of( arg_value.hint() );
	abis::id arg_abi = arg_value.abi();

	value_array v = arg_value.load(arg_abi);
	value_array ret_v = ext_->call_unary_intrin( v[0]->getType(), v, boost::bind(&cg_extension::abs_sv, ext_.get(), _1) );
	return create_value(arg_value.ty(), hint, ret_v, value_kinds::value, arg_abi);
}

multi_value cg_service::emit_sqrt( multi_value const& arg_value )
{
	builtin_types hint = arg_value.hint();
	builtin_types scalar_hint = scalar_of( arg_value.hint() );
	abis::id arg_abi = arg_value.abi();

	value_array v = arg_value.load(arg_abi);

	if( scalar_hint == builtin_types::_float )
	{
		unary_intrin_functor sqrt_sv = ext_->promote_to_unary_sv(
			ext_->bind_to_unary( ext_->vm_intrin<float(float)>(Intrinsic::sqrt) ),
			null_unary,
			ext_->bind_to_unary( ext_->vm_intrin(Intrinsic::x86_sse_sqrt_ps) )
			);
		value_array ret_v = ext_->call_unary_intrin(NULL, v, sqrt_sv);
		return create_value( arg_value.ty(), arg_value.hint(), ret_v, value_kinds::value, arg_abi );
	}
	else
	{
		EFLIB_ASSERT_UNIMPLEMENTED();
		return multi_value();
	}
}

multi_value cg_service::undef_value( builtin_types bt, abis::id abi )
{
	assert( bt != builtin_types::none );
	Type* valty = type_( bt, abi );
	multi_value val = create_value( bt, value_array( parallel_factor_, UndefValue::get(valty) ), value_kinds::value, abi );
	return val;
}

multi_value cg_service::emit_call( cg_function const& fn, vector<multi_value> const& args )
{
	return emit_call(fn, args, NULL);
}

/**
Argument passing rules:

   Arg Passing   |    Mono LLVM ABI    |    Mono C ABI   |  Multi-Value LLVM ABI  |   Multi-Value C ABI
-----------------|---------------------|-----------------|------------------------|-----------------------
Ret  Value small |     direct, <v>     | 1st arg, <&ret> |   direct, array<ret>   | 1st arg, &array<&ret>
-----------------|---------------------|-----------------|------------------------|-----------------------
Ret  Value big   |     direct, <v>     | 1st arg, <&ret> |   direct, array<ret>   | 1st arg, &array<&ret>
-----------------|---------------------|-----------------|------------------------|-----------------------
Out  Value small |        <&v>         |      <&v>       |       array<&v>        |      &array<&v>
-----------------|---------------------|-----------------|------------------------|-----------------------
Out  Value big   |        <&v>         |      <&v>       |       array<&v>        |      &array<&v>
-----------------|---------------------|-----------------|------------------------|-----------------------
In   Value small |         <v>         |       <v>       |        array<v>        |      &array <v>
-----------------|---------------------|-----------------|------------------------|-----------------------
In   Value big   |         <v>         |      <&v>       |        array<v>        |      &array <v>
-----------------|---------------------|-----------------|------------------------|-----------------------
Mask Value       |         N/A         |       N/A       |        uint32_t        |       uint32_t
*/
multi_value cg_service::emit_call( cg_function const& fn, vector<multi_value> const& args, Value* exec_mask )
{
	abis::id arg_abi = fn.c_compatible ? abis::c : abis::llvm;

	vector<Value*>	arg_multi_values(fn.physical_args_count(), NULL);
	vector<Value*>	physical_args   (fn.physical_args_count(), NULL);

	// Create temporary variable for catching result.
	multi_value ret_variable(parallel_factor_);

	// Fill values for arguments.
	{
		size_t physical_index = 0;
	
		// Add return address to args.
		if( fn.return_via_arg() )
		{
			ret_variable = create_variable(fn.result_type(), fn.abi(), ".temp.result");
			arg_multi_values = ret_variable.load_ref();
			if( fn.multi_value_args() )
			{
				physical_args[physical_index++] = ext_->get_array(arg_multi_values);
			}
			else
			{
				physical_args[physical_index++] = arg_multi_values[0];
			}
		}

		// Add execution mask to args.
		if( fn.need_mask() )
		{
			physical_args[physical_index++] = exec_mask ? exec_mask : current_execution_mask();
		}

		// Push arguments to list. 
		for(size_t logical_index = 0; logical_index < args.size(); ++logical_index, ++physical_index)
		{
			multi_value const& arg(args[logical_index]);
			vector<Value*> val;
			
			// Get values, and convert big value to pointer if needed.
			if( !fn.value_arg_as_ref(logical_index) )
			{
				arg_multi_values = arg.load(arg_abi);
			}
			else
			{
				vector<Value*> arg_addr = load_ref(arg, arg_abi);
				if( !arg_addr.empty() )
				{
					arg_multi_values = arg_addr;
				}
				else
				{
					value_array arg_values = arg.load();
					arg_multi_values = restore( arg.load() );
				}
			}

			if( fn.multi_value_args() )
			{
				// Merge all value to array.
				physical_args[physical_index] = ext_->get_array(arg_multi_values);

				if( fn.multi_value_arg_as_ref() )
				{
					physical_args[physical_index] = restore( physical_args[physical_index] );
				}
			}
			else
			{
				physical_args[physical_index] = arg_multi_values[0];
			}
		}
	}

	Value* ret_value = builder().CreateCall( fn.fn, physical_args );

	// Parse result.
	if( fn.return_via_arg() )
	{
		return ret_variable;
	}
	value_array ret_value_array(parallel_factor_, 0);
	if( fn.multi_value_args() )
	{
		ret_value_array = ext_->split_array(ret_value);
	}
	else
	{
		ret_value_array[0] = ret_value;
	}

	abis::id ret_abi = arg_abi;
	return create_value(fn.result_type(), ret_value_array, value_kinds::value, ret_abi);
}

multi_value cg_service::cast_s2v( multi_value const& v )
{
	builtin_types hint = v.hint();
	assert( is_scalar(hint) );
	builtin_types vhint = vector_of(hint, 1);

	multi_value ret = null_value( vhint, v.abi() );
	return emit_insert_val( ret, 0, v );
}

multi_value cg_service::cast_v2s( multi_value const& v )
{
	assert( is_vector(v.hint()) );
	return emit_extract_val( v, 0 );
}

multi_value cg_service::cast_bits( multi_value const& v, cg_type* dest_tyi )
{
	abis::id abi = promote_abi(v.abi(), abis::llvm);

	Type* ty = dest_tyi->ty(abi);
	builtin_types dest_scalar_hint = scalar_of( dest_tyi->hint() );
	Type* dest_scalar_ty = type_( dest_scalar_hint, abis::llvm );
	unary_intrin_functor bitcast_sv = ext_->bind_cast_sv(dest_scalar_ty, cast_ops::bitcast);
	value_array ret = ext_->call_unary_intrin(ty, v.load(abi), bitcast_sv);
	return create_value(dest_tyi, ret, value_kinds::value, abi);
}

void cg_service::jump_to( insert_point_t const& ip )
{
	assert( ip );
	if( !insert_point().block->getTerminator() ){
		builder().CreateBr( ip.block );
	}
}

void cg_service::jump_cond(multi_value const& cond_v, insert_point_t const & true_ip, insert_point_t const& false_ip)
{
	assert( cond_v.is_mono() );
	Value* cond = cond_v.load_i1()[0];
	builder().CreateCondBr(cond, true_ip.block, false_ip.block);
}

bool cg_service::merge_swizzle( multi_value const*& root, char indexes[], multi_value const& v )
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

multi_value cg_service::create_value_by_scalar( multi_value const& scalar, cg_type* tyinfo, builtin_types hint )
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
		vector<multi_value> scalars;
		scalars.insert(scalars.end(), vsize, scalar);
		return create_vector(scalars, scalar.abi());
	}
	else
	{
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	return multi_value();
}

multi_value cg_service::emit_any( multi_value const& v )
{
	builtin_types hint = v.hint();
	builtin_types scalar_hint = scalar_of(v.hint());
	if( is_scalar(hint) )
	{
		return emit_cmp_ne( v, null_value(hint, v.abi()) );
	}
	else if( is_vector(hint) )
	{
		multi_value elem_v = emit_extract_val(v, 0);
		multi_value ret = emit_cmp_ne( elem_v, null_value(scalar_hint, v.abi()) );
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
	return multi_value();
}

multi_value cg_service::emit_all( multi_value const& v )
{
	builtin_types hint = v.hint();
	builtin_types scalar_hint = scalar_of(v.hint());
	if( is_scalar(hint) )
	{
		return emit_cmp_ne( v, null_value(hint, v.abi()) );
	}
	else if( is_vector(hint) )
	{
		multi_value elem_v = emit_extract_val(v, 0);
		multi_value ret = emit_cmp_ne( elem_v, null_value(scalar_hint, v.abi()) );
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
	return multi_value();
}

multi_value cg_service::emit_cmp( multi_value const& lhs, multi_value const& rhs, uint32_t pred_signed, uint32_t pred_unsigned, uint32_t pred_float )
{
	builtin_types hint = lhs.hint();
	builtin_types scalar_hint = scalar_of(hint);
	builtin_types ret_hint = replace_scalar(hint, builtin_types::_boolean);

	assert( hint == rhs.hint() );
	assert( is_scalar(scalar_hint) );

	abis::id promoted_abi = promote_abi(lhs.abi(), rhs.abi(), abis::llvm);

	value_array lhs_v = lhs.load(promoted_abi);
	value_array rhs_v = rhs.load(promoted_abi);
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
	value_array ret = ext_->call_binary_intrin( ret_ty, lhs_v, rhs_v, cmp_fn, cast_fn );

	return create_value( ret_hint, ret, value_kinds::value, promoted_abi );
}

multi_value cg_service::emit_cmp_eq( multi_value const& lhs, multi_value const& rhs )
{
	return emit_cmp( lhs, rhs, CmpInst::ICMP_EQ, CmpInst::ICMP_EQ, CmpInst::FCMP_OEQ );
}

multi_value cg_service::emit_cmp_lt( multi_value const& lhs, multi_value const& rhs )
{
	return emit_cmp( lhs, rhs, CmpInst::ICMP_SLT, CmpInst::ICMP_ULT, CmpInst::FCMP_ULT );
}

multi_value cg_service::emit_cmp_le( multi_value const& lhs, multi_value const& rhs )
{
	return emit_cmp( lhs, rhs, CmpInst::ICMP_SLE, CmpInst::ICMP_ULE, CmpInst::FCMP_ULE );
}

multi_value cg_service::emit_cmp_ne( multi_value const& lhs, multi_value const& rhs )
{
	return emit_cmp( lhs, rhs, CmpInst::ICMP_NE, CmpInst::ICMP_NE, CmpInst::FCMP_UNE );
}

multi_value cg_service::emit_cmp_ge( multi_value const& lhs, multi_value const& rhs )
{
	return emit_cmp( lhs, rhs, CmpInst::ICMP_SGE, CmpInst::ICMP_UGE, CmpInst::FCMP_UGE );
}

multi_value cg_service::emit_cmp_gt( multi_value const& lhs, multi_value const& rhs )
{
	return emit_cmp( lhs, rhs, CmpInst::ICMP_SGT, CmpInst::ICMP_UGT, CmpInst::FCMP_UGT );
}

multi_value cg_service::emit_bin_ps_ta_sva( multi_value const& lhs, multi_value const& rhs, binary_intrin_functor signed_sv_fn, binary_intrin_functor unsigned_sv_fn, binary_intrin_functor float_sv_fn )
{
	builtin_types hint( lhs.hint() );
	assert( hint == rhs.hint() );

	value_array ret(parallel_factor_, NULL);

	builtin_types scalar_hint = is_scalar(hint) ? hint : scalar_of(hint);
	abis::id promoted_abi = promote_abi( rhs.abi(), lhs.abi() );
	abis::id internal_abi = promote_abi( promoted_abi, abis::llvm );

	value_array lhs_v = lhs.load(internal_abi);
	value_array rhs_v = rhs.load(internal_abi);
	
	assert( valid_all(lhs_v) );
	assert( valid_all(rhs_v) );
	assert( lhs_v.size() == rhs_v.size() );

	Type* ret_ty = lhs_v[0]->getType();

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

	multi_value retval = create_value(hint, ret, value_kinds::value, internal_abi);
	abis::id ret_abi = is_scalar(hint) ? internal_abi : promoted_abi;
	return create_value(hint, retval.load(ret_abi), value_kinds::value, ret_abi);
}

multi_value cg_service::emit_and( multi_value const& lhs, multi_value const& rhs )
{
	assert( scalar_of( lhs.hint() ) == builtin_types::_boolean );
	assert( lhs.hint() == rhs.hint() );

	return emit_bit_and( lhs, rhs );
}

multi_value cg_service::emit_or( multi_value const& lhs, multi_value const& rhs )
{
	assert( scalar_of( lhs.hint() ) == builtin_types::_boolean );
	assert( lhs.hint() == rhs.hint() );

	return emit_bit_or( lhs, rhs );
}

multi_value cg_service::emit_tex2Dlod( multi_value const& samp, multi_value const& coord )
{
	return emit_tex_lod_impl(samp, coord, externals::tex2dlod_vs, externals::tex2dlod_ps);
}

multi_value cg_service::emit_tex2Dgrad( multi_value const& samp, multi_value const& coord, multi_value const& ddx, multi_value const& ddy )
{
	return emit_tex_grad_impl(samp, coord, ddx, ddy, externals::tex2dgrad_ps);
}

multi_value cg_service::emit_tex2Dbias( multi_value const& samp, multi_value const& coord )
{
	return emit_tex_bias_impl(samp, coord, externals::tex2dbias_ps);
}

multi_value cg_service::emit_tex2Dproj( multi_value const& samp, multi_value const& coord )
{
	return emit_tex_proj_impl(samp, coord, externals::tex2dproj_ps);
}

multi_value cg_service::create_constant_int( cg_type* tyinfo, builtin_types bt, abis::id abi, uint64_t v )
{
	builtin_types hint = tyinfo ? tyinfo->hint() : bt;
	builtin_types scalar_hint = scalar_of(hint);
	assert( is_integer(scalar_hint) || scalar_hint == builtin_types::_boolean );
	uint32_t bits = static_cast<uint32_t>( storage_size(scalar_hint) ) << 3; 

	Type* ret_ty = type_( hint, abi );
	Value* value_mono = ext_->get_int( ret_ty, APInt( bits, v, is_signed(scalar_hint) ) );
	value_array ret(parallel_factor_, value_mono);
	return create_value(tyinfo, bt, ret, value_kinds::value, abi);
}

multi_value cg_service::emit_unary_ps( std::string const& scalar_external_intrin_name, multi_value const& v )
{
	Function* scalar_intrin = module()->getFunction(scalar_external_intrin_name);
	assert( scalar_intrin );

	unary_intrin_functor intrin_sv = ext_->promote_to_unary_sv(
		ext_->bind_external_to_unary(scalar_intrin), null_unary, null_unary
		);

	value_array ret_v = ext_->call_unary_intrin(NULL, v.load(), intrin_sv);
	return create_value( v.ty(), v.hint(), ret_v, value_kinds::value, v.abi() );
}

multi_value cg_service::emit_bin_ps_ta_sva( std::string const& scalar_external_intrin_name, multi_value const& v0, multi_value const& v1 )
{
	Function* scalar_intrin = module()->getFunction( scalar_external_intrin_name );
	assert( scalar_intrin );

	builtin_types hint = v0.hint();
	assert( hint == v1.hint() );
	abis::id abi = promote_abi( v0.abi(), v1.abi() );

	binary_intrin_functor intrin_sv = ext_->promote_to_binary_sv(
		ext_->bind_external_to_binary(scalar_intrin), null_binary, null_binary
		);

	value_array ret_v = ext_->call_binary_intrin(NULL, v0.load(abi), v1.load(abi), intrin_sv, null_unary);

	return create_value(v0.ty(), v0.hint(), ret_v, value_kinds::value, abi);
}

multi_value cg_service::extend_to_vm( multi_value const& v, builtin_types complex_hint )
{
	builtin_types hint = v.hint();
	assert( is_scalar(hint) );

	if( is_vector(complex_hint) )
	{
		vector<multi_value> values(vector_size(complex_hint), v);
		return create_vector( values, v.abi() );
	}

	if( is_matrix(complex_hint) )
	{
		vector<multi_value> values(vector_size(complex_hint), v);
		multi_value vec_v = create_vector( values, v.abi() );
		multi_value ret_v = undef_value(complex_hint, v.abi());
		for( int i = 0; i < (int)vector_count(complex_hint); ++i )
		{
			ret_v = emit_insert_val( ret_v, i, vec_v );
		}
		return ret_v;
	}

	assert(false);
	return multi_value();
}

multi_value cg_service::emit_bin_es_ta_sva( std::string const& scalar_external_intrin_name, multi_value const& lhs, multi_value const& rhs )
{
	builtin_types lhint = lhs.hint();
	builtin_types rhint = rhs.hint();

	assert( lhint != builtin_types::none );
	assert( rhint != builtin_types::none );

	multi_value lv = lhs;
	multi_value rv = rhs;

	if( lhint != rhint )
	{
		if( is_scalar(lhint) ) { lv = extend_to_vm( lhs, rhint ); }
		else if( is_scalar(rhint) ) { rv = extend_to_vm( rhs, lhint ); }
		else { assert(false); }
	}

	return emit_bin_ps_ta_sva( scalar_external_intrin_name, lv, rv );
}

multi_value cg_service::emit_bin_es_ta_sva( multi_value const& lhs, multi_value const& rhs, binary_intrin_functor signed_sv_fn, binary_intrin_functor unsigned_sv_fn, binary_intrin_functor float_sv_fn )
{
	builtin_types lhint = lhs.hint();
	builtin_types rhint = rhs.hint();

	assert( lhint != builtin_types::none );
	assert( rhint != builtin_types::none );

	multi_value lv = lhs;
	multi_value rv = rhs;

	if( lhint != rhint )
	{
		if( is_scalar(lhint) ) { lv = extend_to_vm( lhs, rhint ); }
		else if( is_scalar(rhint) ) { rv = extend_to_vm( rhs, lhint ); }
		else { assert(false); }
	}

	return emit_bin_ps_ta_sva( lv, rv, signed_sv_fn, unsigned_sv_fn, float_sv_fn );
}

multi_value cg_service::emit_tex_lod_impl( multi_value const& samp, multi_value const& coord, externals::id vs_intrin, externals::id ps_intrin )
{
	builtin_types v4f32_hint = vector_of( builtin_types::_float, 4 );
	abis::id abi = param_abi(false);
	assert(abi == abis::llvm);

	Type* ret_ty = type_(v4f32_hint, abi);
	value_array ret_ptr = ext_->stack_alloc( ret_ty, parallel_factor_, "ret.tmp" );

	Type* coord_ty = ret_ty;
	value_array coord_ptr = ext_->stack_alloc(coord_ty, parallel_factor_, "coord.tmp");
	ext_->store(coord.load(abi), coord_ptr);

	value_array samp_value( samp.load() );
	value_array intrin_fn( parallel_factor_, ext_->external(vs_intrin) );
	if( abi == abis::llvm)
	{
		value_array const* args[] = {&ret_ptr, &samp_value, &coord_ptr};
		ext_->call(intrin_fn, args);
	}
	else
	{
		value_array mask = fn().execution_mask().load();
		value_array const* args[] = {&ret_ptr, &mask, &samp_value, &coord_ptr};
		ext_->call(intrin_fn, args);
	}

	return create_value(NULL, v4f32_hint, ret_ptr, value_kinds::reference, abi);
}

multi_value cg_service::emit_tex_grad_impl( multi_value const& samp, multi_value const& coord, multi_value const& ddx, multi_value const& ddy, externals::id ps_intrin )
{
	builtin_types coord_hint = coord.hint();
	builtin_types v4f32_hint = vector_of( builtin_types::_float, 4 );

	abis::id abi = param_abi(false);

	Type* ret_ty = type_( v4f32_hint, abi );
	value_array ret_ptr = ext_->stack_alloc(ret_ty, parallel_factor_, "ret.tmp");

	Type* coord_ty = type_(coord_hint, abi);

	value_array coord_ptr = ext_->stack_alloc(coord_ty, parallel_factor_, "coord.tmp");
	ext_->store(coord.load(abi), coord_ptr);

	value_array ddx_ptr = ext_->stack_alloc(coord_ty, parallel_factor_, "ddx.tmp");
	ext_->store(ddx.load(abi), ddx_ptr);

	value_array ddy_ptr = ext_->stack_alloc(coord_ty, parallel_factor_, "ddy.tmp");
	ext_->store(ddy.load(abi), ddy_ptr);

	value_array args[] = 
	{
		ret_ptr, fn().execution_mask().load(), samp.load(), coord_ptr, ddx_ptr, ddy_ptr
	};

	value_array intrin_fn( parallel_factor_, ext_->external(ps_intrin) );
	ext_->call( intrin_fn, ArrayRef<value_array>(args) );

	return create_value(NULL, v4f32_hint, ret_ptr, value_kinds::reference, abi);
}

multi_value cg_service::emit_tex_bias_impl( multi_value const& /*samp*/, multi_value const& /*coord*/, externals::id /*ps_intrin*/ )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return multi_value();
}

multi_value cg_service::emit_tex_proj_impl( multi_value const& samp, multi_value const& coord, externals::id ps_intrin )
{
	multi_value ddx = emit_ddx( coord );
	multi_value ddy = emit_ddy( coord );

	builtin_types v4f32_hint = vector_of( builtin_types::_float, 4 );

	abis::id abi = param_abi(false);

	Type* ret_ty = type_( v4f32_hint, abi );
	value_array ret_ptr = ext_->stack_alloc( ret_ty, parallel_factor_, "ret.tmp" );

	Type* v4f32_ty = type_( v4f32_hint, abi );

	value_array coord_ptr = ext_->stack_alloc(v4f32_ty, parallel_factor_, "coord.tmp");
	ext_->store(coord.load(abi), coord_ptr);

	value_array ddx_ptr = ext_->stack_alloc(v4f32_ty, parallel_factor_, "ddx.tmp");
	ext_->store(ddx.load(abi), ddx_ptr);

	value_array ddy_ptr = ext_->stack_alloc(v4f32_ty, parallel_factor_, "ddy.tmp");
	ext_->store(coord.load(abi), ddy_ptr);

	value_array args[] = 
	{
		ret_ptr, fn().execution_mask().load(), samp.load(), coord_ptr, ddx_ptr, ddy_ptr
	};
	value_array intrin_fn( parallel_factor_, ext_->external(ps_intrin) );
	ext_->call( intrin_fn, ArrayRef<value_array>(args) );

	return create_value( NULL, v4f32_hint, ret_ptr, value_kinds::reference, abi );
}

multi_value cg_service::emit_texCUBElod( multi_value const& samp, multi_value const& coord )
{
	return emit_tex_lod_impl(samp, coord, externals::texCUBElod_vs, externals::texCUBElod_ps);
}

multi_value cg_service::emit_texCUBEgrad( multi_value const& samp, multi_value const& coord, multi_value const& ddx, multi_value const& ddy )
{
	return emit_tex_grad_impl(samp, coord, ddx, ddy, externals::texCUBEgrad_ps);
}

multi_value cg_service::emit_texCUBEbias( multi_value const& samp, multi_value const& coord )
{
	return emit_tex_bias_impl(samp, coord, externals::texCUBEbias_ps);
}

multi_value cg_service::emit_texCUBEproj( multi_value const& samp, multi_value const& coord )
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

multi_value cg_service::emit_select( multi_value const& flag, multi_value const& v0, multi_value const& v1 )
{
	abis::id promoted_abi = promote_abi(flag.abi(), v0.abi(), v1.abi() );

	value_array flag_v = flag.load(promoted_abi);
	value_array v0_v = v0.load(promoted_abi);
	value_array v1_v = v1.load(promoted_abi);

	return create_value(
		v0.ty(), v0.hint(), 
		ext_->select(flag_v, v0_v, v1_v), value_kinds::value, promoted_abi
		);
}

multi_value cg_service::emit_not( multi_value const& v )
{
	multi_value mask_value = create_constant_int( NULL, v.hint(), v.abi(), 1 );
	return emit_bit_xor( mask_value, v );
}

multi_value cg_service::inf_from_value(multi_value const& v, bool negative)
{
	value_array v_v = v.load();
	builtin_types scalar_of_v = scalar_of( v.hint() );
	Type* scalar_ty = type_(scalar_of_v, abis::llvm);
	Value* scalar_inf = ConstantFP::getInfinity(scalar_ty, negative);
	Value* inf_value = ext_->get_constant_by_scalar(v_v[0]->getType(), scalar_inf);
	return create_value(
		v.ty(), v.hint(),
		value_array(parallel_factor_, inf_value),
		value_kinds::value, v.abi()
		);
}

multi_value cg_service::emit_isinf(multi_value const& v)
{
	multi_value abs_v = emit_abs(v);
	return emit_cmp(abs_v, inf_from_value(v, false), ICmpInst::ICMP_EQ, ICmpInst::ICMP_EQ, FCmpInst::FCMP_OEQ);
}

multi_value cg_service::emit_isfinite( multi_value const& v )
{
	multi_value is_eq = emit_cmp(v, v, ICmpInst::ICMP_EQ, ICmpInst::ICMP_EQ, FCmpInst::FCMP_OEQ);
	multi_value is_inf = emit_isinf(v);
	return emit_and(emit_not(is_inf), is_eq);
}

multi_value cg_service::emit_isnan( multi_value const& v )
{
	return emit_cmp(v, v, ICmpInst::ICMP_EQ, ICmpInst::ICMP_EQ, FCmpInst::FCMP_UNO);
}

multi_value cg_service::one_value( multi_value const& proto )
{
	return numeric_value(proto, 1.0f, 1);
}

multi_value cg_service::emit_sign( multi_value const& v )
{
	builtin_types ret_btc = replace_scalar(v.hint(), builtin_types::_sint32);
	multi_value zero = null_value( v.hint(), v.abi() );
	multi_value i_zero = null_value( ret_btc, v.abi() );
	multi_value i_one = one_value( i_zero );
	multi_value i_neg_one = emit_sub(i_zero, i_one);

	multi_value v0 = emit_select( emit_cmp_lt(v, zero), i_neg_one, i_zero );
	multi_value v1 = emit_select( emit_cmp_gt(v, zero), i_one, i_zero );
	return emit_add(v0, v1);
}

multi_value cg_service::emit_clamp( multi_value const& v, multi_value const& min_v, multi_value const& max_v )
{
	multi_value ret = emit_select(emit_cmp_ge(v, min_v), v, min_v);
	return emit_select(emit_cmp_le(ret, max_v), ret, max_v);
}

multi_value cg_service::emit_saturate( multi_value const& v )
{
	return emit_clamp(v, null_value( v.hint(), v.abi() ), one_value(v) );
}

multi_value cg_service::numeric_value(multi_value const& proto, double fp, uint64_t ui)
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

	Value* ret_mono = ext_->get_constant_by_scalar(ty, scalar_value);
	value_array ret_value(parallel_factor_, ret_mono);
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

Value* cg_service::restore(Value* v)
{
	Value* addr = ext_->stack_alloc(v->getType(), "restore.tmp");
	builder().CreateStore(v, addr, false);
	return addr;
}

value_array cg_service::restore(value_array const& v)
{
	assert( valid_all(v) );
	value_array addr(v.size(), NULL);
	for(size_t value_index = 0; value_index < v.size(); ++value_index)
	{
		addr[value_index] = restore(v[value_index]);
	}
	return addr;
}

size_t cg_service::parallel_factor() const
{
	return parallel_factor_;
}

multi_value cg_service::create_scalar(Value* val, cg_type* tyinfo, builtin_types hint){
	assert( is_scalar(hint) );
	return create_value( tyinfo, hint, value_array(parallel_factor_, val), value_kinds::value, abis::llvm );
}

abis::id cg_service::param_abi( bool is_c_compatible ) const
{
	return is_c_compatible ? abis::c : abis::llvm;
}

Value* cg_service::combine_flags(value_array const& flags)
{
	assert( flags.size() == parallel_factor_ );
	Value* ret = ext_->get_int<uint32_t>(0);
	for(size_t i_flag = 0; i_flag < parallel_factor_; ++i_flag)
	{
		Value* flag_u32 = builder().CreateZExt( flags[i_flag], Type::getInt32Ty( context() ) );
		Value* shifted_flag = builder().CreateShl(flag_u32, i_flag);
		ret = builder().CreateOr(shifted_flag, ret);
	}
	return ret;
}

END_NS_SASL_CODEGEN();
