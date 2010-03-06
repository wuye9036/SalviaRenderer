
#include "./labels.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const labels labels::_identifier ( 3 );
const labels labels::_default ( 2 );
const labels labels::_case ( 1 );

 
struct enum_hasher: public std::unary_function< labels, std::size_t> {
	std::size_t operator()( labels const& val) const{
		return hash_value(val.val_);
	}
};

struct dict_wrapper_labels {
private:
	boost::unordered_map< labels, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, labels > name_to_enum;

public:
	dict_wrapper_labels(){
		enum_to_name.insert( std::make_pair( labels::_identifier, "_identifier" ) );
		enum_to_name.insert( std::make_pair( labels::_default, "_default" ) );
		enum_to_name.insert( std::make_pair( labels::_case, "_case" ) );

		name_to_enum.insert( std::make_pair( "_identifier", labels::_identifier ) );
		name_to_enum.insert( std::make_pair( "_default", labels::_default ) );
		name_to_enum.insert( std::make_pair( "_case", labels::_case ) );

	}

	std::string to_name( labels const& val ){
		boost::unordered_map< labels, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	labels from_name( const std::string& name){
		boost::unordered_map< std::string, labels >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

static dict_wrapper_labels s_dict;

std::string labels::to_name( const labels& enum_val){
	return s_dict.to_name(enum_val);
}

labels labels::from_name( const std::string& name){
	return s_dict.from_name(name);
}

