#ifndef SASL_SEMANTIC_REFLECTOR_H
#define SASL_SEMANTIC_REFLECTOR_H

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

BEGIN_NS_SASL_SEMANTIC();

class symbol;
EFLIB_DECLARE_CLASS_SHARED_PTR(module_semantic);
EFLIB_DECLARE_CLASS_SHARED_PTR(reflection_impl);

reflection_impl_ptr reflect(module_semantic_ptr const& sem);
reflection_impl_ptr reflect(module_semantic_ptr const& sem, eflib::fixed_string const& entry_name);

END_NS_SASL_SEMANTIC();

#endif