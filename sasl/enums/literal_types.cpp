
#include "./literal_types.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const literal_types literal_types::real ( 3 );
const literal_types literal_types::integer ( 2 );
const literal_types literal_types::boolean ( 1 );
const literal_types literal_types::character ( 5 );
const literal_types literal_types::string ( 4 );

 
struct enum_hasher: public std::unary_function< literal_types, std::size_t> {
	std::size_t operator()( literal_types const& val) const{
		return hash_value(val.val_);
	}
};

struct dict_wrapper_literal_types {
private:
	boost::unordered_map< literal_types, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, literal_types > name_to_enum;

public:
	dict_wrapper_literal_types(){
		enum_to_name.insert( std::make_pair( literal_types::real, "real" ) );
		enum_to_name.insert( std::make_pair( literal_types::integer, "integer" ) );
		enum_to_name.insert( std::make_pair( literal_types::boolean, "boolean" ) );
		enum_to_name.insert( std::make_pair( literal_types::character, "character" ) );
		enum_to_name.insert( std::make_pair( literal_types::string, "string" ) );

		name_to_enum.insert( std::make_pair( "real", literal_types::real ) );
		name_to_enum.insert( std::make_pair( "integer", literal_types::integer ) );
		name_to_enum.insert( std::make_pair( "boolean", literal_types::boolean ) );
		name_to_enum.insert( std::make_pair( "character", literal_types::character ) );
		name_to_enum.insert( std::make_pair( "string", literal_types::string ) );

	}

	std::string to_name( literal_types const& val ){
		boost::unordered_map< literal_types, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	literal_types from_name( const std::string& name){
		boost::unordered_map< std::string, literal_types >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

static dict_wrapper_literal_types s_dict;

std::string literal_types::to_name( const literal_types& enum_val){
	return s_dict.to_name(enum_val);
}

literal_types literal_types::from_name( const std::string& name){
	return s_dict.from_name(name);
}

