#include <sasl/include/code_generator/llvm/cgllvm_service.h>

#include <sasl/include/code_generator/llvm/ty_cache.h>
#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/Support/CFG.h>
#include <eflib/include/platform/enable_warnings.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/foreach.hpp>
#include <eflib/include/platform/boost_end.h>

using llvm::Module;
using llvm::LLVMContext;
using llvm::DefaultIRBuilder;
using llvm::Value;
using llvm::Type;
using llvm::BasicBlock;
using boost::shared_ptr;
using namespace sasl::syntax_tree;
using namespace sasl::semantic;
using sasl::utility::is_vector;
using sasl::utility::scalar_of;
using sasl::utility::is_scalar;

namespace {
	uint32_t indexes_to_mask( char indexes[4] ){
		uint32_t mask = 0;
		for( int i = 0; i < 4; ++i ){
			mask += (uint32_t)( (indexes[i] + 1) << (i*8) );
		}
		return mask;
	}
}


BEGIN_NS_SASL_CODE_GENERATOR();

/// @name Value Type Info
/// @{
value_tyinfo::value_tyinfo(tynode* tyn, Type* ty_c, Type* ty_llvm, Type* ty_vec, Type* ty_pkg )
	: tyn(tyn)
{
	tys[abi_c]			= ty_c;
	tys[abi_llvm]		= ty_llvm;
	tys[abi_vectorize]	= ty_vec;
	tys[abi_package]	= ty_pkg;
}

value_tyinfo::value_tyinfo(): tyn(NULL), cls(unknown_type)
{
	memset( tys, 0, sizeof(tys) );
}

builtin_types value_tyinfo::hint() const{
	if( !tyn || !tyn->is_builtin() ){
		return builtin_types::none;
	}
	return tyn->tycode;
}

tynode* value_tyinfo::tyn_ptr() const{
	return tyn;
}

shared_ptr<tynode> value_tyinfo::tyn_shared() const{
	return tyn->as_handle<tynode>();
}

llvm::Type* value_tyinfo::ty( abis abi ) const{
	return tys[abi];
}

/// @}

/// value_t @{
value_t::value_t()
	: tyinfo_(NULL), val_(NULL), cg_(NULL), kind_(vkind_unknown), hint_(builtin_types::none), abi_(abi_unknown), masks_(0)
{
}

value_t::value_t(
	value_tyinfo* tyinfo,
	llvm::Value* val, value_kinds k, abis abi,
	cg_service* cg 
	) 
	: tyinfo_(tyinfo), val_(val), cg_(cg), kind_(k), hint_(builtin_types::none), abi_(abi), masks_(0)
{
}

value_t::value_t( builtin_types hint,
	llvm::Value* val, value_kinds k, abis abi,
	cg_service* cg 
	)
	: tyinfo_(NULL), hint_(hint), abi_(abi), val_(val), kind_(k), cg_(cg), masks_(0)
{

}

value_t::value_t( value_t const& rhs )
	: tyinfo_(rhs.tyinfo_), hint_(rhs.hint_), abi_(rhs.abi_), val_( rhs.val_ ), kind_(rhs.kind_), cg_(rhs.cg_), masks_(rhs.masks_)
{
	parent(rhs.parent_.get());
}

abis value_t::abi() const{
	return abi_;
}

value_t value_t::swizzle( size_t swz_code ) const{
	assert( is_vector( hint() ) );
	EFLIB_ASSERT_UNIMPLEMENTED();
	return value_t();
}

llvm::Value* value_t::raw() const{
	return val_;
}

value_t value_t::to_rvalue() const
{
	if( tyinfo_ ){
		return value_t( tyinfo_, load(abi_), vkind_value, abi_, cg_ );
	} else {
		return value_t( hint_, load(abi_), vkind_value, abi_, cg_ );
	}
}

builtin_types value_t::hint() const
{
	if( tyinfo_ ) return tyinfo_->hint();
	return hint_;
}

llvm::Value* value_t::load( abis abi ) const{
	return cg_->load( *this, abi );
}

Value* value_t::load() const{
	return cg_->load( *this );
}

value_kinds value_t::kind() const{
	return kind_;
}

bool value_t::storable() const{
	switch( kind_ ){
	case vkind_ref:
		return true;
	case vkind_value:
	case vkind_unknown:
		return false;
	default:
		EFLIB_ASSERT_UNIMPLEMENTED();
		return false;
	}
}

bool value_t::load_only() const
{
	switch( kind_ ){
	case vkind_ref:
	case vkind_unknown:
		return false;
	case vkind_value:
		return true;
	default:
		EFLIB_ASSERT_UNIMPLEMENTED();
		return false;
	}
}

void value_t::emplace( Value* v, value_kinds k, abis abi ){
	val_ = v;
	kind_ = k;
	abi_ = abi;
}

void value_t::emplace( value_t const& v )
{
	emplace( v.raw(), v.kind(), v.abi() );
}

llvm::Value* value_t::load_ref() const
{
	return cg_->load_ref( *this );
}

