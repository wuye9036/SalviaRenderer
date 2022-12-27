# Add definitions that make MSVC much less annoying.
add_definitions(
# For some reason MS wants to deprecate a bunch of standard functions...
  -D_CRT_SECURE_NO_DEPRECATE
  -D_CRT_SECURE_NO_WARNINGS
  -D_CRT_NONSTDC_NO_DEPRECATE
  -D_CRT_NONSTDC_NO_WARNINGS
  -D_SCL_SECURE_NO_DEPRECATE
  -D_SCL_SECURE_NO_WARNINGS
)

# Tell MSVC to use the Unicode version of the Win32 APIs instead of ANSI.
add_definitions(
  -DUNICODE
  -D_UNICODE
)

# Disable not valuable warnings
#C4503: decorated name length exceeded, name was truncated
#C4251: 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
#C4275: non – DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'
#C4819: The file contains a character that cannot be represented in the current code page (number). Save the file in Unicode format to prevent data loss.
append("/W4 /wd4503 /wd4251 /wd4275 /wd4819" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
append("/EHsc /GR" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
append("/utf-8" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)

set(compile_flags "")
set(linker_flags "")

if(CMAKE_BUILD_TYPE_UP STREQUAL "DEBUG")
  append("/Od /Ob0 /RTC1 /MDd /GS /fp:precise /MP /GL /D_DEBUG" compile_flags)
  append("/INCREMENTAL /DEBUG" linker_flags)
else()
  append("/O1 /Ob1 /MD /GS- /fp:fast /DNDEBUG" compiler_flags)
  append("/INCREMENTAL:NO /OPT:REF /OPT:ICF /LTCG" linker_flags)
  if (CMAKE_BUILD_TYPE_UP STREQUAL "RELWITHDEBINFO")
  	append("/DEBUG" linker_flags)
  endif()
endif()

append(${compile_flags} CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UP})
append(${linker_flags}
  CMAKE_EXE_LINKER_FLAGS_${CMAKE_BUILD_TYPE_UP}
  CMAKE_EXE_MODULE_FLAGS_${CMAKE_BUILD_TYPE_UP}
  CMAKE_SHARED_LINKER_FLAGS_${CMAKE_BUILD_TYPE_UP}
)