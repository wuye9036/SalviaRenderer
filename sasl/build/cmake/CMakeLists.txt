macro(config_output_paths TARGETNAME)
  set_target_properties(${TARGETNAME}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin/${SOFTART_PLATFORM_NAME}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib/${SOFTART_PLATFORM_NAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib/${SOFTART_PLATFORM_NAME}
    DEBUG_POSTFIX "_d"
  )
endmacro(config_output_paths)

macro(check_build_with_unicode)
	if(SOFTART_BUILD_WITH_UNICODE)
		ADD_DEFINITIONS(
			-DUNICODE
			-D_UNICODE
		)
	endif ()
endmacro(check_build_with_unicode)

include( Environment.cmake )

