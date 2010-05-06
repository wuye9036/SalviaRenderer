macro(config_output_paths TARGETNAME)
  set_target_properties(${TARGETNAME}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin/${SOFTART_PLATFORM_NAME}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib/${SOFTART_PLATFORM_NAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib/${SOFTART_PLATFORM_NAME}
    DEBUG_POSTFIX "_d"
  )
endmacro(config_output_paths)


# create vcproj.user file for Visual Studio to set debug working directory
function(softart_create_vcproj_userfile TARGETNAME)
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
  endif()
endfunction(softart_create_vcproj_userfile)

macro(config_3rd_party)
  set(3rd_party_path ${CMAKE_HOME_DIRECTORY}/../3rd_party)
  set(3rd_party_includes
    ${3rd_party_path}/threadpool
    ${3rd_party_path}/FreeImage/include
  )
  if(SOFTART_PLATFORM_NAME STREQUAL "x64")
    set(3rd_party_libs ${3rd_party_path}/FreeImage/lib/x64)
  else(SOFTART_PLATFORM_NAME STREQUAL "x64")
    set(3rd_party_libs ${3rd_party_path}/FreeImage/lib/win32)
  endif(SOFTART_PLATFORM_NAME STREQUAL "x64")
endmacro(config_3rd_party)


macro(check_build_with_unicode)
	if(SOFTART_BUILD_WITH_UNICODE)
		ADD_DEFINITIONS(
			-DUNICODE
			-D_UNICODE
		)
	endif ()
endmacro(check_build_with_unicode)
