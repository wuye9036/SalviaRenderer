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

macro(SALVIA_GROUP_BY_DIRECTORY root_group prefix sources)
	foreach(file_path ${${sources}})
		STRING(REGEX REPLACE ${prefix}/\(.*\) \\1 filterated_file_path ${file_path})
		STRING(REGEX REPLACE "\(.*\)/.*" \\1 group_name ${filterated_file_path})
		STRING(COMPARE EQUAL ${filterated_file_path} ${group_name} no_group)
		STRING(REPLACE "/" "\\" group_name ${group_name})
		if(no_group)
			set(group_name "\\")
		endif(no_group)
		SOURCE_GROUP( "${root_group}\\${group_name}" FILES ${file_path})
	endforeach(file_path)
endmacro(SALVIA_GROUP_BY_DIRECTORY)

# create vcproj.user file for Visual Studio to set debug working directory
function(SALVIA_CREATE_VCPROJ_USERFILE TARGETNAME)
  if(MSVC)
    set(SYSTEM_NAME $ENV{USERDOMAIN})
    set(USER_NAME $ENV{USERNAME})
    if(MSVC_VERSION EQUAL 1700)
      configure_file(
	  ${CMAKE_HOME_DIRECTORY}/cmake/VisualStudio2012UserFile.vcxproj.user.in
	  ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcxproj.user
	  @ONLY
	)
	endif()
    if(MSVC_VERSION EQUAL 1600)
      configure_file(
	  ${CMAKE_HOME_DIRECTORY}/cmake/VisualStudio2010UserFile.vcxproj.user.in
	  ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcxproj.user
	  @ONLY
	)
    endif()
  endif()
endfunction(SALVIA_CREATE_VCPROJ_USERFILE)