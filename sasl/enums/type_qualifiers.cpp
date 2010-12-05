
#include "./type_qualifiers.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


 
struct enum_hasher: public std::unary_function< type_qualifiers, std::size_t> {
	std::size_t operator()( type_qualifiers const& val) const{
		return hash_value(val.val_);
	}
};
		
struct dict_wrapper_type_qualifiers {
private:
	boost::unordered_map< type_qualifiers, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, type_qualifiers > name_to_enum;

	dict_wrapper_type_qualifiers(){}
	
public:
	static dict_wrapper_type_qualifiers& instance();
	
	void insert( type_qualifiers const& val, const std::string& name ){
		enum_to_name.insert( std::make_pair( val, name ) );
		name_to_enum.insert( std::make_pair( name, val ) );
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

dict_wrapper_type_qualifiers& dict_wrapper_type_qualifiers::instance(){
	static dict_wrapper_type_qualifiers inst;
	return inst;
}

std::string type_qualifiers::to_name( const type_qualifiers& enum_val){
	return dict_wrapper_type_qualifiers::instance().to_name(enum_val);
}

type_qualifiers type_qualifiers::from_name( const std::string& name){
	return dict_wrapper_type_qualifiers::instance().from_name(name);
}

std::string type_qualifiers::name() const{
	return to_name( * this );
}



		
type_qualifiers::type_qualifiers( const storage_type& val, const std::string& name ): type_qualifiers::base_type(val){
	type_qualifiers tmp(val);
	dict_wrapper_type_qualifiers::instance().insert( tmp, name );
}

const type_qualifiers type_qualifiers::_uniform ( UINT32_C( 2 ), "_uniform" );
const type_qualifiers type_qualifiers::none ( UINT32_C( 1 ), "none" );


