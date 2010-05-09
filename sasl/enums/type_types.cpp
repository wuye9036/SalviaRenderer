
#include "./type_types.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const type_types type_types::alias ( 3 );
const type_types type_types::none ( 0 );
const type_types type_types::buildin ( 1 );
const type_types type_types::composited ( 2 );

 
struct enum_hasher: public std::unary_function< type_types, std::size_t> {
	std::size_t operator()( type_types const& val) const{
		return hash_value(val.val_);
	}
};

struct dict_wrapper_type_types {
private:
	boost::unordered_map< type_types, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, type_types > name_to_enum;

public:
	dict_wrapper_type_types(){
		enum_to_name.insert( std::make_pair( type_types::alias, "alias" ) );
		enum_to_name.insert( std::make_pair( type_types::none, "none" ) );
		enum_to_name.insert( std::make_pair( type_types::buildin, "buildin" ) );
		enum_to_name.insert( std::make_pair( type_types::composited, "composited" ) );

		name_to_enum.insert( std::make_pair( "alias", type_types::alias ) );
		name_to_enum.insert( std::make_pair( "none", type_types::none ) );
		name_to_enum.insert( std::make_pair( "buildin", type_types::buildin ) );
		name_to_enum.insert( std::make_pair( "composited", type_types::composited ) );

	}

	std::string to_name( type_types const& val ){
		boost::unordered_map< type_types, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	type_types from_name( const std::string& name){
		boost::unordered_map< std::string, type_types >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

static dict_wrapper_type_types s_dict;

std::string type_types::to_name( const type_types& enum_val){
	return s_dict.to_name(enum_val);
}

type_types type_types::from_name( const std::string& name){
	return s_dict.from_name(name);
}

