
#include "./compiler_informations.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


 
struct enum_hasher: public std::unary_function< compiler_informations, std::size_t> {
	std::size_t operator()( compiler_informations const& val) const{
		return hash_value(val.val_);
	}
};
		
struct dict_wrapper_compiler_informations {
private:
	boost::unordered_map< compiler_informations, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, compiler_informations > name_to_enum;

	dict_wrapper_compiler_informations(){}
	
public:
	static dict_wrapper_compiler_informations& instance();
	
	void insert( compiler_informations const& val, const std::string& name ){
		enum_to_name.insert( std::make_pair( val, name ) );
		name_to_enum.insert( std::make_pair( name, val ) );
	}
	
	std::string to_name( compiler_informations const& val ){
		boost::unordered_map< compiler_informations, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	compiler_informations from_name( const std::string& name){
		boost::unordered_map< std::string, compiler_informations >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

dict_wrapper_compiler_informations& dict_wrapper_compiler_informations::instance(){
	static dict_wrapper_compiler_informations inst;
	return inst;
}

std::string compiler_informations::to_name( const compiler_informations& enum_val){
	return dict_wrapper_compiler_informations::instance().to_name(enum_val);
}

compiler_informations compiler_informations::from_name( const std::string& name){
	return dict_wrapper_compiler_informations::instance().from_name(name);
}

std::string compiler_informations::name() const{
	return to_name( * this );
}

void compiler_informations::force_initialize(){

	static bool is_initialized = false;
	if ( is_initialized ) return;
	is_initialized = true;
	new ( const_cast<compiler_informations*>(&none) ) compiler_informations ( UINT32_C( 0 ), "none" );
	new ( const_cast<compiler_informations*>(&_error) ) compiler_informations ( UINT32_C( 131072 ), "Error" );
	new ( const_cast<compiler_informations*>(&_message) ) compiler_informations ( UINT32_C( 262144 ), "Message" );
	new ( const_cast<compiler_informations*>(&redef_diff_basic_type) ) compiler_informations ( UINT32_C( 16909290 ), "$anchor:identifier$ : redefinition; different basic types." );
	new ( const_cast<compiler_informations*>(&_link) ) compiler_informations ( UINT32_C( 33554432 ), "Link" );
	new ( const_cast<compiler_informations*>(&_info_level_mask) ) compiler_informations ( UINT32_C( 16711680 ), "_info_level_mask" );
	new ( const_cast<compiler_informations*>(&_stage_mask) ) compiler_informations ( UINT32_C( 4278190080 ), "_stage_mask" );
	new ( const_cast<compiler_informations*>(&unknown_compile_error) ) compiler_informations ( UINT32_C( 16909289 ), "unknown compiler error." );
	new ( const_cast<compiler_informations*>(&uses_a_undef_type) ) compiler_informations ( UINT32_C( 16909292 ), "$anchor:identifier$ uses a undefined $anchor:type_alias$" );
	new ( const_cast<compiler_informations*>(&_info_id_mask) ) compiler_informations ( UINT32_C( 65535 ), "_info_id_mask" );
	new ( const_cast<compiler_informations*>(&_compile) ) compiler_informations ( UINT32_C( 16777216 ), "Compile" );
	new ( const_cast<compiler_informations*>(&redef_cannot_overloaded) ) compiler_informations ( UINT32_C( 16909291 ), "$anchor:identifier$ : redefinition; symbol cannot be overloaded with a typedef." );
	new ( const_cast<compiler_informations*>(&_warning) ) compiler_informations ( UINT32_C( 65536 ), "Warning" );

}


		
compiler_informations::compiler_informations( const storage_type& val, const std::string& name ): compiler_informations::base_type(val){
	compiler_informations tmp(val);
	dict_wrapper_compiler_informations::instance().insert( tmp, name );
}

const compiler_informations compiler_informations::none ( UINT32_C( 0 ), "none" );
const compiler_informations compiler_informations::_error ( UINT32_C( 131072 ), "Error" );
const compiler_informations compiler_informations::_message ( UINT32_C( 262144 ), "Message" );
const compiler_informations compiler_informations::redef_diff_basic_type ( UINT32_C( 16909290 ), "$anchor:identifier$ : redefinition; different basic types." );
const compiler_informations compiler_informations::_link ( UINT32_C( 33554432 ), "Link" );
const compiler_informations compiler_informations::_info_level_mask ( UINT32_C( 16711680 ), "_info_level_mask" );
const compiler_informations compiler_informations::_stage_mask ( UINT32_C( 4278190080 ), "_stage_mask" );
const compiler_informations compiler_informations::unknown_compile_error ( UINT32_C( 16909289 ), "unknown compiler error." );
const compiler_informations compiler_informations::uses_a_undef_type ( UINT32_C( 16909292 ), "$anchor:identifier$ uses a undefined $anchor:type_alias$" );
const compiler_informations compiler_informations::_info_id_mask ( UINT32_C( 65535 ), "_info_id_mask" );
const compiler_informations compiler_informations::_compile ( UINT32_C( 16777216 ), "Compile" );
const compiler_informations compiler_informations::redef_cannot_overloaded ( UINT32_C( 16909291 ), "$anchor:identifier$ : redefinition; symbol cannot be overloaded with a typedef." );
const compiler_informations compiler_informations::_warning ( UINT32_C( 65536 ), "Warning" );


