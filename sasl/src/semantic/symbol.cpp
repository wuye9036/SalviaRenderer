#include <sasl/include/semantic/symbol.h>

#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/semantic/type_converter.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/node.h>

#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <eflib/include/platform/boost_end.h>

#include <algorithm>

using namespace std;

BEGIN_NS_SASL_SEMANTIC();

using ::sasl::syntax_tree::expression;
using ::sasl::syntax_tree::function_type;
using ::sasl::syntax_tree::type_specifier;

using ::boost::shared_ptr;

static shared_ptr<symbol> nullsym;
static vector< shared_ptr<symbol> > empty_syms;

shared_ptr<symbol> symbol::create_root( shared_ptr<struct node> root_node ){
	return create( shared_ptr<symbol>(), root_node, string("") );
}

shared_ptr<symbol> symbol::create(
	shared_ptr<symbol> parent,
	shared_ptr<struct node> correspond_node,
	const string& mangled )
{
	shared_ptr<symbol> ret( new symbol( parent, correspond_node, mangled ) );
	ret->selfptr = ret;
	if( correspond_node ){
		correspond_node->symbol( ret );
	}
	return ret;
}

symbol::symbol( shared_ptr<symbol> parent,
			   shared_ptr<struct node> correspond_node,
			   const string& mangled
			   )
			   :this_parent(parent),
			   correspond_node(correspond_node),
			   umgl_name( mangled ),
			   mgl_name( mangled )
{
}

shared_ptr<symbol> symbol::find_this( const string& mangled ) const
{
	children_t::const_iterator ret_it = children.find(mangled);
	if (ret_it == children.end()){
		return nullsym;
	} else {
		return ret_it->second;
	}
}

shared_ptr<symbol> symbol::find( const string& mangled ) const
{
	shared_ptr<symbol> ret = find_this(mangled);
	if (ret) {	return ret; }
	if ( !parent() ) { return nullsym;	}
	return parent()->find(mangled);
}

const vector< string >& symbol::get_overloads( const string& unmangled_name ) const
{
	overload_table_t::const_iterator found_it = overloads.find( unmangled_name );
	if ( found_it == overloads.end() ){
		return null_mt;
	}
	return found_it->second;
}

vector< shared_ptr<symbol> > symbol::find_overloads( const string& unmangled ) const{
	vector< shared_ptr<symbol> > ret;
	const vector< string >& name_of_ret = get_overloads( unmangled );
	if ( !name_of_ret.empty() ) {
		for( size_t i_name = 0; i_name < name_of_ret.size(); ++i_name ){
			ret.push_back( find( name_of_ret[i_name] ) );
		}
		return ret;
	}
	if ( !parent() ) { return empty_syms; }
	return parent()->find_overloads( unmangled );
}

vector< shared_ptr<symbol> > symbol::find_overloads(
	const string& unmangled,
	shared_ptr<type_converter> conv,
	vector< shared_ptr<expression> > args ) const
{
	// find all overloads
	vector< shared_ptr<symbol> > overloads = find_overloads( unmangled );
	if( overloads.empty() ) { return overloads; }

	// Find candidates.
	// Following steps could impl function overloading :
	//
	//	for each candidate in overloads
	//		if candidate is a valid overload
	//			compare this candidate to evaluated candidates
	//				if candidate is better than evaluated, discard evaluated.
	//				if candidate is worse than evaluated, discard current candidate
	//			after all comparison done, if candidate have not been discarded, add it into candidates.
	//	now the candidates is result.
	//
	// better & worse judgement is as same as C#.
	vector< shared_ptr<symbol> > candidates;
	for( size_t i_func = 0; i_func < overloads.size(); ++i_func ){
		shared_ptr<function_type> matching_func = overloads[i_func]->node()->typed_handle<function_type>();

		// could not matched.
		if ( matching_func->params.size() != args.size() ){ continue; }

		// try to match all parameters.
		bool all_parameter_success = true;
		for( size_t i_param = 0; i_param < args.size(); ++i_param ){
			type_entry::id_t arg_type = extract_semantic_info<type_info_si>( args[i_param] )->entry_id();
			type_entry::id_t par_type = extract_semantic_info<type_info_si>( matching_func->params[i_param] )->entry_id();
			if( arg_type == -1 || par_type == -1 ){
				assert( !"Argument type or parameter type is invalid." );
				// TODO: Here is syntax error. Need to be processed.
				all_parameter_success = false;
				break;
			}
			if ( !( arg_type == par_type || conv->implicit_convertible(par_type, arg_type) ) ){
				all_parameter_success = false;
				break;
			}
		}
		if( !all_parameter_success ){ continue;	}

		// if all parameter could be matched, we will find does it better than others.
		bool is_better = false;
		bool is_worse = false;
		for( vector< shared_ptr<symbol> >::iterator it = candidates.begin(); it != candidates.end();  ){

			shared_ptr<function_type> a_matched_func = (*it)->node()->typed_handle<function_type>();

			// match functions.
			size_t better_param_count = 0;
			size_t worse_param_count = 0;

			for( size_t i_param = 0; i_param < args.size(); ++i_param ){
				type_entry::id_t arg_type = extract_semantic_info<type_info_si>( args[i_param] )->entry_id();
				type_entry::id_t matching_par_type = extract_semantic_info<type_info_si>( matching_func->params[i_param] )->entry_id();
				type_entry::id_t matched_par_type = extract_semantic_info<type_info_si>( a_matched_func->params[i_param] )->entry_id();

				bool par_is_better = false;
				bool par_is_worse = false;
				conv->better_or_worse_convertible( matched_par_type, matching_par_type, arg_type, par_is_better, par_is_worse );
				if( par_is_better ){ ++better_param_count; }
				if( par_is_worse ){ ++worse_param_count; }
			}

			if ( better_param_count > 0 && worse_param_count == 0 ) {
				is_better = true;
			}
			if ( better_param_count == 0 && worse_param_count > 0 ){
				is_worse = true;
				break;
			}

			// if current function is better than matched function, remove matched function.
			if( is_better ){
				it = candidates.erase( it );
			} else {
				++it;
			}
		}
		// if current function is worse than matched function, discard it.
		if ( !is_worse ) {
			candidates.push_back(matching_func->symbol());
		}
	}

	return candidates;
}

