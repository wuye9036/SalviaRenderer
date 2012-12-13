#include <sasl/include/codegen/cgs_objects.h>

#include <sasl/include/codegen/cgs.h>
#include <sasl/include/codegen/utility.h>
#include <sasl/include/codegen/cg_contexts.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/platform/disable_warnings.h>
#include <llvm/Function.h>
#include <llvm/IRBuilder.h>
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
using llvm::Argument;
using boost::shared_ptr;
using std::vector;
using std::string;

BEGIN_NS_SASL_CODEGEN();

cg_type::cg_type(tynode* tyn, Type* ty_c, Type* ty_llvm)
	: tyn(tyn)
{
	tys[abis::c]		= ty_c;
	tys[abis::llvm]		= ty_llvm;
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

llvm::Type* cg_type::ty(abis::id abi) const{
	return tys[abi];
}

multi_value::multi_value(size_t num_value)
	: ty_(NULL), val_(num_value, NULL)
	, cg_(NULL), kind_(value_kinds::unknown)
	, builtin_ty_(builtin_types::none), abi_(abis::unknown)
	, masks_(0)
{
}

multi_value::multi_value(
	cg_type* ty, value_array const& val,
	value_kinds::id k, abis::id abi, cg_service* cg)
	: ty_(ty), val_(val)
	, cg_(cg), kind_(k)
	, builtin_ty_(builtin_types::none), abi_(abi)
	, masks_(0)
{
}

multi_value::multi_value(
	builtin_types hint, value_array const& val,
	value_kinds::id k, abis::id abi, cg_service* cg)
	: ty_(NULL), val_(val)
	, builtin_ty_(hint), abi_(abi)
	, kind_(k), cg_(cg), masks_(0)
	
{
}

multi_value::multi_value(multi_value const& rhs)
	: ty_(rhs.ty_), builtin_ty_(rhs.builtin_ty_)
	, abi_(rhs.abi_), val_( rhs.val_ )
	, kind_(rhs.kind_), masks_(rhs.masks_), cg_(rhs.cg_)
{
	parent(rhs.parent_.get());
	index(rhs.index_.get());
}

abis::id multi_value::abi() const{
	return abi_;
}

multi_value multi_value::swizzle( size_t /*swz_code*/ ) const{
	assert( is_vector( hint() ) );
	EFLIB_ASSERT_UNIMPLEMENTED();
	return multi_value();
}

value_array multi_value::raw() const{
	return val_;
}

multi_value multi_value::to_rvalue() const
{
	if( ty_ ){
		return multi_value( ty_, load(abi_), value_kinds::value, abi_, cg_ );
	} else {
		return multi_value( builtin_ty_, load(abi_), value_kinds::value, abi_, cg_ );
	}
}

builtin_types multi_value::hint() const
{
	return ty_ ? ty_->hint() : builtin_ty_;
}

value_array multi_value::load(abis::id abi) const{
	return cg_->load(*this, abi);
}

value_array multi_value::load() const{
	return cg_->load(*this);
}

value_kinds::id multi_value::kind() const{
	return kind_;
}

bool multi_value::storable() const{
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

bool multi_value::load_only() const
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

bool multi_value::valid() const
{
	return ( valid_all(val_) || parent_ )
		&& kind_ != value_kinds::unknown
		&& abi_ != abis::unknown
		;
}

void multi_value::emplace(value_array const& v, value_kinds::id k, abis::id abi)
{
	val_ = v;
	kind_ = k;
	abi_ = abi;
}

void multi_value::emplace(multi_value const& v)
{
	emplace( v.raw(), v.kind(), v.abi() );
}

value_array multi_value::load_ref() const
{
	return cg_->load_ref(*this);
}

bool multi_value::is_mono() const
{
	return val_.size() == 1;
}

multi_value& multi_value::operator=( multi_value const& rhs )
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

multi_value multi_value::slice(multi_value const& vec, uint32_t masks)
{
	builtin_types hint = vec.hint();
	assert( is_vector(hint) );

	multi_value ret( scalar_of(hint), value_array(vec.value_count(), NULL), value_kinds::elements, vec.abi_, vec.cg_ );
	ret.masks_ = masks;
	ret.parent(vec);

	return ret;
}

multi_value multi_value::slice(multi_value const& vec, multi_value const& index)
{
	builtin_types hint = vec.hint();
	assert( is_vector(hint) );

	multi_value ret( scalar_of(hint), value_array(vec.value_count(), NULL), value_kinds::elements, vec.abi_, vec.cg_ );
	ret.index(index);
	ret.parent(vec);

	return ret;
}

multi_value multi_value::as_ref() const
{
	multi_value ret(*this);

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

void multi_value::store(multi_value const& v) const
{
	cg_->store( *(const_cast<multi_value*>(this)), v );
}

void multi_value::index(size_t index)
{
	char indexes[4] = { (char)index, -1, -1, -1 };
	masks_ = indexes_to_mask( indexes );
}

cg_type*		multi_value::ty() const		{ return ty_; }
void			multi_value::ty(cg_type* v)	{ ty_ = v; }

void			multi_value::hint( builtin_types bt ){ builtin_ty_ = bt; }
void			multi_value::abi( abis::id abi ){ this->abi_ = abi; }
uint32_t		multi_value::masks() const{ return masks_; }
void			multi_value::masks( uint32_t v ){ masks_ = v; }

void			multi_value::kind( value_kinds::id vkind ) { kind_ = vkind; }
void			multi_value::parent( multi_value const& v ){ parent_.reset( new multi_value(v) ); }
void			multi_value::parent( multi_value const* v ){ if(v){ parent(*v); } }
multi_value*	multi_value::parent() const { return parent_.get(); }

multi_value*	multi_value::index() const { return index_.get(); }
void			multi_value::index( multi_value const& v ){ index_.reset( new multi_value(v) ); }
void			multi_value::index( multi_value const* v ){ if(v) index(*v); }

size_t			multi_value::value_count() const { return val_.size(); }

//Workaround for llvm issue 12618
value_array multi_value::load_i1() const{
	if( hint() == builtin_types::_boolean )
	{
		return cg_->extension()->i8toi1_sv( load(abis::llvm) );
	}
	
	assert(false);
	return value_array(val_.size(), (Value*)NULL);
}

void cg_function::allocation_block( insert_point_t const& ip )
{
	alloc_block = ip;
}

void cg_function::arg_name( size_t index, std::string const& name ){
	assert( index < fn->arg_size() );

	Function::arg_iterator arg_it = fn->arg_begin();
	if( return_via_arg() ){
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

	if( return_via_arg() ){
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

cg_type* cg_function::result_type() const{
	assert( fnty->is_function() );
	return cg->get_node_context( fnty->retval_type.get() )->ty;
}

size_t cg_function::physical_args_count() const
{
	assert(fn);
	return fn ? fn->arg_size() : 0;
}

size_t cg_function::logical_args_count() const
{
	assert(fn);
	return fnty->params.size();
}

size_t cg_function::logical_arg_offset() const
{
	size_t ret = 0;
	if( return_via_arg() ){ ++ret; }
	if( partial_execution ){ ++ret; }
	return ret;
}

multi_value cg_function::arg(size_t index) const
{
	shared_ptr<parameter> par = fnty->params[index];
	cg_type* par_ty = cg->get_node_context( par.get() )->ty;

	Function::ArgumentListType::iterator it = fn->arg_begin();
	std::advance(it, logical_arg_offset() + index);
	abis::id arg_abi = cg->param_abi(c_compatible);

	Value* physical_arg_value = &(*it);
	value_array physical_multi_value;

	// Dereference array if need.
	if( multi_value_arg_as_ref() )
	{
		physical_arg_value = cg->builder().CreateLoad(physical_arg_value);
	}

	// Split array or just consider as mono value.
	if( multi_value_arg_as_ref() || multi_value_args() )
	{
		physical_multi_value = cg->extension()->split_array(physical_arg_value);
	}
	else
	{
		physical_multi_value.push_back(physical_arg_value);
	}
	
	value_kinds::id vkind = value_arg_as_ref(index) ? value_kinds::reference : value_kinds::value;
	return cg->create_value(par_ty, physical_multi_value, vkind, arg_abi);
}

multi_value cg_function::execution_mask() const
{
	if( !partial_execution ){ return multi_value(); }
	Function::ArgumentListType::iterator it = fn->arg_begin();
	if( return_via_arg() ){ ++it; }
	return cg->create_value(
		builtin_types::_uint32, value_array(1, &(*it)), value_kinds::value, abis::llvm
		);
}

cg_function::cg_function()
	: fn(NULL), fnty(NULL), ret_void(true), partial_execution(false)
	, c_compatible(false), external(false), cg(NULL)
{
}

bool cg_function::value_arg_as_ref(size_t logical_index) const
{
	assert( logical_index < fnty->params.size() );
	parameter* param = fnty->params[logical_index].get();
	node_semantic* param_semantic = cg->get_node_semantic(param);
	builtin_types param_hint = param_semantic->value_builtin_type();
	return (c_compatible || external) && !is_scalar(param_hint) && !is_sampler(param_hint);
}

bool cg_function::return_via_arg() const
{
	return (c_compatible || external) && !ret_void;
}

bool cg_function::multi_value_arg_as_ref() const
{
	return (c_compatible || external) && multi_value_args();
}

bool cg_function::multi_value_args() const
{
	return cg->parallel_factor() > 1;
}

abis::id cg_function::abi() const
{
	return cg->param_abi( c_compatible );
}

value_array cg_function::return_address() const
{
	if( return_via_arg() )
	{
		Value* addr_value = &(*fn->arg_begin());
		if( multi_value_arg_as_ref() )
		{
			return cg->extension()->split_array( cg->builder().CreateLoad(addr_value) );
		}
		else
		{
			return value_array(1, addr_value);
		}
	}
	return value_array(cg->parallel_factor(), NULL);
}

void cg_function::return_name( std::string const& s )
{
	if( return_via_arg() ){
		fn->arg_begin()->setName( s );
	}
}

void cg_function::inline_hint()
{
	fn->addAttribute( 0, llvm::Attribute::InlineHint );
}

bool cg_function::need_mask() const
{
	return cg->parallel_factor() > 1 && partial_execution;
}

insert_point_t::insert_point_t(): block(NULL)
{
}

insert_point_t cg_function::allocation_block() const
{
	return alloc_block;
}

bool valid_any(value_array const& arr)
{
	if (arr.size() == 0) return false;
	for(value_array::const_iterator it = arr.begin(); it != arr.end(); ++it)
	{
		if(*it != NULL)
		{
			return true;
		}
	}
	return false;
}

bool valid_all(value_array const& arr)
{
	if (arr.size() == 0) return false;
	for(value_array::const_iterator it = arr.begin(); it != arr.end(); ++it)
	{
		if(*it == NULL) return false;
	}
	return true;
}
END_NS_SASL_CODEGEN();
