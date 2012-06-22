#ifndef SASL_COMMON_SCOPE_GUARD_H
#define SASL_COMMON_SCOPE_GUARD_H

#include <sasl/include/common/common_fwd.h>

BEGIN_NS_SASL_COMMON();

template <typename T>
class scope_guard
{
public:
	scope_guard(T& value_ref, T const& new_value): value_ref(value_ref), stored_value(value_ref)
	{
		value_ref = new_value;
	}
	~scope_guard()
	{
		value_ref = stored_value;
	}
private:
	scope_guard<T>& operator = (scope_guard<T> const&);
	scope_guard<T>(scope_guard<T> const&);
	T&	value_ref;
	T	stored_value;
};

END_NS_SASL_COMMON();

#endif