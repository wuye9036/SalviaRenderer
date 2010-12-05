
#include "./literal_constant_types.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


 
struct enum_hasher: public std::unary_function< literal_constant_types, std::size_t> {
	std::size_t operator()( literal_constant_types const& val) const{
		return hash_value(val.val_);
	}
};
		
struct dict_wrapper_literal_constant_types {
private:
	boost::unordered_map< literal_constant_types, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, literal_constant_types > name_to_enum;

	dict_wrapper_literal_constant_types(){}
	
public:
	static dict_wrapper_literal_constant_types& instance();
	
	void insert( literal_constant_types const& val, const std::string& name ){
		enum_to_name.insert( std::make_pair( val, name ) );
		name_to_enum.insert( std::make_pair( name, val ) );
	}
	
	std::string to_name( literal_constant_types const& val ){
		boost::unordered_map< literal_constant_types, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	literal_constant_types from_name( const std::string& name){
		boost::unordered_map< std::string, literal_constant_types >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

dict_wrapper_literal_constant_types& dict_wrapper_literal_constant_types::instance(){
	static dict_wrapper_literal_constant_types inst;
	return inst;
}

std::string literal_constant_types::to_name( const literal_constant_types& enum_val){
	return dict_wrapper_literal_constant_types::instance().to_name(enum_val);
}

literal_constant_types literal_constant_types::from_name( const std::string& name){
	return dict_wrapper_literal_constant_types::instance().from_name(name);
}

std::string literal_constant_types::name() const{
	return to_name( * this );
}



		
literal_constant_types::literal_constant_types( const storage_type& val, const std::string& name ): literal_constant_types::base_type(val){
	literal_constant_types tmp(val);
	dict_wrapper_literal_constant_types::instance().insert( tmp, name );
}

const literal_constant_types literal_constant_types::real ( UINT32_C( 4 ), "real" );
const literal_constant_types literal_constant_types::none ( UINT32_C( 1 ), "none" );
const literal_constant_types literal_constant_types::string ( UINT32_C( 5 ), "string" );
const literal_constant_types literal_constant_types::character ( UINT32_C( 6 ), "character" );
const literal_constant_types literal_constant_types::boolean ( UINT32_C( 2 ), "boolean" );
const literal_constant_types literal_constant_types::integer ( UINT32_C( 3 ), "integer" );


