#include <sasl/include/semantic/symbol.h>

#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/type_checker.h>
#include <sasl/include/semantic/type_converter.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/syntax_tree/node.h>

#include <eflib/include/platform/disable_warnings.h>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <eflib/include/platform/enable_warnings.h>
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
	correspond_node->symbol( ret );
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
	//			after all comparison done, if candidate have not be discarded, add it into candidates.
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
			shared_ptr<type_specifier> arg_type = extract_semantic_info<type_info_si>( args[i_param] )->type_info();
			shared_ptr<type_specifier> par_type = extract_semantic_info<type_info_si>( matching_func->params[i_param] )->type_info();
			if ( !( type_equal( arg_type, par_type) || conv->convert(matching_func->params[i_param], args[i_param]) ) ){
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
				shared_ptr<type_specifier> arg_type = extract_semantic_info<type_info_si>( args[i_param] )->type_info();
				shared_ptr<type_specifier> matching_par_type = extract_semantic_info<type_info_si>( matching_func->params[i_param] )->type_info();
				shared_ptr<type_specifier> matched_par_type = extract_semantic_info<type_info_si>( a_matched_func->params[i_param] )->type_info();

				if( type_equal( matched_par_type, arg_type ) ){
					if ( !type_equal( matching_par_type, arg_type ) ){
						++worse_param_count;
					}
				} else {
					if ( type_equal( matching_par_type, arg_type ) ){
						++better_param_count;
					} else if (
						conv->convert( matching_par_type, matched_par_type ) == type_converter::implicit_conv
						&& conv->convert( matched_par_type, matching_par_type ) != type_converter::implicit_conv
						)
					{
						++better_param_count;
					} else if (
						conv->convert( matching_par_type, matched_par_type ) != type_converter::implicit_conv
						&& conv->convert( matched_par_type, matching_par_type ) == type_converter::implicit_conv
						)
					{
						++worse_param_count;
					}
				}
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

shared_ptr<symbol> symbol::add_overloaded_child(
	const string& unmangled,
	const string& mangled,
	shared_ptr<struct node> child_node
	)
{
	shared_ptr<symbol> added_sym = add_child( mangled, child_node );
	if ( added_sym ){
		if( overloads.count( unmangled ) == 0 ){
			overloads[ unmangled ] = vector<string>();
		}
		overloads[ unmangled ].push_back( mangled );
		added_sym->umgl_name = unmangled;
	}
	return added_sym;
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
	return correspond_node.lock();
}

void symbol::relink( shared_ptr<struct node> n ){
	correspond_node = n;
}

const string& symbol::mangled_name() const{
	return mgl_name;
}

const string& symbol::unmangled_name() const{
	return umgl_name;
}

void symbol::add_mangling( const string& mangled ){
	mgl_name = mangled;

	if( !parent() ){
		return;
	}

	// add new name and remove old name
	this_parent.lock()->children.insert( make_pair( mangled, node()->symbol() ) );
	this_parent.lock()->children.erase( umgl_name );

	// add to overloaded items table
	if ( this_parent.lock()->get_overloads(umgl_name).empty() ){
		this_parent.lock()->overloads[umgl_name] = overload_table_t::mapped_type();
	}
	this_parent.lock()->overloads[umgl_name].push_back( mangled );
}

int anonymous_name_count = 0;
boost::mutex mtx;
string anonymous_name(){
	boost::mutex::scoped_lock locker(mtx);
	return string("__sasl__anonymous__name__66EE3DF2AC1746aaA963B98C7F1985E1_") + boost::lexical_cast<string>(++anonymous_name_count);
}

shared_ptr<symbol> symbol::add_anonymous_child( shared_ptr<struct node> child_node ){
	return add_child( anonymous_name(), child_node );
}

shared_ptr<symbol> symbol::add_type_node( const string& mangled, shared_ptr<type_specifier> tnode )
{
	if( tnode->node_class() == syntax_node_types::buildin_type ){

	}
}

END_NS_SASL_SEMANTIC();
