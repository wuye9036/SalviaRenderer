PROJECT( SASL )

CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

option( SOFTART_BUILD_WITH_UNICODE "Build with unicode(wide-byte) or ansi(multi-byte)." TRUE )
option( SOFTART_BUILD_WITH_LLVM "Build llvm code generator & llvm test." TRUE )
option( SOFTART_ENABLE_SASL_REGRESSION_TEST "Regression test will combine all test to one test module." FALSE )
option( SOFTART_ENABLE_SASL_SEPERATED_TESTS_ALWAYS "Seperated tests are enabled when regression test is used if set to true, otherwise seperated tests were disabled." FALSE )

if ( NOT MSVC AND NOT CMAKE_BUILD_TYPE )
	MESSAGE( WARNING "Please input a build type.( one of Debug, Release, MinSizeRel, RelWithDbgInfo)" )
	set( CMAKE_BUILD_TYPE "Debug" )
endif( )

include (${CMAKE_HOME_DIRECTORY}/build/cmake/SystemInfo.cmake)
include (${CMAKE_HOME_DIRECTORY}/build/cmake/ConfigCompiler.cmake)
include (${CMAKE_HOME_DIRECTORY}/build/cmake/Variables.cmake)
include (${CMAKE_HOME_DIRECTORY}/build/cmake/ConfigurePath.cmake)
include (${CMAKE_HOME_DIRECTORY}/build/cmake/Environment.cmake)
include (${CMAKE_HOME_DIRECTORY}/build/cmake/ConfigBoostLibs.cmake)

set ( EFLIB_HOME_PATH ${SOFTART_HOME_DIR} )

ADD_SUBDIRECTORY( build/cmake )
ADD_SUBDIRECTORY( "${SOFTART_HOME_DIR}/eflib" "${CMAKE_CURRENT_BINARY_DIR}/eflib" )
ADD_SUBDIRECTORY( "${SOFTART_HOME_DIR}/logrout" "${CMAKE_CURRENT_BINARY_DIR}/logrout" )
ADD_SUBDIRECTORY( enums )
ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( test )