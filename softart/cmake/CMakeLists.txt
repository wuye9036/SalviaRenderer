
macro(config_output_paths TARGETNAME)
	set_target_properties(${TARGETNAME}
		PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin
		 ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib
		 LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/lib
		 DEBUG_POSTFIX "_d"
		)
endmacro(config_output_paths)


# create vcproj.user file for Visual Studio to set debug working directory
function(ogre_create_vcproj_userfile TARGETNAME)
  if (MSVC)
    configure_file(
	  ${CMAKE_HOME_DIRECTORY}/cmake/VisualStudioUserFile.vcproj.user.in
	  ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcproj.user
	  @ONLY
	)
  endif ()
endfunction(ogre_create_vcproj_userfile)

macro(config_3rd_party)
	set(3rd_party_path ${CMAKE_HOME_DIRECTORY}/../3rd_party)
	set(3rd_party_includes
		${3rd_party_path}/threadpool
		${3rd_party_path}/FreeImage/include
	)
	if(CMAKE_CL_64)
		set(3rd_party_libs ${3rd_party_path}/FreeImage/lib/x64)
	else(CMAKE_CL_64)
		set(3rd_party_libs ${3rd_party_path}/FreeImage/lib/win32)
	endif(CMAKE_CL_64)
endmacro(config_3rd_party)


macro(check_build_with_unicode)
	if(SOFTART_BUILD_WITH_UNICODE)
		ADD_DEFINITIONS(
			-DUNICODE
			-D_UNICODE
		)
	endif ()
endmacro(check_build_with_unicode)
