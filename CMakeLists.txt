PROJECT( SoftArt )

CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

option(SALVIA_BUILD_WITH_UNICODE "Build with unicode(wide-byte) or ansi(multi-byte)." TRUE)


#set(CMAKE_USE_RELATIVE_PATHS TRUE)

set( SALVIA_HOME_PATH ${CMAKE_HOME_DIRECTORY} )
set( EFLIB_HOME_PATH ${SALVIA_HOME_PATH} )
include(${CMAKE_HOME_DIRECTORY}/cmake/SystemInfo.cmake)

if(MSVC)
	include(${CMAKE_HOME_DIRECTORY}/cmake/MSVC.cmake)
endif(MSVC)

ADD_SUBDIRECTORY( cmake )

ADD_SUBDIRECTORY( eflib )
ADD_SUBDIRECTORY( logrout )
ADD_SUBDIRECTORY( softart )
ADD_SUBDIRECTORY( softartx )
ADD_SUBDIRECTORY( samples )
