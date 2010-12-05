
#include "./buildin_type_code.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


 
struct enum_hasher: public std::unary_function< buildin_type_code, std::size_t> {
	std::size_t operator()( buildin_type_code const& val) const{
		return hash_value(val.val_);
	}
};
		
struct dict_wrapper_buildin_type_code {
private:
	boost::unordered_map< buildin_type_code, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, buildin_type_code > name_to_enum;

	dict_wrapper_buildin_type_code(){}
	
public:
	static dict_wrapper_buildin_type_code& instance();
	
	void insert( buildin_type_code const& val, const std::string& name ){
		enum_to_name.insert( std::make_pair( val, name ) );
		name_to_enum.insert( std::make_pair( name, val ) );
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

dict_wrapper_buildin_type_code& dict_wrapper_buildin_type_code::instance(){
	static dict_wrapper_buildin_type_code inst;
	return inst;
}

std::string buildin_type_code::to_name( const buildin_type_code& enum_val){
	return dict_wrapper_buildin_type_code::instance().to_name(enum_val);
}

buildin_type_code buildin_type_code::from_name( const std::string& name){
	return dict_wrapper_buildin_type_code::instance().from_name(name);
}

std::string buildin_type_code::name() const{
	return to_name( * this );
}



		
buildin_type_code::buildin_type_code( const storage_type& val, const std::string& name ): buildin_type_code::base_type(val){
	buildin_type_code tmp(val);
	dict_wrapper_buildin_type_code::instance().insert( tmp, name );
}

const buildin_type_code buildin_type_code::_unsigned ( UINT32_C( 34603008 ), "_unsigned" );
const buildin_type_code buildin_type_code::_sint32 ( UINT32_C( 35848192 ), "_sint32" );
const buildin_type_code buildin_type_code::_c_int ( UINT32_C( 35979264 ), "_c_int" );
const buildin_type_code buildin_type_code::_sint16 ( UINT32_C( 35782656 ), "_sint16" );
const buildin_type_code buildin_type_code::_generic_type_field_shift ( UINT32_C( 24 ), "_generic_type_field_shift" );
const buildin_type_code buildin_type_code::_scalar_type_mask ( UINT32_C( 268369920 ), "_scalar_type_mask" );
const buildin_type_code buildin_type_code::_sign_mask ( UINT32_C( 267386880 ), "_sign_mask" );
const buildin_type_code buildin_type_code::_dim1_mask ( UINT32_C( 255 ), "_dim1_mask" );
const buildin_type_code buildin_type_code::_boolean ( UINT32_C( 50331648 ), "_boolean" );
const buildin_type_code buildin_type_code::_generic_type_mask ( UINT32_C( 251658240 ), "_generic_type_mask" );
const buildin_type_code buildin_type_code::_sint8 ( UINT32_C( 35717120 ), "_sint8" );
const buildin_type_code buildin_type_code::_scalar ( UINT32_C( 0 ), "_scalar" );
const buildin_type_code buildin_type_code::_sign_field_shift ( UINT32_C( 20 ), "_sign_field_shift" );
const buildin_type_code buildin_type_code::_float ( UINT32_C( 16842752 ), "_float" );
const buildin_type_code buildin_type_code::_dim0_field_shift ( UINT32_C( 8 ), "_dim0_field_shift" );
const buildin_type_code buildin_type_code::_void ( UINT32_C( 67108864 ), "_void" );
const buildin_type_code buildin_type_code::_uint16 ( UINT32_C( 34734080 ), "_uint16" );
const buildin_type_code buildin_type_code::_dimension_mask ( UINT32_C( 4026531840 ), "_dimension_mask" );
const buildin_type_code buildin_type_code::_dim1_field_shift ( UINT32_C( 0 ), "_dim1_field_shift" );
const buildin_type_code buildin_type_code::_dimension_field_shift ( UINT32_C( 28 ), "_dimension_field_shift" );
const buildin_type_code buildin_type_code::_double ( UINT32_C( 16908288 ), "_double" );
const buildin_type_code buildin_type_code::_matrix ( UINT32_C( 536870912 ), "_matrix" );
const buildin_type_code buildin_type_code::_sint64 ( UINT32_C( 35913728 ), "_sint64" );
const buildin_type_code buildin_type_code::_real ( UINT32_C( 16777216 ), "_real" );
const buildin_type_code buildin_type_code::_scalar_field_shift ( UINT32_C( 16 ), "_scalar_field_shift" );
const buildin_type_code buildin_type_code::_uint8 ( UINT32_C( 34668544 ), "_uint8" );
const buildin_type_code buildin_type_code::_signed ( UINT32_C( 35651584 ), "_signed" );
const buildin_type_code buildin_type_code::_vector ( UINT32_C( 268435456 ), "_vector" );
const buildin_type_code buildin_type_code::none ( UINT32_C( 0 ), "none" );
const buildin_type_code buildin_type_code::_uint32 ( UINT32_C( 34799616 ), "_uint32" );
const buildin_type_code buildin_type_code::_precision_field_shift ( UINT32_C( 16 ), "_precision_field_shift" );
const buildin_type_code buildin_type_code::_uint64 ( UINT32_C( 34865152 ), "_uint64" );
const buildin_type_code buildin_type_code::_dim0_mask ( UINT32_C( 65280 ), "_dim0_mask" );
const buildin_type_code buildin_type_code::_integer ( UINT32_C( 33554432 ), "_integer" );


