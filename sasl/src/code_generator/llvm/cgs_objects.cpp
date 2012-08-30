#include <sasl/include/code_generator/llvm/cgs_objects.h>

#include <sasl/include/code_generator/llvm/cgs.h>
#include <sasl/include/code_generator/llvm/utility.h>
#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/semantic/semantics.h>
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
using sasl::semantic::node_semantic;
using llvm::Type;
using llvm::Value;
using llvm::Function;
using boost::shared_ptr;
using std::vector;
using std::string;

BEGIN_NS_SASL_CODE_GENERATOR();

cg_type::cg_type(tynode* tyn, Type* ty_c, Type* ty_llvm, Type* ty_vec, Type* ty_pkg )
	: tyn(tyn)
{
	tys[abi_c]			= ty_c;
	tys[abi_llvm]		= ty_llvm;
	tys[abi_vectorize]	= ty_vec;
	tys[abi_package]	= ty_pkg;
}

cg_type::cg_type(): tyn(NULL), cls(unknown_type)
{
	memset( tys, 0, sizeof(tys) );
}

builtin_types cg_type::hint() const{
	if( !tyn || !tyn->is_builtin() ){
		return builtin_types::none;
	}
	return tyn->tycode;
}

tynode* cg_type::tyn_ptr() const{
	return tyn;
}

llvm::Type* cg_type::ty( abis abi ) const{
	return tys[abi];
}

cg_value::cg_value()
	: tyinfo_(NULL), val_(NULL), cg_(NULL), kind_(vkind_unknown), hint_(builtin_types::none), abi_(abi_unknown), masks_(0)
{
}

cg_value::cg_value(
	cg_type* tyinfo,
	llvm::Value* val, value_kinds k, abis abi,
	cg_service* cg 
	) 
	: tyinfo_(tyinfo), val_(val), cg_(cg), kind_(k), hint_(builtin_types::none), abi_(abi), masks_(0)
{
}

cg_value::cg_value( builtin_types hint,
	llvm::Value* val, value_kinds k, abis abi,
	cg_service* cg 
	)
	: tyinfo_(NULL), hint_(hint), abi_(abi), val_(val), kind_(k), cg_(cg), masks_(0)
{

}

cg_value::cg_value( cg_value const& rhs )
	: tyinfo_(rhs.tyinfo_), hint_(rhs.hint_), abi_(rhs.abi_), val_( rhs.val_ ), kind_(rhs.kind_), cg_(rhs.cg_), masks_(rhs.masks_)
{
	parent(rhs.parent_.get());
	index(rhs.index_.get());
}

abis cg_value::abi() const{
	return abi_;
}

cg_value cg_value::swizzle( size_t /*swz_code*/ ) const{
	assert( is_vector( hint() ) );
	EFLIB_ASSERT_UNIMPLEMENTED();
	return cg_value();
}

llvm::Value* cg_value::raw() const{
	return val_;
}

cg_value cg_value::to_rvalue() const
{
	if( tyinfo_ ){
		return cg_value( tyinfo_, load(abi_), vkind_value, abi_, cg_ );
	} else {
		return cg_value( hint_, load(abi_), vkind_value, abi_, cg_ );
	}
}

builtin_types cg_value::hint() const
{
	if( tyinfo_ ) return tyinfo_->hint();
	return hint_;
}

llvm::Value* cg_value::load( abis abi ) const{
	return cg_->load( *this, abi );
}

Value* cg_value::load() const{
	return cg_->load( *this );
}

value_kinds cg_value::kind() const{
	return kind_;
}

