check_build_with_unicode()
set_environment()

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
)

set( SOURCE_FILES
	syntax_cases.cpp
)

ADD_LIBRARY( sasl_test_cases ${HEADER_FILES} ${SOURCE_FILES} )

config_output_paths( sasl_test_cases )