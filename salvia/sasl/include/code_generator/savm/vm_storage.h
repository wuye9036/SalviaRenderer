#ifndef SASL_VM_VM_STORAGE_H
#define SASL_VM_VM_STORAGE_H

#include "../../enums/storage_mode.h"

template <typename AddressT> struct vm_storage{
	typedef AddressT address_t;
	
	vm_storage(storage_mode m, address_t addr): mode(m), addr(addr){}
	
	storage_mode mode;
	address_t addr;	
};

#endif //SASL_VM_VM_STORAGE_H