#ifndef SASL_VM_OP_CODE_OPERAND_TYPE_H
#define SASL_VM_OP_CODE_OPERAND_TYPE_H

#include "forward.h"
#include <eflib/include/platform.h>
#include <boost/preprocessor/seq.hpp>

/***********************************
	创建指令参数的类型的类型Tag到实际类型的转换
***********************************/

#define SASL_MAPPING_OPERAND_TAG_AND_TYPE_I( r, data, elem ) SASL_MAPPING_OPERAND_TAG_AND_TYPE_T( elem )
#define SASL_MAPPING_OPERAND_TAG_AND_TYPE_T( TAG_TYPE )	SASL_MAPPING_OPERAND_TAG_AND_TYPE_P TAG_TYPE
#define SASL_MAPPING_OPERAND_TAG_AND_TYPE_P( TAG, TYPE ) struct TAG { typedef TYPE type; };

#define SASL_MAPPING_OPERAND_TAG_AND_TYPES( TAG_TYPE_SEQ ) BOOST_PP_SEQ_FOR_EACH( SASL_MAPPING_OPERAND_TAG_AND_TYPE_I, _, TAG_TYPE_SEQ )

BEGIN_NS_SASL_VM_OP_CODE_OPERAND_TYPE()

template <typename MachineT> struct operand_types{
	struct null{};
	SASL_MAPPING_OPERAND_TAG_AND_TYPES(
		((_, null))
		((i32, int32_t))
		((i64, int64_t))
		((r, typename MachineT::raw_t))
		((raw, typename MachineT::raw_t))
		);
};

#define SASL_OPERAND_TYPE_FULL_TAG( TAG, MACHINE_T ) BOOST_PP_CAT( NS_SASL_VM_OP_CODE_OPERAND_TYPE( operand_types< MACHINE_T > )::, TAG )
#define SASL_OTFT( TAG, MACHINE_T ) SASL_OPERAND_TYPE_FULL_TAG( TAG, MACHINE_T )
#define SASL_OPERAND_TYPE( TAG, MACHINE_T ) SASL_OTFT(TAG, MACHINE_T)::type

END_NS_SASL_VM_OP_CODE_OPERAND_TYPE()

#endif