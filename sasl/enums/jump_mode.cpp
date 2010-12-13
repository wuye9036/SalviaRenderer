
#include "./jump_mode.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


 
struct enum_hasher: public std::unary_function< jump_mode, std::size_t> {
	std::size_t operator()( jump_mode const& val) const{
		return hash_value(val.val_);
	}
};
		
struct dict_wrapper_jump_mode {
private:
	boost::unordered_map< jump_mode, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, jump_mode > name_to_enum;

	dict_wrapper_jump_mode(){}
	
public:
	static dict_wrapper_jump_mode& instance();
	
	void insert( jump_mode const& val, const std::string& name ){
		enum_to_name.insert( std::make_pair( val, name ) );
		name_to_enum.insert( std::make_pair( name, val ) );
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

dict_wrapper_jump_mode& dict_wrapper_jump_mode::instance(){
	static dict_wrapper_jump_mode inst;
	return inst;
}

std::string jump_mode::to_name( const jump_mode& enum_val){
	return dict_wrapper_jump_mode::instance().to_name(enum_val);
}

jump_mode jump_mode::from_name( const std::string& name){
	return dict_wrapper_jump_mode::instance().from_name(name);
}

std::string jump_mode::name() const{
	return to_name( * this );
}

void jump_mode::force_initialize(){

	static bool is_initialized = false;
	if ( is_initialized ) return;
	is_initialized = true;
	new ( const_cast<jump_mode*>(&_return) ) jump_mode ( UINT32_C( 3 ), "_return" );
	new ( const_cast<jump_mode*>(&none) ) jump_mode ( UINT32_C( 0 ), "none" );
	new ( const_cast<jump_mode*>(&_continue) ) jump_mode ( UINT32_C( 2 ), "_continue" );
	new ( const_cast<jump_mode*>(&_break) ) jump_mode ( UINT32_C( 1 ), "_break" );

}


		
jump_mode::jump_mode( const storage_type& val, const std::string& name ): jump_mode::base_type(val){
	jump_mode tmp(val);
	dict_wrapper_jump_mode::instance().insert( tmp, name );
}

const jump_mode jump_mode::_return ( UINT32_C( 3 ), "_return" );
const jump_mode jump_mode::none ( UINT32_C( 0 ), "none" );
const jump_mode jump_mode::_continue ( UINT32_C( 2 ), "_continue" );
const jump_mode jump_mode::_break ( UINT32_C( 1 ), "_break" );


