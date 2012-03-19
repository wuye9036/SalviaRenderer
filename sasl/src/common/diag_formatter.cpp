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
			return ( format("%s(%d): error C%04d: %s") % item->file() % item->span().line_beg % item->id() % item->str() ).str();
		case dl_fatal_error:
			// EFLIB_ASSERT_UNIMPLEMENTED();
			return item->str();
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

