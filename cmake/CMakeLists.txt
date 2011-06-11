macro(config_output_paths TARGETNAME)
  set_target_properties(${TARGETNAME}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${SALVIA_RUNTIME_OUTPUT_DIRECTORY}
    ARCHIVE_OUTPUT_DIRECTORY ${SALVIA_LIBRARY_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${SALVIA_LIBRARY_OUTPUT_DIRECTORY}
    DEBUG_POSTFIX "_d"
  )
endmacro(config_output_paths)

macro(check_build_with_unicode)
	if(SALVIA_BUILD_WITH_UNICODE)
		ADD_DEFINITIONS(
			-DUNICODE
			-D_UNICODE
		)
	endif ()
endmacro(check_build_with_unicode)

include( Environment.cmake )

