#ifndef EFLIB_UTILITY_SCOPED_VALUE
#define EFLIB_UTILITY_SCOPED_VALUE

namespace eflib
{
	template <typename T> class scoped_value
	{
	public:
		scoped_value(T& value_ref, T const& new_value): value_ref(value_ref), stored_value(value_ref)
		{
			value_ref = new_value;
		}
		~scoped_value()
		{
			value_ref = stored_value;
		}
	private:
		scoped_value<T>& operator = (scoped_value<T> const&);
		scoped_value<T>(scoped_value<T> const&);
		T&	value_ref;
		T	stored_value;
	};
}

#endif