check_build_with_unicode()
set_environment()

INCLUDE_DIRECTORIES(
	${sasl_home_dir}
	${boost_home_dir}
	${softart_home_dir}
	${3rd_party_llvm_include_path}
)
	
set( HEADER_FILES
	${sasl_home_dir}/sasl/include/code_generator/llvm/cgllvm_api.h
	${sasl_home_dir}/sasl/include/code_generator/llvm/cgllvm_contexts.h
	${sasl_home_dir}/sasl/include/code_generator/llvm/cgllvm_globalctxt.h
	${sasl_home_dir}/sasl/include/code_generator/llvm/cgllvm_impl.h
)

set( SOURCE_FILES
	cgllvm_api.cpp
	cgllvm_contexts.cpp
	cgllvm_globalctxt.cpp
	cgllvm_impl.cpp
)

ADD_LIBRARY( sasl_cgllvm STATIC	${HEADER_FILES} ${SOURCE_FILES} )

config_output_paths( sasl_cgllvm )
