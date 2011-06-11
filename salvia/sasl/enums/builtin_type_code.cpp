
#include "./builtin_type_code.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


 
struct enum_hasher: public std::unary_function< builtin_type_code, std::size_t> {
	std::size_t operator()( builtin_type_code const& val) const{
		return hash_value(val.val_);
	}
};
		
struct dict_wrapper_builtin_type_code {
private:
	boost::unordered_map< builtin_type_code, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, builtin_type_code > name_to_enum;

	dict_wrapper_builtin_type_code(){}
	
public:
	static dict_wrapper_builtin_type_code& instance();
	
	void insert( builtin_type_code const& val, const std::string& name ){
		enum_to_name.insert( std::make_pair( val, name ) );
		name_to_enum.insert( std::make_pair( name, val ) );
	}
	
	std::string to_name( builtin_type_code const& val ){
		boost::unordered_map< builtin_type_code, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	builtin_type_code from_name( const std::string& name){
		boost::unordered_map< std::string, builtin_type_code >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

dict_wrapper_builtin_type_code& dict_wrapper_builtin_type_code::instance(){
	static dict_wrapper_builtin_type_code inst;
	return inst;
}

std::string builtin_type_code::to_name( const builtin_type_code& enum_val){
	return dict_wrapper_builtin_type_code::instance().to_name(enum_val);
}

builtin_type_code builtin_type_code::from_name( const std::string& name){
	return dict_wrapper_builtin_type_code::instance().from_name(name);
}

std::string builtin_type_code::name() const{
	return to_name( * this );
}

void builtin_type_code::force_initialize(){

	static bool is_initialized = false;
	if ( is_initialized ) return;
	is_initialized = true;
	new ( const_cast<builtin_type_code*>(&_unsigned) ) builtin_type_code ( UINT32_C( 34603008 ), "_unsigned" );
	new ( const_cast<builtin_type_code*>(&_sint32) ) builtin_type_code ( UINT32_C( 35848192 ), "_sint32" );
	new ( const_cast<builtin_type_code*>(&_c_int) ) builtin_type_code ( UINT32_C( 35979264 ), "_c_int" );
	new ( const_cast<builtin_type_code*>(&_sint16) ) builtin_type_code ( UINT32_C( 35782656 ), "_sint16" );
	new ( const_cast<builtin_type_code*>(&_generic_type_field_shift) ) builtin_type_code ( UINT32_C( 24 ), "_generic_type_field_shift" );
	new ( const_cast<builtin_type_code*>(&_scalar_type_mask) ) builtin_type_code ( UINT32_C( 268369920 ), "_scalar_type_mask" );
	new ( const_cast<builtin_type_code*>(&_sign_mask) ) builtin_type_code ( UINT32_C( 267386880 ), "_sign_mask" );
	new ( const_cast<builtin_type_code*>(&_dim1_mask) ) builtin_type_code ( UINT32_C( 255 ), "_dim1_mask" );
	new ( const_cast<builtin_type_code*>(&_boolean) ) builtin_type_code ( UINT32_C( 50331648 ), "_boolean" );
	new ( const_cast<builtin_type_code*>(&_generic_type_mask) ) builtin_type_code ( UINT32_C( 251658240 ), "_generic_type_mask" );
	new ( const_cast<builtin_type_code*>(&_sint8) ) builtin_type_code ( UINT32_C( 35717120 ), "_sint8" );
	new ( const_cast<builtin_type_code*>(&_scalar) ) builtin_type_code ( UINT32_C( 0 ), "_scalar" );
	new ( const_cast<builtin_type_code*>(&_sign_field_shift) ) builtin_type_code ( UINT32_C( 20 ), "_sign_field_shift" );
	new ( const_cast<builtin_type_code*>(&_float) ) builtin_type_code ( UINT32_C( 16842752 ), "_float" );
	new ( const_cast<builtin_type_code*>(&_dim0_field_shift) ) builtin_type_code ( UINT32_C( 8 ), "_dim0_field_shift" );
	new ( const_cast<builtin_type_code*>(&_void) ) builtin_type_code ( UINT32_C( 67108864 ), "_void" );
	new ( const_cast<builtin_type_code*>(&_uint16) ) builtin_type_code ( UINT32_C( 34734080 ), "_uint16" );
	new ( const_cast<builtin_type_code*>(&_dimension_mask) ) builtin_type_code ( UINT32_C( 4026531840 ), "_dimension_mask" );
	new ( const_cast<builtin_type_code*>(&_dim1_field_shift) ) builtin_type_code ( UINT32_C( 0 ), "_dim1_field_shift" );
	new ( const_cast<builtin_type_code*>(&_dimension_field_shift) ) builtin_type_code ( UINT32_C( 28 ), "_dimension_field_shift" );
	new ( const_cast<builtin_type_code*>(&_double) ) builtin_type_code ( UINT32_C( 16908288 ), "_double" );
	new ( const_cast<builtin_type_code*>(&_matrix) ) builtin_type_code ( UINT32_C( 536870912 ), "_matrix" );
	new ( const_cast<builtin_type_code*>(&_sint64) ) builtin_type_code ( UINT32_C( 35913728 ), "_sint64" );
	new ( const_cast<builtin_type_code*>(&_real) ) builtin_type_code ( UINT32_C( 16777216 ), "_real" );
	new ( const_cast<builtin_type_code*>(&_scalar_field_shift) ) builtin_type_code ( UINT32_C( 16 ), "_scalar_field_shift" );
	new ( const_cast<builtin_type_code*>(&_uint8) ) builtin_type_code ( UINT32_C( 34668544 ), "_uint8" );
	new ( const_cast<builtin_type_code*>(&_signed) ) builtin_type_code ( UINT32_C( 35651584 ), "_signed" );
	new ( const_cast<builtin_type_code*>(&_vector) ) builtin_type_code ( UINT32_C( 268435456 ), "_vector" );
	new ( const_cast<builtin_type_code*>(&none) ) builtin_type_code ( UINT32_C( 0 ), "none" );
	new ( const_cast<builtin_type_code*>(&_uint32) ) builtin_type_code ( UINT32_C( 34799616 ), "_uint32" );
	new ( const_cast<builtin_type_code*>(&_precision_field_shift) ) builtin_type_code ( UINT32_C( 16 ), "_precision_field_shift" );
	new ( const_cast<builtin_type_code*>(&_uint64) ) builtin_type_code ( UINT32_C( 34865152 ), "_uint64" );
	new ( const_cast<builtin_type_code*>(&_dim0_mask) ) builtin_type_code ( UINT32_C( 65280 ), "_dim0_mask" );
	new ( const_cast<builtin_type_code*>(&_integer) ) builtin_type_code ( UINT32_C( 33554432 ), "_integer" );

}


		
builtin_type_code::builtin_type_code( const storage_type& val, const std::string& name ): builtin_type_code::base_type(val){
	builtin_type_code tmp(val);
	dict_wrapper_builtin_type_code::instance().insert( tmp, name );
}

const builtin_type_code builtin_type_code::_unsigned ( UINT32_C( 34603008 ), "_unsigned" );
const builtin_type_code builtin_type_code::_sint32 ( UINT32_C( 35848192 ), "_sint32" );
const builtin_type_code builtin_type_code::_c_int ( UINT32_C( 35979264 ), "_c_int" );
const builtin_type_code builtin_type_code::_sint16 ( UINT32_C( 35782656 ), "_sint16" );
const builtin_type_code builtin_type_code::_generic_type_field_shift ( UINT32_C( 24 ), "_generic_type_field_shift" );
const builtin_type_code builtin_type_code::_scalar_type_mask ( UINT32_C( 268369920 ), "_scalar_type_mask" );
const builtin_type_code builtin_type_code::_sign_mask ( UINT32_C( 267386880 ), "_sign_mask" );
const builtin_type_code builtin_type_code::_dim1_mask ( UINT32_C( 255 ), "_dim1_mask" );
const builtin_type_code builtin_type_code::_boolean ( UINT32_C( 50331648 ), "_boolean" );
const builtin_type_code builtin_type_code::_generic_type_mask ( UINT32_C( 251658240 ), "_generic_type_mask" );
const builtin_type_code builtin_type_code::_sint8 ( UINT32_C( 35717120 ), "_sint8" );
const builtin_type_code builtin_type_code::_scalar ( UINT32_C( 0 ), "_scalar" );
const builtin_type_code builtin_type_code::_sign_field_shift ( UINT32_C( 20 ), "_sign_field_shift" );
const builtin_type_code builtin_type_code::_float ( UINT32_C( 16842752 ), "_float" );
const builtin_type_code builtin_type_code::_dim0_field_shift ( UINT32_C( 8 ), "_dim0_field_shift" );
const builtin_type_code builtin_type_code::_void ( UINT32_C( 67108864 ), "_void" );
const builtin_type_code builtin_type_code::_uint16 ( UINT32_C( 34734080 ), "_uint16" );
const builtin_type_code builtin_type_code::_dimension_mask ( UINT32_C( 4026531840 ), "_dimension_mask" );
const builtin_type_code builtin_type_code::_dim1_field_shift ( UINT32_C( 0 ), "_dim1_field_shift" );
const builtin_type_code builtin_type_code::_dimension_field_shift ( UINT32_C( 28 ), "_dimension_field_shift" );
const builtin_type_code builtin_type_code::_double ( UINT32_C( 16908288 ), "_double" );
const builtin_type_code builtin_type_code::_matrix ( UINT32_C( 536870912 ), "_matrix" );
const builtin_type_code builtin_type_code::_sint64 ( UINT32_C( 35913728 ), "_sint64" );
const builtin_type_code builtin_type_code::_real ( UINT32_C( 16777216 ), "_real" );
const builtin_type_code builtin_type_code::_scalar_field_shift ( UINT32_C( 16 ), "_scalar_field_shift" );
const builtin_type_code builtin_type_code::_uint8 ( UINT32_C( 34668544 ), "_uint8" );
const builtin_type_code builtin_type_code::_signed ( UINT32_C( 35651584 ), "_signed" );
const builtin_type_code builtin_type_code::_vector ( UINT32_C( 268435456 ), "_vector" );
const builtin_type_code builtin_type_code::none ( UINT32_C( 0 ), "none" );
const builtin_type_code builtin_type_code::_uint32 ( UINT32_C( 34799616 ), "_uint32" );
const builtin_type_code builtin_type_code::_precision_field_shift ( UINT32_C( 16 ), "_precision_field_shift" );
const builtin_type_code builtin_type_code::_uint64 ( UINT32_C( 34865152 ), "_uint64" );
const builtin_type_code builtin_type_code::_dim0_mask ( UINT32_C( 65280 ), "_dim0_mask" );
const builtin_type_code builtin_type_code::_integer ( UINT32_C( 33554432 ), "_integer" );


