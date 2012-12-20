#include <sasl/include/semantic/symbol.h>

#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/semantic/caster.h>
#include <sasl/include/semantic/semantic_diags.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/enums/enums_utility.h>

#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/utility/polymorphic_cast.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <eflib/include/platform/boost_end.h>

#include <algorithm>

using boost::bind;
using eflib::polymorphic_cast;
using eflib::fixed_string;
using namespace std;


BEGIN_NS_SASL_SEMANTIC();

using sasl::common::diag_chat;
using sasl::syntax_tree::expression;
using sasl::syntax_tree::function_full_def;
using sasl::syntax_tree::tynode;
using sasl::utility::is_scalar;
using sasl::utility::is_vector;
using sasl::utility::is_matrix;
using sasl::utility::scalar_of;
using sasl::utility::vector_size;
using sasl::utility::vector_count;

using ::boost::shared_ptr;

fixed_string symbol::null_name;

symbol* symbol::create_root(module_semantic* owner, node* root_node){
	null_name = fixed_string("");
	return create(owner, NULL, root_node);
}

symbol* symbol::create(module_semantic* owner, symbol* parent, node* assoc_node, fixed_string const& mangled)
{
	assert( owner->get_symbol(assoc_node) == NULL );
	symbol* ret = new ( owner->alloc_symbol() ) symbol(owner, parent, NULL, &mangled);
	owner->link_symbol(assoc_node, ret);
	return ret;
}

symbol* symbol::create( module_semantic* owner, symbol* parent, node* assoc_node )
{
	assert( owner->get_symbol(assoc_node) == NULL );
	symbol* ret = new ( owner->alloc_symbol() ) symbol(owner, parent, NULL, NULL);
	owner->link_symbol(assoc_node, ret);
	return ret;
}

symbol::symbol(module_semantic* owner, symbol* parent, node* assoc_node, fixed_string const* mangled)
	: owner_(owner), parent_(parent), associated_node_(assoc_node)
	, unmangled_name_(mangled ? *mangled : null_name), mangled_name_(mangled ? *mangled : null_name)
{
}

symbol* symbol::find_this( fixed_string const& mangled ) const
{
	named_children_dict::const_iterator iter = named_children_.find(mangled);
	return iter == named_children_.end() ? NULL : iter->second;
}

symbol* symbol::find( fixed_string const& mangled ) const
{
	symbol* ret = find_this(mangled);
	if (ret) {	return ret; }
	if ( !parent_ ) { return NULL;	}
	return parent_->find(mangled);
}

vector<fixed_string> empty_strings;
vector<fixed_string> const& symbol::get_overloads(fixed_string const& unmangled_name) const
{
	overload_dict::const_iterator iter = overloads_.find(unmangled_name);
	return iter == overloads_.end() ? empty_strings : iter->second;
}

symbol::symbol_array symbol::find_overloads(fixed_string const& unmangled) const
{
	symbol_array ret;
	vector<fixed_string> const& name_of_ret = get_overloads( unmangled );
	if ( !name_of_ret.empty() )
	{
		for( size_t i_name = 0; i_name < name_of_ret.size(); ++i_name )
		{
			ret.push_back( find(name_of_ret[i_name]) );
		}
		return ret;
	}
	if ( !parent_ ) { return symbol_array(); }
	return parent_->find_overloads(unmangled);
}

symbol::symbol_array symbol::find_overloads(const fixed_string& unmangled, caster_t* conv, expression_array const& args) const
{
	// find all overloads_
	symbol_array overloads_ = find_overloads_impl(unmangled, conv, args);
	collapse_vector1_overloads(overloads_);
	return overloads_;
}

