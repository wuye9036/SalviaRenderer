
#include "./literal_classifications.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


 
struct enum_hasher: public std::unary_function< literal_classifications, std::size_t> {
	std::size_t operator()( literal_classifications const& val) const{
		return hash_value(val.val_);
	}
};
		
struct dict_wrapper_literal_classifications {
private:
	boost::unordered_map< literal_classifications, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, literal_classifications > name_to_enum;

	dict_wrapper_literal_classifications(){}
	
public:
	static dict_wrapper_literal_classifications& instance();
	
	void insert( literal_classifications const& val, const std::string& name ){
		enum_to_name.insert( std::make_pair( val, name ) );
		name_to_enum.insert( std::make_pair( name, val ) );
	}
	
	std::string to_name( literal_classifications const& val ){
		boost::unordered_map< literal_classifications, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	literal_classifications from_name( const std::string& name){
		boost::unordered_map< std::string, literal_classifications >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

dict_wrapper_literal_classifications& dict_wrapper_literal_classifications::instance(){
	static dict_wrapper_literal_classifications inst;
	return inst;
}

std::string literal_classifications::to_name( const literal_classifications& enum_val){
	return dict_wrapper_literal_classifications::instance().to_name(enum_val);
}

literal_classifications literal_classifications::from_name( const std::string& name){
	return dict_wrapper_literal_classifications::instance().from_name(name);
}

std::string literal_classifications::name() const{
	return to_name( * this );
}

void literal_classifications::force_initialize(){

	static bool is_initialized = false;
	if ( is_initialized ) return;
	is_initialized = true;
	new ( const_cast<literal_classifications*>(&real) ) literal_classifications ( UINT32_C( 4 ), "real" );
	new ( const_cast<literal_classifications*>(&none) ) literal_classifications ( UINT32_C( 1 ), "none" );
	new ( const_cast<literal_classifications*>(&string) ) literal_classifications ( UINT32_C( 5 ), "string" );
	new ( const_cast<literal_classifications*>(&character) ) literal_classifications ( UINT32_C( 6 ), "character" );
	new ( const_cast<literal_classifications*>(&boolean) ) literal_classifications ( UINT32_C( 2 ), "boolean" );
	new ( const_cast<literal_classifications*>(&integer) ) literal_classifications ( UINT32_C( 3 ), "integer" );

}


		
literal_classifications::literal_classifications( const storage_type& val, const std::string& name ): literal_classifications::base_type(val){
	literal_classifications tmp(val);
	dict_wrapper_literal_classifications::instance().insert( tmp, name );
}

const literal_classifications literal_classifications::real ( UINT32_C( 4 ), "real" );
const literal_classifications literal_classifications::none ( UINT32_C( 1 ), "none" );
const literal_classifications literal_classifications::string ( UINT32_C( 5 ), "string" );
const literal_classifications literal_classifications::character ( UINT32_C( 6 ), "character" );
const literal_classifications literal_classifications::boolean ( UINT32_C( 2 ), "boolean" );
const literal_classifications literal_classifications::integer ( UINT32_C( 3 ), "integer" );


