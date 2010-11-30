
#include "./storage_mode.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


 
struct enum_hasher: public std::unary_function< storage_mode, std::size_t> {
	std::size_t operator()( storage_mode const& val) const{
		return hash_value(val.val_);
	}
};

struct dict_wrapper_storage_mode {
private:
	boost::unordered_map< storage_mode, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, storage_mode > name_to_enum;

public:
	dict_wrapper_storage_mode(){
		enum_to_name.insert( std::make_pair( storage_mode::none, "none" ) );
		enum_to_name.insert( std::make_pair( storage_mode::constant, "constant" ) );
		enum_to_name.insert( std::make_pair( storage_mode::stack_based_address, "stack_based_address" ) );
		enum_to_name.insert( std::make_pair( storage_mode::register_id, "register_id" ) );

		name_to_enum.insert( std::make_pair( "none", storage_mode::none ) );
		name_to_enum.insert( std::make_pair( "constant", storage_mode::constant ) );
		name_to_enum.insert( std::make_pair( "stack_based_address", storage_mode::stack_based_address ) );
		name_to_enum.insert( std::make_pair( "register_id", storage_mode::register_id ) );

	}

	std::string to_name( storage_mode const& val ){
		boost::unordered_map< storage_mode, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	storage_mode from_name( const std::string& name){
		boost::unordered_map< std::string, storage_mode >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

static dict_wrapper_storage_mode s_dict;

std::string storage_mode::to_name( const storage_mode& enum_val){
	return s_dict.to_name(enum_val);
}

storage_mode storage_mode::from_name( const std::string& name){
	return s_dict.from_name(name);
}

std::string storage_mode::name() const{
	return to_name( * this );
}



