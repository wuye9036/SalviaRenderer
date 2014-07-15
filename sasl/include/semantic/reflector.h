#pragma once

#include <sasl/include/semantic/semantic_forward.h>

#include <salviar/include/shader.h>

#include <eflib/include/utility/shared_declaration.h>
#include <eflib/include/string/ustring.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

namespace sasl
{
	namespace common
	{
		class diag_chat;
	}

	namespace syntax_tree
	{
		EFLIB_DECLARE_STRUCT_SHARED_PTR(node);
	}
}

BEGIN_NS_SASL_SEMANTIC();

class symbol;
EFLIB_DECLARE_CLASS_SHARED_PTR(module_semantic);
EFLIB_DECLARE_CLASS_SHARED_PTR(reflection_impl);

reflection_impl_ptr reflect(
	module_semantic_ptr const& sem,
	sasl::common::diag_chat* diags
	);

reflection_impl_ptr reflect(
	module_semantic_ptr const& sem,
	eflib::fixed_string const& entry_name,
	sasl::common::diag_chat* diags
	);

END_NS_SASL_SEMANTIC();
