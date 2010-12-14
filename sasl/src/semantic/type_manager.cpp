#include <sasl/include/semantic/type_manager.h>

#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/node.h>
#include <sasl/include/syntax_tree/utility.h>
#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/enums/buildin_type_code.h>
#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

using namespace ::sasl::syntax_tree;

using ::boost::make_shared;
using ::boost::shared_ptr; // prevent conflicting with std::tr1.
using ::boost::shared_polymorphic_cast;

BEGIN_NS_SASL_SEMANTIC();

type_entry::id_t type_entry_id_of_node( shared_ptr<node> nd ){
	if( !nd ) { return -1; }
	shared_ptr<type_info_si> tinfo = extract_semantic_info<type_info_si>( nd );
	if( !tinfo ) { return -1; }
	return tinfo->entry_id();
}

type_entry::id_t type_entry_id_of_symbol( shared_ptr<symbol> sym ){
	if( !sym ) { return -1; }
	return type_entry_id_of_node( sym->node() );
}

// some utility functions
std::string buildin_type_name( buildin_type_code btc ){
	if( sasl_ehelper::is_vector(btc) ) {
		return 
			( boost::format("%1%_%2%") 
			% buildin_type_name( sasl_ehelper::scalar_of(btc) )
			% sasl_ehelper::len_0( btc )
			).str();
	}

	if( sasl_ehelper::is_matrix(btc) ) {
		return 
			( boost::format("%1%_%2%x%3%") 
			% buildin_type_name( sasl_ehelper::scalar_of(btc) )
			% sasl_ehelper::len_0( btc )
			% sasl_ehelper::len_1( btc )
			).str();
	}

	return ( boost::format("0%1%") % btc.name() ).str();
}

//	description:
//		remove qualifier from type.
//	return:
//		means does the peeling was executed actually.
//		If the src is unqualfied type, it returns 'false',
//		naked was assgined from src, and qual return a null ptr.
bool peel_qualifier(
	shared_ptr<type_specifier> src,
	shared_ptr<type_specifier>& naked,
	type_entry::id_ptr_t& qual
	)
{
	syntax_node_types tnode = src->node_class();
	if ( tnode == syntax_node_types::buildin_type
		|| tnode == syntax_node_types::struct_type )
	{
		if( src->is_uniform() ){
			naked = shared_polymorphic_cast<type_specifier>( duplicate( src ) );
			naked->qual = type_qualifiers::none;
			qual = &type_entry::u_qual;
			return true;
		}
	}

	naked = src;
	qual = NULL;
	return false;
}

shared_ptr<type_specifier> duplicate_type_specifier( shared_ptr<type_specifier> typespec ){
	return duplicate(typespec)->typed_handle<type_specifier>();
}

std::string name_of_unqualified_type( shared_ptr<type_specifier> typespec ){
	// Only build in, struct and function are potential unqualified type.
	// Array type is qualified type.

	syntax_node_types actual_node_type = typespec->node_class();

	if( actual_node_type == syntax_node_types::alias_type ){
		return typespec->typed_handle<alias_type>()->alias->str;
	} else if( actual_node_type == syntax_node_types::buildin_type ){
		return buildin_type_name( typespec->value_typecode );
	} else if ( actual_node_type == syntax_node_types::function_type ){
		return mangle( typespec->typed_handle<function_type>() );
	} else if ( actual_node_type == syntax_node_types::struct_type ){
		return typespec->typed_handle<struct_type>()->name->str;
	}

	EFLIB_ASSERT_AND_IF( false, "Type type code is unrecgonized!" ){
		return std::string("!undefined!");
	}
}

// type_entry

type_entry::type_entry()
	: u_qual(-1)
{
}

// type_manager

shared_ptr<type_manager> type_manager::handle() const{
	return self_handle.lock();
}

