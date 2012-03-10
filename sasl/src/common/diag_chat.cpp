#include <sasl/include/common/diag_chat.h>

#include <eflib/include/diagnostics/assert.h>

using boost::shared_ptr;

BEGIN_NS_SASL_COMMON();

shared_ptr<diag_chat> diag_chat::create()
{
	EFLIB_ASSERT_UNIMPLEMENTED();
	return shared_ptr<diag_chat>();
}

END_NS_SASL_COMMON();