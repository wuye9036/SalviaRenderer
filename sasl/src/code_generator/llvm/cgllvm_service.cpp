#include <sasl/include/code_generator/llvm/cgllvm_service.h>

#include <sasl/include/code_generator/llvm/ty_cache.h>
#include <sasl/include/code_generator/llvm/cgllvm_globalctxt.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/IRBuilder.h>
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
value_tyinfo::value_tyinfo(tynode* tyn, Type* cty, Type* llty )
	: tyn(tyn)
{
	tys[abi_c] = cty;
	tys[abi_llvm] = llty;
}

value_tyinfo::value_tyinfo(): tyn(NULL), cls(unknown_type)
{
	tys[0] = NULL;
	tys[1] = NULL;
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
		ret->cls = value_tyinfo::builtin;
	} else {
		ret->cls = value_tyinfo::aggregated;

		if( tyn->is_struct() ){
			shared_ptr<struct_type> struct_tyn = tyn->as_handle<struct_type>();

			vector<Type*> c_member_types;
			vector<Type*> llvm_member_types;

			BOOST_FOREACH( shared_ptr<declaration> const& decl, struct_tyn->decls){

				if( decl->node_class() == node_ids::variable_declaration ){
					shared_ptr<variable_declaration> decl_tyn = decl->as_handle<variable_declaration>();
					shared_ptr<value_tyinfo> decl_tyinfo = create_tyinfo( decl_tyn->type_info->si_ptr<type_info_si>()->type_info() );
					size_t declarator_count = decl_tyn->declarators.size();
					// c_member_types.insert( c_member_types.end(), (Type*)NULL );
					c_member_types.insert( c_member_types.end(), declarator_count, decl_tyinfo->ty(abi_c) );
					llvm_member_types.insert( llvm_member_types.end(), declarator_count, decl_tyinfo->ty(abi_llvm) );
				}
			}

			StructType* ty_c = StructType::get( context(), c_member_types, true );
			StructType* ty_llvm = StructType::get( context(), llvm_member_types, false );

			ret->tys[abi_c] = ty_c;
			ret->tys[abi_llvm] = ty_llvm;

			if( !ty_c->isLiteral() ){
				ty_c->setName( struct_tyn->name->str + ".abi.c" );
			}
			if( !ty_llvm->isLiteral() ){
				ty_llvm->setName( struct_tyn->name->str + ".abi.llvm" );
			}
		} else {
			EFLIB_ASSERT_UNIMPLEMENTED();
		}
	}

	ctxt->data().tyinfo = shared_ptr<value_tyinfo>(ret);
	return ctxt->data().tyinfo;
}

insert_point_t cg_service::new_block( std::string const& hint, bool set_as_current )
{
	assert( in_function() );

	insert_point_t ret;

	ret.block = BasicBlock::Create( context(), hint, fn().fn );
	// dbg_print_blocks( fn().fn );

	if( set_as_current ){
		set_insert_point( ret );
	}

	return ret;
}

bool cg_service::in_function() const{
	return !fn_ctxts.empty();
}

function_t& cg_service::fn(){
	return fn_ctxts.back();
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