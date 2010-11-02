PROJECT( SASL )

CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

option( SOFTART_BUILD_WITH_UNICODE "Build with unicode(wide-byte) or ansi(multi-byte)." TRUE )
option( SOFTART_BUILD_WITH_LLVM "Build llvm code generator & llvm test." TRUE )
option( SOFTART_ENABLE_SASL_REGRESSION_TEST "Regression test will combine all test to one test module." FALSE )
option( SOFTART_ENABLE_SASL_SEPERATED_TESTS_ALWAYS "Seperated tests are enabled when regression test is used if set to true, otherwise seperated tests were disabled." FALSE )

include(${CMAKE_HOME_DIRECTORY}/build/cmake/SystemInfo.cmake)
if(MSVC)
	include(${CMAKE_HOME_DIRECTORY}/build/cmake/MSVC.cmake)
endif(MSVC)
include (${CMAKE_HOME_DIRECTORY}/build/cmake/Variables.cmake)
include (${CMAKE_HOME_DIRECTORY}/build/cmake/ConfigurePath.cmake)

include (${CMAKE_HOME_DIRECTORY}/build/cmake/Environment.cmake)
set_environment()
set ( EFLIB_HOME_PATH ${SOFTART_HOME_DIR} )
ADD_DEFINITIONS(
	-DBOOST_ALL_DYN_LINK
)

ADD_SUBDIRECTORY( build/cmake )
ADD_SUBDIRECTORY( "${SOFTART_HOME_DIR}/eflib" "${CMAKE_CURRENT_BINARY_DIR}/eflib" )
ADD_SUBDIRECTORY( enums )
ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( test )