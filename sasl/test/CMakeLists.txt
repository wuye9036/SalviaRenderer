SALVIA_CHECK_BUILD_WITH_UNICODE()

function(SASL_TEST_CREATE_VCPROJ_USERFILE TARGETNAME)
  if(MSVC)
    set(SYSTEM_NAME $ENV{USERDOMAIN})
    set(USER_NAME $ENV{USERNAME})
	
	if(MSVC_VERSION EQUAL 1700)
      configure_file(
	  ${SASL_HOME_DIR}/sasl/test/test_resources/vs2012.test.vcxproj.user.in
	  ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcxproj.user
	  @ONLY
	)
	elseif(MSVC_VERSION EQUAL 1800)
	  configure_file(
	    ${SASL_HOME_DIR}/sasl/test/test_resources/vs2013.test.vcxproj.user.in
	    ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcxproj.user
	    @ONLY
	  )
	elseif(MSVC_VERSION GREATER 1920)
		configure_file(
			${SASL_HOME_DIR}/sasl/test/test_resources/vs2019.test.vcxproj.user.in
			${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcxproj.user
			@ONLY
	    )
	else()
	  MESSAGE(FATAL_ERROR "Cannot support Microsoft Visual C++ version ${MSVC_VERSION}. Build will be stopped." )
	endif()
  endif()
endfunction(SASL_TEST_CREATE_VCPROJ_USERFILE)

ADD_SUBDIRECTORY( repo )
ADD_SUBDIRECTORY( abi_test )
ADD_SUBDIRECTORY( death_test )
ADD_SUBDIRECTORY( jit_test )
