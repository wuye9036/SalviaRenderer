
#include "./control_statements.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const control_statements control_statements::_return ( 1 );
const control_statements control_statements::_continue ( 3 );
const control_statements control_statements::_break ( 2 );

 
struct enum_hasher: public std::unary_function< control_statements, std::size_t> {
	std::size_t operator()( control_statements const& val) const{
		return hash_value(val.val_);
	}
};

struct dict_wrapper_control_statements {
private:
	boost::unordered_map< control_statements, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, control_statements > name_to_enum;

public:
	dict_wrapper_control_statements(){
		enum_to_name.insert( std::make_pair( control_statements::_return, "_return" ) );
		enum_to_name.insert( std::make_pair( control_statements::_continue, "_continue" ) );
		enum_to_name.insert( std::make_pair( control_statements::_break, "_break" ) );

		name_to_enum.insert( std::make_pair( "_return", control_statements::_return ) );
		name_to_enum.insert( std::make_pair( "_continue", control_statements::_continue ) );
		name_to_enum.insert( std::make_pair( "_break", control_statements::_break ) );

	}

	std::string to_name( control_statements const& val ){
		boost::unordered_map< control_statements, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	control_statements from_name( const std::string& name){
		boost::unordered_map< std::string, control_statements >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

static dict_wrapper_control_statements s_dict;

std::string control_statements::to_name( const control_statements& enum_val){
	return s_dict.to_name(enum_val);
}

control_statements control_statements::from_name( const std::string& name){
	return s_dict.from_name(name);
}

