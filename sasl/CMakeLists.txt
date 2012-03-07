option( SALVIA_BUILD_WITH_LLVM "Build llvm code generator & llvm test." TRUE )
option( SALVIA_ENABLE_SASL_REGRESSION_TEST "Regression test will combine all test to one test module." FALSE )
option( SALVIA_ENABLE_SASL_SEPERATED_TESTS "Seperated tests are enabled when regression test is used if set to true, otherwise seperated tests were disabled." FALSE )

SET( EFLIB_HOME_PATH ${SALVIA_HOME_DIR} )
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