
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

public:
	dict_wrapper_compiler_informations(){
		enum_to_name.insert( std::make_pair( compiler_informations::none, "none" ) );
		enum_to_name.insert( std::make_pair( compiler_informations::_error, "Error" ) );
		enum_to_name.insert( std::make_pair( compiler_informations::_message, "Message" ) );
		enum_to_name.insert( std::make_pair( compiler_informations::redef_diff_basic_type, "$anchor:identifier$ : redefinition; different basic types." ) );
		enum_to_name.insert( std::make_pair( compiler_informations::_link, "Link" ) );
		enum_to_name.insert( std::make_pair( compiler_informations::_info_level_mask, "_info_level_mask" ) );
		enum_to_name.insert( std::make_pair( compiler_informations::_stage_mask, "_stage_mask" ) );
		enum_to_name.insert( std::make_pair( compiler_informations::unknown_compile_error, "unknown compiler error." ) );
		enum_to_name.insert( std::make_pair( compiler_informations::uses_a_undef_type, "$anchor:identifier$ uses a undefined $anchor:type_alias$" ) );
		enum_to_name.insert( std::make_pair( compiler_informations::_info_id_mask, "_info_id_mask" ) );
		enum_to_name.insert( std::make_pair( compiler_informations::_compile, "Compile" ) );
		enum_to_name.insert( std::make_pair( compiler_informations::redef_cannot_overloaded, "$anchor:identifier$ : redefinition; symbol cannot be overloaded with a typedef." ) );
		enum_to_name.insert( std::make_pair( compiler_informations::_warning, "Warning" ) );

		name_to_enum.insert( std::make_pair( "none", compiler_informations::none ) );
		name_to_enum.insert( std::make_pair( "Error", compiler_informations::_error ) );
		name_to_enum.insert( std::make_pair( "Message", compiler_informations::_message ) );
		name_to_enum.insert( std::make_pair( "$anchor:identifier$ : redefinition; different basic types.", compiler_informations::redef_diff_basic_type ) );
		name_to_enum.insert( std::make_pair( "Link", compiler_informations::_link ) );
		name_to_enum.insert( std::make_pair( "_info_level_mask", compiler_informations::_info_level_mask ) );
		name_to_enum.insert( std::make_pair( "_stage_mask", compiler_informations::_stage_mask ) );
		name_to_enum.insert( std::make_pair( "unknown compiler error.", compiler_informations::unknown_compile_error ) );
		name_to_enum.insert( std::make_pair( "$anchor:identifier$ uses a undefined $anchor:type_alias$", compiler_informations::uses_a_undef_type ) );
		name_to_enum.insert( std::make_pair( "_info_id_mask", compiler_informations::_info_id_mask ) );
		name_to_enum.insert( std::make_pair( "Compile", compiler_informations::_compile ) );
		name_to_enum.insert( std::make_pair( "$anchor:identifier$ : redefinition; symbol cannot be overloaded with a typedef.", compiler_informations::redef_cannot_overloaded ) );
		name_to_enum.insert( std::make_pair( "Warning", compiler_informations::_warning ) );

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

static dict_wrapper_compiler_informations s_dict;

std::string compiler_informations::to_name( const compiler_informations& enum_val){
	return s_dict.to_name(enum_val);
}

compiler_informations compiler_informations::from_name( const std::string& name){
	return s_dict.from_name(name);
}

std::string compiler_informations::name() const{
	return to_name( * this );
}



