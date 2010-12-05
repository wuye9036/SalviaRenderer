
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

	dict_wrapper_storage_mode(){}
	
public:
	static dict_wrapper_storage_mode& instance();
	
	void insert( storage_mode const& val, const std::string& name ){
		enum_to_name.insert( std::make_pair( val, name ) );
		name_to_enum.insert( std::make_pair( name, val ) );
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

dict_wrapper_storage_mode& dict_wrapper_storage_mode::instance(){
	static dict_wrapper_storage_mode inst;
	return inst;
}

std::string storage_mode::to_name( const storage_mode& enum_val){
	return dict_wrapper_storage_mode::instance().to_name(enum_val);
}

storage_mode storage_mode::from_name( const std::string& name){
	return dict_wrapper_storage_mode::instance().from_name(name);
}

std::string storage_mode::name() const{
	return to_name( * this );
}



		
storage_mode::storage_mode( const storage_type& val, const std::string& name ): storage_mode::base_type(val){
	storage_mode tmp(val);
	dict_wrapper_storage_mode::instance().insert( tmp, name );
}

const storage_mode storage_mode::none ( UINT32_C( 0 ), "none" );
const storage_mode storage_mode::constant ( UINT32_C( 1 ), "constant" );
const storage_mode storage_mode::stack_based_address ( UINT32_C( 3 ), "stack_based_address" );
const storage_mode storage_mode::register_id ( UINT32_C( 2 ), "register_id" );


