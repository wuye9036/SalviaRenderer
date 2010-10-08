
#ifndef SASL_ENUMS_OPERATORS_H
#define SASL_ENUMS_OPERATORS_H

#include "../enums/enum_base.h" 

struct operators :
	public enum_base< operators, int >
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
#endif