symbol::symbol_array symbol::find_assign_overloads(const fixed_string& unmangled, caster_t* conv, expression_array const& args) const
{
	symbol_array candidates = find_overloads_impl(unmangled, conv, args);
	tid_t lhs_arg_tid = owner_->get_semantic( args.back() )->tid();
	symbol_array ret;
	BOOST_FOREACH( symbol* proto, candidates )
	{
		function_full_def* proto_fn = dynamic_cast<function_full_def*>( proto->associated_node() );
		tid_t lhs_par_tid = owner_->get_semantic( proto_fn->params.back() )->tid();
		if( lhs_par_tid == lhs_arg_tid )
		{
			ret.push_back(proto);
		}
	}
	return ret;
}

symbol* symbol::add_named_child( const fixed_string& mangled, node* child_node )
{
	named_children_dict_iterator iter = named_children_.find(mangled);
	if ( iter != named_children_.end() )
	{
		return NULL;
	}

	symbol* ret = add_child(child_node);
	ret->mangled_name_ = ret->unmangled_name_ = mangled;
	named_children_.insert( make_pair(mangled, ret) );

	return ret;
}

symbol* symbol::add_function_begin(function_full_def* child_fn){
	if( !child_fn ){ return NULL; }
	symbol* ret = new ( owner_->alloc_symbol() ) symbol(owner_, this, child_fn, &child_fn->name->str);
	return ret;
}

bool symbol::add_function_end(symbol* sym){
	EFLIB_ASSERT_AND_IF( sym, "Input symbol is NULL." ){
		return false;
	}

	EFLIB_ASSERT_AND_IF(
		sym->associated_node(),
		"Node of input symbol is NULL. Maybe the symbol is not created by add_function_begin()."
		)
	{
		return false;
	}

	sym->mangled_name_ = mangle( owner_, polymorphic_cast<function_full_def*>( sym->associated_node() ) );

	named_children_dict_iterator ret_it = named_children_.find(sym->mangled_name_);
	if ( ret_it != named_children_.end() ){
		return false;
	}

	children_.insert( make_pair(sym->associated_node(), sym) );
	owner_->link_symbol(sym->associated_node(), sym);
	named_children_.insert( make_pair( sym->mangled_name_, sym ) );
	if( overloads_.count(sym->unmangled_name_) == 0 ){
		overloads_[sym->unmangled_name_].resize(0);
	}
	overloads_[sym->unmangled_name_].push_back( sym->mangled_name_ );
	return true;
}

void symbol::cancel_function(symbol* /*sym*/)
{
	// Do nothing while function is cancelled.
}

void symbol::remove_child( const fixed_string& mangled ){
	named_children_dict_iterator ret_it = named_children_.find( mangled );
	if ( ret_it == named_children_.end() ){
		return;
	}
	// remove symbol from corresponding node.
	symbol* rmsym = ret_it->second;
	if( rmsym->associated_node() )
	{
		children_.erase( rmsym->associated_node() );
		owner_->link_symbol(NULL, rmsym);
	}

	// remove itself.
	named_children_.erase( ret_it );

	//remove mangled item from overloaded items table.
	if ( overloads_.count( rmsym->unmangled_name() ) > 0)
	{
		overload_dict::mapped_type& mt = overloads_[unmangled_name_];
		overload_dict::mapped_type::iterator mt_it
			= std::find(mt.begin(), mt.end(), mangled_name_);
		mt.erase(mt_it);
		if ( mt.empty() ) { overloads_.erase(unmangled_name_); }
	}
}

void symbol::remove_child( symbol* sym )
{
	EFLIB_ASSERT_UNIMPLEMENTED();
}

void symbol::remove(){
	if (parent_)
	{
		parent_->remove_child(this);
	}
}

symbol* symbol::parent() const{
	return parent_;
}

node* symbol::associated_node() const
{
	return associated_node_;
}

void symbol::associated_node( node* v )
{
	associated_node_ = v;
}

fixed_string const& symbol::mangled_name() const{
	return mangled_name_;
}

fixed_string const& symbol::unmangled_name() const{
	return unmangled_name_;
}

