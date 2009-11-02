
#include "./integer_type_suffixes.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const integer_type_suffixes integer_type_suffixes::ul ( 3 );
const integer_type_suffixes integer_type_suffixes::lu ( 4 );
const integer_type_suffixes integer_type_suffixes::u ( 1 );
const integer_type_suffixes integer_type_suffixes::l ( 2 );

 
struct enum_hasher: public std::unary_function< integer_type_suffixes, std::size_t> {
	std::size_t operator()( integer_type_suffixes const& val) const{
		return hash_value(val.val_);
	}
};

struct dict_wrapper_integer_type_suffixes {
private:
	boost::unordered_map< integer_type_suffixes, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, integer_type_suffixes > name_to_enum;

public:
	dict_wrapper_integer_type_suffixes(){
		enum_to_name.insert( std::make_pair( integer_type_suffixes::ul, "ul" ) );
		enum_to_name.insert( std::make_pair( integer_type_suffixes::lu, "lu" ) );
		enum_to_name.insert( std::make_pair( integer_type_suffixes::u, "u" ) );
		enum_to_name.insert( std::make_pair( integer_type_suffixes::l, "l" ) );

		name_to_enum.insert( std::make_pair( "ul", integer_type_suffixes::ul ) );
		name_to_enum.insert( std::make_pair( "lu", integer_type_suffixes::lu ) );
		name_to_enum.insert( std::make_pair( "u", integer_type_suffixes::u ) );
		name_to_enum.insert( std::make_pair( "l", integer_type_suffixes::l ) );

	}

	std::string to_name( integer_type_suffixes const& val ){
		boost::unordered_map< integer_type_suffixes, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	integer_type_suffixes from_name( const std::string& name){
		boost::unordered_map< std::string, integer_type_suffixes >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

static dict_wrapper_integer_type_suffixes s_dict;

std::string integer_type_suffixes::to_name( const integer_type_suffixes& enum_val){
	return s_dict.to_name(enum_val);
}

integer_type_suffixes integer_type_suffixes::from_name( const std::string& name){
	return s_dict.from_name(name);
}

