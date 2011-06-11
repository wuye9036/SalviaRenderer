check_build_with_unicode()
set_environment()

function(sasl_test_create_vcproj_userfile TARGETNAME)
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
  endif()
endfunction(sasl_test_create_vcproj_userfile)

if( ( NOT SALVIA_ENABLE_SASL_REGRESSION_TEST ) OR SALVIA_ENABLE_SASL_SEPERATED_TESTS_ALWAYS )
	
	ADD_SUBDIRECTORY( parser_test )
	ADD_SUBDIRECTORY( test_cases )
	ADD_SUBDIRECTORY( st_combinator_test )
	# ADD_SUBDIRECTORY( feedback_test )
	ADD_SUBDIRECTORY( semantic_test )
	ADD_SUBDIRECTORY( auto_test )
	
	if( SALVIA_BUILD_WITH_LLVM )
		ADD_SUBDIRECTORY( cgllvm_test )
	endif( SALVIA_BUILD_WITH_LLVM )
	
endif( ( NOT SALVIA_ENABLE_SASL_REGRESSION_TEST ) OR SALVIA_ENABLE_SASL_SEPERATED_TESTS_ALWAYS )

if( SALVIA_ENABLE_SASL_REGRESSION_TEST )
ADD_SUBDIRECTORY( regression_test )
endif( SALVIA_ENABLE_SASL_REGRESSION_TEST )

