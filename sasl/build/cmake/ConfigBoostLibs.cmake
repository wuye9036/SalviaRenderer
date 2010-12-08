macro( add_boost_lib SHORT_NAME )
	if( MINGW AND WIN32 )
	
		if( SOFTART_BUILD_TYPE_LOWERCASE STREQUAL "debug" )
			set ( SOFTART_BOOST_LIBS_POSTFIX "-d" )
		endif()
		
		set( SOFTART_BOOST_LIBS ${SOFTART_BOOST_LIBS} "boost_${SHORT_NAME}-mgw${GCC_VERSION_STR_MAJOR_MINOR}-mt${SOFTART_BOOST_LIBS_POSTFIX}-${BOOST_VERSION_STRING}.dll" )
		
	endif( MINGW AND WIN32 )
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
	else( MINGW OR UNIX )
		if( WIN32 AND MSVC )
			#auto linking support.
			set( SOFTART_BOOST_LIBS "" )
		endif( WIN32 AND MSVC )
	endif( MINGW OR UNIX )
endmacro( config_boost_libs )

config_boost_libs()