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
	namespace syntax_tree
	{
		EFLIB_DECLARE_STRUCT_SHARED_PTR(node);
	}
}

namespace salviar
{
	EFLIB_DECLARE_CLASS_SHARED_PTR(shader_reflection2);
}

BEGIN_NS_SASL_SEMANTIC();

class symbol;
EFLIB_DECLARE_CLASS_SHARED_PTR(module_semantic);

salviar::shader_reflection2_ptr reflect2(module_semantic_ptr const& sem);
salviar::shader_reflection2_ptr reflect2(module_semantic_ptr const& sem, eflib::fixed_string const& entry_name);

END_NS_SASL_SEMANTIC();