
#include "./builtin_types.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


 
struct enum_hasher: public std::unary_function< builtin_types, std::size_t> {
	std::size_t operator()( builtin_types const& val) const{
		return hash_value(val.val_);
	}
};
		
struct dict_wrapper_builtin_types {
private:
	boost::unordered_map< builtin_types, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, builtin_types > name_to_enum;

	dict_wrapper_builtin_types(){}
	
public:
	static dict_wrapper_builtin_types& instance();
	
	void insert( builtin_types const& val, const std::string& name ){
		enum_to_name.insert( std::make_pair( val, name ) );
		name_to_enum.insert( std::make_pair( name, val ) );
	}
	
	std::string to_name( builtin_types const& val ){
		boost::unordered_map< builtin_types, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	builtin_types from_name( const std::string& name){
		boost::unordered_map< std::string, builtin_types >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

dict_wrapper_builtin_types& dict_wrapper_builtin_types::instance(){
	static dict_wrapper_builtin_types inst;
	return inst;
}

std::string builtin_types::to_name( const builtin_types& enum_val){
	return dict_wrapper_builtin_types::instance().to_name(enum_val);
}

builtin_types builtin_types::from_name( const std::string& name){
	return dict_wrapper_builtin_types::instance().from_name(name);
}

std::string builtin_types::name() const{
	return to_name( * this );
}

void builtin_types::force_initialize(){

	static bool is_initialized = false;
	if ( is_initialized ) return;
	is_initialized = true;
	new ( const_cast<builtin_types*>(&_unsigned) ) builtin_types ( UINT32_C( 34603008 ), "_unsigned" );
	new ( const_cast<builtin_types*>(&_sint32) ) builtin_types ( UINT32_C( 35848192 ), "_sint32" );
	new ( const_cast<builtin_types*>(&_c_int) ) builtin_types ( UINT32_C( 35979264 ), "_c_int" );
	new ( const_cast<builtin_types*>(&_sint16) ) builtin_types ( UINT32_C( 35782656 ), "_sint16" );
	new ( const_cast<builtin_types*>(&_generic_type_field_shift) ) builtin_types ( UINT32_C( 24 ), "_generic_type_field_shift" );
	new ( const_cast<builtin_types*>(&_scalar_type_mask) ) builtin_types ( UINT32_C( 268369920 ), "_scalar_type_mask" );
	new ( const_cast<builtin_types*>(&_sign_mask) ) builtin_types ( UINT32_C( 267386880 ), "_sign_mask" );
	new ( const_cast<builtin_types*>(&_dim1_mask) ) builtin_types ( UINT32_C( 255 ), "_dim1_mask" );
	new ( const_cast<builtin_types*>(&_boolean) ) builtin_types ( UINT32_C( 50331648 ), "_boolean" );
	new ( const_cast<builtin_types*>(&_generic_type_mask) ) builtin_types ( UINT32_C( 251658240 ), "_generic_type_mask" );
	new ( const_cast<builtin_types*>(&_sint8) ) builtin_types ( UINT32_C( 35717120 ), "_sint8" );
	new ( const_cast<builtin_types*>(&_scalar) ) builtin_types ( UINT32_C( 0 ), "_scalar" );
	new ( const_cast<builtin_types*>(&_sign_field_shift) ) builtin_types ( UINT32_C( 20 ), "_sign_field_shift" );
	new ( const_cast<builtin_types*>(&_sampler) ) builtin_types ( UINT32_C( 83886080 ), "_sampler" );
	new ( const_cast<builtin_types*>(&_float) ) builtin_types ( UINT32_C( 16842752 ), "_float" );
	new ( const_cast<builtin_types*>(&_dim0_field_shift) ) builtin_types ( UINT32_C( 8 ), "_dim0_field_shift" );
	new ( const_cast<builtin_types*>(&_void) ) builtin_types ( UINT32_C( 67108864 ), "_void" );
	new ( const_cast<builtin_types*>(&_uint16) ) builtin_types ( UINT32_C( 34734080 ), "_uint16" );
	new ( const_cast<builtin_types*>(&_dimension_mask) ) builtin_types ( UINT32_C( 4026531840 ), "_dimension_mask" );
	new ( const_cast<builtin_types*>(&_dim1_field_shift) ) builtin_types ( UINT32_C( 0 ), "_dim1_field_shift" );
	new ( const_cast<builtin_types*>(&_dimension_field_shift) ) builtin_types ( UINT32_C( 28 ), "_dimension_field_shift" );
	new ( const_cast<builtin_types*>(&_double) ) builtin_types ( UINT32_C( 16908288 ), "_double" );
	new ( const_cast<builtin_types*>(&_matrix) ) builtin_types ( UINT32_C( 536870912 ), "_matrix" );
	new ( const_cast<builtin_types*>(&_sint64) ) builtin_types ( UINT32_C( 35913728 ), "_sint64" );
	new ( const_cast<builtin_types*>(&_real) ) builtin_types ( UINT32_C( 16777216 ), "_real" );
	new ( const_cast<builtin_types*>(&_scalar_field_shift) ) builtin_types ( UINT32_C( 16 ), "_scalar_field_shift" );
	new ( const_cast<builtin_types*>(&_uint8) ) builtin_types ( UINT32_C( 34668544 ), "_uint8" );
	new ( const_cast<builtin_types*>(&_signed) ) builtin_types ( UINT32_C( 35651584 ), "_signed" );
	new ( const_cast<builtin_types*>(&_vector) ) builtin_types ( UINT32_C( 268435456 ), "_vector" );
	new ( const_cast<builtin_types*>(&none) ) builtin_types ( UINT32_C( 0 ), "none" );
	new ( const_cast<builtin_types*>(&_uint32) ) builtin_types ( UINT32_C( 34799616 ), "_uint32" );
	new ( const_cast<builtin_types*>(&_precision_field_shift) ) builtin_types ( UINT32_C( 16 ), "_precision_field_shift" );
	new ( const_cast<builtin_types*>(&_uint64) ) builtin_types ( UINT32_C( 34865152 ), "_uint64" );
	new ( const_cast<builtin_types*>(&_dim0_mask) ) builtin_types ( UINT32_C( 65280 ), "_dim0_mask" );
	new ( const_cast<builtin_types*>(&_integer) ) builtin_types ( UINT32_C( 33554432 ), "_integer" );

}


		
builtin_types::builtin_types( const storage_type& val, const std::string& name ): builtin_types::base_type(val){
	builtin_types tmp(val);
	dict_wrapper_builtin_types::instance().insert( tmp, name );
}

const builtin_types builtin_types::_unsigned ( UINT32_C( 34603008 ), "_unsigned" );
const builtin_types builtin_types::_sint32 ( UINT32_C( 35848192 ), "_sint32" );
const builtin_types builtin_types::_c_int ( UINT32_C( 35979264 ), "_c_int" );
const builtin_types builtin_types::_sint16 ( UINT32_C( 35782656 ), "_sint16" );
const builtin_types builtin_types::_generic_type_field_shift ( UINT32_C( 24 ), "_generic_type_field_shift" );
const builtin_types builtin_types::_scalar_type_mask ( UINT32_C( 268369920 ), "_scalar_type_mask" );
const builtin_types builtin_types::_sign_mask ( UINT32_C( 267386880 ), "_sign_mask" );
const builtin_types builtin_types::_dim1_mask ( UINT32_C( 255 ), "_dim1_mask" );
const builtin_types builtin_types::_boolean ( UINT32_C( 50331648 ), "_boolean" );
const builtin_types builtin_types::_generic_type_mask ( UINT32_C( 251658240 ), "_generic_type_mask" );
const builtin_types builtin_types::_sint8 ( UINT32_C( 35717120 ), "_sint8" );
const builtin_types builtin_types::_scalar ( UINT32_C( 0 ), "_scalar" );
const builtin_types builtin_types::_sign_field_shift ( UINT32_C( 20 ), "_sign_field_shift" );
const builtin_types builtin_types::_sampler ( UINT32_C( 83886080 ), "_sampler" );
const builtin_types builtin_types::_float ( UINT32_C( 16842752 ), "_float" );
const builtin_types builtin_types::_dim0_field_shift ( UINT32_C( 8 ), "_dim0_field_shift" );
const builtin_types builtin_types::_void ( UINT32_C( 67108864 ), "_void" );
const builtin_types builtin_types::_uint16 ( UINT32_C( 34734080 ), "_uint16" );
const builtin_types builtin_types::_dimension_mask ( UINT32_C( 4026531840 ), "_dimension_mask" );
const builtin_types builtin_types::_dim1_field_shift ( UINT32_C( 0 ), "_dim1_field_shift" );
const builtin_types builtin_types::_dimension_field_shift ( UINT32_C( 28 ), "_dimension_field_shift" );
const builtin_types builtin_types::_double ( UINT32_C( 16908288 ), "_double" );
const builtin_types builtin_types::_matrix ( UINT32_C( 536870912 ), "_matrix" );
const builtin_types builtin_types::_sint64 ( UINT32_C( 35913728 ), "_sint64" );
const builtin_types builtin_types::_real ( UINT32_C( 16777216 ), "_real" );
const builtin_types builtin_types::_scalar_field_shift ( UINT32_C( 16 ), "_scalar_field_shift" );
const builtin_types builtin_types::_uint8 ( UINT32_C( 34668544 ), "_uint8" );
const builtin_types builtin_types::_signed ( UINT32_C( 35651584 ), "_signed" );
const builtin_types builtin_types::_vector ( UINT32_C( 268435456 ), "_vector" );
const builtin_types builtin_types::none ( UINT32_C( 0 ), "none" );
const builtin_types builtin_types::_uint32 ( UINT32_C( 34799616 ), "_uint32" );
const builtin_types builtin_types::_precision_field_shift ( UINT32_C( 16 ), "_precision_field_shift" );
const builtin_types builtin_types::_uint64 ( UINT32_C( 34865152 ), "_uint64" );
const builtin_types builtin_types::_dim0_mask ( UINT32_C( 65280 ), "_dim0_mask" );
const builtin_types builtin_types::_integer ( UINT32_C( 33554432 ), "_integer" );


