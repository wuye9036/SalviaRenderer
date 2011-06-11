macro( boost_lib_fullname FULL_NAME SHORT_NAME )
	if( MINGW AND WIN32 )
	
		if( SOFTART_BUILD_TYPE_LOWERCASE STREQUAL "debug" )
			set ( SOFTART_BOOST_LIBS_POSTFIX "-d" )
		endif()
		
		set( ${FULL_NAME} "boost_${SHORT_NAME}-mgw${GCC_VERSION_STR_MAJOR_MINOR}-mt${SOFTART_BOOST_LIBS_POSTFIX}-${BOOST_VERSION_STRING}.dll" )
		
	endif( MINGW AND WIN32 )
endmacro( boost_lib_fullname )

macro( add_boost_lib SHORT_NAME )
	set( FULL_NAME "" )
	boost_lib_fullname( FULL_NAME ${SHORT_NAME} )
	message( STATUS ${FULL_NAME} )
	set( SOFTART_BOOST_LIBS ${SOFTART_BOOST_LIBS} ${FULL_NAME} )
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
			set( SOFTART_BOOST_LIBS "" )
		endif( WIN32 AND MSVC )
	endif( MINGW OR UNIX )
endmacro( config_boost_libs )

config_boost_libs()