type_entry::id_t type_manager::get( shared_ptr<type_specifier> node, shared_ptr<symbol> parent ){
	/////////////////////////////////////////////////////
	// if node has id yet, return it.
	type_entry::id_t ret = type_entry_id_of_node( node );
	if( ret != -1 ){ return ret; }

	// otherwise process the node for getting right id;
	type_entry::id_ptr_t qual;
	boost::shared_ptr< type_specifier > inner_type;

	if( peel_qualifier( node, inner_type, qual ) ){
		type_entry::id_t decoratee_id = get( inner_type, parent );
		if( decoratee_id == -1 ) { return -1; }
		if( entries[decoratee_id].*qual >= 0 ){
			// The qualfied node is in entries yet.
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
			if( node->node_class() == syntax_node_types::alias_type ){
				// TODO:
				//	If type specifier is an alias specifier and the name didn't exist in symbol,
				//	it must be error.
				EFLIB_ASSERT_UNIMPLEMENTED();
				return -1;
			}
			type_entry::id_t entry_id = allocate_and_assign_id( node );
			parent->add_child( name, entries[entry_id].stored );
			return entry_id;
		}
	}


	//////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////
	// store type informations and calculate id.
//	syntax_node_types actual_node_type = node->node_class();
//
//	type_entry::id_t decoratee_id(-1);
//	type_entry::id_t decorated_id(-1);
//
//	// get the type symbol name.
//	string type_name = unqualified_type_symbol_name( node );
//
//	// First, try to look up type from pool.
//	// If it is a type definition or primitive type and didn't exist in type manager,
//	// add it into list.
//	decoratee_id = parent->find(type_name);
//
//	if ( actual_node_type == syntax_node_types::alias_type && decoratee_id == -1 ){
//		// Error: alias could not use undefined/undeclared type.
//		return id;
//	}
//
//	shared_ptr<type_specifier> dup_node;
//	if ( decoratee_id == -1 ){
//		// Type is not existed in pool, add a shallow copy into.
//		if ( actual_node_type == syntax_node_types::buildin_type ||
//			actual_node_type == syntax_node_types::struct_type )
//		{
//
//			dup_node = duplicate( node );
//			dup_node->type_qual = type_qualifiers::none;
//		}
//		EFLIB_ASSERT_AND_IF( dup_node, "Node type duplicated failure." ){
//			return -1;
//		}
//
//		entries.push_back( type_entry() );
//		decoratee_id = entries.size() - 1;
//		entries.back()->stored = dup_node;
//		get_or_create_semantic_info<type_info>( dup_node )->type_id = decoratee_id;
//		parent->add_child( type_name, dup_node );
//	}
//
//	// Then, add decorators according qualifiers,
//	// and attach decorated type to decoratee type.
//	shared_ptr<type_specifier> decorated_node;
//	type_entry::id_t type_entry::*qual_ptr;
//
//	if( actual_node_type == syntax_node_types::buildin_type ||
//		actual_node_type == syntax_node_types::struct_type )
//	{
//		if( node->is_const() ){
//			if( node->is_volatile() ){
//				// CV
//				qual_ptr = &(type_entry::cv_qual);
//			} else {
//				// C
//				qual_ptr = &(type_entry::c_qual);
//			}
//		} else {
//			if( node->is_volatile() ){
//				// V
//				qual_ptr = &(type_entry::v_qual);
//			} else {
//				decorated_id = decoratee_id;
//			}
//		}
//	} else {
//		// TODO: need support complex types.
//		EFLIB_ASSERT_UNIMPLEMENTED();
//	}
//
//	if( decoratee_id == decorated_id ){ return decorated_id; }
//	entries.push_back( type_entry() );
//	decorated_id = entries.size() - 1;
//	entries.back().stored = decorated_node;
//	get_or_create_semantic_info<type_info_si>( decorated_node )->entry_id( decorated_id );
//
//	// Associate the decorated type id to decoratee type id
//	entries[decoratee_id].*qual_ptr = decorated_id;
//
//	return decorated_id;
}

shared_ptr< type_specifier > type_manager::get( type_entry::id_t id ){
	if( id < 0 ){
		return shared_ptr<type_specifier>();
	}
	return entries[id].stored;
}

// Get type id by an buildin type code
type_entry::id_t type_manager::get( const buildin_type_code& btc ){
	// If it existed in symbol, return it.
	// Otherwise create a new type and push into type manager.
	type_entry::id_t ret_id = type_entry_id_of_symbol( rootsym.lock()->find( buildin_type_name( btc ) ) );
	if ( ret_id == -1 ){
		shared_ptr< buildin_type > bt = create_node<buildin_type>( token_attr::null() );
		bt->value_typecode = btc;
		return get( bt, rootsym.lock() );
	} else {
		return ret_id;
	}
}

boost::shared_ptr< type_manager > type_manager::create(){
	boost::shared_ptr<type_manager> ret = make_shared<type_manager>();
	ret->self_handle = ret;
	return ret;
}

void type_manager::root_symbol( boost::shared_ptr<symbol> sym )
{
	rootsym = sym;
}

void assign_entry_id( shared_ptr<type_specifier> node, shared_ptr<type_manager> typemgr, type_entry::id_t id ){
	get_or_create_semantic_info<type_si>( node, typemgr )->entry_id( id );
}

type_entry::id_t semantic::type_manager::allocate_and_assign_id( shared_ptr<type_specifier> node  ){
	// Get a duplication from node.
	// It assures that the node storaged in pool is always avaliable.
	shared_ptr<type_specifier> dup_node = duplicate_type_specifier( node );

	// add to pool and allocate an id
	type_entry ret_entry;
	ret_entry.stored = dup_node;
	entries.push_back( ret_entry );
	type_entry::id_t ret_id = (type_entry::id_t)( entries.size() - 1 );

	// assign id to source node and duplicated node.
	assign_entry_id(node, handle(), ret_id);
	assign_entry_id(dup_node, handle(), ret_id);
	return ret_id;
}
END_NS_SASL_SEMANTIC();
