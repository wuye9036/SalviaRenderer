#include <sasl/include/semantic/type_manager.h>

#include <string>

using namespace ::std;
using namespace ::boost;
using namespace ::sasl::syntax_tree;

using ::boost::shared_ptr; // prevent conflicting with std::tr1.

BEGIN_NS_SASL_SEMANTIC();

// some utility functions
std::string buildin_type_name( buildin_type_codes btc ){
	// TODO: translate buildin type name to a symbol name
}

std::string unqualified_type_symbol_name( shared_ptr<type_specifier> node ){
	syntax_node_types actual_node_type = node->node_class();
	
	if( actual_node_type == syntax_node_types::alias ){
		type_name = node->typed_handle<alias_type>()->alias->str;
	} else if( actual_node_type == syntax_node_types::buildin_type ){
		return buildin_type_name( node->valtype_code );
	} else if ( actual_node_type == syntax_node_types::function_type ){
		return mangle(node);
	} else if ( actual_node_type == syntax_node_types::struct_type ){
		return node->typed_handle<struct_type>()->name->str;
	} else if ( actual_node_type == syntax_node_types::array_type ){
		return unqualified_type_symbol_name( node->typed_handle<array_type>()->elem_type );
	}
}

// type_entry

type_entry::type_entry()
: c_qual(-1), u_qual(-1)
{
}

// type_manager

type_entry::id_t type_manager::get( shared_ptr<type_specifier> node, shared_ptr<symbol> parent ){
	/////////////////////////////////////////////////////
	// if node has id yet, return it.
	shared_ptr<type_info> existed_info = extract_semantic_info<type_info>( node );
	if ( existed_info && existed_info->id != -1 ){
		return existed_info->id;
	}
	
	/////////////////////////////////////////////////////
	// store type informations and calculate id.
	syntax_node_types actual_node_type = node->node_class();
	
	type_entry::id_t decoratee_id(-1);
	type_entry::id_t decorated_id(-1);
	
	// get the type symbol name.
	string type_name = unqualified_type_symbol_name( node );
	
	// First, try to look up type from pool.
	// If it is a type definition or primitive type and didn't exist in type manager,
	// add it into list.
	decoratee_id = parent->find(type_name);
	
	if ( actual_node_type == syntax_node_types::alias_type && decoratee_id == -1 ){
		// Error: alias could not use undefined/undeclared type.
		return id;
	}
	
	shared_ptr<type_specifier> dup_node;
	if ( decoratee_id == -1 ){
		// Type is not existed in pool, add a shallow copy into.
		if ( actual_node_type == syntax_node_types::buildin_type ||
			actual_node_type == syntax_node_types::struct_type )
		{
			
			dup_node = duplicate( node );
			dup_node->type_qual = type_qualifiers::none;
		}
		EFLIB_ASSERT_AND_IF( dup_node, "Node type duplicated failure." ){
			return -1;
		}
		
		entries.push_back( type_entry() );
		decoratee_id = entries.size() - 1;
		entries.back()->stored = dup_node;
		get_or_create_semantic_info<type_info>( dup_node )->type_id = decoratee_id;
		parent->add_child( type_name, dup_node );
	}
	
	// Then, add decorators according qualifiers,
	// and attach decorated type to decoratee type.
	shared_ptr<type_specifier> decorated_node;
	type_entry::id_t type_entry::*qual_ptr;
	
	if( actual_node_type == syntax_node_types::buildin_type ||
		actual_node_type == syntax_node_types::struct_type )
	{
		if( node->is_const() ){
			if( node->is_volatile() ){
				// CV
				qual_ptr = &(type_entry::cv_qual);
			} else {
				// C
				qual_ptr = &(type_entry::c_qual);
			}
		} else {
			if( node->is_volatile() ){
				// V
				qual_ptr = &(type_entry::v_qual);
			} else {
				decorated_id = decoratee_id;
			}
		}
	} else {
		// TODO: need support complex types.
		EFLIB_ASSERT_UNIMPLEMENTED();
	}
	
	if( decoratee_id == decorated_id ){ return decorated_id; }
	entries.push_back( type_entry() );
	decorated_id = entries.size() - 1;
	entries.back()->stored = decorated_node;
	get_or_create_semantic_info<type_info>( decorated_node )->type_id = decorated_id;
	
	// Associate the decorated type id to decoratee type id
	entries[decoratee_id].*qual_ptr = decorated_id;
	
	return decorated_id;
}

END_NS_SASL_SEMANTIC();