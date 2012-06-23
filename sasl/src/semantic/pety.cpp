#include <sasl/include/semantic/pety.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/utility.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/semantics.h>
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
using eflib::polymorphic_cast;
using boost::make_shared;
using boost::shared_ptr; // prevent conflicting with std::tr1.
using boost::shared_polymorphic_cast;
using boost::unordered_map;
using std::make_pair;

BEGIN_NS_SASL_SEMANTIC();

tid_t get_node_tid( unordered_map<tynode*, tid_t> const& dict, tynode* nd )
{
	if( !nd ) { return -1; }
	unordered_map<tynode*, tid_t>::const_iterator it = dict.find(nd);
	if( it == dict.end() ) { return -1; }
	return it->second;
}

tid_t get_symbol_tid( unordered_map<tynode*, tid_t> const& dict, symbol* sym ){
	if( !sym ) { return -1; }
	return get_node_tid( dict, polymorphic_cast<tynode*>( sym->associated_node() ) );
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
bool peel_qualifier(tynode* src, tynode*& naked, pety_item_t::id_ptr_t& qual)
{
	if( src->is_uniform() )
	{
		naked = duplicate( src->as_handle() )->as_handle<tynode>().get();
		naked->qual = type_qualifiers::none;
		qual = &pety_item_t::u_qual;
		return true;
	}

	if( src->is_array() )
	{
		array_type_ptr derived_naked = duplicate( src->as_handle() )->as_handle<array_type>();
		derived_naked->array_lens.pop_back();
		naked = derived_naked->array_lens.empty() ? derived_naked->elem_type.get() : derived_naked.get();
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

std::string name_of_unqualified_type(module_semantic* sem, tynode* typespec){
	// Only build in, struct and function are potential unqualified type.
	// Array type is qualified type.

	node_ids actual_node_type = typespec->node_class();

	if( actual_node_type == node_ids::alias_type ){
		return polymorphic_cast<alias_type*>(typespec)->alias->str;
	} else if( actual_node_type == node_ids::builtin_type ){
		return builtin_type_name( typespec->tycode );
	} else if ( actual_node_type == node_ids::function_type ){
		return mangle( sem, polymorphic_cast<function_type*>(typespec) );
	} else if ( actual_node_type == node_ids::struct_type ){
		return polymorphic_cast<struct_type*>(typespec)->name->str;
	}

	assert( !"Type type code is unrecognized!" );
	return std::string("!undefined!");
}

// type_entry

pety_item_t::pety_item_t()
	: u_qual(-1), a_qual(-1)
{
}

tynode* pety_t::get_proto(tid_t id)
{
	return id < 0 ? NULL : type_items_[id].stored.get();
}

tynode* pety_t::get_proto_by_builtin(builtin_types bt)
{
	return get_proto( get(bt) );
}

// Get type id by an builtin type code
tid_t pety_t::get(builtin_types const& btc)
{
	// If it existed in symbol, return it.
	// Otherwise create a new type and push into type manager.
	tid_t ret_id = get_symbol_tid( tynode_dict_, root_symbol_->find( builtin_type_name( btc ) ) );
	if ( ret_id == -1 ){
		shared_ptr<builtin_type> bt = create_node<builtin_type>( token_t::null(), token_t::null() );
		bt->tycode = btc;
		return get(bt.get(), root_symbol_);
	} else {
		return ret_id;
	}
}

tid_t pety_t::get(tynode* v, symbol* scope)
{
	// Return id if existed.
	tid_t ret = get_node_tid(tynode_dict_, v);
	if( ret != -1 ){ return ret; }

	// otherwise process the node for getting right id;
	pety_item_t::id_ptr_t qual;
	tynode* inner_type;

	if( peel_qualifier(v, inner_type, qual) )
	{
		tid_t decoratee_id = get(inner_type, scope);
		if( decoratee_id == -1 ) { return -1; }
		if( type_items_[decoratee_id].*qual >= 0 ){
			// The qualified node is in items yet.
			return type_items_[decoratee_id].*qual;
		} else {
			// else allocate an new node.
			return allocate_and_assign_id(v);
		}
	}
	else
	{
		// Here type specifier is a unqualified type.
		// Look up the name of type in symbol.
		// If it did not exist, throw an error or add it into symbol(as an swallow copy).
		std::string name = name_of_unqualified_type(scope->owner(), v);
		symbol* sym = scope->find( name );
		if( sym )
		{
			return get_symbol_tid(tynode_dict_, sym);
		}
		else
		{
			if( v->node_class() == node_ids::alias_type )
			{
				return -1;
			}
			tid_t tid = allocate_and_assign_id(v);
			scope->add_named_child( name, type_items_[tid].stored.get() );
			return tid;
		}
	}
}

shared_ptr<pety_t> pety_t::create(module_semantic* owner)
{
	pety_t* ret = new pety_t();
	ret->owner_ = owner;
	return shared_ptr<pety_t>(ret);
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
		ret_tid = type_items_[ret_tid].a_qual;
	}
	return ret_tid;
}

void assign_entry_id( module_semantic* msem, tynode* node, tid_t id ){
	msem->get_or_create_semantic(node)->tid(id);
}

tid_t semantic::pety_t::allocate_and_assign_id(tynode* node){
	// Get a duplication from node.
	// It assures that the node stored in pool is always available.
	shared_ptr<tynode> dup_node = duplicate_tynode( node->as_handle<tynode>() );

	// add to pool and allocate an id
	pety_item_t ret_item;
	ret_item.stored = dup_node;
	
	type_items_.push_back( ret_item );
	tid_t ret_id = (tid_t)( type_items_.size() - 1 );
	tynode_dict_.insert( make_pair(ret_item.stored.get(), ret_id) );

	// assign id to source node and duplicated node.
	assign_entry_id(owner_, node, ret_id);
	assign_entry_id(owner_, dup_node.get(), ret_id);
	return ret_id;
}
END_NS_SASL_SEMANTIC();
