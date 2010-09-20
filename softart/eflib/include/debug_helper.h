#ifndef EFLIB_DEBUG_HELPER_H
#define EFLIB_DEBUG_HELPER_H

#include "config.h"

#include <iostream>

#ifdef _DEBUG
#	define custom_assert(exp, desc) \
				{\
					static bool isIgnoreAlways = false;\
					if(!isIgnoreAlways) {\
					if((*efl::detail::ProcPreAssert)(exp?true:false, #exp, desc, __LINE__, __FILE__, __FUNCTION__, &isIgnoreAlways))\
						{ abort(); }\
					}\
				}
#else
#	define custom_assert(exp, desc)
#endif
namespace efl{
	const bool Debug_Interupt = false;
}

#define NO_IMPL() custom_assert(false, "该函数目前尚未实现！");
#define interupt(desc) custom_assert(efl::Debug_Interupt, desc)

namespace efl{
	namespace detail{
		extern bool (*ProcPreAssert)(bool exp, const char* expstr, const char* desc, int line, const char* file, const char* func, bool* ignore);

		bool ProcPreAssert_Init(bool exp, const char* expstr, const char* desc, int line, const char* file, const char* func, bool* ignore);
		bool ProcPreAssert_Defalut(bool exp, const char* expstr, const char* desc, int line, const char* file, const char* func, bool* ignore);
		bool ProcPreAssert_MsgBox(bool exp, const char* expstr, const char* desc, int line, const char* file, const char* func, bool* ignore);
	}

	template<class T>
	void print_vector(std::ostream& os, const T& v)
	{
                for(typename T::const_iterator cit = v.begin(); cit != v.end(); ++cit)
		{
			os << *cit << " ";
		}
	}
}

#endif
