
#include "./operators.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


 
struct enum_hasher: public std::unary_function< operators, std::size_t> {
	std::size_t operator()( operators const& val) const{
		return hash_value(val.val_);
	}
};
		
struct dict_wrapper_operators {
private:
	boost::unordered_map< operators, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, operators > name_to_enum;

	dict_wrapper_operators(){}
	
public:
	static dict_wrapper_operators& instance();
	
	void insert( operators const& val, const std::string& name ){
		enum_to_name.insert( std::make_pair( val, name ) );
		name_to_enum.insert( std::make_pair( name, val ) );
	}
	
	std::string to_name( operators const& val ){
		boost::unordered_map< operators, std::string >::const_iterator
			find_result_it = enum_to_name.find(val);
			
		if ( find_result_it != enum_to_name.end() ){
			return find_result_it->second;
		}

		return "__unknown_enum_val__";
	}

	operators from_name( const std::string& name){
		boost::unordered_map< std::string, operators >::const_iterator
			find_result_it = name_to_enum.find(name);
			
		if ( find_result_it != name_to_enum.end() ){
			return find_result_it->second;
		}

		throw "unexcepted enumuration name!";
	}
};

dict_wrapper_operators& dict_wrapper_operators::instance(){
	static dict_wrapper_operators inst;
	return inst;
}

std::string operators::to_name( const operators& enum_val){
	return dict_wrapper_operators::instance().to_name(enum_val);
}

operators operators::from_name( const std::string& name){
	return dict_wrapper_operators::instance().from_name(name);
}

std::string operators::name() const{
	return to_name( * this );
}



		
operators::operators( const storage_type& val, const std::string& name ): operators::base_type(val){
	operators tmp(val);
	dict_wrapper_operators::instance().insert( tmp, name );
}

const operators operators::sub_assign ( UINT32_C( 8 ), "sub_assign" );
const operators operators::less ( UINT32_C( 19 ), "less" );
const operators operators::bit_and ( UINT32_C( 34 ), "bit_and" );
const operators operators::bit_or_assign ( UINT32_C( 13 ), "bit_or_assign" );
const operators operators::prefix_incr ( UINT32_C( 25 ), "prefix_incr" );
const operators operators::logic_and ( UINT32_C( 32 ), "logic_and" );
const operators operators::postfix_incr ( UINT32_C( 27 ), "postfix_incr" );
const operators operators::lshift_assign ( UINT32_C( 15 ), "lshift_assign" );
const operators operators::mul_assign ( UINT32_C( 9 ), "mul_assign" );
const operators operators::prefix_decr ( UINT32_C( 26 ), "prefix_decr" );
const operators operators::bit_xor_assign ( UINT32_C( 14 ), "bit_xor_assign" );
const operators operators::sub ( UINT32_C( 2 ), "sub" );
const operators operators::positive ( UINT32_C( 29 ), "positive" );
const operators operators::rshift_assign ( UINT32_C( 16 ), "rshift_assign" );
const operators operators::negative ( UINT32_C( 30 ), "negative" );
const operators operators::logic_not ( UINT32_C( 33 ), "logic_not" );
const operators operators::add ( UINT32_C( 1 ), "add" );
const operators operators::right_shift ( UINT32_C( 24 ), "right_shift" );
const operators operators::mul ( UINT32_C( 3 ), "mul" );
const operators operators::bit_and_assign ( UINT32_C( 12 ), "bit_and_assign" );
const operators operators::mod_assign ( UINT32_C( 11 ), "mod_assign" );
const operators operators::greater ( UINT32_C( 21 ), "greater" );
const operators operators::bit_or ( UINT32_C( 35 ), "bit_or" );
const operators operators::bit_not ( UINT32_C( 37 ), "bit_not" );
const operators operators::bit_xor ( UINT32_C( 36 ), "bit_xor" );
const operators operators::add_assign ( UINT32_C( 7 ), "add_assign" );
const operators operators::mod ( UINT32_C( 5 ), "mod" );
const operators operators::none ( UINT32_C( 0 ), "none" );
const operators operators::not_equal ( UINT32_C( 18 ), "not_equal" );
const operators operators::logic_or ( UINT32_C( 31 ), "logic_or" );
const operators operators::greater_equal ( UINT32_C( 22 ), "greater_equal" );
const operators operators::left_shift ( UINT32_C( 23 ), "left_shift" );
const operators operators::equal ( UINT32_C( 17 ), "equal" );
const operators operators::postfix_decr ( UINT32_C( 28 ), "postfix_decr" );
const operators operators::div_assign ( UINT32_C( 10 ), "div_assign" );
const operators operators::less_equal ( UINT32_C( 20 ), "less_equal" );
const operators operators::div ( UINT32_C( 4 ), "div" );
const operators operators::assign ( UINT32_C( 6 ), "assign" );


