
#include "./syntax_node_types.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const syntax_node_types syntax_node_types::node ( 2 );
const syntax_node_types syntax_node_types::token ( 1 );
const syntax_node_types syntax_node_types::constant ( 3 );

 
struct enum_hasher: public std::unary_function< syntax_node_types, std::size_t> {
	std::size_t operator()( syntax_node_types const& val) const{
		return hash_value(val.val_);
	}
};

struct dict_wrapper_syntax_node_types {
private:
	boost::unordered_map< syntax_node_types, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, syntax_node_types > name_to_enum;

public:
	dict_wrapper_syntax_node_types(){
		enum_to_name.insert( std::make_pair( syntax_node_types::node, "node" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::token, "token" ) );
		enum_to_name.insert( std::make_pair( syntax_node_types::constant, "constant" ) );

		name_to_enum.insert( std::make_pair( "node", syntax_node_types::node ) );
		name_to_enum.insert( std::make_pair( "token", syntax_node_types::token ) );
		name_to_enum.insert( std::make_pair( "constant", syntax_node_types::constant ) );

	}

	std::string to_name( syntax_node_types const& val ){
		boost::unordered_map< syntax_node_types, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	syntax_node_types from_name( const std::string& name){
		boost::unordered_map< std::string, syntax_node_types >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

static dict_wrapper_syntax_node_types s_dict;

std::string syntax_node_types::to_name( const syntax_node_types& enum_val){
	return s_dict.to_name(enum_val);
}

syntax_node_types syntax_node_types::from_name( const std::string& name){
	return s_dict.from_name(name);
}

