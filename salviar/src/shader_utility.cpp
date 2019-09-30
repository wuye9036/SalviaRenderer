#include <salviar/include/shader_utility.h>

BEGIN_NS_SALVIAR()

namespace shader_constant
{
	constexpr size_t types_count = std::tuple_size<shader_constant_types>::value;
	std::array<assign_impl_functype, types_count> assign_impl_table;

	template <size_t I>
	struct initialize_element
	{
		initialize_element()
		{
			using data_type = std::tuple_element<I, shader_constant_types>::type;
			assign_impl_table[I] = &assign_impl<data_type>;
		}
	};

	template <size_t I>
	struct initialize_range_0_to_I
		: public initialize_range_0_to_I<I-1>
		, public initialize_element<I> {};

	template <> struct initialize_range_0_to_I<0>
		: public initialize_element<0> {};

	struct assign_impl_table_initializer
		: public initialize_range_0_to_I<types_count-1>{};

	assign_impl_table_initializer initializer_obj;
}

END_NS_SALVIAR()
