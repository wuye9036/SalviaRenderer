
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

