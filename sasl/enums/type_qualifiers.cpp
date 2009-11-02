
#include "./type_qualifiers.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const type_qualifiers type_qualifiers::_uniform ( 2 );
const type_qualifiers type_qualifiers::_const ( 0 );
const type_qualifiers type_qualifiers::_volatile ( 1 );

 
struct enum_hasher: public std::unary_function< type_qualifiers, std::size_t> {
	std::size_t operator()( type_qualifiers const& val) const{
		return hash_value(val.val_);
	}
};

struct dict_wrapper_type_qualifiers {
private:
	boost::unordered_map< type_qualifiers, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, type_qualifiers > name_to_enum;

public:
	dict_wrapper_type_qualifiers(){
		enum_to_name.insert( std::make_pair( type_qualifiers::_uniform, "_uniform" ) );
		enum_to_name.insert( std::make_pair( type_qualifiers::_const, "_const" ) );
		enum_to_name.insert( std::make_pair( type_qualifiers::_volatile, "_volatile" ) );

		name_to_enum.insert( std::make_pair( "_uniform", type_qualifiers::_uniform ) );
		name_to_enum.insert( std::make_pair( "_const", type_qualifiers::_const ) );
		name_to_enum.insert( std::make_pair( "_volatile", type_qualifiers::_volatile ) );

	}

	std::string to_name( type_qualifiers const& val ){
		boost::unordered_map< type_qualifiers, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	type_qualifiers from_name( const std::string& name){
		boost::unordered_map< std::string, type_qualifiers >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

static dict_wrapper_type_qualifiers s_dict;

std::string type_qualifiers::to_name( const type_qualifiers& enum_val){
	return s_dict.to_name(enum_val);
}

type_qualifiers type_qualifiers::from_name( const std::string& name){
	return s_dict.from_name(name);
}

