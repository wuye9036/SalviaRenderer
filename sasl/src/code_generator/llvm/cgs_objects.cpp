#include <sasl/include/code_generator/llvm/cgs_objects.h>

#include <sasl/include/code_generator/llvm/cgs.h>
#include <sasl/include/code_generator/llvm/utility.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/enums/enums_utility.h>

using sasl::syntax_tree::tynode;
using sasl::utility::is_vector;
using sasl::utility::scalar_of;
using llvm::Type;
using llvm::Value;
using boost::shared_ptr;

BEGIN_NS_SASL_CODE_GENERATOR();

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

value_t value_t::swizzle( size_t /*swz_code*/ ) const{
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
	case vkind_swizzle:
		return parent()->storable();
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
void			value_t::masks( uint32_t v ){ masks_ = v; }

void			value_t::kind( value_kinds vkind ) { kind_ = vkind; }
void			value_t::parent( value_t const& v ){ parent_.reset( new value_t(v) ); }
void			value_t::parent( value_t const* v ){ if(v){ parent(*v); } }
value_t*		value_t::parent() const { return parent_.get(); }

//Workaround for llvm issue 12618
llvm::Value* value_t::load_i1() const{
	if( hint() == builtin_types::_boolean )
	{
		return cg_->i8toi1_( load(abi_llvm) );
	}
	else
	{
		assert(false);
		return NULL;
	}
}

void function_t::allocation_block( insert_point_t const& ip )
{
	alloc_block = ip;
}

insert_point_t function_t::allocation_block() const
{
	return alloc_block;
}

END_NS_SASL_CODE_GENERATOR();