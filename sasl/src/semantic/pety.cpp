#include <sasl/include/semantic/pety.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/utility.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/enums/builtin_types.h>
#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

using namespace sasl::syntax_tree;
using namespace sasl::utility;

using boost::make_shared;
using boost::shared_ptr; // prevent conflicting with std::tr1.
using boost::shared_polymorphic_cast;
using boost::unordered_map;

BEGIN_NS_SASL_SEMANTIC();

tid_t type_entry_id_of_node( shared_ptr<node> const& nd ){
	if( !nd ) { return -1; }
	shared_ptr<type_info_si> tinfo = extract_semantic_info<type_info_si>( nd );
	if( !tinfo ) { return -1; }
	return tinfo->entry_id();
}

tid_t type_entry_id_of_symbol( shared_ptr<symbol> const& sym ){
	if( !sym ) { return -1; }
	return type_entry_id_of_node( sym->node() );
}

unordered_map<builtin_types, std::string> bt_to_name;

// some utility functions
std::string builtin_type_name( builtin_types btc ){
	unordered_map<builtin_types, std::string>::iterator it = bt_to_name.find(btc);
	if( it != bt_to_name.end() )
	{
		return it->second;
	}

	std::string ret;
	if( is_vector(btc) )
	{
		ret = 
			( boost::format("%1%_%2%") 
			% builtin_type_name( scalar_of(btc) )
			% vector_size( btc )
			).str();

	}
	else if( is_matrix(btc) )
	{
		ret =
			( boost::format("%1%_%2%x%3%") 
			% builtin_type_name( scalar_of(btc) )
			% vector_size( btc )
			% vector_count( btc )
			).str();

	}
	else
	{
		ret = ( boost::format("0%1%") % btc.name() ).str();
	}

	bt_to_name.insert( make_pair(btc, ret) );
	return ret;
}

//	description:
//		remove qualifier from type.
//	return:
//		means does the peeling was executed actually.
//		If the src is unqualified type, it returns 'false',
//		naked was assigned from src, and qual return a null ptr.
bool peel_qualifier(tynode_ptr const& src, tynode_ptr& naked, pety_item_t::id_ptr_t& qual)
{
	if( src->is_uniform() )
	{
		naked = duplicate(src)->as_handle<tynode>();
		naked->qual = type_qualifiers::none;
		qual = &pety_item_t::u_qual;
		return true;
	}

	if( src->is_array() )
	{
		array_type_ptr derived_naked = duplicate(src)->as_handle<array_type>();
		derived_naked->array_lens.pop_back();
		naked = derived_naked->array_lens.empty() ? derived_naked->elem_type : derived_naked;
		qual = &pety_item_t::a_qual;
		return true;
	}

	naked = src;
	qual = NULL;
	return false;
}

shared_ptr<tynode> duplicate_tynode( shared_ptr<tynode> const& typespec ){
	if( typespec->is_struct() ){
		// NOTE:
		//	Clear declarations of duplicated since they must be filled by struct visitor.
		shared_ptr<struct_type> ret_struct = duplicate(typespec)->as_handle<struct_type>();
		ret_struct->decls.clear();
		return ret_struct;
	} else {
		return duplicate(typespec)->as_handle<tynode>();
	}
}

std::string name_of_unqualified_type( shared_ptr<tynode> const& typespec ){
	// Only build in, struct and function are potential unqualified type.
	// Array type is qualified type.

	node_ids actual_node_type = typespec->node_class();

	if( actual_node_type == node_ids::alias_type ){
		return typespec->as_handle<alias_type>()->alias->str;
	} else if( actual_node_type == node_ids::builtin_type ){
		return builtin_type_name( typespec->tycode );
	} else if ( actual_node_type == node_ids::function_type ){
		return mangle( typespec->as_handle<function_type>() );
	} else if ( actual_node_type == node_ids::struct_type ){
		return typespec->as_handle<struct_type>()->name->str;
	}

	assert( !"Type type code is unrecognized!" );
	return std::string("!undefined!");
}

