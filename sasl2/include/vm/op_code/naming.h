#ifndef SASL_VM_OP_CODE_NAMING_H
#define SASL_VM_OP_CODE_NAMING_H

#include "utility.h"

/*************************************
	该文件提供了一组宏，用于根据指令名称、参数存储方式和类型信息生成唯一的指令名称。

	注意：
		如果虚拟机添加了新的存储方式和类型，
		需要在“指令名称参数存储方式修饰宏”和“指令名称参数类型修饰宏”两节中添加对应的名称修饰宏，
		使之能够正确的产生指令名称。
*************************************/

/**************************************
	指令名称参数存储方式修饰宏
**************************************/
#define SASL_S_NAME_MODIFIER__( NAME )			NAME
#define SASL_S_NAME_MODIFIER_stk( NAME )		BOOST_PP_CAT(NAME, _stk)
#define SASL_S_NAME_MODIFIER_gr( NAME )			BOOST_PP_CAT(NAME, _gr)
#define SASL_S_NAME_MODIFIER_fr( NAME )			BOOST_PP_CAT(NAME, _fr)
#define SASL_S_NAME_MODIFIER_dr( NAME )			BOOST_PP_CAT(NAME, _dr)
#define SASL_S_NAME_MODIFIER_a( NAME )			BOOST_PP_CAT(NAME, _a)
#define SASL_S_NAME_MODIFIER_c( NAME )			BOOST_PP_CAT(NAME, _c)
#define SASL_S_NAME_MODIFIER_ia( NAME )			BOOST_PP_CAT(NAME, _ia)
#define SASL_S_NAME_MODIFIER_igr( NAME )		BOOST_PP_CAT(NAME, _igr)


/**************************************
	指令名称参数类型修饰宏
**************************************/
#define SASL_T_NAME_MODIFIER__( NAME )			NAME
#define SASL_T_NAME_MODIFIER_i32( NAME )		BOOST_PP_CAT( NAME, _i32)
#define SASL_T_NAME_MODIFIER_i64( NAME )		BOOST_PP_CAT( NAME, _64)
#define SASL_T_NAME_MODIFIER_r( NAME )			BOOST_PP_CAT( NAME, _r )
#define SASL_T_NAME_MODIFIER_raw( NAME )		BOOST_PP_CAT(NAME, _raw)

/**************************************
	根据修饰后缀产生正确的修饰宏
**************************************/
#define SASL_S_NAME_MODIFIER_NAME( POSTFIX )	BOOST_PP_CAT( SASL_S_NAME_MODIFIER_, POSTFIX )
#define SASL_MODIFY_NAME_S( NAME, POSTFIX )		SASL_S_NAME_MODIFIER_NAME(POSTFIX)( NAME )

#define SASL_T_NAME_MODIFIER_NAME( POSTFIX )	BOOST_PP_CAT( SASL_T_NAME_MODIFIER_, POSTFIX )
#define SASL_MODIFY_NAME_T( NAME, POSTFIX )		SASL_T_NAME_MODIFIER_NAME(POSTFIX)( NAME )

/**************************************
	根据参数的存储方式和值类型修饰名称
**************************************/
#define SASL_MODIFY_NAME_ST( NAME, S, T )		SASL_MODIFY_NAME_T( SASL_MODIFY_NAME_S( NAME, S ), T )

/**************************************
	根据指令信息生成指令全称
	例如 SASL_INSTRUCTION_FULL_NAME_P(add, r, i32, r, i32) 将生成 add_r_i32_r_i32
**************************************/
#define SASL_INSTRUCTION_FULL_NAME_P( NAME, S0, T0, S1, T1)		\
	SASL_MODIFY_NAME_ST( SASL_MODIFY_NAME_ST( NAME, S0, T0 ), S1, T1 )

/**************************************
	根据元组方式表达的指令信息生成指令全称
**************************************/
#define SASL_INSTRUCTION_FULL_NAME_T( INSTRUCTION )	\
	SASL_CALL_EXPANDED_INSTRUCTION( SASL_INSTRUCTION_FULL_NAME_P, INSTRUCTION)
#define SASL_INSTRUCTION_SHORT_NAME_T( INSTRUCTION )			\
	BOOST_PP_TUPLE_ELEM( 5, 0, INSTRUCTION )

// SASL_INSTRUCTION_FULL_NAME_T 的别名
#define SASL_IFN( INSTRUCTION ) SASL_INSTRUCTION_FULL_NAME_T( INSTRUCTION )
#define SASL_ISN( INSTRUCTION ) SASL_INSTRUCTION_SHORT_NAME_T( INSTRUCTION )
#endif