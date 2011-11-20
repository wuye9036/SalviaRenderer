#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_INTRINS_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_INTRINS_H

#include <sasl/include/code_generator/forward.h>

#include <eflib/include/platform/boost_begin.h>
#include <boost/smart_ptr/scoped_ptr.hpp>
#include <eflib/include/platform/boost_end.h>

namespace llvm{
	class Function;
	class Module;
	class LLVMContext;
}

BEGIN_NS_SASL_CODE_GENERATOR();

class llvm_intrin_cache_impl;

class llvm_intrin_cache{
public:
	llvm_intrin_cache( llvm::Module* mod, llvm::LLVMContext& ctxt );
	~llvm_intrin_cache();

	virtual llvm::Function* get( char const* );

private:
	boost::scoped_ptr<llvm_intrin_cache_impl> impl;
};

END_NS_SASL_CODE_GENERATOR();


#endif