#include <sasl/include/code_generator/llvm/cgs_objects.h>

#include <sasl/include/code_generator/llvm/cgs.h>
#include <sasl/include/code_generator/llvm/utility.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Function.h>
#include <eflib/include/platform/enable_warnings.h>

using sasl::syntax_tree::tynode;
using sasl::syntax_tree::parameter;
using sasl::utility::is_vector;
using sasl::utility::scalar_of;
using sasl::utility::is_scalar;
using sasl::utility::is_sampler;
using sasl::semantic::storage_si;
using llvm::Type;
using llvm::Value;
using llvm::Function;
using boost::shared_ptr;
using std::vector;
using std::string;

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
	index(rhs.index_.get());
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
	index(rhs.index_.get());
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

value_t value_t::slice( value_t const& vec, value_t const& index )
{
	builtin_types hint = vec.hint();
	assert( is_vector(hint) );

	value_t ret( scalar_of(hint), NULL, vkind_swizzle, vec.abi_, vec.cg_ );
	ret.index(index);
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

value_t*		value_t::index() const { return index_.get(); }
void			value_t::index( value_t const& v ){ index_.reset( new value_t(v) ); }
void			value_t::index( value_t const* v ){ if(v) index(*v); }

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
void function_t::arg_name( size_t index, std::string const& name ){
	size_t param_size = fn->arg_size();

	assert( index < param_size );

	Function::arg_iterator arg_it = fn->arg_begin();
	if( first_arg_is_return_address() ){
		++arg_it;
	}
	if( partial_execution ){
		++arg_it;
	}

	for( size_t i = 0; i < index; ++i ){ ++arg_it; }
	arg_it->setName( name );
}

void function_t::args_name( vector<string> const& names )
{
	Function::arg_iterator arg_it = fn->arg_begin();
	vector<string>::const_iterator name_it = names.begin();

	if( first_arg_is_return_address() ){
		arg_it->setName(".ret");
		++arg_it;
	}
	if( partial_execution ){
		arg_it->setName(".exec_mask");
		++arg_it;
	}

	for( size_t i = 0; i < names.size(); ++i ){
		arg_it->setName( *name_it );
		++arg_it;
		++name_it;
	}
}

shared_ptr<value_tyinfo> function_t::get_return_ty() const{
	assert( fnty->is_function() );
	return shared_ptr<value_tyinfo>( cg->node_ctxt( fnty->retval_type.get(), false )->get_tysp() );
}

size_t function_t::arg_size() const{
	assert( fn );
	size_t arg_size = fn->arg_size();
	if( fn ){
		if( first_arg_is_return_address() ){ --arg_size; }
		if( partial_execution ) { --arg_size; }
		return arg_size;
	}
	return 0;
}

value_t function_t::arg( size_t index ) const
{
	// If c_compatible and not void return, the first argument is address of return value.
	size_t arg_index = index;
	if( first_arg_is_return_address() ){ ++arg_index; }
	if( partial_execution ){ ++arg_index; }

	shared_ptr<parameter> par = fnty->params[index];
	value_tyinfo* par_typtr = cg->node_ctxt( par.get(), false )->get_typtr();

	Function::ArgumentListType::iterator it = fn->arg_begin();
	for( size_t idx_counter = 0; idx_counter < arg_index; ++idx_counter ){
		++it;
	}

	abis arg_abi = cg->param_abi( c_compatible );
	return cg->create_value( par_typtr, &(*it), arg_is_ref(index) ? vkind_ref : vkind_value, arg_abi );
}

value_t function_t::packed_execution_mask() const
{
	if( !partial_execution ){ return value_t(); }

	Function::ArgumentListType::iterator it = fn->arg_begin();
	if( first_arg_is_return_address() ){ ++it; }

	return cg->create_value( builtin_types::_uint16, &(*it), vkind_value, abi_llvm );
}

function_t::function_t(): fn(NULL), fnty(NULL), ret_void(true), c_compatible(false), cg(NULL), external(false), partial_execution(false)
{
}

bool function_t::arg_is_ref( size_t index ) const{
	assert( index < fnty->params.size() );

	builtin_types hint = fnty->params[index]->si_ptr<storage_si>()->type_info()->tycode;
	return c_compatible && !is_scalar(hint) && !is_sampler(hint);
}

bool function_t::first_arg_is_return_address() const
{
	return ( c_compatible || external ) && !ret_void;
}

abis function_t::abi() const
{
	return cg->param_abi( c_compatible );
}

llvm::Value* function_t::return_address() const
{
	if( first_arg_is_return_address() ){
		return &(*fn->arg_begin());
	}
	return NULL;
}

void function_t::return_name( std::string const& s )
{
	if( first_arg_is_return_address() ){
		fn->arg_begin()->setName( s );
	}
}

void function_t::inline_hint()
{
	fn->addAttribute( 0, llvm::Attribute::InlineHint );
}

insert_point_t::insert_point_t(): block(NULL)
{
}

insert_point_t function_t::allocation_block() const
{
	return alloc_block;
}

END_NS_SASL_CODE_GENERATOR();