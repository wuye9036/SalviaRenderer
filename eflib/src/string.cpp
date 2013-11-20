#include <eflib/include/string/string.h>
#include <eflib/include/string/ustring.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/locale.hpp>
#include <eflib/include/platform/boost_end.h>

#include <cstdlib>
#include <vector>
#include <locale>

using namespace std;

namespace eflib{
	bool to_ansi_string(string& outstr, const wstring& instr){
		boost::locale::generator gen;
		outstr = boost::locale::conv::from_utf(instr, gen(""));
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
		boost::locale::generator gen;
		outstr = boost::locale::conv::to_utf<wchar_t>(instr, gen(""));
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