symbol* symbol::add_child(node* child_node){
	children_dict::iterator iter = children_.find(child_node);
	if( iter != children_.end() ) { return NULL; }

	symbol* ret = create(owner_, this, child_node);
	children_.insert( make_pair(child_node, ret) );

	return ret;
}

bool is_equiva( builtin_types bt0, builtin_types bt1 )
{
	if( ! (is_scalar(bt0) || is_vector(bt0) || is_matrix(bt0)) ){
		return false;
	}

	if( ! (is_scalar(bt1) || is_vector(bt1) || is_matrix(bt1)) ){
		return false;
	}

	if( is_scalar(bt0) && is_scalar(bt1) ){
		return false;
	}

	if( is_scalar(bt0) && is_vector(bt1) ){
		return (
			scalar_of(bt1) == bt0 && vector_size(bt1) == 1
			);
	}

	if( is_scalar(bt0) && is_matrix(bt1) ){
		return (
			scalar_of(bt1) == bt0 && vector_size(bt1) == 1 && vector_count(bt1) == 1
			);
	}

	if( is_vector(bt0) && is_vector(bt1) ){
		return false;
	}

	if( is_vector(bt0) && is_matrix(bt1) ){
		return (
			scalar_of(bt1) == scalar_of(bt0) && vector_size(bt1) == vector_size(bt0) && vector_count(bt1) == 1
			);
	}

	if( is_matrix(bt0) && is_matrix(bt1) ){
		return false;
	}

	return is_equiva( bt1, bt0 );
}

void is_same_or_equiva( module_semantic* msem, node* nd0, node* nd1, bool& same, bool& equiva ){
	same = false;
	equiva = false;

	node_semantic* nd0_sem = msem->get_semantic(nd0);
	node_semantic* nd1_sem = msem->get_semantic(nd1);

	bool same_tid = ( nd0_sem->tid() == nd1_sem->tid() );

	if( same_tid ){
		same = true;
		equiva = true;
		return;
	}

	builtin_types nd0_bt = nd0_sem->ty_proto()->tycode;
	builtin_types nd1_bt = nd1_sem->ty_proto()->tycode;
	equiva = is_equiva(nd0_bt, nd1_bt);

	return;
}

void symbol::collapse_vector1_overloads( symbol_array& candidates ) const
{
	symbol_array ret;

	BOOST_FOREACH(symbol* cand, candidates)
	{
		shared_ptr<function_full_def> cand_fn = cand->associated_node()->as_handle<function_full_def>();

		bool matched = false;
		BOOST_FOREACH(symbol* filterated, ret)
		{
			shared_ptr<function_full_def> filterated_fn = filterated->associated_node()->as_handle<function_full_def>();
			size_t param_count = filterated_fn->params.size();

			bool same_function = true;
			for( size_t i_param = 0; i_param < param_count; ++i_param ){
				bool same = false;
				bool equiva = false;
				is_same_or_equiva( owner_, cand_fn->params[i_param].get(), filterated_fn->params[i_param].get(), same, equiva );
				if( !(same || equiva) ){
					same_function = false;
					break;
				}
			}
			if( same_function ){ matched = true; break; }
		}

		if( !matched ){ ret.push_back(cand); }
	}

	return std::swap( candidates, ret );
}

bool get_deprecated_and_next( symbol* const& sym, symbol* const* begin_addr, vector<bool> const& deprecated )
{
	intptr_t i = (intptr_t)std::distance( begin_addr, boost::addressof(sym) );
	return !deprecated[i];
}

