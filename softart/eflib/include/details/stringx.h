#ifndef EFLIB_STRINGX_H
#define EFLIB_STRINGX_H

#include "../config.h"

#include <string>
namespace std{
	#ifdef EFLIB_UNICODE
		typedef wchar_t _tchar;
		typedef std::wstring _tstring;
	#else
		typedef char _tchar;
		typedef std::string _tstring;
	#endif
}

#define _EFLIB_U(str) L#str
#ifdef EFLIB_UNICODE
	#define _EFLIB_T(str) L##str
#else
	#define _EFLIB_T(str) str
#endif

namespace std{
	std::string to_ansi_string(const std::wstring& instr);
	std::string to_ansi_string(const std::string& instr);
	bool to_ansi_string(std::string& outstr, const std::wstring& instr);
	bool to_ansi_string(std::string& outstr, const std::string& instr);

	std::wstring to_wide_string(const std::wstring& instr);
	std::wstring to_wide_string(const std::string& instr);
	bool to_wide_string(std::wstring& outstr, const std::string& instr);
	bool to_wide_string(std::wstring& outstr, const std::wstring& instr);

	#ifdef EFLIB_UNICODE
		#define to_tstring(instr) to_wide_string(instr)
		#define to_tstring2(outstr, instr) to_wide_string(outstr, instr)
	#else
		#define to_tstring(instr) to_ansi_string(instr)
		#define to_tstring2(outstr, instr) to_ansi_string(outstr, instr)
	#endif

	template<class Container>
	Container& push_null(Container& ct, size_t num){
		ct.resize(ct.size() + num);
		return ct;
	}
}

#endif
