
#include "./buildin_types.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const buildin_types buildin_types::sasl_struct ( 131072 );
const buildin_types buildin_types::sasl_bool ( 2 );
const buildin_types buildin_types::sasl_int64 ( 256 );
const buildin_types buildin_types::sasl_int32 ( 64 );
const buildin_types buildin_types::sasl_vector ( 8192 );
const buildin_types buildin_types::sasl_unknown ( 262144 );
const buildin_types buildin_types::sasl_uint32 ( 128 );
const buildin_types buildin_types::sasl_float ( 2048 );
const buildin_types buildin_types::sasl_array ( 65536 );
const buildin_types buildin_types::sasl_matrix ( 16384 );
const buildin_types buildin_types::sasl_uint64 ( 512 );
const buildin_types buildin_types::sasl_half ( 1024 );
const buildin_types buildin_types::sasl_int16 ( 16 );
const buildin_types buildin_types::sasl_uint16 ( 32 );
const buildin_types buildin_types::sasl_int8 ( 4 );
const buildin_types buildin_types::sasl_uint8 ( 8 );
const buildin_types buildin_types::sasl_double ( 4096 );
const buildin_types buildin_types::sasl_function ( 32768 );
const buildin_types buildin_types::sasl_void ( 1 );

 
struct enum_hasher: public std::unary_function< buildin_types, std::size_t> {
	std::size_t operator()( buildin_types const& val) const{
		return hash_value(val.val_);
	}
};

struct dict_wrapper_buildin_types {
private:
	boost::unordered_map< buildin_types, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, buildin_types > name_to_enum;

public:
	dict_wrapper_buildin_types(){
		enum_to_name.insert( std::make_pair( buildin_types::sasl_struct, "sasl_struct" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_bool, "sasl_bool" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_int64, "sasl_int64" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_int32, "sasl_int32" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_vector, "sasl_vector" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_unknown, "sasl_unknown" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_uint32, "sasl_uint32" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_float, "sasl_float" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_array, "sasl_array" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_matrix, "sasl_matrix" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_uint64, "sasl_uint64" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_half, "sasl_half" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_int16, "sasl_int16" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_uint16, "sasl_uint16" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_int8, "sasl_int8" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_uint8, "sasl_uint8" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_double, "sasl_double" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_function, "sasl_function" ) );
		enum_to_name.insert( std::make_pair( buildin_types::sasl_void, "sasl_void" ) );

		name_to_enum.insert( std::make_pair( "sasl_struct", buildin_types::sasl_struct ) );
		name_to_enum.insert( std::make_pair( "sasl_bool", buildin_types::sasl_bool ) );
		name_to_enum.insert( std::make_pair( "sasl_int64", buildin_types::sasl_int64 ) );
		name_to_enum.insert( std::make_pair( "sasl_int32", buildin_types::sasl_int32 ) );
		name_to_enum.insert( std::make_pair( "sasl_vector", buildin_types::sasl_vector ) );
		name_to_enum.insert( std::make_pair( "sasl_unknown", buildin_types::sasl_unknown ) );
		name_to_enum.insert( std::make_pair( "sasl_uint32", buildin_types::sasl_uint32 ) );
		name_to_enum.insert( std::make_pair( "sasl_float", buildin_types::sasl_float ) );
		name_to_enum.insert( std::make_pair( "sasl_array", buildin_types::sasl_array ) );
		name_to_enum.insert( std::make_pair( "sasl_matrix", buildin_types::sasl_matrix ) );
		name_to_enum.insert( std::make_pair( "sasl_uint64", buildin_types::sasl_uint64 ) );
		name_to_enum.insert( std::make_pair( "sasl_half", buildin_types::sasl_half ) );
		name_to_enum.insert( std::make_pair( "sasl_int16", buildin_types::sasl_int16 ) );
		name_to_enum.insert( std::make_pair( "sasl_uint16", buildin_types::sasl_uint16 ) );
		name_to_enum.insert( std::make_pair( "sasl_int8", buildin_types::sasl_int8 ) );
		name_to_enum.insert( std::make_pair( "sasl_uint8", buildin_types::sasl_uint8 ) );
		name_to_enum.insert( std::make_pair( "sasl_double", buildin_types::sasl_double ) );
		name_to_enum.insert( std::make_pair( "sasl_function", buildin_types::sasl_function ) );
		name_to_enum.insert( std::make_pair( "sasl_void", buildin_types::sasl_void ) );

	}

	std::string to_name( buildin_types const& val ){
		boost::unordered_map< buildin_types, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	buildin_types from_name( const std::string& name){
		boost::unordered_map< std::string, buildin_types >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

static dict_wrapper_buildin_types s_dict;

std::string buildin_types::to_name( const buildin_types& enum_val){
	return s_dict.to_name(enum_val);
}

buildin_types buildin_types::from_name( const std::string& name){
	return s_dict.from_name(name);
}

