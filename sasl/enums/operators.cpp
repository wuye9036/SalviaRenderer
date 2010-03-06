
#include "./operators.h" 
#include <boost/unordered_map.hpp>

using namespace boost;
using namespace std;


const operators operators::sub_assign ( 8 );
const operators operators::bit_not_assign ( 12 );
const operators operators::less ( 20 );
const operators operators::bit_and ( 37 );
const operators operators::bit_or_assign ( 14 );
const operators operators::prefix_incr ( 28 );
const operators operators::logic_and ( 35 );
const operators operators::postfix_incr ( 33554460 );
const operators operators::lshift_assign ( 16 );
const operators operators::mul_assign ( 9 );
const operators operators::unary_op ( 65536 );
const operators operators::prefix_decr ( 29 );
const operators operators::bit_xor_assign ( 15 );
const operators operators::sub ( 2 );
const operators operators::positive ( 65537 );
const operators operators::rshift_assign ( 17 );
const operators operators::negative ( 65538 );
const operators operators::logic_not ( 36 );
const operators operators::add ( 1 );
const operators operators::right_shift ( 25 );
const operators operators::mul ( 3 );
const operators operators::bit_and_assign ( 13 );
const operators operators::mod_assign ( 11 );
const operators operators::greater ( 22 );
const operators operators::bit_or ( 38 );
const operators operators::bit_not ( 40 );
const operators operators::bit_xor ( 39 );
const operators operators::add_assign ( 7 );
const operators operators::mod ( 5 );
const operators operators::none ( 0 );
const operators operators::not_equal ( 19 );
const operators operators::logic_or ( 34 );
const operators operators::greater_equal ( 23 );
const operators operators::left_shift ( 24 );
const operators operators::equal ( 18 );
const operators operators::postfix_decr ( 33554461 );
const operators operators::postfix_op ( 33554432 );
const operators operators::div_assign ( 10 );
const operators operators::less_equal ( 21 );
const operators operators::div ( 4 );
const operators operators::assign ( 6 );

 
struct enum_hasher: public std::unary_function< operators, std::size_t> {
	std::size_t operator()( operators const& val) const{
		return hash_value(val.val_);
	}
};

