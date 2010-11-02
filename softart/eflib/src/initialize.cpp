#include <eflib/include/diagnostics/assert.h>

namespace eflib{
	namespace detail{
		void do_init()
		{
			//此处添加初始化代码

			//initialize debug helper
			#ifndef EFLIB_WINDOWS
			eflib::detail::ProcPreAssert = &eflib::detail::ProcPreAssert_Defalut;
			#else
			eflib::detail::ProcPreAssert = &eflib::detail::ProcPreAssert_MsgBox;
			#endif
		}
	}
}