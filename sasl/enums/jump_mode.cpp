
#include "./jump_mode.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const jump_mode jump_mode::_return ( 3 );
const jump_mode jump_mode::none ( 0 );
const jump_mode jump_mode::_continue ( 2 );
const jump_mode jump_mode::_break ( 1 );

 
struct enum_hasher: public std::unary_function< jump_mode, std::size_t> {
	std::size_t operator()( jump_mode const& val) const{
		return hash_value(val.val_);
	}
};

struct dict_wrapper_jump_mode {
private:
	boost::unordered_map< jump_mode, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, jump_mode > name_to_enum;

public:
	dict_wrapper_jump_mode(){
		enum_to_name.insert( std::make_pair( jump_mode::_return, "_return" ) );
		enum_to_name.insert( std::make_pair( jump_mode::none, "none" ) );
		enum_to_name.insert( std::make_pair( jump_mode::_continue, "_continue" ) );
		enum_to_name.insert( std::make_pair( jump_mode::_break, "_break" ) );

		name_to_enum.insert( std::make_pair( "_return", jump_mode::_return ) );
		name_to_enum.insert( std::make_pair( "none", jump_mode::none ) );
		name_to_enum.insert( std::make_pair( "_continue", jump_mode::_continue ) );
		name_to_enum.insert( std::make_pair( "_break", jump_mode::_break ) );

	}

	std::string to_name( jump_mode const& val ){
		boost::unordered_map< jump_mode, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	jump_mode from_name( const std::string& name){
		boost::unordered_map< std::string, jump_mode >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

static dict_wrapper_jump_mode s_dict;

std::string jump_mode::to_name( const jump_mode& enum_val){
	return s_dict.to_name(enum_val);
}

jump_mode jump_mode::from_name( const std::string& name){
	return s_dict.from_name(name);
}

