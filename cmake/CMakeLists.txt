macro(SALVIA_CONFIG_OUTPUT_PATHS TARGETNAME)
  set_target_properties(${TARGETNAME}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${SALVIA_RUNTIME_OUTPUT_DIRECTORY}
    ARCHIVE_OUTPUT_DIRECTORY ${SALVIA_LIBRARY_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${SALVIA_LIBRARY_OUTPUT_DIRECTORY}
    DEBUG_POSTFIX "_d"
  )
endmacro(SALVIA_CONFIG_OUTPUT_PATHS)

macro(SALVIA_CHECK_BUILD_WITH_UNICODE)
	if(SALVIA_BUILD_WITH_UNICODE)
		ADD_DEFINITIONS(
			-DUNICODE
			-D_UNICODE
		)
	endif ()
endmacro(SALVIA_CHECK_BUILD_WITH_UNICODE)

# create vcproj.user file for Visual Studio to set debug working directory
function(SALVIA_CREATE_VCPROJ_USERFILE TARGETNAME)
  if(MSVC)
    set(SYSTEM_NAME $ENV{USERDOMAIN})
    set(USER_NAME $ENV{USERNAME})

    if(MSVC_VERSION EQUAL 1600)
      configure_file(
	  ${CMAKE_HOME_DIRECTORY}/cmake/VisualStudio2010UserFile.vcxproj.user.in
	  ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcxproj.user
	  @ONLY
	)
    endif()
    if(MSVC_VERSION EQUAL 1500)
      configure_file(
	  ${CMAKE_HOME_DIRECTORY}/cmake/VisualStudio2008UserFile.vcproj.user.in
	  ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcproj.${SYSTEM_NAME}.${USER_NAME}.user
	  @ONLY
	)
    endif()
	if(MSVC_VERSION EQUAL 1400)
      configure_file(
	  ${CMAKE_HOME_DIRECTORY}/cmake/VisualStudio2005UserFile.vcproj.user.in
	  ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcproj.${SYSTEM_NAME}.${USER_NAME}.user
	  @ONLY
	)
    endif()
  endif()
endfunction(SALVIA_CREATE_VCPROJ_USERFILE)