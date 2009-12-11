
#include "./token_types.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const token_types token_types::_identifier ( 2 );
const token_types token_types::_keyword ( 1 );
const token_types token_types::_operator ( 4 );
const token_types token_types::_constant ( 3 );

 
struct enum_hasher: public std::unary_function< token_types, std::size_t> {
	std::size_t operator()( token_types const& val) const{
		return hash_value(val.val_);
	}
};

struct dict_wrapper_token_types {
private:
	boost::unordered_map< token_types, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, token_types > name_to_enum;

public:
	dict_wrapper_token_types(){
		enum_to_name.insert( std::make_pair( token_types::_identifier, "_identifier" ) );
		enum_to_name.insert( std::make_pair( token_types::_keyword, "_keyword" ) );
		enum_to_name.insert( std::make_pair( token_types::_operator, "_operator" ) );
		enum_to_name.insert( std::make_pair( token_types::_constant, "_constant" ) );

		name_to_enum.insert( std::make_pair( "_identifier", token_types::_identifier ) );
		name_to_enum.insert( std::make_pair( "_keyword", token_types::_keyword ) );
		name_to_enum.insert( std::make_pair( "_operator", token_types::_operator ) );
		name_to_enum.insert( std::make_pair( "_constant", token_types::_constant ) );

	}

	std::string to_name( token_types const& val ){
		boost::unordered_map< token_types, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	token_types from_name( const std::string& name){
		boost::unordered_map< std::string, token_types >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

static dict_wrapper_token_types s_dict;

std::string token_types::to_name( const token_types& enum_val){
	return s_dict.to_name(enum_val);
}

token_types token_types::from_name( const std::string& name){
	return s_dict.from_name(name);
}

