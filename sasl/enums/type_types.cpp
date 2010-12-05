
#include "./type_types.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


 
struct enum_hasher: public std::unary_function< type_types, std::size_t> {
	std::size_t operator()( type_types const& val) const{
		return hash_value(val.val_);
	}
};
		
struct dict_wrapper_type_types {
private:
	boost::unordered_map< type_types, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, type_types > name_to_enum;

	dict_wrapper_type_types(){}
	
public:
	static dict_wrapper_type_types& instance();
	
	void insert( type_types const& val, const std::string& name ){
		enum_to_name.insert( std::make_pair( val, name ) );
		name_to_enum.insert( std::make_pair( name, val ) );
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

dict_wrapper_type_types& dict_wrapper_type_types::instance(){
	static dict_wrapper_type_types inst;
	return inst;
}

std::string type_types::to_name( const type_types& enum_val){
	return dict_wrapper_type_types::instance().to_name(enum_val);
}

type_types type_types::from_name( const std::string& name){
	return dict_wrapper_type_types::instance().from_name(name);
}

std::string type_types::name() const{
	return to_name( * this );
}



		
type_types::type_types( const storage_type& val, const std::string& name ): type_types::base_type(val){
	type_types tmp(val);
	dict_wrapper_type_types::instance().insert( tmp, name );
}

const type_types type_types::alias ( UINT32_C( 3 ), "alias" );
const type_types type_types::none ( UINT32_C( 0 ), "none" );
const type_types type_types::buildin ( UINT32_C( 1 ), "buildin" );
const type_types type_types::composited ( UINT32_C( 2 ), "composited" );


