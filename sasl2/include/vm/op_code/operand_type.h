#ifndef SASL_VM_OP_CODE_OPERAND_TYPE_H
#define SASL_VM_OP_CODE_OPERAND_TYPE_H

#include "forward.h"
#include <boost/preprocessor/tuple.hpp>

/***********************************
	创建指令参数的类型的类型Tag到实际类型的转换
***********************************/
#define SASL_MAPPING_OPERAND_TAG_AND_TYPES( TAG_TYPE_SEQ ) BOOST_PP_SEQ_FOR_EACH( SASL_MAPPING_OPERAND_TAG_AND_TYPE_I, _, SEQ )
#define SASL_MAPPING_OPERAND_TAG_AND_TYPE_I( r, data, elem ) SASL_MAPPING_OPERAND_TAG_AND_TYPE_T( elem )
#define SASL_MAPPING_OPERAND_TAG_AND_TYPE_T( TAG_TYPE )	SASL_MAPPING_OPERAND_TAG_AND_TYPE_P( BOOST_PP_TUPLE_REM(2) TAG_TYPE )
#define SASL_MAPPING_OPERAND_TAG_AND_TYPE_P( TAG, TYPE ) struct TAG { typedef TYPE type; }

BEGIN_NS_SASL_VM_OP_CODE_OPERAND_TYPE()

template <typename MachineT> struct operand_types{
	struct null;
	SASL_DEFINE_PARAMETER_TYPES(
							(_,   null)
							(i32, int32_t)
							(i64, int64_t)
							(raw, MachineT::raw_t)
							);
};

#define SASL_OPERAND_TYPE( TAG, MACHINE_T ) BOOST_PP_CAT( NS_SASL_VM_OP_CODE_OPERAND_TYPE( operand_types< MACHINE_T > )::, TAG )::type

END_NS_SASL_VM_OP_CODE_OPERAND_TYPE()

#endif