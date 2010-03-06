
#include "./real_type_suffixes.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const real_type_suffixes real_type_suffixes::e ( 1 );
const real_type_suffixes real_type_suffixes::f ( 2 );

 
struct enum_hasher: public std::unary_function< real_type_suffixes, std::size_t> {
	std::size_t operator()( real_type_suffixes const& val) const{
		return hash_value(val.val_);
	}
};

struct dict_wrapper_real_type_suffixes {
private:
	boost::unordered_map< real_type_suffixes, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, real_type_suffixes > name_to_enum;

public:
	dict_wrapper_real_type_suffixes(){
		enum_to_name.insert( std::make_pair( real_type_suffixes::e, "e" ) );
		enum_to_name.insert( std::make_pair( real_type_suffixes::f, "f" ) );

		name_to_enum.insert( std::make_pair( "e", real_type_suffixes::e ) );
		name_to_enum.insert( std::make_pair( "f", real_type_suffixes::f ) );

	}

	std::string to_name( real_type_suffixes const& val ){
		boost::unordered_map< real_type_suffixes, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	real_type_suffixes from_name( const std::string& name){
		boost::unordered_map< std::string, real_type_suffixes >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

static dict_wrapper_real_type_suffixes s_dict;

std::string real_type_suffixes::to_name( const real_type_suffixes& enum_val){
	return s_dict.to_name(enum_val);
}

real_type_suffixes real_type_suffixes::from_name( const std::string& name){
	return s_dict.from_name(name);
}

