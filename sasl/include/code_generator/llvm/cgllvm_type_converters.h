#ifndef SASL_CODE_GENERATOR_LLVM_CGLLVM_TYPE_CONVERTER_H
#define SASL_CODE_GENERATOR_LLVM_CGLLVM_TYPE_CONVERTER_H

#include <sasl/include/code_generator/forward.h>
#include <boost/shared_ptr.hpp>

namespace sasl{
	namespace semantic{
		class type_converter;
		class type_manager;
		class symbol;
	}
}

namespace llvm{
	class IRBuilderBase;
}

BEGIN_NS_SASL_CODE_GENERATOR();

boost::shared_ptr< ::sasl::semantic::type_converter> create_type_converter(
		boost::shared_ptr<llvm::IRBuilderBase> builder
		);

void register_buildin_typeconv(
	boost::shared_ptr< ::sasl::semantic::type_converter> typeconv,
	boost::shared_ptr< sasl::semantic::type_manager> typemgr
	);

END_NS_SASL_CODE_GENERATOR();

#endif
