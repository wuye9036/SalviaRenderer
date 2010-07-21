check_build_with_unicode()
set_environment()

INCLUDE_DIRECTORIES(
	${sasl_home_dir}
	${boost_home_dir}
	${softart_home_dir}
)
	
set( HEADER_FILES
	${sasl_home_dir}/sasl/include/common/common_fwd.h
	${sasl_home_dir}/sasl/include/common/compiler_info_manager.h
	${sasl_home_dir}/sasl/include/common/compiler_information.h
	${sasl_home_dir}/sasl/include/common/lex_context.h
	${sasl_home_dir}/sasl/include/common/token_attr.h
)

set( SOURCE_FILES
	compiler_info_manager.cpp
	compiler_information_impl.cpp
)

ADD_LIBRARY( sasl_common STATIC
	${HEADER_FILES} ${SOURCE_FILES}
)

config_output_paths( sasl_common )

