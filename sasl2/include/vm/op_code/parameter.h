#ifndef SASL_VM_OP_CODE_PARAMETER_H
#define SASL_VM_OP_CODE_PARAMETER_H

#include "forward.h"
#include "storage.h"
#include "operand_type.h"

#include <boost/mpl/if.hpp>
#include <boost/mpl/or.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_base_of.hpp>

BEGIN_NS_SASL_VM_OP_CODE()

struct parameter_base{
};

template <typename T>
struct is_parameter: public boost::is_base_of<parameter_base, T>
{};

template <typename StorageTag, typename ValueT, typename MachineT>
struct parameter: public parameter_base{
public:
	typedef StorageTag						storage_tag;
	typedef ValueT							value_t_tag;
	typedef typename MachineT::operand_t	operand_t;
	typedef typename ValueT::type			value_t;
	
private:
	typedef boost::is_same< storage_tag, SASL_SFN(gr) > is_gr_storage;
	typedef boost::is_same< storage_tag, SASL_SFN(fr) > is_fr_storage;
	typedef boost::is_same< storage_tag, SASL_SFN(dr) > is_dr_storage;
	typedef boost::mpl::or_< is_gr_storage, boost::mpl::or_< is_fr_storage, is_dr_storage > > is_r_storage;

public:
	// 如果输入的存储是寄存器，则选用寄存器号作为输入。
	// 如果输入的存储是其他类型，则选用类型本身作为输入类型。
	// 该类型主要为了支持代码构造器产生强类型的辅助代码构造函数。
	typedef boost::mpl::if_<is_r_storage, typename SASL_OPERAND_TYPE(r, MachineT), ValueT > input_t;

	parameter( operand_t addr ): addr(addr){}
	operand_t addr;
};

END_NS_SASL_VM_OP_CODE()

#endif

#define SASL_AGUMENT_VALUE_TO_PARAMETER( INSTRUCTION, PAR_I, MACHINE_T, VALUE )	\
	SASL_FULL_PARAMETER_TYPE( INSTRUCTION, PAR_I, MACHINE_T )( VALUE )