bool cg_value::storable() const{
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

bool cg_value::load_only() const
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

void cg_value::emplace( Value* v, value_kinds k, abis abi ){
	val_ = v;
	kind_ = k;
	abi_ = abi;
}

void cg_value::emplace( cg_value const& v )
{
	emplace( v.raw(), v.kind(), v.abi() );
}

llvm::Value* cg_value::load_ref() const
{
	return cg_->load_ref( *this );
}

cg_value& cg_value::operator=( cg_value const& rhs )
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

cg_value cg_value::slice( cg_value const& vec, uint32_t masks )
{
	builtin_types hint = vec.hint();
	assert( is_vector(hint) );

	cg_value ret( scalar_of(hint), NULL, vkind_swizzle, vec.abi_, vec.cg_ );
	ret.masks_ = masks;
	ret.parent(vec);

	return ret;
}

cg_value cg_value::slice( cg_value const& vec, cg_value const& index )
{
	builtin_types hint = vec.hint();
	assert( is_vector(hint) );

	cg_value ret( scalar_of(hint), NULL, vkind_swizzle, vec.abi_, vec.cg_ );
	ret.index(index);
	ret.parent(vec);

	return ret;
}

cg_value cg_value::as_ref() const
{
	cg_value ret(*this);

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

void cg_value::store( cg_value const& v ) const
{
	cg_->store( *(const_cast<cg_value*>(this)), v );
}

void cg_value::index( size_t index )
{
	char indexes[4] = { (char)index, -1, -1, -1 };
	masks_ = indexes_to_mask( indexes );
}

cg_type*	cg_value::tyinfo() const{ return tyinfo_; }
void			cg_value::tyinfo( cg_type* v ){ tyinfo_ = v; }

void			cg_value::hint( builtin_types bt ){ hint_ = bt; }
void			cg_value::abi( abis abi ){ this->abi_ = abi; }
uint32_t		cg_value::masks() const{ return masks_; }
void			cg_value::masks( uint32_t v ){ masks_ = v; }

void			cg_value::kind( value_kinds vkind ) { kind_ = vkind; }
void			cg_value::parent( cg_value const& v ){ parent_.reset( new cg_value(v) ); }
void			cg_value::parent( cg_value const* v ){ if(v){ parent(*v); } }
cg_value*		cg_value::parent() const { return parent_.get(); }

cg_value*		cg_value::index() const { return index_.get(); }
void			cg_value::index( cg_value const& v ){ index_.reset( new cg_value(v) ); }
void			cg_value::index( cg_value const* v ){ if(v) index(*v); }

//Workaround for llvm issue 12618
llvm::Value* cg_value::load_i1() const{
	if( hint() == builtin_types::_boolean )
	{
		return cg_->extension()->i8toi1_sv( load(abi_llvm) );
	}
	else
	{
		assert(false);
		return NULL;
	}
}

void cg_function::allocation_block( insert_point_t const& ip )
{
	alloc_block = ip;
}
void cg_function::arg_name( size_t index, std::string const& name ){
	assert( index < fn->arg_size() );

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

void cg_function::args_name( vector<string> const& names )
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

cg_type* cg_function::get_return_ty() const{
	assert( fnty->is_function() );
	return cg->get_node_context( fnty->retval_type.get() )->ty;
}

size_t cg_function::arg_size() const{
	assert( fn );
	size_t arg_size = fn->arg_size();
	if( fn ){
		if( first_arg_is_return_address() ){ --arg_size; }
		if( partial_execution ) { --arg_size; }
		return arg_size;
	}
	return 0;
}

cg_value cg_function::arg( size_t index ) const
{
	// If c_compatible and not void return, the first argument is address of return value.
	size_t arg_index = index;
	if( first_arg_is_return_address() ){ ++arg_index; }
	if( partial_execution ){ ++arg_index; }

	shared_ptr<parameter> par = fnty->params[index];
	cg_type* par_ty = cg->get_node_context( par.get() )->ty;

	Function::ArgumentListType::iterator it = fn->arg_begin();
	for( size_t idx_counter = 0; idx_counter < arg_index; ++idx_counter ){
		++it;
	}

	abis arg_abi = cg->param_abi( c_compatible );
	return cg->create_value( par_ty, &(*it), arg_is_ref(index) ? vkind_ref : vkind_value, arg_abi );
}

cg_value cg_function::packed_execution_mask() const
{
	if( !partial_execution ){ return cg_value(); }

	Function::ArgumentListType::iterator it = fn->arg_begin();
	if( first_arg_is_return_address() ){ ++it; }

	return cg->create_value( builtin_types::_uint16, &(*it), vkind_value, abi_llvm );
}

cg_function::cg_function(): fn(NULL), fnty(NULL), ret_void(true), c_compatible(false), cg(NULL), external(false), partial_execution(false)
{
}

bool cg_function::arg_is_ref( size_t index ) const{
	assert( index < fnty->params.size() );

	builtin_types hint = cg->get_node_semantic( fnty->params[index].get() )->value_builtin_type();
	return c_compatible && !is_scalar(hint) && !is_sampler(hint);
}

bool cg_function::first_arg_is_return_address() const
{
	return ( c_compatible || external ) && !ret_void;
}

abis cg_function::abi() const
{
	return cg->param_abi( c_compatible );
}

llvm::Value* cg_function::return_address() const
{
	if( first_arg_is_return_address() ){
		return &(*fn->arg_begin());
	}
	return NULL;
}

void cg_function::return_name( std::string const& s )
{
	if( first_arg_is_return_address() ){
		fn->arg_begin()->setName( s );
	}
}

void cg_function::inline_hint()
{
	fn->addAttribute( 0, llvm::Attribute::InlineHint );
}

insert_point_t::insert_point_t(): block(NULL)
{
}

insert_point_t cg_function::allocation_block() const
{
	return alloc_block;
}

END_NS_SASL_CODE_GENERATOR();