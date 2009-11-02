
#include "./ast_node_types.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const ast_node_types ast_node_types::unknown ( 1 );
const ast_node_types ast_node_types::expression_list ( 16 );
const ast_node_types ast_node_types::type_qual ( 2 );
const ast_node_types ast_node_types::expression ( 8 );
const ast_node_types ast_node_types::array_type ( 32 );
const ast_node_types ast_node_types::type ( 4 );

 
struct enum_hasher: public std::unary_function< ast_node_types, std::size_t> {
	std::size_t operator()( ast_node_types const& val) const{
		return hash_value(val.val_);
	}
};

struct dict_wrapper_ast_node_types {
private:
	boost::unordered_map< ast_node_types, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, ast_node_types > name_to_enum;

public:
	dict_wrapper_ast_node_types(){
		enum_to_name.insert( std::make_pair( ast_node_types::unknown, "unknown" ) );
		enum_to_name.insert( std::make_pair( ast_node_types::expression_list, "expression_list" ) );
		enum_to_name.insert( std::make_pair( ast_node_types::type_qual, "type_qual" ) );
		enum_to_name.insert( std::make_pair( ast_node_types::expression, "expression" ) );
		enum_to_name.insert( std::make_pair( ast_node_types::array_type, "array_type" ) );
		enum_to_name.insert( std::make_pair( ast_node_types::type, "type" ) );

		name_to_enum.insert( std::make_pair( "unknown", ast_node_types::unknown ) );
		name_to_enum.insert( std::make_pair( "expression_list", ast_node_types::expression_list ) );
		name_to_enum.insert( std::make_pair( "type_qual", ast_node_types::type_qual ) );
		name_to_enum.insert( std::make_pair( "expression", ast_node_types::expression ) );
		name_to_enum.insert( std::make_pair( "array_type", ast_node_types::array_type ) );
		name_to_enum.insert( std::make_pair( "type", ast_node_types::type ) );

	}

	std::string to_name( ast_node_types const& val ){
		boost::unordered_map< ast_node_types, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	ast_node_types from_name( const std::string& name){
		boost::unordered_map< std::string, ast_node_types >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

static dict_wrapper_ast_node_types s_dict;

std::string ast_node_types::to_name( const ast_node_types& enum_val){
	return s_dict.to_name(enum_val);
}

ast_node_types ast_node_types::from_name( const std::string& name){
	return s_dict.from_name(name);
}

