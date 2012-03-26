#include <sasl/include/common/diag_formatter.h>

#include <sasl/include/common/diag_item.h>
#include <eflib/include/diagnostics/assert.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/format.hpp>
#include <eflib/include/platform/boost_end.h>

using std::string;
using boost::format;

BEGIN_NS_SASL_COMMON();

string str( diag_item const* item, compiler_compatibility cc )
{
	std::string error_level;

	switch ( item->level() )
	{
	case dl_info:
		error_level = "info";
		break;
	case dl_warning:
		error_level = "warning";
		break;
	case dl_error:
		error_level = "error";
		break;
	case dl_fatal_error:
		error_level = "fatal error";
		break;
	}

	switch(cc)
	{
	case cc_msvc:
		switch ( item->level() )
		{
		case dl_text:
			return item->str();
		case dl_info:
		case dl_warning:
		case dl_error:
		case dl_fatal_error:
			return ( format("%s(%d): %s C%04d: %s") % item->file() % item->span().line_beg % error_level % item->id() % item->str() ).str();
		}
		
		break;
	case cc_gcc:
		EFLIB_ASSERT_UNIMPLEMENTED();
		return item->str();
		break;
	}

	EFLIB_ASSERT_UNIMPLEMENTED();
	return item->str();
}

END_NS_SASL_COMMON();

