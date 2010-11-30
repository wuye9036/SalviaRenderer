
#ifndef SASL_ENUMS_OPERATORS_H
#define SASL_ENUMS_OPERATORS_H

#include "../enums/enum_base.h" 

struct operators :
	public enum_base< operators, uint32_t >
	, public bitwise_op< operators >, public equal_op< operators >
{
	friend struct enum_hasher;
private:
	operators( const storage_type& val ): base_type( val ){}
	
public:
	operators( const this_type& rhs )
		:base_type(rhs.val_)
	{}
	
	this_type& operator = ( const this_type& rhs){
		val_ = rhs.val_;
		return *this;
	}

	const static this_type sub_assign;
	const static this_type less;
	const static this_type bit_and;
	const static this_type bit_or_assign;
	const static this_type prefix_incr;
	const static this_type logic_and;
	const static this_type postfix_incr;
	const static this_type lshift_assign;
	const static this_type mul_assign;
	const static this_type prefix_decr;
	const static this_type bit_xor_assign;
	const static this_type sub;
	const static this_type positive;
	const static this_type rshift_assign;
	const static this_type negative;
	const static this_type logic_not;
	const static this_type add;
	const static this_type right_shift;
	const static this_type mul;
	const static this_type bit_and_assign;
	const static this_type mod_assign;
	const static this_type greater;
	const static this_type bit_or;
	const static this_type bit_not;
	const static this_type bit_xor;
	const static this_type add_assign;
	const static this_type mod;
	const static this_type none;
	const static this_type not_equal;
	const static this_type logic_or;
	const static this_type greater_equal;
	const static this_type left_shift;
	const static this_type equal;
	const static this_type postfix_decr;
	const static this_type div_assign;
	const static this_type less_equal;
	const static this_type div;
	const static this_type assign;


	static std::string to_name( const this_type& enum_val );
	static this_type from_name( const std::string& name );
	std::string name() const;

};

const operators operators::sub_assign ( UINT32_C( 8 ) );
const operators operators::less ( UINT32_C( 19 ) );
const operators operators::bit_and ( UINT32_C( 34 ) );
const operators operators::bit_or_assign ( UINT32_C( 13 ) );
const operators operators::prefix_incr ( UINT32_C( 25 ) );
const operators operators::logic_and ( UINT32_C( 32 ) );
const operators operators::postfix_incr ( UINT32_C( 27 ) );
const operators operators::lshift_assign ( UINT32_C( 15 ) );
const operators operators::mul_assign ( UINT32_C( 9 ) );
const operators operators::prefix_decr ( UINT32_C( 26 ) );
const operators operators::bit_xor_assign ( UINT32_C( 14 ) );
const operators operators::sub ( UINT32_C( 2 ) );
const operators operators::positive ( UINT32_C( 29 ) );
const operators operators::rshift_assign ( UINT32_C( 16 ) );
const operators operators::negative ( UINT32_C( 30 ) );
const operators operators::logic_not ( UINT32_C( 33 ) );
const operators operators::add ( UINT32_C( 1 ) );
const operators operators::right_shift ( UINT32_C( 24 ) );
const operators operators::mul ( UINT32_C( 3 ) );
const operators operators::bit_and_assign ( UINT32_C( 12 ) );
const operators operators::mod_assign ( UINT32_C( 11 ) );
const operators operators::greater ( UINT32_C( 21 ) );
const operators operators::bit_or ( UINT32_C( 35 ) );
const operators operators::bit_not ( UINT32_C( 37 ) );
const operators operators::bit_xor ( UINT32_C( 36 ) );
const operators operators::add_assign ( UINT32_C( 7 ) );
const operators operators::mod ( UINT32_C( 5 ) );
const operators operators::none ( UINT32_C( 0 ) );
const operators operators::not_equal ( UINT32_C( 18 ) );
const operators operators::logic_or ( UINT32_C( 31 ) );
const operators operators::greater_equal ( UINT32_C( 22 ) );
const operators operators::left_shift ( UINT32_C( 23 ) );
const operators operators::equal ( UINT32_C( 17 ) );
const operators operators::postfix_decr ( UINT32_C( 28 ) );
const operators operators::div_assign ( UINT32_C( 10 ) );
const operators operators::less_equal ( UINT32_C( 20 ) );
const operators operators::div ( UINT32_C( 4 ) );
const operators operators::assign ( UINT32_C( 6 ) );


#endif
