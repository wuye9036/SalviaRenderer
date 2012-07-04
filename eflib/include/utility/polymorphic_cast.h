#ifndef EFLIB_UTILITY_POLYMORPHIC_CAST_H
#define EFLIB_UTILITY_POLYMORPHIC_CAST_H

#include <cassert>

namespace eflib
{
	template <typename T, typename U>
	T polymorphic_cast(U const* v)
	{
		T ret = NULL;

#if defined(EFLIB_DEBUG)
		ret = dynamic_cast<T>(v);
		if(v) { assert(ret); }
#endif

		ret = static_cast<T>(v);
		return ret;
	}

	template <typename T, typename U>
	T polymorphic_cast(U* v)
	{
		T ret = NULL;

#if defined(EFLIB_DEBUG)
		ret = dynamic_cast<T>(v);
		if(v) { assert(ret); }
#endif

		ret = static_cast<T>(v);
		return ret;
	}
}

#endif