value_t& value_t::operator=( value_t const& rhs )
{
	kind_ = rhs.kind_;
	val_ = rhs.val_;
	tyinfo_ = rhs.tyinfo_;
	hint_ = rhs.hint_;
	abi_ = rhs.abi_;
	cg_ = rhs.cg_;
	masks_ = rhs.masks_;

	parent(rhs.parent_.get());

	return *this;
}

value_t value_t::slice( value_t const& vec, uint32_t masks )
{
	builtin_types hint = vec.hint();
	assert( is_vector(hint) );

	value_t ret( scalar_of(hint), NULL, vkind_swizzle, vec.abi_, vec.cg_ );
	ret.masks_ = masks;
	ret.parent(vec);

	return ret;
}

value_t value_t::as_ref() const
{
	value_t ret(*this);

	switch( ret.kind_ ){
	case vkind_value:
		ret.kind_ = vkind_ref;
		break;
	case vkind_swizzle:
		ret.kind_ = (value_kinds)( vkind_swizzle | vkind_ref );
		break;
	}

	return ret;
}

void value_t::store( value_t const& v ) const
{
	cg_->store( *(const_cast<value_t*>(this)), v );
}

void value_t::index( size_t index )
{
	char indexes[4] = { (char)index, -1, -1, -1 };
	masks_ = indexes_to_mask( indexes );
}

value_tyinfo*	value_t::tyinfo() const{ return tyinfo_; }
void			value_t::hint( builtin_types bt ){ hint_ = bt; }
void			value_t::abi( abis abi ){ this->abi_ = abi; }
uint32_t		value_t::masks() const{ return masks_; }
void			value_t::kind( value_kinds vkind ) { kind_ = vkind; }
void			value_t::parent( value_t const& v ){ parent_.reset( new value_t(v) ); }
void			value_t::parent( value_t const* v ){ if(v){ parent(*v); } }
value_t*		value_t::parent() const { return parent_.get(); }
/// @}

bool cg_service::initialize( llvm_module_impl* mod, node_ctxt_fn const& fn )
{
	assert ( mod );

	mod_impl = mod;
	node_ctxt = fn;

	return true;
}

Module* cg_service::module() const{
	return mod_impl->module();
}

LLVMContext& cg_service::context() const{
	return mod_impl->context();
}

DefaultIRBuilder& cg_service::builder() const{
	return *( mod_impl->builder() );
}

function_t cg_service::fetch_function( shared_ptr<function_type> const& fn_node ){

	cgllvm_sctxt* fn_ctxt = node_ctxt( fn_node.get(), false );
	if( fn_ctxt->data().self_fn ){
		return fn_ctxt->data().self_fn;
	}

	function_t ret;
	ret.fnty = fn_node.get();
	ret.c_compatible = fn_node->si_ptr<storage_si>()->c_compatible();

	abis abi = param_abi( ret.c_compatible );

	vector<Type*> par_tys;

	Type* ret_ty = node_ctxt( fn_node->retval_type.get(), false )->get_typtr()->ty( abi );

	ret.ret_void = true;
	if( abi == abi_c ){
		if( fn_node->retval_type->tycode != builtin_types::_void ){
			// If function need C compatible and return value is not void, The first parameter is set to point to return value, and parameters moves right.
			Type* ret_ptr = PointerType::getUnqual( ret_ty );
			par_tys.push_back( ret_ptr );
			ret.ret_void = false;
		}

		ret_ty = Type::getVoidTy( context() );
	}

	// Create function type.
	BOOST_FOREACH( shared_ptr<parameter> const& par, fn_node->params )
	{
		cgllvm_sctxt* par_ctxt = node_ctxt( par.get(), false );
		value_tyinfo* par_ty = par_ctxt->get_typtr();
		assert( par_ty );

		// bool is_ref = par->si_ptr<storage_si>()->is_reference();

		Type* par_llty = par_ty->ty( abi ); 
		if( ret.c_compatible && !is_scalar(par_ty->hint()) ){
			par_tys.push_back( PointerType::getUnqual( par_llty ) );
		} else {
			par_tys.push_back( par_llty );
		}
	}


	FunctionType* fty = FunctionType::get( ret_ty, par_tys, false );

	// Create function
	ret.fn = Function::Create( fty, Function::ExternalLinkage, fn_node->symbol()->mangled_name(), module() );

	ret.cg = this;
	return ret;
}

value_t cg_service::create_value( value_tyinfo* tyinfo, Value* val, value_kinds k, abis abi ){
	return value_t( tyinfo, val, k, abi, this );
}

value_t cg_service::create_value( builtin_types hint, Value* val, value_kinds k, abis abi )
{
	return value_t( hint, val, k, abi, this );
}

value_t cg_service::create_value( value_tyinfo* tyinfo, builtin_types hint, Value* val, value_kinds k, abis abi )
{
	if( tyinfo ){
		return create_value( tyinfo, val, k, abi );
	} else {
		return create_value( hint, val, k ,abi );
	}
}

