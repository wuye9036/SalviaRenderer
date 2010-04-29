
#include "./buildin_type_code.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const buildin_type_code buildin_type_code::_scalar ( 0 );
const buildin_type_code buildin_type_code::none ( 0 );
const buildin_type_code buildin_type_code::_unsigned ( 65536 );
const buildin_type_code buildin_type_code::float ( 5 );
const buildin_type_code buildin_type_code::double ( 6 );
const buildin_type_code buildin_type_code::_matrix ( 33554432 );
const buildin_type_code buildin_type_code::_int64 ( 4 );
const buildin_type_code buildin_type_code::_uint64 ( 4 );
const buildin_type_code buildin_type_code::_uint32 ( 3 );
const buildin_type_code buildin_type_code::_int16 ( 2 );
const buildin_type_code buildin_type_code::_uint8 ( 1 );
const buildin_type_code buildin_type_code::_signed ( 0 );
const buildin_type_code buildin_type_code::_vector ( 16777216 );
const buildin_type_code buildin_type_code::_int8 ( 1 );
const buildin_type_code buildin_type_code::_int32 ( 3 );
const buildin_type_code buildin_type_code::_uint16 ( 2 );

 
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
		enum_to_name.insert( std::make_pair( buildin_type_code::_scalar, "_scalar" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::none, "none" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_unsigned, "_unsigned" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::float, "float" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::double, "double" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_matrix, "_matrix" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_int64, "_int64" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_uint64, "_uint64" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_uint32, "_uint32" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_int16, "_int16" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_uint8, "_uint8" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_signed, "_signed" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_vector, "_vector" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_int8, "_int8" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_int32, "_int32" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_uint16, "_uint16" ) );

		name_to_enum.insert( std::make_pair( "_scalar", buildin_type_code::_scalar ) );
		name_to_enum.insert( std::make_pair( "none", buildin_type_code::none ) );
		name_to_enum.insert( std::make_pair( "_unsigned", buildin_type_code::_unsigned ) );
		name_to_enum.insert( std::make_pair( "float", buildin_type_code::float ) );
		name_to_enum.insert( std::make_pair( "double", buildin_type_code::double ) );
		name_to_enum.insert( std::make_pair( "_matrix", buildin_type_code::_matrix ) );
		name_to_enum.insert( std::make_pair( "_int64", buildin_type_code::_int64 ) );
		name_to_enum.insert( std::make_pair( "_uint64", buildin_type_code::_uint64 ) );
		name_to_enum.insert( std::make_pair( "_uint32", buildin_type_code::_uint32 ) );
		name_to_enum.insert( std::make_pair( "_int16", buildin_type_code::_int16 ) );
		name_to_enum.insert( std::make_pair( "_uint8", buildin_type_code::_uint8 ) );
		name_to_enum.insert( std::make_pair( "_signed", buildin_type_code::_signed ) );
		name_to_enum.insert( std::make_pair( "_vector", buildin_type_code::_vector ) );
		name_to_enum.insert( std::make_pair( "_int8", buildin_type_code::_int8 ) );
		name_to_enum.insert( std::make_pair( "_int32", buildin_type_code::_int32 ) );
		name_to_enum.insert( std::make_pair( "_uint16", buildin_type_code::_uint16 ) );

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

