#pragma once

#if defined(DEBUG) || defined(_DEBUG)
#define EFLIB_DEBUG
#endif

#if defined(_UNICODE)
#define EFLIB_UNICODE
#endif

#ifdef _MSC_VER
#define EFLIB_MSVC 1
#define EFLIB_COMPILE_VER _MSC_VER
#elif defined(__MINGW32__)
#include <_mingw.h>
#define EFLIB_MINGW 1
#if defined(__MINGW64_VERSION_MAJOR)
#define EFLIB_MINGW64 1
// Fix the C++ linkage problem in GCC 4.8.2
#include <intrin.h>
#else
#define EFLIB_MINGW32
#endif
#elif defined(__clang__)
#define EFLIB_CLANG 1
#elif defined(__GNUC__)
#define EFLIB_GCC 1
#endif

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define EFLIB_WINDOWS 1
#endif

#if defined(__APPLE__)
#define EFLIB_MACOS 1
#endif

#if defined(__linux__)
#define EFLIB_LINUX 1
#endif

#ifdef EFLIB_WINDOWS
#ifndef BOOST_ALL_DYN_LINK
#define BOOST_ALL_DYN_LINK
#endif
#endif

#if defined(EFLIB_MSVC)
#if defined(_M_X64)
#define EFLIB_CPU_X64
#define EFLIB_COMPILER_TARGET x64
#elif defined(_M_IX86)
#define EFLIB_CPU_X86
#define EFLIB_COMPILER_TARGET x86
#else
#error Unknown CPU type.
#endif
#elif defined(EFLIB_GCC) || defined(EFLIB_MINGW) || defined(EFLIB_CLANG)
#if defined(__x86_64__)
#define EFLIB_CPU_X64 1
#define EFLIB_COMPILER_TARGET x64
#elif defined(__i386__)
#define EFLIB_CPU_X86 1
#define EFLIB_COMPILER_TARGET x86
#elif defined(__aarch64__)
#define EFLIB_CPU_AARCH64 1
#define EFLIB_COMPILER_TARGET aarch64
#else
#error Unknown CPU type.
#endif
#endif

#ifdef EFLIB_MSVC
#define EFLIB_ALIGN(x) __declspec(align(x))
#else
#define EFLIB_ALIGN(x) __attribute__((aligned(x)))
#endif

#if (defined(EFLIB_MSVC) && _HAS_CXX23) || __cplusplus > 202002L
#define EF_CONSTEXPR23 constexpr
#else
#define EF_CONSTEXPR23 inline
#endif

#if defined(EFLIB_MSVC)
#define EF_CONSTEXPR_STR constexpr
#else
#define EF_CONSTEXPR_STR inline
#endif

namespace eflib {
#if defined(EFLIB_DEBUG)
constexpr bool is_debug_mode = true;
#else
constexpr bool is_debug_mode = false;
#endif
}; // namespace eflib
