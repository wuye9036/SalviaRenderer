/*********************
Name Mangling Grammar:
Look at the documentation in sasl/docs/Name Mangling Syntax.docx
*********************/

#include <sasl/include/semantic/name_mangler.h>
#include <sasl/enums/default_hasher.h>
#include <sasl/enums/enums_utility.h>
#include <sasl/enums/operators.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/syntax_tree/expression.h>
#include <sasl/include/semantic/semantics.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/semantic/type_checker.h>
#include <eflib/include/utility/polymorphic_cast.h>

#include <eflib/include/platform/disable_warnings.h>
#include <boost/assign/list_inserter.hpp>
#include <boost/thread.hpp>
#include <eflib/include/platform/enable_warnings.h>

#include <cassert>
#include <stdio.h>

using sasl::syntax_tree::array_type;
using sasl::syntax_tree::builtin_type;
using sasl::syntax_tree::expression;
using sasl::syntax_tree::function_type;
using sasl::syntax_tree::node;
using sasl::syntax_tree::struct_type;
using sasl::syntax_tree::tynode;
using sasl::syntax_tree::variable_declaration;

using eflib::polymorphic_cast;
using namespace sasl::utility;

//////////////////////////////////////////////////////////////////////////
// lookup table for translating enumerations to string.
// static boost::mutex lookup_table_mtx;

static std::string mangling_tag("M");
static boost::unordered_map< builtin_types, std::string, enum_hasher > btc_decorators;
static bool is_initialized(false);

static void initialize_lookup_table(){
	// boost::mutex::scoped_lock locker(lookup_table_mtx);

	if ( is_initialized ){ return; }

	boost::assign::insert( btc_decorators )
		( builtin_types::_void, "O" )
		( builtin_types::_boolean, "B" )
		( builtin_types::_sint8, "S1" )
		( builtin_types::_sint16, "S2" )
		( builtin_types::_sint32, "S4" )
		( builtin_types::_sint64, "S8" )
		( builtin_types::_uint8, "U1" )
		( builtin_types::_uint16, "U2" )
		( builtin_types::_uint32, "U4" )
		( builtin_types::_uint64, "U8" )
		( builtin_types::_float, "F" )
		( builtin_types::_double, "D" )
		;

	is_initialized = true;
}

//////////////////////////////////////////////////////////////////////////
// some free function for manging
static void append( std::string& str, tynode* typespec );

static void append( std::string& str, builtin_types btc, bool is_component = false ){
	if ( is_scalar( btc ) ) {
		if ( !is_component ){
			// if it is not a component of a vector or matrix,
			// add a lead char 'B' since is a BaseTypeName of a builtin scalar type.
			str.append("B");
		}
		str.append( btc_decorators[btc] );
	} else if( is_vector( btc ) ) {
		char vector_len_buf[2];
		str.append("V");
		sprintf( vector_len_buf, "%ld", vector_size( btc ) );
		str.append( vector_len_buf );
		append( str, scalar_of(btc), true );
	} else if ( is_matrix(btc) ) {
		char matrix_len_buf[2] = {0};
		str.append("M");
		sprintf( matrix_len_buf, "%ld", vector_size( btc ) );
		str.append( matrix_len_buf );
		sprintf( matrix_len_buf, "%ld", vector_count( btc ) );
		str.append( matrix_len_buf );
		append( str, scalar_of(btc), true );
	}
}

static void append( std::string& str, type_qualifiers qual ){
	if ( qual.included( type_qualifiers::_uniform ) ){
		str.append("U");
	}
	str.append("Q");
}

static void append( std::string& str, struct_type* stype ){
	str.append("S");
	str.append( stype->name->str );
}

static void append( std::string& str, array_type* atype ){
	for ( size_t i_dim = 0; i_dim < atype->array_lens.size(); ++i_dim ){
		str.append("A");
		// str.append( atype->array_lens[i_dim] );
		// if ( i_dim < atype->array_lens.size() - 1 ){
		//	str.append("Q");
		// }
	}
	append( str, atype->elem_type.get() );
}

static void append( std::string& str, tynode* typespec ){
	append(str, typespec->qual);
	// append (str, scope_qualifier(typespec) );
	if ( typespec->node_class() == node_ids::builtin_type ){
		append( str, typespec->tycode );
	} else if ( typespec->node_class() == node_ids::struct_type ) {
		append( str, polymorphic_cast<struct_type*>(typespec) );
	} else if( typespec->node_class() == node_ids::array_type ){
		append( str, polymorphic_cast<array_type*>(typespec) );
	} else if ( typespec->node_class() == node_ids::function_type ){
		// append( str, boost::shared_polymorphic_cast<function_type>(typespec) );
	}
}

BEGIN_NS_SASL_SEMANTIC();

std::string mangle( module_semantic* sem, function_type* mangling_function ){
	initialize_lookup_table();

	// start char
	std::string mangled_name;
	mangled_name.reserve(32);
	mangled_name = "M";

	// qualified name
	// append( str, scope_qualifier( mangling_function ) );
	mangled_name += mangling_function->name->str;

	// splitter
	mangled_name.append("@@");

	// parameter types
	for (size_t i_param = 0; i_param < mangling_function->params.size(); ++i_param){
		tynode* par_type = sem->get_semantic( mangling_function->params[i_param] )->ty_proto();
		append( mangled_name, par_type );
		mangled_name.append( "@@" );
	}

	// calling convention
	// append( str, callingconv( mangling_function ) );

	return mangled_name;
}

std::string operator_name( const operators& op ){
	return std::string("0") + op.name();
}
END_NS_SASL_SEMANTIC();