symbol::symbol_array symbol::find_overloads_impl(
	fixed_string const& unmangled, caster_t* conv, expression_array const& args) const
{
	// find all overloads_
	symbol_array overloads_ = find_overloads(unmangled);
	if( overloads_.empty() ) { return overloads_; }

	// Extract type info of args
	vector<tid_t> arg_tids;
	vector<node_semantic*> arg_sems; 
	BOOST_FOREACH(expression* arg, args)
	{
		arg_sems.push_back( owner_->get_semantic(arg) );
		if( !arg_sems.back() )
		{
			overloads_.clear();
			return overloads_;
		}
		arg_tids.push_back( arg_sems.back()->tid() );
	}

	// Find candidates.
	// Following steps could impl function overloading :
	//
	//	for each candidate in overloads_
	//		if candidate is a valid overload
	//		add to candidates
	//	now the candidates is result.
	//
	// better & worse judgment is as same as C#.
	symbol_array candidates;
	for( size_t i_func = 0; i_func < overloads_.size(); ++i_func )
	{
		shared_ptr<function_full_def> matching_func = overloads_[i_func]->associated_node()->as_handle<function_full_def>();

		// could not matched.
		if ( matching_func->params.size() != args.size() ){ continue; }

		// try to match all parameters.
		bool all_parameter_success = true;
		for( size_t i_param = 0; i_param < args.size(); ++i_param ){
			node_semantic* arg_sem = arg_sems[i_param];
			node_semantic* par_sem = owner_->get_semantic(matching_func->params[i_param]);
			tid_t arg_type = arg_tids[i_param];
			tid_t par_type = par_sem->tid();
			if( arg_type == -1 || par_type == -1 ){
			all_parameter_success = false;
				break;
			}
			if ( !( arg_type == par_type || conv->try_implicit(par_type, arg_type) ) ){
				all_parameter_success = false;
				break;
			}
		}
		if( all_parameter_success )
		{
			candidates.push_back( overloads_[i_func] );
		}
	}

	//  for each candidate in candidates
	//		for each evaluated in candidate successors. 
	//			if candidate is better than evaluated, deprecate evaluated.
	//			if candidate is worse than evaluated, deprecated candidate.
	vector<bool> deprecated(candidates.size(), false) ;
	for ( size_t i_cand = 0; i_cand < candidates.size(); ++i_cand )
	{
		if( deprecated[i_cand] ){ continue; }
		shared_ptr<function_full_def> a_matched_func = (candidates[i_cand])->associated_node()->as_handle<function_full_def>();
		for( size_t j_cand = i_cand+1; j_cand < candidates.size(); ++j_cand )
		{
			if( deprecated[j_cand] ){ continue; }

			shared_ptr<function_full_def> matching_func = (candidates[j_cand])->associated_node()->as_handle<function_full_def>();

			size_t better_param_count = 0;
			size_t worse_param_count = 0;

			for( size_t i_param = 0; i_param < args.size(); ++i_param ){
				tid_t arg_type			= owner_->get_semantic(args[i_param])->tid();
				tid_t matching_par_type	= owner_->get_semantic(matching_func->params[i_param])->tid();
				tid_t matched_par_type	= owner_->get_semantic(a_matched_func->params[i_param])->tid();

				bool par_is_better = false;
				bool par_is_worse = false;
				conv->better_or_worse( matched_par_type, matching_par_type, arg_type, par_is_better, par_is_worse );
				if( par_is_better ){ ++better_param_count; }
				if( par_is_worse ){ ++worse_param_count; }
			}

			if ( better_param_count > 0 && worse_param_count == 0 ) {
				deprecated[i_cand] = true;
			}
			if ( better_param_count == 0 && worse_param_count > 0 ){
				deprecated[j_cand] = true;
			}
		}
	}

	// Gather candidates.
	if( !candidates.empty() )
	{
		symbol_array::iterator it
			= partition( candidates.begin(), candidates.end(), boost::bind( get_deprecated_and_next, _1, boost::addressof(candidates[0]), boost::cref(deprecated) ) );

		symbol_array ret( candidates.begin(), it ); 
		candidates.resize( distance(candidates.begin(), it) );
	}

	return candidates;
}

module_semantic* symbol::owner() const
{
	return owner_;
}

END_NS_SASL_SEMANTIC();
