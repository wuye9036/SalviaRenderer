PROJECT( SoftArt )

CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

if (NOT APPLE)
  # Create debug libraries with _d postfix
  set(CMAKE_DEBUG_POSTFIX "_d")
endif ()

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

macro(config_output_paths)
	SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/bin)
	SET(LIBRARY_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/lib)
endmacro(config_output_paths)


ADD_SUBDIRECTORY( eflib )
ADD_SUBDIRECTORY( softart )
ADD_SUBDIRECTORY( softartx )
ADD_SUBDIRECTORY( samples )
