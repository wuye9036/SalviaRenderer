PROJECT( SoftArt )

CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

option(SOFTART_BUILD_WITH_UNICODE "Build with unicode." TRUE)

#set(CMAKE_USE_RELATIVE_PATHS TRUE)

macro(check_build_with_unicode)
	if(SOFTART_BUILD_WITH_UNICODE)
		ADD_DEFINITIONS(
			-DUNICODE
			-D_UNICODE
		)
	endif ()
endmacro(check_build_with_unicode)

ADD_SUBDIRECTORY( cmake )

ADD_SUBDIRECTORY( eflib )
ADD_SUBDIRECTORY( softart )
ADD_SUBDIRECTORY( softartx )
ADD_SUBDIRECTORY( samples )
