#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_IMPL_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_IMPL_H

#include <sasl/include/code_generator/forward.h>

#include <sasl/include/syntax_tree/visitor.h>
#include <sasl/include/code_generator/llvm/cgs_sisd.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <eflib/include/platform/boost_end.h>

#include <string>

namespace sasl{
	namespace syntax_tree{
		struct node;
	}
	namespace semantic{
		class module_si;
		class abi_info;
		class caster_t;
	}
}

namespace llvm{

	class ConstantFolder;
	class LLVMContext;
	class Module;
	class Type;
	
	template <bool preserveNames> class IRBuilderDefaultInserter;
	template< bool preserveNames, typename T, typename Inserter
        > class IRBuilder;
    typedef IRBuilder<true, ConstantFolder, IRBuilderDefaultInserter<true> >
        DefaultIRBuilder;
}

struct builtin_types;

BEGIN_NS_SASL_CODE_GENERATOR();

class llvm_module;
class llvm_module_impl;
class cgllvm_sctxt;
typedef cgllvm_sctxt* sctxt_handle;

class cgllvm : public sasl::syntax_tree::syntax_tree_visitor{
public:
	virtual bool generate(
		sasl::semantic::module_si* mod,
		sasl::semantic::abi_info const* abii
		) = 0;
};

class cgllvm_impl: public cgllvm{
public:
	boost::shared_ptr<llvm_module> cg_module() const;
	bool generate(
		sasl::semantic::module_si* mod,
		sasl::semantic::abi_info const* abii
		);

	// Get context by node.
	cgllvm_sctxt* node_ctxt( sasl::syntax_tree::node* n, bool create_if_need = false );

protected:
	cgllvm_impl(): abii(NULL), msi(NULL){}

	SASL_VISIT_DCL( declaration ){}
	SASL_VISIT_DCL( program );

	/// It is called in program visitor BEFORE declaration was visited.
	/// If any additional initialization you want to add before visit, override it.
	/// DONT FORGET call parent function before your code.
	SASL_SPECIFIC_VISIT_DCL( before_decls_visit, program ) = 0;
	SASL_SPECIFIC_VISIT_DCL( process_intrinsics, program );

	// Easy to visit child with context data.
	template <typename NodeT> boost::any& visit_child( boost::any& child_ctxt, const boost::any& child_ctxt_init, boost::shared_ptr<NodeT> const& child );
	template <typename NodeT> boost::any& visit_child( boost::any& child_ctxt, boost::shared_ptr<NodeT> const& child );

	template <typename NodeT>
	cgllvm_sctxt* node_ctxt( boost::shared_ptr<NodeT> const&, bool create_if_need = false );

	// Direct access member from module.
	llvm::DefaultIRBuilder*	builder() const;
	llvm::LLVMContext&		context() const;
	llvm::Module*			module() const;
	
protected:
	virtual cg_service*		service() const = 0;

	// Store global informations
	sasl::semantic::module_si* msi;
	sasl::semantic::abi_info const* abii;

	boost::shared_ptr<llvm_module_impl> mod;
	
	// For type conversation.
	boost::shared_ptr< ::sasl::semantic::caster_t > caster;

	// Store node-context pairs.
	typedef boost::unordered_map< sasl::syntax_tree::node*, boost::shared_ptr<cgllvm_sctxt> > ctxts_t;
	ctxts_t ctxts ;
};

END_NS_SASL_CODE_GENERATOR()

#endif
