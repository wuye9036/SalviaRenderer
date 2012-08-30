#include <sasl/include/codegen/cgs_objects.h>

#include <sasl/include/codegen/cgs.h>
#include <sasl/include/codegen/utility.h>
#include <sasl/include/codegen/cg_contexts.h>
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

BEGIN_NS_SASL_CODEGEN();

cg_type::cg_type(tynode* tyn, Type* ty_c, Type* ty_llvm, Type* ty_vec, Type* ty_pkg )
	: tyn(tyn)
{
	tys[abis::c]			= ty_c;
	tys[abis::llvm]		= ty_llvm;
	tys[abis::vectorize]	= ty_vec;
	tys[abis::package]	= ty_pkg;
}

cg_type::cg_type(): tyn(NULL)
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

llvm::Type* cg_type::ty( abis::id abi ) const{
	return tys[abi];
}

cg_value::cg_value()
	: ty_(NULL), val_(NULL), cg_(NULL), kind_(value_kinds::unknown), builtin_ty_(builtin_types::none), abi_(abis::unknown), masks_(0)
{
}

cg_value::cg_value(cg_type* ty, Value* val, value_kinds::id k, abis::id abi, cg_service* cg)
	: ty_(ty), val_(val), cg_(cg), kind_(k), builtin_ty_(builtin_types::none), abi_(abi), masks_(0)
{
}

cg_value::cg_value(builtin_types hint, Value* val, value_kinds::id k, abis::id abi, cg_service* cg)
	: ty_(NULL), builtin_ty_(hint), abi_(abi), val_(val), kind_(k), cg_(cg), masks_(0)
{

}

cg_value::cg_value( cg_value const& rhs )
	: ty_(rhs.ty_), builtin_ty_(rhs.builtin_ty_)
	, abi_(rhs.abi_), val_( rhs.val_ )
	, kind_(rhs.kind_), cg_(rhs.cg_), masks_(rhs.masks_)
{
	parent(rhs.parent_.get());
	index(rhs.index_.get());
}

abis::id cg_value::abi() const{
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
	if( ty_ ){
		return cg_value( ty_, load(abi_), value_kinds::value, abi_, cg_ );
	} else {
		return cg_value( builtin_ty_, load(abi_), value_kinds::value, abi_, cg_ );
	}
}

builtin_types cg_value::hint() const
{
	return ty_ ? ty_->hint() : builtin_ty_;
}

llvm::Value* cg_value::load( abis::id abi ) const{
	return cg_->load( *this, abi );
}

Value* cg_value::load() const{
	return cg_->load( *this );
}

value_kinds::id cg_value::kind() const{
	return kind_;
}

bool cg_value::storable() const{
	switch( kind_ ){
	case value_kinds::reference:
		return true;
	case value_kinds::value:
	case value_kinds::unknown:
		return false;
	case value_kinds::elements:
		return parent()->storable();
	default:
		EFLIB_ASSERT_UNIMPLEMENTED();
		return false;
	}
}

bool cg_value::load_only() const
{
	switch( kind_ ){
	case value_kinds::reference:
	case value_kinds::unknown:
		return false;
	case value_kinds::value:
		return true;
	default:
		EFLIB_ASSERT_UNIMPLEMENTED();
		return false;
	}
}

void cg_value::emplace( Value* v, value_kinds::id k, abis::id abi ){
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
	ty_ = rhs.ty_;
	builtin_ty_ = rhs.builtin_ty_;
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

	cg_value ret( scalar_of(hint), NULL, value_kinds::elements, vec.abi_, vec.cg_ );
	ret.masks_ = masks;
	ret.parent(vec);

	return ret;
}

cg_value cg_value::slice( cg_value const& vec, cg_value const& index )
{
	builtin_types hint = vec.hint();
	assert( is_vector(hint) );

	cg_value ret( scalar_of(hint), NULL, value_kinds::elements, vec.abi_, vec.cg_ );
	ret.index(index);
	ret.parent(vec);

	return ret;
}

cg_value cg_value::as_ref() const
{
	cg_value ret(*this);

	switch( ret.kind_ ){
	case value_kinds::value:
		ret.kind_ = value_kinds::reference;
		break;
	case value_kinds::elements:
		ret.kind_ = (value_kinds::id)( value_kinds::elements | value_kinds::reference );
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

cg_type*		cg_value::ty() const		{ return ty_; }
void			cg_value::ty( cg_type* v )	{ ty_ = v; }

void			cg_value::hint( builtin_types bt ){ builtin_ty_ = bt; }
void			cg_value::abi( abis::id abi ){ this->abi_ = abi; }
uint32_t		cg_value::masks() const{ return masks_; }
void			cg_value::masks( uint32_t v ){ masks_ = v; }

void			cg_value::kind( value_kinds::id vkind ) { kind_ = vkind; }
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
		return cg_->extension()->i8toi1_sv( load(abis::llvm) );
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

	abis::id arg_abi = cg->param_abi( c_compatible );
	return cg->create_value( par_ty, &(*it), arg_is_ref(index) ? value_kinds::reference : value_kinds::value, arg_abi );
}

cg_value cg_function::packed_execution_mask() const
{
	if( !partial_execution ){ return cg_value(); }

	Function::ArgumentListType::iterator it = fn->arg_begin();
	if( first_arg_is_return_address() ){ ++it; }

	return cg->create_value( builtin_types::_uint16, &(*it), value_kinds::value, abis::llvm );
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

abis::id cg_function::abi() const
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

END_NS_SASL_CODEGEN();
