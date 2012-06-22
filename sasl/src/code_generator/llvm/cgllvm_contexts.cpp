#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/code_generator/llvm/cgs.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/pool/pool.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

using sasl::syntax_tree::node;
using boost::unordered_map;
using boost::pool;
using std::make_pair;
using std::vector;
using std::ostream;

BEGIN_NS_SASL_CODE_GENERATOR();

class cgllvm_caster;

class module_context_impl : public module_context
{
public:
	virtual node_context* get_node_context(node* v) const
	{
		node_context_dict::const_iterator it = context_dict_.find(v);
		if( it != context_dict_.end() ) return it->second;
		return NULL;
	}

	virtual node_context* get_or_create_node_context(node* v)
	{
		node_context* ret = get_node_context(v);
		if( ret ){ return ret; }
		ret = static_cast<node_context*>( contexts_pool_.malloc() );
		new (ret) node_context(this);
		contexts_.push_back(ret);
		context_dict_.insert( make_pair(v, ret) );
		return ret;
	}

	virtual cg_type* create_cg_type()
	{
		cg_type* ret = static_cast<cg_type*>( types_pool_.malloc() );
		new (ret) cg_type(this);
		cg_types_.push_back(ret);
		return ret;
	}

	virtual function_t* create_cg_function()
	{
		EFLIB_ASSERT_UNIMPLEMENTED();
		return NULL;
	}

	virtual llvm::Module*	llvm_module() const;
	virtual llvm::Module*	take_ownership();
	virtual llvm::DefaultIRBuilder*
							llvm_builder() const;
	virtual llvm::LLVMContext&
							context() const;
	virtual void			dump(ostream& ostr) const;

	module_context_impl()
		: contexts_pool_( sizeof(node_context) )
		, types_pool_( sizeof(cg_type) )
		, functions_pool_( sizeof(function_t) )
		, caster_(NULL)
	{
	}

	~module_context_impl()
	{
		clean_contexts();
		clean_cg_types();
	}
private:
	void clean_contexts()
	{
		for(vector<node_context*>::iterator it = contexts_.begin(); it != contexts_.end(); ++it)
		{
			(*it)->~node_context();
		}
	}

	void clean_cg_types()
	{
		for(vector<cg_type*>::iterator it = cg_types_.begin(); it != cg_types_.end(); ++it)
		{
			(*it)->~cg_type();
		}
	}

	pool<>	contexts_pool_;
	pool<>	types_pool_;
	pool<>	functions_pool_;

	typedef unordered_map<node*, node_context*> node_context_dict;
	node_context_dict		context_dict_;
	vector<node_context*>	contexts_;
	vector<cg_type*>		cg_types_;
	vector<function_t*>		functions_;
	cgllvm_caster*			caster_;
};

cgllvm_sctxt_data::cgllvm_sctxt_data()
: declarator_count(0), semantic_mode(false)
{
}

cgllvm_sctxt::cgllvm_sctxt()
{
}

cgllvm_sctxt::cgllvm_sctxt( cgllvm_sctxt const& rhs ){
	copy(&rhs);
}

cgllvm_sctxt_data& cgllvm_sctxt::data(){
	return hold_data;
}

cgllvm_sctxt_data const& cgllvm_sctxt::data() const{
	return hold_data;
}

void cgllvm_sctxt::data( cgllvm_sctxt_data const& rhs ){
	if( &rhs == &data() ) return;
	hold_data = rhs;
}

void cgllvm_sctxt::data( cgllvm_sctxt const* rhs ){
	hold_data = rhs->data();
}

void cgllvm_sctxt::copy( cgllvm_sctxt const* rhs ){
	if( rhs == this ){ return; }
	env( rhs->env() );
	data( rhs->data() );
}

cgllvm_sctxt_env& cgllvm_sctxt::env(){
	return hold_env;
}

cgllvm_sctxt_env const& cgllvm_sctxt::env() const{
	return hold_env;
}

void cgllvm_sctxt::env( cgllvm_sctxt const* rhs ){
	hold_env = rhs->env();
}

void cgllvm_sctxt::env( cgllvm_sctxt_env const& rhs ){
	hold_env = rhs;
}

cgllvm_sctxt& cgllvm_sctxt::operator=( cgllvm_sctxt const& rhs ){
	copy( &rhs );
	return *this;
}

void cgllvm_sctxt::clear_data(){
	data( cgllvm_sctxt().data() );
}

value_t const& cgllvm_sctxt::value() const{
	return data().val;
}

value_t& cgllvm_sctxt::value(){
	return data().val;
}

value_t cgllvm_sctxt::get_rvalue() const{
	return data().val.to_rvalue();
}

cg_type* cgllvm_sctxt::get_typtr() const
{
	return get_tysp().get();
}

boost::shared_ptr<cg_type> cgllvm_sctxt::get_tysp() const{
	return data().tyinfo;
}

cgllvm_sctxt_env::cgllvm_sctxt_env() 
	: block(NULL)
	, parent_struct(NULL)
	, is_semantic_mode(false)
{
}

END_NS_SASL_CODE_GENERATOR();