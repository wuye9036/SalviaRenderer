
#include "./operators.h"


void register_enum_name( std::function<void (char const*, operators)> const& reg_fn )
{
	reg_fn("sub_assign", operators::sub_assign);
	reg_fn("less", operators::less);
	reg_fn("bit_and", operators::bit_and);
	reg_fn("bit_or_assign", operators::bit_or_assign);
	reg_fn("prefix_incr", operators::prefix_incr);
	reg_fn("logic_and", operators::logic_and);
	reg_fn("postfix_incr", operators::postfix_incr);
	reg_fn("lshift_assign", operators::lshift_assign);
	reg_fn("mul_assign", operators::mul_assign);
	reg_fn("prefix_decr", operators::prefix_decr);
	reg_fn("bit_xor_assign", operators::bit_xor_assign);
	reg_fn("sub", operators::sub);
	reg_fn("positive", operators::positive);
	reg_fn("rshift_assign", operators::rshift_assign);
	reg_fn("negative", operators::negative);
	reg_fn("logic_not", operators::logic_not);
	reg_fn("add", operators::add);
	reg_fn("right_shift", operators::right_shift);
	reg_fn("mul", operators::mul);
	reg_fn("bit_and_assign", operators::bit_and_assign);
	reg_fn("mod_assign", operators::mod_assign);
	reg_fn("greater", operators::greater);
	reg_fn("bit_or", operators::bit_or);
	reg_fn("bit_not", operators::bit_not);
	reg_fn("bit_xor", operators::bit_xor);
	reg_fn("add_assign", operators::add_assign);
	reg_fn("mod", operators::mod);
	reg_fn("none", operators::none);
	reg_fn("not_equal", operators::not_equal);
	reg_fn("logic_or", operators::logic_or);
	reg_fn("greater_equal", operators::greater_equal);
	reg_fn("left_shift", operators::left_shift);
	reg_fn("equal", operators::equal);
	reg_fn("postfix_decr", operators::postfix_decr);
	reg_fn("div_assign", operators::div_assign);
	reg_fn("less_equal", operators::less_equal);
	reg_fn("div", operators::div);
	reg_fn("assign", operators::assign);

}

