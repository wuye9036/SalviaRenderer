
if(MSVC)
	include(${CMAKE_CURRENT_LIST_DIR}/MSVC.cmake)
endif(MSVC)

if(MINGW OR UNIX)
	include(${CMAKE_CURRENT_LIST_DIR}/GCC.cmake)
endif(MINGW OR UNIX)
