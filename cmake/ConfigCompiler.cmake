
if(MSVC)
	include(${CMAKE_CURRENT_LIST_DIR}/MSVC.cmake)
endif(MSVC)

if(MINGW)
	include(${CMAKE_CURRENT_LIST_DIR}/GCC.cmake)
endif(MINGW)