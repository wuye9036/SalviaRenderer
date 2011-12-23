
set( SALVIA_BOOST_DIRECTORY CACHE PATH "Specify a path to boost." )
if( SALVIA_BOOST_DIRECTORY )
	set( BOOST_ROOT ${SALVIA_BOOST_DIRECTORY} )
endif()
FIND_PACKAGE( Boost 1.44.0 )

if( Boost_FOUND )
	set( SALVIA_BOOST_HOME_DIR "${Boost_INCLUDE_DIRS}" )
else ( Boost_FOUND )	
	MESSAGE( FATAL_ERROR "Can not find boost 1.44 or later. Please specify a path with 'SALVIA_BOOST_DIRECOTRY' or run './build_all.py'." )
endif()

if( NOT EXISTS ${SALVIA_BOOST_LIB_DIR} )
	MESSAGE( FATAL_ERROR "Cannot find libraries in ${SALVIA_BOOST_LIB_DIR}. Please compile libraries and copy lib files into directory or run './build_all.py'.")
endif()

set( SALVIA_BOOST_VERSION_STRING "${Boost_MAJOR_VERSION}_${Boost_MINOR_VERSION}" )

# From short name to full path name.
macro( boost_lib_fullname FULL_NAME SHORT_NAME )
	if( MINGW AND WIN32 )
		if( SALVIA_BUILD_TYPE_LOWERCASE STREQUAL "debug" )
			set ( SALVIA_BOOST_LIBS_POSTFIX "-d" )
		endif()
		set( ${FULL_NAME} "boost_${SHORT_NAME}-mgw${GCC_VERSION_STR_MAJOR_MINOR}-mt${SALVIA_BOOST_LIBS_POSTFIX}-${SALVIA_BOOST_VERSION_STRING}.lib" )
	endif( MINGW AND WIN32 )
endmacro( boost_lib_fullname )

macro( add_boost_lib SHORT_NAME )
	set( FULL_NAME "" )
	boost_lib_fullname( FULL_NAME ${SHORT_NAME} )
	FIND_PATH( FILE_FULL_PATH FULL_NAME PATHS ${SALVIA_BOOST_LIB_DIR} )
	if( NOT FILE_FULL_PATH )
		MESSAGE( FATAL_ERROR "Cannot find lib ${FULL_NAME}. Please compile this library and copy into ${SALVIA_BOOST_LIB_DIR}.")
	endif()
	set( SALVIA_BOOST_LIBS ${SALVIA_BOOST_LIBS} ${FULL_NAME} )
endmacro( add_boost_lib )

macro( config_boost_libs )
	ADD_DEFINITIONS( -DBOOST_ALL_DYN_LINK )
	if( MINGW OR UNIX )
		# without auto-linking.
		# add them by ourselves.
		add_boost_lib( date_time )
		add_boost_lib( unit_test_framework )
		add_boost_lib( thread )
		add_boost_lib( system )
		add_boost_lib( wave )
	else( MINGW OR UNIX )
		if( WIN32 AND MSVC )
			#auto linking support.
			set( SALVIA_BOOST_LIBS "" )
		endif( WIN32 AND MSVC )
	endif( MINGW OR UNIX )
endmacro( config_boost_libs )

config_boost_libs()