struct dict_wrapper_operators {
private:
	boost::unordered_map< operators, std::string, enum_hasher > enum_to_name;
	boost::unordered_map< std::string, operators > name_to_enum;

public:
	dict_wrapper_operators(){
		enum_to_name.insert( std::make_pair( operators::sub_assign, "sub_assign" ) );
		enum_to_name.insert( std::make_pair( operators::bit_not_assign, "bit_not_assign" ) );
		enum_to_name.insert( std::make_pair( operators::less, "less" ) );
		enum_to_name.insert( std::make_pair( operators::bit_and, "bit_and" ) );
		enum_to_name.insert( std::make_pair( operators::bit_or_assign, "bit_or_assign" ) );
		enum_to_name.insert( std::make_pair( operators::prefix_incr, "prefix_incr" ) );
		enum_to_name.insert( std::make_pair( operators::logic_and, "logic_and" ) );
		enum_to_name.insert( std::make_pair( operators::postfix_incr, "postfix_incr" ) );
		enum_to_name.insert( std::make_pair( operators::lshift_assign, "lshift_assign" ) );
		enum_to_name.insert( std::make_pair( operators::mul_assign, "mul_assign" ) );
		enum_to_name.insert( std::make_pair( operators::unary_op, "unary_op" ) );
		enum_to_name.insert( std::make_pair( operators::prefix_decr, "prefix_decr" ) );
		enum_to_name.insert( std::make_pair( operators::bit_xor_assign, "bit_xor_assign" ) );
		enum_to_name.insert( std::make_pair( operators::sub, "sub" ) );
		enum_to_name.insert( std::make_pair( operators::positive, "positive" ) );
		enum_to_name.insert( std::make_pair( operators::rshift_assign, "rshift_assign" ) );
		enum_to_name.insert( std::make_pair( operators::negative, "negative" ) );
		enum_to_name.insert( std::make_pair( operators::logic_not, "logic_not" ) );
		enum_to_name.insert( std::make_pair( operators::add, "add" ) );
		enum_to_name.insert( std::make_pair( operators::right_shift, "right_shift" ) );
		enum_to_name.insert( std::make_pair( operators::mul, "mul" ) );
		enum_to_name.insert( std::make_pair( operators::bit_and_assign, "bit_and_assign" ) );
		enum_to_name.insert( std::make_pair( operators::mod_assign, "mod_assign" ) );
		enum_to_name.insert( std::make_pair( operators::greater, "greater" ) );
		enum_to_name.insert( std::make_pair( operators::bit_or, "bit_or" ) );
		enum_to_name.insert( std::make_pair( operators::bit_not, "bit_not" ) );
		enum_to_name.insert( std::make_pair( operators::bit_xor, "bit_xor" ) );
		enum_to_name.insert( std::make_pair( operators::add_assign, "add_assign" ) );
		enum_to_name.insert( std::make_pair( operators::mod, "mod" ) );
		enum_to_name.insert( std::make_pair( operators::none, "none" ) );
		enum_to_name.insert( std::make_pair( operators::not_equal, "not_equal" ) );
		enum_to_name.insert( std::make_pair( operators::logic_or, "logic_or" ) );
		enum_to_name.insert( std::make_pair( operators::greater_equal, "greater_equal" ) );
		enum_to_name.insert( std::make_pair( operators::left_shift, "left_shift" ) );
		enum_to_name.insert( std::make_pair( operators::equal, "equal" ) );
		enum_to_name.insert( std::make_pair( operators::postfix_decr, "postfix_decr" ) );
		enum_to_name.insert( std::make_pair( operators::postfix_op, "postfix_op" ) );
		enum_to_name.insert( std::make_pair( operators::div_assign, "div_assign" ) );
		enum_to_name.insert( std::make_pair( operators::less_equal, "less_equal" ) );
		enum_to_name.insert( std::make_pair( operators::div, "div" ) );
		enum_to_name.insert( std::make_pair( operators::assign, "assign" ) );

		name_to_enum.insert( std::make_pair( "sub_assign", operators::sub_assign ) );
		name_to_enum.insert( std::make_pair( "bit_not_assign", operators::bit_not_assign ) );
		name_to_enum.insert( std::make_pair( "less", operators::less ) );
		name_to_enum.insert( std::make_pair( "bit_and", operators::bit_and ) );
		name_to_enum.insert( std::make_pair( "bit_or_assign", operators::bit_or_assign ) );
		name_to_enum.insert( std::make_pair( "prefix_incr", operators::prefix_incr ) );
		name_to_enum.insert( std::make_pair( "logic_and", operators::logic_and ) );
		name_to_enum.insert( std::make_pair( "postfix_incr", operators::postfix_incr ) );
		name_to_enum.insert( std::make_pair( "lshift_assign", operators::lshift_assign ) );
		name_to_enum.insert( std::make_pair( "mul_assign", operators::mul_assign ) );
		name_to_enum.insert( std::make_pair( "unary_op", operators::unary_op ) );
		name_to_enum.insert( std::make_pair( "prefix_decr", operators::prefix_decr ) );
		name_to_enum.insert( std::make_pair( "bit_xor_assign", operators::bit_xor_assign ) );
		name_to_enum.insert( std::make_pair( "sub", operators::sub ) );
		name_to_enum.insert( std::make_pair( "positive", operators::positive ) );
		name_to_enum.insert( std::make_pair( "rshift_assign", operators::rshift_assign ) );
		name_to_enum.insert( std::make_pair( "negative", operators::negative ) );
		name_to_enum.insert( std::make_pair( "logic_not", operators::logic_not ) );
		name_to_enum.insert( std::make_pair( "add", operators::add ) );
		name_to_enum.insert( std::make_pair( "right_shift", operators::right_shift ) );
		name_to_enum.insert( std::make_pair( "mul", operators::mul ) );
		name_to_enum.insert( std::make_pair( "bit_and_assign", operators::bit_and_assign ) );
		name_to_enum.insert( std::make_pair( "mod_assign", operators::mod_assign ) );
		name_to_enum.insert( std::make_pair( "greater", operators::greater ) );
		name_to_enum.insert( std::make_pair( "bit_or", operators::bit_or ) );
		name_to_enum.insert( std::make_pair( "bit_not", operators::bit_not ) );
		name_to_enum.insert( std::make_pair( "bit_xor", operators::bit_xor ) );
		name_to_enum.insert( std::make_pair( "add_assign", operators::add_assign ) );
		name_to_enum.insert( std::make_pair( "mod", operators::mod ) );
		name_to_enum.insert( std::make_pair( "none", operators::none ) );
		name_to_enum.insert( std::make_pair( "not_equal", operators::not_equal ) );
		name_to_enum.insert( std::make_pair( "logic_or", operators::logic_or ) );
		name_to_enum.insert( std::make_pair( "greater_equal", operators::greater_equal ) );
		name_to_enum.insert( std::make_pair( "left_shift", operators::left_shift ) );
		name_to_enum.insert( std::make_pair( "equal", operators::equal ) );
		name_to_enum.insert( std::make_pair( "postfix_decr", operators::postfix_decr ) );
		name_to_enum.insert( std::make_pair( "postfix_op", operators::postfix_op ) );
		name_to_enum.insert( std::make_pair( "div_assign", operators::div_assign ) );
		name_to_enum.insert( std::make_pair( "less_equal", operators::less_equal ) );
		name_to_enum.insert( std::make_pair( "div", operators::div ) );
		name_to_enum.insert( std::make_pair( "assign", operators::assign ) );

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

static dict_wrapper_operators s_dict;

std::string operators::to_name( const operators& enum_val){
	return s_dict.to_name(enum_val);
}

operators operators::from_name( const std::string& name){
	return s_dict.from_name(name);
}

