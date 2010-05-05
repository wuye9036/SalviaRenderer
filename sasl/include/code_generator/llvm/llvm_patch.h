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