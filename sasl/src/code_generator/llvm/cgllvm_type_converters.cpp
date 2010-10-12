#include <eflib/include/disable_warnings.h>
#include <LLVM/Support/IRBuilder.h>
#include <eflib/include/enable_warnings.h>

#include <sasl/include/code_generator/llvm/cgllvm_type_converters.h>

#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/type_converter.h>
#include <sasl/include/syntax_tree/node.h>

using ::llvm::IRBuilderBase;
using ::llvm::IRBuilder;
using ::llvm::Value;

using ::sasl::semantic::type_converter;
using ::sasl::semantic::type_info_si;

using ::sasl::syntax_tree::node;

BEGIN_NS_SASL_CODE_GENERATOR();

class cgllvm_type_converter : public type_converter{
public:
	cgllvm_type_converter( boost::shared_ptr<IRBuilderBase> builder )
		: builder( boost::shared_static_cast<IRBuilder<> >(builder) )
	{
	}

	type_converter::conv_type int2int( boost::shared_ptr<node> dest, boost::shared_ptr<node> src ){
		boost::shared_ptr<cgllvm_common_context> dest_ctxt = extract_codegen_context<cgllvm_common_context>(dest);
		boost::shared_ptr<cgllvm_common_context> src_ctxt = extract_codegen_context<cgllvm_common_context>(src);

		Value* dest_v = builder->CreateIntCast( src_ctxt->val, dest_ctxt->type, dest_ctxt->is_signed );
		dest_ctxt->val = dest_v;
	}

	type_converter::conv_type int2float( boost::shared_ptr<node> dest, boost::shared_ptr<node> src ){
		boost::shared_ptr<cgllvm_common_context> dest_ctxt = extract_codegen_context<cgllvm_common_context>(dest);
		boost::shared_ptr<cgllvm_common_context> src_ctxt = extract_codegen_context<cgllvm_common_context>(src);

		Value* dest_v = NULL;
		if ( src_ctxt->is_signed ){
			dest_v = builder->CreateSIToFP( src_ctxt->val, dest_ctxt->type );
		} else {
			dest_v = builder->CreateUIToFP( src_ctxt->val, dest_ctxt->type );
		}
		dest_ctxt->val = dest_v;
	}

	type_converter::conv_type float2int( boost::shared_ptr<node> dest, boost::shared_ptr<node> src ){
		boost::shared_ptr<cgllvm_common_context> dest_ctxt = extract_codegen_context<cgllvm_common_context>(dest);
		boost::shared_ptr<cgllvm_common_context> src_ctxt = extract_codegen_context<cgllvm_common_context>(src);

		Value* dest_v = NULL;
		if ( dest_ctxt->is_signed ){
			dest_v = builder->CreateFPToSI( src_ctxt->val, dest_ctxt->type );
		} else {
			dest_v = builder->CreateFPToUI( src_ctxt->val, dest_ctxt->type );
		}
		dest_ctxt->val = dest_v;
	}

	type_converter::conv_type float2float( boost::shared_ptr<node> dest, boost::shared_ptr<node> src ){
		boost::shared_ptr<cgllvm_common_context> dest_ctxt = extract_codegen_context<cgllvm_common_context>(dest);
		boost::shared_ptr<cgllvm_common_context> src_ctxt = extract_codegen_context<cgllvm_common_context>(src);

		Value* dest_v = NULL;
		dest_v = builder->CreateFPCast( src_ctxt->val, dest_ctxt->type );
		dest_ctxt->val = dest_v;
	}
private:
	boost::shared_ptr< IRBuilder<> > builder;
};

void sasl::code_generator::register_buildin_typeconv( boost::shared_ptr<::sasl::semantic::symbol> global_sym, boost::shared_ptr<::sasl::semantic::type_converter> typeconv )
{
}

boost::shared_ptr<::sasl::semantic::type_converter> create_type_converter( boost::shared_ptr<IRBuilderBase> builder ){
	return boost::make_shared<cgllvm_type_converter>( builder );
}

END_NS_SASL_CODE_GENERATOR();