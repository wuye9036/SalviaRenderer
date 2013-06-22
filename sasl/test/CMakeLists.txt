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
	endif()
    if(MSVC_VERSION EQUAL 1600)
      configure_file(
	  ${SASL_HOME_DIR}/sasl/test/test_resources/vs2010.test.vcxproj.user.in
	  ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcxproj.user
	  @ONLY
	)
	endif()
  endif()
endfunction(SASL_TEST_CREATE_VCPROJ_USERFILE)

ADD_SUBDIRECTORY( auto_test )
ADD_SUBDIRECTORY( robust_test )
ADD_SUBDIRECTORY( jit_test )