// type_entry

pety_item_t::pety_item_t()
	: u_qual(-1), a_qual(-1)
{
}

// type_manager

shared_ptr<pety_t> pety_t::handle() const{
	return self_handle.lock();
}

tid_t pety_t::get( tynode_ptr const& node, symbol* parent ){
	/////////////////////////////////////////////////////
	// if node has id yet, return it.
	tid_t ret = type_entry_id_of_node( node );
	if( ret != -1 ){ return ret; }

	// otherwise process the node for getting right id;
	pety_item_t::id_ptr_t qual;
	boost::shared_ptr< tynode > inner_type;

	if( peel_qualifier( node, inner_type, qual ) ){
		tid_t decoratee_id = get(inner_type, parent);
		if( decoratee_id == -1 ) { return -1; }
		if( entries[decoratee_id].*qual >= 0 ){
			// The qualified node is in entries yet.
			return entries[decoratee_id].*qual;
		} else {
			// else allocate an new node.
			return allocate_and_assign_id( node );
		}
	} else {
		// Here type specifier is a unqualified type.
		// Look up the name of type in symbol.
		// If it did not exist, throw an error or add it into symbol(as an swallow copy).
		std::string name = name_of_unqualified_type( node );
		shared_ptr<symbol> sym = parent->find( name );
		if( sym ){
			return type_entry_id_of_symbol( sym );
		} else {
			if( node->node_class() == node_ids::alias_type ){
				return -1;
			}
			tid_t entry_id = allocate_and_assign_id( node );
			parent->add_child( name, entries[entry_id].stored );
			return entry_id;
		}
	}
}

shared_ptr< tynode > pety_t::get( tid_t id ){
	if( id < 0 ){
		return shared_ptr<tynode>();
	}
	return entries[id].stored;
}

// Get type id by an builtin type code
tid_t pety_t::get( const builtin_types& btc ){
	// If it existed in symbol, return it.
	// Otherwise create a new type and push into type manager.
	tid_t ret_id = type_entry_id_of_symbol( root_symbol_->find( builtin_type_name( btc ) ) );
	if ( ret_id == -1 ){
		shared_ptr< builtin_type > bt = create_node<builtin_type>( token_t::null(), token_t::null() );
		bt->tycode = btc;
		return get( bt, root_symbol_ );
	} else {
		return ret_id;
	}
}

boost::shared_ptr< pety_t > pety_t::create(){
	boost::shared_ptr<pety_t> ret = make_shared<pety_t>();
	ret->self_handle = ret;
	return ret;
}

void pety_t::root_symbol( symbol* sym )
{
	root_symbol_ = sym;
}

tid_t pety_t::get_array( tid_t elem_type, size_t dimension )
{
	tid_t ret_tid = elem_type;
	for(size_t i = 1; i < dimension; ++i)
	{
		if( ret_tid == -1 ) break;
		ret_tid = entries[ret_tid].a_qual;
	}
	return ret_tid;
}

void assign_entry_id( shared_ptr<tynode> const& node, pety_t* typemgr, tid_t id ){
	get_or_create_semantic_info<type_si>( node, typemgr )->entry_id( id );
}

tid_t semantic::pety_t::allocate_and_assign_id( shared_ptr<tynode> const& node  ){
	// Get a duplication from node.
	// It assures that the node stored in pool is always available.
	shared_ptr<tynode> dup_node = duplicate_tynode( node );

	// add to pool and allocate an id
	pety_item_t ret_entry;
	ret_entry.stored = dup_node;
	entries.push_back( ret_entry );
	tid_t ret_id = (tid_t)( entries.size() - 1 );

	// assign id to source node and duplicated node.
	assign_entry_id(node, this, ret_id);
	assign_entry_id(dup_node, this, ret_id);
	return ret_id;
}
END_NS_SASL_SEMANTIC();