shared_ptr<symbol> symbol::add_child( const string& mangled, shared_ptr<struct node> child_node )
{
	children_iterator_t ret_it = children.find(mangled);
	if ( ret_it != children.end() ){
		return shared_ptr<symbol>();
	}
	shared_ptr<symbol> ret = create( selfptr.lock(), child_node, mangled );
	children.insert( make_pair( mangled, ret ) );
	return ret;
}

shared_ptr<symbol> symbol::add_function_begin( shared_ptr<function_type> child_fn ){
	shared_ptr<symbol> ret;
	if( !child_fn ){ return ret; }

	return create( selfptr.lock(), child_fn->handle(), child_fn->name->str );
}

bool symbol::add_function_end( boost::shared_ptr<symbol> sym ){
	EFLIB_ASSERT_AND_IF( sym, "Input symbol is NULL." ){
		return false;
	}

	EFLIB_ASSERT_AND_IF( sym->node(), "Node of input symbol is NULL. Maybe the symbol is not created by add_function_begin()." ){
		return false;
	}

	sym->mgl_name = mangle( sym->node()->typed_handle<function_type>() );

	children_iterator_t ret_it = children.find(sym->mgl_name);
	if ( ret_it != children.end() ){
		return false;
	}

	children.insert( make_pair( sym->mgl_name, sym ) );
	if( overloads.count( sym->umgl_name ) == 0 ){
		overloads[ sym->umgl_name ] = vector<string>();
	}
	overloads[ sym->umgl_name ].push_back( sym->mgl_name );
	return true;
}

void symbol::remove_child( const string& mangled ){
	children_iterator_t ret_it = children.find( mangled );
	if ( ret_it == children.end() ){
		return;
	}
	// remove symbol from corresponding node.
	shared_ptr<symbol> rmsym = ret_it->second;
	if( rmsym->node() ){
		rmsym->node()->symbol( shared_ptr<symbol>() );
	}
	// remove itself.
	children.erase( ret_it );

	//remove mangled item from overloaded items table.
	if ( overloads.count( rmsym->unmangled_name() ) > 0){
		overload_table_t::mapped_type& mt = overloads[umgl_name];
		overload_table_t::mapped_type::iterator mt_it
			= ::std::find( mt.begin(), mt.end(), mgl_name );
		mt.erase( mt_it );
		if ( mt.empty() ) { overloads.erase( umgl_name ); }
	}
}

void symbol::remove(){
	if ( parent() ){
		parent()->remove_child( mangled_name() );
	}
}

shared_ptr<symbol> symbol::parent() const{
	return this_parent.lock();
}

shared_ptr<node> symbol::node() const
{
	return correspond_node;
}

void symbol::relink( shared_ptr<struct node> n ){
	if ( correspond_node == n ){
		return;
	}
	if( n->symbol() ){
		n->symbol()->remove();
	}

	correspond_node = n;
	n->symbol( selfptr.lock() );
}

const string& symbol::mangled_name() const{
	return mgl_name;
}

const string& symbol::unmangled_name() const{
	return umgl_name;
}

int anonymous_name_count = 0;
boost::mutex mtx;
string symbol::unique_name( symbol::unique_name_types unique_type ){
	boost::uuids::uuid uid = boost::uuids::random_generator()();
	
	switch( unique_type ){
	case unique_in_unit:
		{
			boost::mutex::scoped_lock locker(mtx);
			return string("0unnamed_") + boost::lexical_cast<string>(++anonymous_name_count);
		}
	case unnamed_struct:
		return std::string("0unnamed_struct") + boost::uuids::to_string( uid );
	case unique_in_module:
	default:
		return std::string("unnamed") + boost::uuids::to_string( uid );
	}
}

shared_ptr<symbol> symbol::add_anonymous_child( shared_ptr<struct node> child_node ){
	return add_child( unique_name(), child_node );
}

END_NS_SASL_SEMANTIC();
