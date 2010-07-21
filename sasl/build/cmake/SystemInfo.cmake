# define a set of string with may-be useful readable name
# this file is meant to be included in a CMakeLists.txt
# not as a standalone CMake script
set(SOFTART_COMPILER_NAME "")
set(SOFTART_PLATFORM_NAME "")
set(SOFTART_PLATFORM_BITNESS "")

if(WIN32)
  # Compilers
  # taken from http://predef.sourceforge.net/precomp.html#sec34
  if(MSVC)
    if(MSVC_VERSION EQUAL 1200)
      set(SOFTART_COMPILER_NAME "vc-6.0")
    endif(MSVC_VERSION EQUAL 1200)
    if(MSVC_VERSION EQUAL 1300)
      set(SOFTART_COMPILER_NAME "vc-7.0")
    endif(MSVC_VERSION EQUAL 1300)
    if(MSVC_VERSION EQUAL 1310)
      set(SOFTART_COMPILER_NAME "vc-7.1") #Visual Studio 2003
    endif(MSVC_VERSION EQUAL 1310)
    if(MSVC_VERSION EQUAL 1400)
      set(SOFTART_COMPILER_NAME "vc-8.0") #Visual Studio 2005
    endif(MSVC_VERSION EQUAL 1400)
    if(MSVC_VERSION EQUAL 1500)
      set(SOFTART_COMPILER_NAME "vc-9.0") #Visual Studio 2008
    endif(MSVC_VERSION EQUAL 1500)
    if(MSVC_VERSION EQUAL 1600)
      set(SOFTART_COMPILER_NAME "vc-10.0") #Visual Studio 2010
    endif(MSVC_VERSION EQUAL 1600)
  endif(MSVC)
  if(MINGW)
    set(SOFTART_COMPILER_NAME "mgw-"${GCC_VERSION})
  endif(MINGW)
  if(CMAKE_GENERATOR MATCHES "Win64")
    set(SOFTART_PLATFORM_NAME "x64")
    set(SOFTART_PLATFORM_BITNESS 64)
  else(CMAKE_GENERATOR MATCHES "Win64")
    set(SOFTART_PLATFORM_NAME "Win32")
    set(SOFTART_PLATFORM_BITNESS 32)
  endif(CMAKE_GENERATOR MATCHES "Win64")
endif(WIN32)

if(UNIX)
  set(SOFTART_COMPILER_NAME "gcc-"${GCC_VERSION})
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(SOFTART_PLATFORM_NAME "x64")
    set(SOFTART_PLATFORM_BITNESS 64)
  else(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
    set(SOFTART_PLATFORM_NAME "x86")
    set(SOFTART_PLATFORM_BITNESS 32)
  endif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
endif(UNIX)
