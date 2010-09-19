check_build_with_unicode()
set_environment()

include ( llvm_test_cases.cmake )

INCLUDE_DIRECTORIES(
	${boost_home_dir}
	${softart_home_dir}
	${sasl_home_dir}
)

LINK_DIRECTORIES(
	${boost_lib_dir}
)

set( HEADER_FILES
	utility.h
	syntax_cases.h
	semantic_cases.h
	${SOFTART_LLVM_TEST_CASE_HEADERS}
)

set( SOURCE_FILES
	syntax_cases.cpp
	semantic_cases.cpp
	${SOFTART_LLVM_TEST_CASE_SOURCES}
)

ADD_LIBRARY( sasl_test_cases ${HEADER_FILES} ${SOURCE_FILES} )

config_output_paths( sasl_test_cases )