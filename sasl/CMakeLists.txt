PROJECT( SASL )

CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

MESSAGE( STATUS "CMake Tested" )

option( SOFTART_BUILD_WITH_UNICODE "Build with unicode(wide-byte) or ansi(multi-byte)." TRUE )

include(${CMAKE_HOME_DIRECTORY}/build/cmake/SystemInfo.cmake)

if(MSVC)
	include(${CMAKE_HOME_DIRECTORY}/build/cmake/MSVC.cmake)
endif(MSVC)

ADD_DEFINITIONS(
	-DBOOST_ALL_DYN_LINK
)

ADD_SUBDIRECTORY( build/cmake )

ADD_SUBDIRECTORY( enums )
ADD_SUBDIRECTORY( test )
ADD_SUBDIRECTORY( src )