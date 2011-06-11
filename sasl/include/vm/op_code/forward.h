#ifndef SASL_VM_OP_CODE_FORWARD_H
#define SASL_VM_OP_CODE_FORWARD_H

#define BEGIN_NS_SASL_VM_OP_CODE() namespace sasl { namespace vm{ namespace op_code{
#define END_NS_SASL_VM_OP_CODE() }}}

#define BEGIN_NS_SASL_VM_OP_CODE_STORAGE() namespace sasl { namespace vm{ namespace op_code{ namespace storage{
#define END_NS_SASL_VM_OP_CODE_STORAGE() }}}}

#define BEGIN_NS_SASL_VM_OP_CODE_OPERAND_TYPE() namespace sasl { namespace vm{ namespace op_code{ namespace operand_type{
#define END_NS_SASL_VM_OP_CODE_OPERAND_TYPE() }}}}

#define NS_SASL_VM_OP_CODE( NAME ) BOOST_PP_CAT( sasl::vm::op_code::, NAME )
#define NS_SASL_VM_OP_CODE_STORAGE( NAME ) NS_SASL_VM_OP_CODE( BOOST_PP_CAT( storage::, NAME ) )
#define NS_SASL_VM_OP_CODE_OPERAND_TYPE( NAME ) NS_SASL_VM_OP_CODE( BOOST_PP_CAT ( operand_type::, NAME ) )

#endif