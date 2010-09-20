
#include "./literal_constant_types.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const literal_constant_types literal_constant_types::real ( 4 );
const literal_constant_types literal_constant_types::none ( 1 );
const literal_constant_types literal_constant_types::string ( 5 );
const literal_constant_types literal_constant_types::character ( 6 );
const literal_constant_types literal_constant_types::boolean ( 2 );
const literal_constant_types literal_constant_types::integer ( 3 );

 
struct enum_hasher: public std::unary_function< literal_constant_types, std::size_t> {
	std::size_t operator()( literal_constant_types const& val) const{
		return hash_value(val.val_);
	}
};

struct dict_wrapper_literal_constant_types {
private:
	boost::unordered_map< literal_constant_types, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, literal_constant_types > name_to_enum;

public:
	dict_wrapper_literal_constant_types(){
		enum_to_name.insert( std::make_pair( literal_constant_types::real, "real" ) );
		enum_to_name.insert( std::make_pair( literal_constant_types::none, "none" ) );
		enum_to_name.insert( std::make_pair( literal_constant_types::string, "string" ) );
		enum_to_name.insert( std::make_pair( literal_constant_types::character, "character" ) );
		enum_to_name.insert( std::make_pair( literal_constant_types::boolean, "boolean" ) );
		enum_to_name.insert( std::make_pair( literal_constant_types::integer, "integer" ) );

		name_to_enum.insert( std::make_pair( "real", literal_constant_types::real ) );
		name_to_enum.insert( std::make_pair( "none", literal_constant_types::none ) );
		name_to_enum.insert( std::make_pair( "string", literal_constant_types::string ) );
		name_to_enum.insert( std::make_pair( "character", literal_constant_types::character ) );
		name_to_enum.insert( std::make_pair( "boolean", literal_constant_types::boolean ) );
		name_to_enum.insert( std::make_pair( "integer", literal_constant_types::integer ) );

	}

	std::string to_name( literal_constant_types const& val ){
		boost::unordered_map< literal_constant_types, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	literal_constant_types from_name( const std::string& name){
		boost::unordered_map< std::string, literal_constant_types >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

static dict_wrapper_literal_constant_types s_dict;

std::string literal_constant_types::to_name( const literal_constant_types& enum_val){
	return s_dict.to_name(enum_val);
}

literal_constant_types literal_constant_types::from_name( const std::string& name){
	return s_dict.from_name(name);
}

std::string literal_constant_types::name() const{
	return to_name( * this );
}



