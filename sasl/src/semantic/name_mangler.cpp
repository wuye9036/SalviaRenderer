/*********************
Name Mangling Grammar:
mangled_name = 'M' basic_name '@' return_value_type parameter_type_list '@' 'Z'
basic_name = string '@'
return_value_type = value_type
parameter_type_list = ( value_type )*
value_type = qualifier_code type_code
qualifier_code = "UN" | "CN" | "NN" | "UC"
type_code = buildin_typecode | struct_class_typecode | array_type_code
buildin_typecode = dimension_code basic_type
dimension_code = scalar | vector | matrix
scalar = 'B'
vector = 'V' (1|2|3|4)
matrix = 'M' (1|2|3|4){2}
basic_type = 
  'S1' | 'U1' | 'S2' | 'U2' | 'S4' | 'U4' | 'S8' | 'U8' | 'F' | 'D' | 'V' | 'B'
struct_class_typecode = 'S' string '@@'
array_class_typecode = 'A' type_code size '@@'

*********************/

#include <sasl/include/semantic/name_mangler.h>
#include <sasl/include/semantic/symbol_infos.h>
#include <sasl/include/semantic/symbol.h>
#include <sasl/include/syntax_tree/declaration.h>
#include <sasl/include/semantic/type_checker.h>
#include <boost/assign/list_inserter.hpp>
#include <cassert>

BEGIN_NS_SASL_SEMANTIC();

using ::sasl::syntax_tree::type_specifier;
using ::sasl::syntax_tree::struct_type;
using ::sasl::syntax_tree::array_type;
using ::sasl::syntax_tree::function_type;
using ::sasl::syntax_tree::variable_declaration;
using ::sasl::syntax_tree::buildin_type;
using ::sasl::syntax_tree::qualified_type;

name_mangler::name_mangler(){
	boost::assign::insert( btc_decorators )
		( buildin_type_code::none, "V0" )
		( buildin_type_code::_boolean, "B0" )
		( buildin_type_code::_sint8, "S1" )
		( buildin_type_code::_sint16, "S2" )
		( buildin_type_code::_sint32, "S4" )
		( buildin_type_code::_sint64, "S8" )
		( buildin_type_code::_uint8, "U1" )
		( buildin_type_code::_uint16, "U2" )
		( buildin_type_code::_uint32, "U4" )
		( buildin_type_code::_uint64, "U8" )
		( buildin_type_code::_float, "F" )
		( buildin_type_code::_double, "D" )
		;

	boost::assign::insert( qual_decorators )
		( type_qualifiers::none, std::string("NN") )
		( type_qualifiers::_uniform, std::string("UN") )
		;
}

std::string name_mangler::mangle( boost::shared_ptr<function_type> mangling_function ){
	mangled_name = "M";
	mangle_basic_name( mangling_function->name->lit );
	mangled_name += '@';
	mangle_type( actual_type(mangling_function->retval_type) );
	for (size_t i_param = 0; i_param < mangling_function->params.size(); ++i_param){
		boost::shared_ptr<type_specifier> par_type
			= actual_type( mangling_function->params[i_param]->typed_handle<variable_declaration>()->type_info );
		mangle_type( par_type );
	}
	mangled_name += "@Z";
	return mangled_name;
}

void name_mangler::mangle_basic_name( const std::string& str ){
	mangled_name += str;
	mangled_name += "@";
}

void name_mangler::mangle_type( boost::shared_ptr<::sasl::syntax_tree::type_specifier> mtype ){
	if ( mtype->node_class() == syntax_node_types::qualified_type ){
		boost::shared_ptr<qualified_type> qtype = mtype->typed_handle<qualified_type>();
		if( qtype->qual.included( type_qualifiers::_uniform ) ){
			mangled_name += "UN";
		} else {
			mangled_name += "NN";
		}
		mangle_type( actual_type(qtype->inner_type) );
	} else {
		mangled_name += "NN";

		if ( mtype->node_class() == syntax_node_types::buildin_type ){
			buildin_type_code btc = mtype->typed_handle<buildin_type>()->value_typecode;
			if ( btc.included( buildin_type_code::_vector ) ) {
				assert( !"Unimplemented!" );
			} else if ( btc.included( buildin_type_code::_matrix ) ) {
				assert( !"Unimplemented!" );
			} else {
				mangled_name += "B";
			}
			mangled_name += btc_decorators[btc & buildin_type_code::_element_type_mask];
		} else if ( mtype->node_class() == syntax_node_types::struct_type ){
			boost::shared_ptr< struct_type > stype = mtype->typed_handle<struct_type>();
			mangled_name += "S";
			mangled_name += stype->name->lit;
			mangled_name += "@@";
		} else {
			assert( !"Unimplemented!" );
		}
	}
}
END_NS_SASL_SEMANTIC();