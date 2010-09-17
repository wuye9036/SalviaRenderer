check_build_with_unicode()
set_environment()

ADD_SUBDIRECTORY( test_cases )
ADD_SUBDIRECTORY( st_combinator_test )
# ADD_SUBDIRECTORY( feedback_test )
# ADD_SUBDIRECTORY( semantic_test )

if( SOFTART_ENABLE_SASL_REGRESSION_TEST )
ADD_SUBDIRECTORY( regression_test )
endif( SOFTART_ENABLE_SASL_REGRESSION_TEST )

if( SOFTART_BUILD_WITH_LLVM )
ADD_SUBDIRECTORY( cgllvm_test )
endif( SOFTART_BUILD_WITH_LLVM )