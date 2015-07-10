#include <salviau/include/common/path.h>

#include <eflib/include/string/string.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/filesystem.hpp>
#include <eflib/include/platform/boost_end.h>

using std::_tstring;
using std::vector;

BEGIN_NS_SALVIAU();

#if defined(EFLIB_UNICODE)
	using _tpath = boost::filesystem::wpath;
#else
	using _tpath = boost::filesystem::path;
#endif

_tstring find_file(_tstring const& relative_path)
{
	vector<_tstring> candidate_folders
	{
		_EFLIB_T("./resources"),
		_EFLIB_T("../resources"),
		_EFLIB_T("../../resources"),
		_EFLIB_T("../../../resources")
	};

	return find_file(relative_path, candidate_folders);
}

_tstring find_file(_tstring const& relative_path, vector<_tstring> const& candidates)
{
	for (auto const& parent_folder: candidates)
	{
		auto full_path = _tpath(parent_folder) / _tpath(relative_path);
		if (boost::filesystem::exists(full_path))
		{
			return full_path.generic_string<_tstring>();
		}
	}
	return _tstring();
}

END_NS_SALVIAU();
