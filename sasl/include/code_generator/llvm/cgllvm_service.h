#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_SERVICE_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_SERVICE_H

#include <sasl/include/code_generator/forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/function.hpp>
#include <eflib/include/platform/boost_end.h>

namespace llvm{
	class LLVMContext;
	class Module;

	template <bool preserveNames> class IRBuilderDefaultInserter;
	template< bool preserveNames, typename T, typename Inserter
	> class IRBuilder;
	class ConstantFolder;
	typedef IRBuilder<true, ConstantFolder, IRBuilderDefaultInserter<true> >
		DefaultIRBuilder;
}

namespace sasl{
	namespace syntax_tree{
		struct node;
	}
}

BEGIN_NS_SASL_CODE_GENERATOR();

class value_t;
class cgllvm_sctxt;
class llvm_module_impl;

class cg_service
{
public:
	typedef boost::function< cgllvm_sctxt* (sasl::syntax_tree::node*, bool) > node_ctxt_fn;
	virtual bool initialize( llvm_module_impl* mod, node_ctxt_fn const& fn );
	
	llvm::Module*			module () const;
	llvm::LLVMContext&		context() const;
	llvm::DefaultIRBuilder& builder() const;

protected:
	node_ctxt_fn			node_ctxt;
	llvm_module_impl*		mod_impl;
};

END_NS_SASL_CODE_GENERATOR();

#endif