#include <eflib/include/platform/config.h>
#include <eflib/include/diagnostics/assert.h>
#include <eflib/include/common/initialize.h>

#ifdef EFLIB_WINDOWS
#	define EFLIB_INCLUDE_WINDOWS_H
#	include <eflib/include/platform/win32.h>
#endif

#include <stdio.h>

namespace eflib{
	namespace detail{
		bool (*ProcPreAssert)(bool exp, const char* expstr, const char* desc, int line, const char* file, const char* func, bool* ignore) = 
			&ProcPreAssert_Init;

		bool ProcPreAssert_Init(bool exp, const char* expstr, const char* desc, int line, const char* file, const char* func, bool* ignore)
		{
			eflib::detail::do_init();
			return (*ProcPreAssert)(exp, expstr, desc, line, file, func, ignore);
		}

		bool ProcPreAssert_Defalut(bool exp, const char* /*expstr*/, const char* /* desc */, int /* line */, const char* /* file */, const char* /* func */, bool* /* ignore */)
		{
			if(exp) return false;
			return true;
		}
		
		#ifdef EFLIB_WINDOWS
		bool ProcPreAssert_MsgBox(bool exp, const char* expstr, const char* desc, int line, const char* file, const char* func, bool* ignore)
		{
			if(exp) return false;
			static char buf[1024];
			sprintf(buf, " Expression: %s \r\n Description: %s \r\n SourceFile: %s \r\n Line: %05d \r\n Function: %s \r\n ", expstr, desc, file, line, func);  
			int rv = MessageBoxA(NULL, buf, "Assert!", MB_ABORTRETRYIGNORE | MB_SYSTEMMODAL);
			switch (rv)
			{
			case IDABORT:
				::exit(-1);
			case IDIGNORE:
				*ignore = true;
				return false;
			case IDRETRY:
				return true;
			default:
				return true;
			}
		}
		#endif
	}
}