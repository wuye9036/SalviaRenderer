#include <eflib/include/platform/disable_warnings.h>
#include <LLVM/Support/IRBuilder.h>
#include <eflib/include/platform/enable_warnings.h>

#include <sasl/include/code_generator/llvm/cgllvm_type_converters.h>

#include <sasl/include/code_generator/llvm/cgllvm_contexts.h>
#include <sasl/include/semantic/semantic_infos.h>
#include <sasl/include/semantic/type_converter.h>
#include <sasl/include/syntax_tree/node.h>

using ::llvm::IRBuilderBase;
using ::llvm::IRBuilder;
using ::llvm::Value;

using ::sasl::semantic::symbol;
using ::sasl::semantic::type_converter;
using ::sasl::semantic::type_info_si;

using ::sasl::syntax_tree::node;
using ::sasl::syntax_tree::type_specifier;

using ::boost::make_shared;
using ::boost::shared_ptr;
using ::boost::shared_static_cast;

BEGIN_NS_SASL_CODE_GENERATOR();

class cgllvm_type_converter : public type_converter{
public:
	cgllvm_type_converter( shared_ptr<IRBuilderBase> builder )
		: builder( shared_static_cast<IRBuilder<> >(builder) )
	{
	}

	type_converter::conv_type int2int( shared_ptr<node> dest, shared_ptr<node> src ){
		shared_ptr<cgllvm_common_context> dest_ctxt = extract_codegen_context<cgllvm_common_context>(dest);
		shared_ptr<cgllvm_common_context> src_ctxt = extract_codegen_context<cgllvm_common_context>(src);

		Value* dest_v = builder->CreateIntCast( src_ctxt->val, dest_ctxt->type, dest_ctxt->is_signed );
		dest_ctxt->val = dest_v;
	}

	type_converter::conv_type int2float( shared_ptr<node> dest, shared_ptr<node> src ){
		shared_ptr<cgllvm_common_context> dest_ctxt = extract_codegen_context<cgllvm_common_context>(dest);
		shared_ptr<cgllvm_common_context> src_ctxt = extract_codegen_context<cgllvm_common_context>(src);

		Value* dest_v = NULL;
		if ( src_ctxt->is_signed ){
			dest_v = builder->CreateSIToFP( src_ctxt->val, dest_ctxt->type );
		} else {
			dest_v = builder->CreateUIToFP( src_ctxt->val, dest_ctxt->type );
		}
		dest_ctxt->val = dest_v;
	}

	type_converter::conv_type float2int( shared_ptr<node> dest, shared_ptr<node> src ){
		shared_ptr<cgllvm_common_context> dest_ctxt = extract_codegen_context<cgllvm_common_context>(dest);
		shared_ptr<cgllvm_common_context> src_ctxt = extract_codegen_context<cgllvm_common_context>(src);

		Value* dest_v = NULL;
		if ( dest_ctxt->is_signed ){
			dest_v = builder->CreateFPToSI( src_ctxt->val, dest_ctxt->type );
		} else {
			dest_v = builder->CreateFPToUI( src_ctxt->val, dest_ctxt->type );
		}
		dest_ctxt->val = dest_v;
	}

	type_converter::conv_type float2float( shared_ptr<node> dest, shared_ptr<node> src ){
		shared_ptr<cgllvm_common_context> dest_ctxt = extract_codegen_context<cgllvm_common_context>(dest);
		shared_ptr<cgllvm_common_context> src_ctxt = extract_codegen_context<cgllvm_common_context>(src);

		Value* dest_v = NULL;
		dest_v = builder->CreateFPCast( src_ctxt->val, dest_ctxt->type );
		dest_ctxt->val = dest_v;
	}
private:
	shared_ptr< IRBuilder<> > builder;
};

