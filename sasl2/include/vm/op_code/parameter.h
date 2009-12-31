#ifndef SASL_VM_OP_CODE_PARAMETER_H
#define SASL_VM_OP_CODE_PARAMETER_H

BEGIN_NS_SASL_VM_OP_CODE()

template <typename StorageTag, typename ValueT, typename AddressT>
struct parameter{
	typedef StorageTag	storage_tag;
	typedef AddressT	address_t;

	parameter( address_t addr ): addr(addr){}
	address_t addr;
};

END_NS_SASL_VM_OP_CODE()

#endif