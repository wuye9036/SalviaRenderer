PROJECT( SoftArt )

CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

if (NOT APPLE)
  # Create debug libraries with _d postfix
  set(CMAKE_DEBUG_POSTFIX "_d")
endif ()

option(SOFTART_BUILD_WITH_UNICODE "Build with unicode." TRUE)

set(SOFTART_BINARY_DIR "../Win32")

macro(check_build_with_unicode)
	if(SOFTART_BUILD_WITH_UNICODE)
		ADD_DEFINITIONS(
			-DUNICODE
			-D_UNICODE
		)
	endif ()
endmacro(check_build_with_unicode)

ADD_SUBDIRECTORY( eflib )
ADD_SUBDIRECTORY( softart )
ADD_SUBDIRECTORY( softartx )
ADD_SUBDIRECTORY( samples )
