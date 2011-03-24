#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_IMPL_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_IMPL_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/include/syntax_tree/visitor.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

namespace sasl{
	namespace semantic{
		class module_si;
		class abi_info;
	}
}

BEGIN_NS_SASL_CODE_GENERATOR();

class llvm_code;

class cgllvm : public sasl::syntax_tree::syntax_tree_visitor{
public:
	virtual bool generate(
		sasl::semantic::module_si* mod,
		sasl::semantic::abi_info const* abii
		) = 0;

	virtual boost::shared_ptr<llvm_code> module() = 0;
};

class cgllvm_impl: public cgllvm{
public:

protected:
	template <typename NodeT> boost::any& visit_child( boost::any& child_ctxt, const boost::any& child_ctxt_init, boost::shared_ptr<NodeT> child );
	template <typename NodeT> boost::any& visit_child( boost::any& child_ctxt, boost::shared_ptr<NodeT> child );

};

END_NS_SASL_CODE_GENERATOR()

#endif
