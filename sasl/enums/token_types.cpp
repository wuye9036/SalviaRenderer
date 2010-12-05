
#include "./token_types.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


 
struct enum_hasher: public std::unary_function< token_types, std::size_t> {
	std::size_t operator()( token_types const& val) const{
		return hash_value(val.val_);
	}
};
		
struct dict_wrapper_token_types {
private:
	boost::unordered_map< token_types, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, token_types > name_to_enum;

	dict_wrapper_token_types(){}
	
public:
	static dict_wrapper_token_types& instance();
	
	void insert( token_types const& val, const std::string& name ){
		enum_to_name.insert( std::make_pair( val, name ) );
		name_to_enum.insert( std::make_pair( name, val ) );
	}
	
	std::string to_name( token_types const& val ){
		boost::unordered_map< token_types, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	token_types from_name( const std::string& name){
		boost::unordered_map< std::string, token_types >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

dict_wrapper_token_types& dict_wrapper_token_types::instance(){
	static dict_wrapper_token_types inst;
	return inst;
}

std::string token_types::to_name( const token_types& enum_val){
	return dict_wrapper_token_types::instance().to_name(enum_val);
}

token_types token_types::from_name( const std::string& name){
	return dict_wrapper_token_types::instance().from_name(name);
}

std::string token_types::name() const{
	return to_name( * this );
}



		
token_types::token_types( const storage_type& val, const std::string& name ): token_types::base_type(val){
	token_types tmp(val);
	dict_wrapper_token_types::instance().insert( tmp, name );
}

const token_types token_types::_comment ( UINT32_C( 7 ), "_comment" );
const token_types token_types::_preprocessor ( UINT32_C( 6 ), "_preprocessor" );
const token_types token_types::_operator ( UINT32_C( 4 ), "_operator" );
const token_types token_types::_whitespace ( UINT32_C( 5 ), "_whitespace" );
const token_types token_types::_constant ( UINT32_C( 3 ), "_constant" );
const token_types token_types::_newline ( UINT32_C( 8 ), "_newline" );
const token_types token_types::_identifier ( UINT32_C( 2 ), "_identifier" );
const token_types token_types::_keyword ( UINT32_C( 1 ), "_keyword" );


