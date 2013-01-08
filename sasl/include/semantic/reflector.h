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

class reflector
{
public:
	reflection_impl_ptr reflect(module_semantic_ptr const& sem, eflib::fixed_string const& entry_name);
	reflection_impl_ptr reflect(module_semantic_ptr const& sem);

private:
	reflection_impl_ptr do_reflect();

	bool add_semantic(
		sasl::syntax_tree::node_ptr const& v,
		bool is_member, bool enable_nested,
		salviar::languages lang, bool is_output_semantic
		);

	module_semantic*	sem_;
	symbol*				entry_;
	reflection_impl*	reflection_;
};

reflection_impl_ptr reflect(module_semantic_ptr const& sem);
reflection_impl_ptr reflect(module_semantic_ptr const& sem, eflib::fixed_string const& entry_name);
END_NS_SASL_SEMANTIC();

#endif