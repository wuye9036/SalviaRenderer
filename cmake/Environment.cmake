macro ( set_environment )
	set( SALVIA_BOOST_PATH CACHE PATH "Specify a path to boost." )
	FIND_PACKAGE( Boost 1.44.0 PATHS SALVIA_BOOST_PATH )
	
	if( Boost_FOUND )
		set( BOOST_HOME_DIR "${Boost_INCLUDE_DIRS}/.." )
	else ( Boost_FOUND )	
		MESSAGE( FATAL_ERROR "Can not find boost 1.44 or later. Please specify a path." )
	endif()

	if(SALVIA_PLATFORM_NAME STREQUAL "x64")
		set( BOOST_LIB_DIR ${BOOST_HOME_DIR}/bin/x64)
	else(SALVIA_PLATFORM_NAME STREQUAL "x64")
		set( BOOST_LIB_DIR ${BOOST_HOME_DIR}/bin/x86)
	endif()

	if( NOT EXISTS ${BOOST_LIB_DIR} )
		MESSAGE( FATAL_ERROR "Cannot find libraries in ${BOOST_LIB_DIR}. Please compile libraries and copy lib files into directory.")
	endif()
endmacro( set_environment )

set_environment()