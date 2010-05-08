// This patch will resolve some compile time error.
//********************************************************
// PLEASE INCLUDE THIS FILE BEFORE ANY LLVM HEADERS
//********************************************************

// This patch will be fixed std integer type redefinition error while include eflib.typedef before llvm
#if defined(_MSC_VER) && defined(BOOST_CSTDINT_HPP)
#define SUPPORT_DATATYPES_H

#if __GNUC__ > 3
#define END_WITH_NULL __attribute__((sentinel))
#else
#define END_WITH_NULL
#endif

#ifndef HUGE_VALF
#define HUGE_VALF (float)HUGE_VAL
#endif

#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning( disable: 4800 4146 )
#endif

#if defined(LLVM_PATCH_BEGIN)
#	error Including "llvm_patch_begin.h" without "llvm_patch_end.h" is invalid.
#else
#	define LLVM_PATCH_BEGIN
#endif