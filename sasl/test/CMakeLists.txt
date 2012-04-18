SALVIA_CHECK_BUILD_WITH_UNICODE()

function(SASL_TEST_CREATE_VCPROJ_USERFILE TARGETNAME)
  if(MSVC)
    set(SYSTEM_NAME $ENV{USERDOMAIN})
    set(USER_NAME $ENV{USERNAME})
	
    if(MSVC_VERSION EQUAL 1600)
      configure_file(
	  ${SASL_HOME_DIR}/sasl/test/test_resources/vs2010.test.vcxproj.user.in
	  ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcxproj.user
	  @ONLY
	)
	endif()
	if(MSVC_VERSION EQUAL 1500)
      configure_file(
	  ${SASL_HOME_DIR}/sasl/test/test_resources/vs2008.test.vcproj.user.in
	  ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcproj.${SYSTEM_NAME}.${USER_NAME}.user
	  @ONLY
	)
	endif()
	if(MSVC_VERSION EQUAL 1400)
      configure_file(
	  ${SASL_HOME_DIR}/sasl/test/test_resources/vs2005.test.vcproj.user.in
	  ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcproj.${SYSTEM_NAME}.${USER_NAME}.user
	  @ONLY
	)
    endif()
  endif()
endfunction(SASL_TEST_CREATE_VCPROJ_USERFILE)

if( SALVIA_ENABLE_SASL_REGRESSION_TEST OR SALVIA_ENABLE_SASL_SEPERATED_TESTS )
	ADD_SUBDIRECTORY( test_cases )
endif()

if( SALVIA_ENABLE_SASL_SEPERATED_TESTS )

	ADD_SUBDIRECTORY( parser_test )
	ADD_SUBDIRECTORY( st_combinator_test )
	ADD_SUBDIRECTORY( semantic_test )
	ADD_SUBDIRECTORY( auto_test )
	
	if( SALVIA_BUILD_WITH_LLVM )
		ADD_SUBDIRECTORY( robust_test )
		ADD_SUBDIRECTORY( jit_test )
	endif( SALVIA_BUILD_WITH_LLVM )
	
endif( )

if( SALVIA_ENABLE_SASL_REGRESSION )
	ADD_SUBDIRECTORY( regression_test )
endif( SALVIA_ENABLE_SASL_REGRESSION )

