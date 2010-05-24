
#include "./buildin_type_code.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const buildin_type_code buildin_type_code::_unsigned ( 131328 );
const buildin_type_code buildin_type_code::_sint32 ( 131587 );
const buildin_type_code buildin_type_code::_sint16 ( 131586 );
const buildin_type_code buildin_type_code::_sign_mask ( 65280 );
const buildin_type_code buildin_type_code::_boolean ( 262144 );
const buildin_type_code buildin_type_code::_generic_type_mask ( 16711680 );
const buildin_type_code buildin_type_code::_sint8 ( 131585 );
const buildin_type_code buildin_type_code::_scalar ( 16777216 );
const buildin_type_code buildin_type_code::_float ( 65537 );
const buildin_type_code buildin_type_code::_uint16 ( 131330 );
const buildin_type_code buildin_type_code::_uint32 ( 131331 );
const buildin_type_code buildin_type_code::_element_type_mask ( 16777215 );
const buildin_type_code buildin_type_code::_double ( 65538 );
const buildin_type_code buildin_type_code::_matrix ( 67108864 );
const buildin_type_code buildin_type_code::_sint64 ( 131588 );
const buildin_type_code buildin_type_code::_real ( 65536 );
const buildin_type_code buildin_type_code::_uint8 ( 131329 );
const buildin_type_code buildin_type_code::_signed ( 131584 );
const buildin_type_code buildin_type_code::_vector ( 33554432 );
const buildin_type_code buildin_type_code::none ( 0 );
const buildin_type_code buildin_type_code::_dimension_mask ( 4278190080 );
const buildin_type_code buildin_type_code::_uint64 ( 131332 );
const buildin_type_code buildin_type_code::_integer ( 131072 );

 
struct enum_hasher: public std::unary_function< buildin_type_code, std::size_t> {
	std::size_t operator()( buildin_type_code const& val) const{
		return hash_value(val.val_);
	}
};

struct dict_wrapper_buildin_type_code {
private:
	boost::unordered_map< buildin_type_code, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, buildin_type_code > name_to_enum;

public:
	dict_wrapper_buildin_type_code(){
		enum_to_name.insert( std::make_pair( buildin_type_code::_unsigned, "_unsigned" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_sint32, "_sint32" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_sint16, "_sint16" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_sign_mask, "_sign_mask" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_boolean, "_boolean" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_generic_type_mask, "_generic_type_mask" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_sint8, "_sint8" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_scalar, "_scalar" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_float, "_float" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_uint16, "_uint16" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_uint32, "_uint32" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_element_type_mask, "_element_type_mask" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_double, "_double" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_matrix, "_matrix" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_sint64, "_sint64" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_real, "_real" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_uint8, "_uint8" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_signed, "_signed" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_vector, "_vector" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::none, "none" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_dimension_mask, "_dimension_mask" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_uint64, "_uint64" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_integer, "_integer" ) );

		name_to_enum.insert( std::make_pair( "_unsigned", buildin_type_code::_unsigned ) );
		name_to_enum.insert( std::make_pair( "_sint32", buildin_type_code::_sint32 ) );
		name_to_enum.insert( std::make_pair( "_sint16", buildin_type_code::_sint16 ) );
		name_to_enum.insert( std::make_pair( "_sign_mask", buildin_type_code::_sign_mask ) );
		name_to_enum.insert( std::make_pair( "_boolean", buildin_type_code::_boolean ) );
		name_to_enum.insert( std::make_pair( "_generic_type_mask", buildin_type_code::_generic_type_mask ) );
		name_to_enum.insert( std::make_pair( "_sint8", buildin_type_code::_sint8 ) );
		name_to_enum.insert( std::make_pair( "_scalar", buildin_type_code::_scalar ) );
		name_to_enum.insert( std::make_pair( "_float", buildin_type_code::_float ) );
		name_to_enum.insert( std::make_pair( "_uint16", buildin_type_code::_uint16 ) );
		name_to_enum.insert( std::make_pair( "_uint32", buildin_type_code::_uint32 ) );
		name_to_enum.insert( std::make_pair( "_element_type_mask", buildin_type_code::_element_type_mask ) );
		name_to_enum.insert( std::make_pair( "_double", buildin_type_code::_double ) );
		name_to_enum.insert( std::make_pair( "_matrix", buildin_type_code::_matrix ) );
		name_to_enum.insert( std::make_pair( "_sint64", buildin_type_code::_sint64 ) );
		name_to_enum.insert( std::make_pair( "_real", buildin_type_code::_real ) );
		name_to_enum.insert( std::make_pair( "_uint8", buildin_type_code::_uint8 ) );
		name_to_enum.insert( std::make_pair( "_signed", buildin_type_code::_signed ) );
		name_to_enum.insert( std::make_pair( "_vector", buildin_type_code::_vector ) );
		name_to_enum.insert( std::make_pair( "none", buildin_type_code::none ) );
		name_to_enum.insert( std::make_pair( "_dimension_mask", buildin_type_code::_dimension_mask ) );
		name_to_enum.insert( std::make_pair( "_uint64", buildin_type_code::_uint64 ) );
		name_to_enum.insert( std::make_pair( "_integer", buildin_type_code::_integer ) );

	}

	std::string to_name( buildin_type_code const& val ){
		boost::unordered_map< buildin_type_code, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	buildin_type_code from_name( const std::string& name){
		boost::unordered_map< std::string, buildin_type_code >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

static dict_wrapper_buildin_type_code s_dict;

std::string buildin_type_code::to_name( const buildin_type_code& enum_val){
	return s_dict.to_name(enum_val);
}

buildin_type_code buildin_type_code::from_name( const std::string& name){
	return s_dict.from_name(name);
}

