SET( EFLIB_HOME_PATH ${SALVIA_HOME_DIR} )

SET(SASL_HOME_DIR ${CMAKE_CURRENT_LIST_DIR}/../)

include( ConfigureSASLProject.cmake )

# Check CMAKE_BUILD_TYPE
if ( NOT MSVC AND NOT CMAKE_BUILD_TYPE )
	MESSAGE( WARNING "Please input a build type.( one of Debug, Release, MinSizeRel, RelWithDbgInfo)" )
	set( CMAKE_BUILD_TYPE "Debug" )
endif( )

# Add Projects
ADD_SUBDIRECTORY( enums )
ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( test )