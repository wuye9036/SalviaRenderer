
#include "./buildin_type_code.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const buildin_type_code buildin_type_code::_unsigned ( 34603008 );
const buildin_type_code buildin_type_code::_sint32 ( 35848192 );
const buildin_type_code buildin_type_code::_sint16 ( 35782656 );
const buildin_type_code buildin_type_code::_generic_type_field_shift ( 24 );
const buildin_type_code buildin_type_code::_scalar_type_mask ( 268369920 );
const buildin_type_code buildin_type_code::_sign_mask ( 15728640 );
const buildin_type_code buildin_type_code::_dim1_mask ( 255 );
const buildin_type_code buildin_type_code::_boolean ( 50331648 );
const buildin_type_code buildin_type_code::_generic_type_mask ( 251658240 );
const buildin_type_code buildin_type_code::_sint8 ( 35717120 );
const buildin_type_code buildin_type_code::_scalar ( 0 );
const buildin_type_code buildin_type_code::_sign_field_shift ( 20 );
const buildin_type_code buildin_type_code::_float ( 16842752 );
const buildin_type_code buildin_type_code::_dim0_field_shift ( 8 );
const buildin_type_code buildin_type_code::_void ( 67108864 );
const buildin_type_code buildin_type_code::_uint16 ( 34734080 );
const buildin_type_code buildin_type_code::_dimension_mask ( 4026531840 );
const buildin_type_code buildin_type_code::_dim1_field_shift ( 0 );
const buildin_type_code buildin_type_code::_dimension_field_shift ( 28 );
const buildin_type_code buildin_type_code::_double ( 16908288 );
const buildin_type_code buildin_type_code::_matrix ( 536870912 );
const buildin_type_code buildin_type_code::_sint64 ( 35913728 );
const buildin_type_code buildin_type_code::_real ( 16777216 );
const buildin_type_code buildin_type_code::_scalar_field_shift ( 16 );
const buildin_type_code buildin_type_code::_uint8 ( 34668544 );
const buildin_type_code buildin_type_code::_signed ( 35651584 );
const buildin_type_code buildin_type_code::_vector ( 268435456 );
const buildin_type_code buildin_type_code::none ( 0 );
const buildin_type_code buildin_type_code::_uint32 ( 34799616 );
const buildin_type_code buildin_type_code::_precision_field_shift ( 16 );
const buildin_type_code buildin_type_code::_uint64 ( 34865152 );
const buildin_type_code buildin_type_code::_dim0_mask ( 65280 );
const buildin_type_code buildin_type_code::_integer ( 33554432 );

 
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
		enum_to_name.insert( std::make_pair( buildin_type_code::_generic_type_field_shift, "_generic_type_field_shift" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_scalar_type_mask, "_scalar_type_mask" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_sign_mask, "_sign_mask" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_dim1_mask, "_dim1_mask" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_boolean, "_boolean" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_generic_type_mask, "_generic_type_mask" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_sint8, "_sint8" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_scalar, "_scalar" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_sign_field_shift, "_sign_field_shift" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_float, "_float" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_dim0_field_shift, "_dim0_field_shift" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_void, "_void" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_uint16, "_uint16" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_dimension_mask, "_dimension_mask" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_dim1_field_shift, "_dim1_field_shift" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_dimension_field_shift, "_dimension_field_shift" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_double, "_double" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_matrix, "_matrix" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_sint64, "_sint64" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_real, "_real" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_scalar_field_shift, "_scalar_field_shift" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_uint8, "_uint8" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_signed, "_signed" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_vector, "_vector" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::none, "none" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_uint32, "_uint32" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_precision_field_shift, "_precision_field_shift" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_uint64, "_uint64" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_dim0_mask, "_dim0_mask" ) );
		enum_to_name.insert( std::make_pair( buildin_type_code::_integer, "_integer" ) );

		name_to_enum.insert( std::make_pair( "_unsigned", buildin_type_code::_unsigned ) );
		name_to_enum.insert( std::make_pair( "_sint32", buildin_type_code::_sint32 ) );
		name_to_enum.insert( std::make_pair( "_sint16", buildin_type_code::_sint16 ) );
		name_to_enum.insert( std::make_pair( "_generic_type_field_shift", buildin_type_code::_generic_type_field_shift ) );
		name_to_enum.insert( std::make_pair( "_scalar_type_mask", buildin_type_code::_scalar_type_mask ) );
		name_to_enum.insert( std::make_pair( "_sign_mask", buildin_type_code::_sign_mask ) );
		name_to_enum.insert( std::make_pair( "_dim1_mask", buildin_type_code::_dim1_mask ) );
		name_to_enum.insert( std::make_pair( "_boolean", buildin_type_code::_boolean ) );
		name_to_enum.insert( std::make_pair( "_generic_type_mask", buildin_type_code::_generic_type_mask ) );
		name_to_enum.insert( std::make_pair( "_sint8", buildin_type_code::_sint8 ) );
		name_to_enum.insert( std::make_pair( "_scalar", buildin_type_code::_scalar ) );
		name_to_enum.insert( std::make_pair( "_sign_field_shift", buildin_type_code::_sign_field_shift ) );
		name_to_enum.insert( std::make_pair( "_float", buildin_type_code::_float ) );
		name_to_enum.insert( std::make_pair( "_dim0_field_shift", buildin_type_code::_dim0_field_shift ) );
		name_to_enum.insert( std::make_pair( "_void", buildin_type_code::_void ) );
		name_to_enum.insert( std::make_pair( "_uint16", buildin_type_code::_uint16 ) );
		name_to_enum.insert( std::make_pair( "_dimension_mask", buildin_type_code::_dimension_mask ) );
		name_to_enum.insert( std::make_pair( "_dim1_field_shift", buildin_type_code::_dim1_field_shift ) );
		name_to_enum.insert( std::make_pair( "_dimension_field_shift", buildin_type_code::_dimension_field_shift ) );
		name_to_enum.insert( std::make_pair( "_double", buildin_type_code::_double ) );
		name_to_enum.insert( std::make_pair( "_matrix", buildin_type_code::_matrix ) );
		name_to_enum.insert( std::make_pair( "_sint64", buildin_type_code::_sint64 ) );
		name_to_enum.insert( std::make_pair( "_real", buildin_type_code::_real ) );
		name_to_enum.insert( std::make_pair( "_scalar_field_shift", buildin_type_code::_scalar_field_shift ) );
		name_to_enum.insert( std::make_pair( "_uint8", buildin_type_code::_uint8 ) );
		name_to_enum.insert( std::make_pair( "_signed", buildin_type_code::_signed ) );
		name_to_enum.insert( std::make_pair( "_vector", buildin_type_code::_vector ) );
		name_to_enum.insert( std::make_pair( "none", buildin_type_code::none ) );
		name_to_enum.insert( std::make_pair( "_uint32", buildin_type_code::_uint32 ) );
		name_to_enum.insert( std::make_pair( "_precision_field_shift", buildin_type_code::_precision_field_shift ) );
		name_to_enum.insert( std::make_pair( "_uint64", buildin_type_code::_uint64 ) );
		name_to_enum.insert( std::make_pair( "_dim0_mask", buildin_type_code::_dim0_mask ) );
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

std::string buildin_type_code::name() const{
	return to_name( * this );
}



