# create vcproj.user file for Visual Studio to set debug working directory
function(SALVIA_CREATE_VCPROJ_USERFILE TARGETNAME)
  if(MSVC)
    set(SYSTEM_NAME $ENV{USERDOMAIN})
    set(USER_NAME $ENV{USERNAME})
    if(MSVC_VERSION GREATER 1920)
      configure_file(
        ${CMAKE_HOME_DIRECTORY}/cmake/VisualStudio2019UserFile.vcxproj.user.in
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcxproj.user
        @ONLY
      )
    else()
       MESSAGE(FATAL_ERROR "Cannot support Microsoft Visual C++ version ${MSVC_VERSION}. Build will be stopped." )
    endif()
  endif()
endfunction(SALVIA_CREATE_VCPROJ_USERFILE)
