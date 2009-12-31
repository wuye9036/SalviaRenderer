#ifndef SASL_VM_OP_CODE_STORAGE_H
#define SASL_VM_OP_CODE_STORAGE_H

#include "forward.h"

BEGIN_NS_SASL_VM_OP_CODE_STORAGE()

/**********************
	storage type tags
**********************/
struct fr;	// float register
struct gr;	// general register
struct dr;	// double register

struct stk;	// stack
struct a;	// absolute( address )
struct c;	// constant

struct _;	// no storage( for unused param )

END_NS_SASL_VM_OP_CODE_STORAGE()

#endif