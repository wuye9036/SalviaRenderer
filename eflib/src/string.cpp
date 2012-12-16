#include <eflib/include/string/string.h>
#include <eflib/include/string/ustring.h>

#include <cstdlib>
#include <vector>

using namespace std;

namespace eflib{
	bool to_ansi_string(string& outstr, const wstring& instr){
		// Preserve space for converting.
#ifdef EFLIB_MSVC
		size_t required;
		wcstombs_s(&required, NULL, 0, instr.c_str(), 0);
#else
		size_t required = std::wcstombs(NULL, instr.c_str(), 0);
#endif
		outstr.resize(required);

		// Convert.
#ifdef EFLIB_MSVC
		size_t l;
		wcstombs_s(&l, &(outstr[0]), outstr.size(), instr.c_str(), required-1);
#else
		size_t l = std::wcstombs(&(outstr[0]), instr.c_str(), required-1);
#endif
		return true;
	}

	bool to_ansi_string(string& outstr, const string& instr){
		outstr = instr;
		return true;
	}

	string to_ansi_string(const wstring& instr){
		std::string ret;
		to_ansi_string( ret, instr );
		return ret;
	}

	string to_ansi_string(const string& instr){
		return instr;
	}

	bool to_wide_string(wstring& outstr, const string& instr)
	{
		// Preserve space for converting.
#ifdef EFLIB_MSVC
		size_t required;
		mbstowcs_s(&required, NULL, 0, instr.c_str(), 0);
#else
		size_t required = mbstowcs(NULL, instr.c_str(), 0);
#endif
		outstr.resize(required);

		// Convert.
#ifdef EFLIB_MSVC
		size_t len;
		mbstowcs_s(&len, &(outstr[0]), required, instr.c_str(), required-1);
#else
		size_t len = mbstowcs(&(outstr[0]), instr.c_str(), required-1);
#endif

		outstr.resize(required-1);
		return true;
	}

	bool to_wide_string(wstring& outstr, const wstring& instr){
		outstr = instr;
		return true;
	}

	wstring to_wide_string(const wstring& instr){
		return instr;
	}

	wstring to_wide_string(const string& instr){
		wstring ret;
		to_wide_string( ret, instr );
		return ret;
	}
}
