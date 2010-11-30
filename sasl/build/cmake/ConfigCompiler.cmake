
if(MSVC)
	include(${CMAKE_HOME_DIRECTORY}/build/cmake/MSVC.cmake)
endif(MSVC)
if(MINGW)
	include(${CMAKE_HOME_DIRECTORY}/build/cmake/GCC.cmake)
endif(MINGW)