#include <salviau/include/common/path.h>

#include <eflib/include/string/string.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/filesystem.hpp>
#include <eflib/include/platform/boost_end.h>

using std::vector;

using std::string;
using std::wstring;

using boost::filesystem::wpath;
using boost::filesystem::path;

BEGIN_NS_SALVIAU();

template <typename StringT>
StringT find_path_impl(StringT const& relative_path, vector<StringT> const& candidates)
{
	for (auto const& parent_folder: candidates)
	{
		auto full_path = path(parent_folder) / path(relative_path);
		if (boost::filesystem::exists(full_path))
		{
			return full_path.generic_string<StringT>();
		}
	}
	return StringT();
}

string find_path(string const& relative_path)
{
	vector<string> candidate_folders
	{
		"./resources",
		"../resources",
		"../../resources",
		"../../../resources"
	};

	return find_path_impl(relative_path, candidate_folders);
}

wstring find_path(wstring const& relative_path)
{
	vector<wstring> candidate_folders
	{
		L"./resources",
		L"../resources",
		L"../../resources",
		L"../../../resources"
	};

	return find_path_impl(relative_path, candidate_folders);
}

string find_path(string const& relative_path, vector<string> const& candidates)
{
	return find_path_impl(relative_path, candidates);
}

wstring find_path(wstring const& relative_path, vector<wstring> const& candidates)
{
	return find_path_impl(relative_path, candidates);
}

END_NS_SALVIAU();