shared_ptr<value_tyinfo> cg_service::create_tyinfo( shared_ptr<tynode> const& tyn ){
	cgllvm_sctxt* ctxt = node_ctxt(tyn.get(), true);
	if( ctxt->get_tysp() ){
		return ctxt->get_tysp();
	}

	value_tyinfo* ret = new value_tyinfo();
	ret->tyn = tyn.get();
	ret->cls = value_tyinfo::unknown_type;

	if( tyn->is_builtin() ){
		ret->tys[abi_c] = type_( tyn->tycode, abi_c );
		ret->tys[abi_llvm] = type_( tyn->tycode, abi_llvm );
		ret->tys[abi_vectorize] = type_( tyn->tycode, abi_vectorize );
		ret->tys[abi_package] = type_( tyn->tycode, abi_package );
		ret->cls = value_tyinfo::builtin;
	} else {
		ret->cls = value_tyinfo::aggregated;

		if( tyn->is_struct() ){
			shared_ptr<struct_type> struct_tyn = tyn->as_handle<struct_type>();

			vector<Type*> c_member_types;
			vector<Type*> llvm_member_types;
			vector<Type*> vectorize_member_types;
			vector<Type*> package_member_types;

			BOOST_FOREACH( shared_ptr<declaration> const& decl, struct_tyn->decls){
				if( decl->node_class() == node_ids::variable_declaration ){
					shared_ptr<variable_declaration> decl_tyn = decl->as_handle<variable_declaration>();
					shared_ptr<value_tyinfo> decl_tyinfo = create_tyinfo( decl_tyn->type_info->si_ptr<type_info_si>()->type_info() );
					size_t declarator_count = decl_tyn->declarators.size();
					c_member_types.insert( c_member_types.end(), declarator_count, decl_tyinfo->ty(abi_c) );
					llvm_member_types.insert( llvm_member_types.end(), declarator_count, decl_tyinfo->ty(abi_llvm) );
					vectorize_member_types.insert( vectorize_member_types.end(), declarator_count, decl_tyinfo->ty(abi_vectorize) );
					package_member_types.insert( package_member_types.end(), declarator_count, decl_tyinfo->ty(abi_package) );
				}
			}

			StructType* ty_c	= StructType::create( c_member_types,			struct_tyn->name->str + ".abi.c" );
			StructType* ty_llvm	= StructType::create( llvm_member_types,		struct_tyn->name->str + ".abi.llvm" );
			StructType* ty_vec	= NULL;
			if( vectorize_member_types[0] != NULL ){
				StructType* ty_vec	= StructType::create( vectorize_member_types,	struct_tyn->name->str + ".abi.vec" );
			}
			StructType* ty_pkg	= NULL;
			if( package_member_types[0] != NULL ){
				ty_pkg	= StructType::create( package_member_types,		struct_tyn->name->str + ".abi.pkg" );
			}

			ret->tys[abi_c]			= ty_c;
			ret->tys[abi_llvm]		= ty_llvm;
			ret->tys[abi_vectorize]	= ty_vec;
			ret->tys[abi_package]	= ty_pkg;
		} else {
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	}

	ctxt->data().tyinfo = shared_ptr<value_tyinfo>(ret);
	return ctxt->data().tyinfo;
}

value_tyinfo* cg_service::member_tyinfo( value_tyinfo const* agg, size_t index ) const
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
					return const_cast<cg_service*>(this)->node_ctxt( vardecl.get(), false )->get_typtr();
				}
			}
		}

		assert(!"Out of struct bound.");
	} else {
		EFLIB_ASSERT_UNIMPLEMENTED();
	}

	return NULL;
}

value_t cg_service::create_variable( builtin_types bt, abis abi, std::string const& name )
{
	Type* var_ty = type_( bt, abi );
	Value* var_val = builder().CreateAlloca( var_ty );
	return create_value( bt, var_val, vkind_ref, abi );
}

value_t cg_service::create_variable( value_tyinfo const* ty, abis abi, std::string const& name )
{
	Type* var_ty = type_(ty, abi);
	Value* var_val = builder().CreateAlloca( var_ty );
	return create_value( const_cast<value_tyinfo*>(ty), var_val, vkind_ref, abi );
}

insert_point_t cg_service::new_block( std::string const& hint, bool set_as_current )
{
	assert( in_function() );
	insert_point_t ret;
	ret.block = BasicBlock::Create( context(), hint, fn().fn );
	// dbg_print_blocks( fn().fn );
	if( set_as_current ){ set_insert_point( ret ); }
	return ret;
}

void cg_service::clean_empty_blocks()
{
	assert( in_function() );

	typedef Function::BasicBlockListType::iterator block_iterator_t;
	block_iterator_t beg = fn().fn->getBasicBlockList().begin();
	block_iterator_t end = fn().fn->getBasicBlockList().end();

	// dbg_print_blocks( fn().fn );

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

function_t& cg_service::fn(){
	return fn_ctxts.back();
}

void cg_service::push_fn( function_t const& fn ){
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

Type* cg_service::type_( builtin_types bt, abis abi )
{
	assert( abi != abi_unknown );
	return get_llvm_type( context(), bt, abi );
}

Type* cg_service::type_( value_tyinfo const* ty, abis abi )
{
	assert( ty->ty(abi) );
	return ty->ty(abi);
}

END_NS_SASL_CODE_GENERATOR();