void register_buildin_typeconv(
	shared_ptr<type_converter> typeconv
	)
{
	//boost::shared_ptr<type_specifier> sint8_ts = create_buildin_type( buildin_type_code::_sint8 );
	//boost::shared_ptr<type_specifier> sint16_ts = create_buildin_type( buildin_type_code::_sint16 );
	//boost::shared_ptr<type_specifier> sint32_ts = create_buildin_type( buildin_type_code::_sint32 );
	//boost::shared_ptr<type_specifier> sint64_ts = create_buildin_type( buildin_type_code::_sint64 );

	//boost::shared_ptr<type_specifier> uint8_ts = create_buildin_type( buildin_type_code::_uint8 );
	//boost::shared_ptr<type_specifier> uint16_ts = create_buildin_type( buildin_type_code::_uint16 );
	//boost::shared_ptr<type_specifier> uint32_ts = create_buildin_type( buildin_type_code::_uint32 );
	//boost::shared_ptr<type_specifier> uint64_ts = create_buildin_type( buildin_type_code::_uint64 );

	//boost::shared_ptr<type_specifier> float_ts = create_buildin_type( buildin_type_code::_float );
	//boost::shared_ptr<type_specifier> double_ts = create_buildin_type( buildin_type_code::_double );

	//boost::shared_ptr<type_specifier> bool_ts = create_buildin_type( buildin_type_code::_boolean );

	//typeconv->register_converter( type_converter::implicit_conv, sint8_ts, sint16_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, sint8_ts, sint32_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, sint8_ts, sint64_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint8_ts, uint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint8_ts, uint16_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint8_ts, uint32_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint8_ts, uint64_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, sint8_ts, float_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, sint8_ts, double_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, sint8_ts, bool_ts, default_conv );

	//typeconv->register_converter( type_converter::explicit_conv, sint16_ts, sint8_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, sint16_ts, sint32_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, sint16_ts, sint64_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint16_ts, uint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint16_ts, uint16_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint16_ts, uint32_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint16_ts, uint64_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, sint16_ts, float_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, sint16_ts, double_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, sint16_ts, bool_ts, default_conv );

	//typeconv->register_converter( type_converter::explicit_conv, sint32_ts, sint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint32_ts, sint16_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, sint32_ts, sint64_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint32_ts, uint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint32_ts, uint16_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint32_ts, uint32_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint32_ts, uint64_ts, default_conv );
	//typeconv->register_converter( type_converter::warning_conv, sint32_ts, float_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, sint32_ts, double_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, sint32_ts, bool_ts, default_conv );

	//typeconv->register_converter( type_converter::explicit_conv, sint64_ts, sint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint64_ts, sint16_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint64_ts, sint32_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint64_ts, uint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint64_ts, uint16_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint64_ts, uint32_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, sint64_ts, uint64_ts, default_conv );
	//typeconv->register_converter( type_converter::warning_conv, sint64_ts, float_ts, default_conv );
	//typeconv->register_converter( type_converter::warning_conv, sint64_ts, double_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, sint64_ts, bool_ts, default_conv );

	//typeconv->register_converter( type_converter::explicit_conv, uint8_ts, sint8_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint8_ts, sint16_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint8_ts, sint32_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint8_ts, sint64_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint8_ts, uint16_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint8_ts, uint32_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint8_ts, uint64_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint8_ts, float_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint8_ts, double_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint8_ts, bool_ts, default_conv );

	//typeconv->register_converter( type_converter::explicit_conv, uint16_ts, sint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, uint16_ts, sint16_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint16_ts, sint32_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint16_ts, sint64_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, uint16_ts, uint8_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint16_ts, uint32_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint16_ts, uint64_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint16_ts, float_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint16_ts, double_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint16_ts, bool_ts, default_conv );

	//typeconv->register_converter( type_converter::explicit_conv, uint32_ts, sint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, uint32_ts, sint16_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, uint32_ts, sint32_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint32_ts, sint64_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, uint32_ts, uint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, uint32_ts, uint16_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint32_ts, uint64_ts, default_conv );
	//typeconv->register_converter( type_converter::warning_conv, uint32_ts, float_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint32_ts, double_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint32_ts, bool_ts, default_conv );

	//typeconv->register_converter( type_converter::explicit_conv, uint64_ts, sint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, uint64_ts, sint16_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, uint64_ts, sint32_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, uint64_ts, sint64_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, uint64_ts, uint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, uint64_ts, uint16_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, uint64_ts, uint32_ts, default_conv );
	//typeconv->register_converter( type_converter::warning_conv, uint64_ts, float_ts, default_conv );
	//typeconv->register_converter( type_converter::warning_conv, uint64_ts, double_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, uint64_ts, bool_ts, default_conv );

	//typeconv->register_converter( type_converter::explicit_conv, float_ts, sint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, float_ts, sint16_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, float_ts, sint32_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, float_ts, sint64_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, float_ts, uint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, float_ts, uint16_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, float_ts, uint32_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, float_ts, uint64_ts, default_conv );
	//typeconv->register_converter( type_converter::implicit_conv, float_ts, double_ts, default_conv );
	//typeconv->register_converter( type_converter::warning_conv, float_ts, bool_ts, default_conv );

	//typeconv->register_converter( type_converter::explicit_conv, double_ts, sint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, double_ts, sint16_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, double_ts, sint32_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, double_ts, sint64_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, double_ts, uint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, double_ts, uint16_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, double_ts, uint32_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, double_ts, uint64_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, double_ts, float_ts, default_conv );
	//typeconv->register_converter( type_converter::warning_conv, double_ts, bool_ts, default_conv );

	//typeconv->register_converter( type_converter::explicit_conv, double_ts, sint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, double_ts, sint16_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, double_ts, sint32_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, double_ts, sint64_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, double_ts, uint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, double_ts, uint16_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, double_ts, uint32_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, double_ts, uint64_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, double_ts, float_ts, default_conv );
	//typeconv->register_converter( type_converter::warning_conv, double_ts, bool_ts, default_conv );

	//typeconv->register_converter( type_converter::explicit_conv, bool_ts, sint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, bool_ts, sint16_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, bool_ts, sint32_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, bool_ts, sint64_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, bool_ts, uint8_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, bool_ts, uint16_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, bool_ts, uint32_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, bool_ts, uint64_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, bool_ts, float_ts, default_conv );
	//typeconv->register_converter( type_converter::explicit_conv, bool_ts, double_ts, default_conv );
}

shared_ptr<type_converter> create_type_converter( shared_ptr<IRBuilderBase> builder ){
	return make_shared<cgllvm_type_converter>( builder );
}

END_NS_SASL_CODE_GENERATOR();