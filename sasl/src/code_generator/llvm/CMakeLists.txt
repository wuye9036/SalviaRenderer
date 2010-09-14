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
	${sasl_home_dir}/sasl/include/code_generator/llvm/cgllvm_context.h
	${sasl_home_dir}/sasl/include/code_generator/llvm/cgllvm.h
	${sasl_home_dir}/sasl/include/code_generator/llvm/cgllvm_info.h
)

set( SOURCE_FILES
	cgllvm.cpp
	cgllvm_api.cpp
	cgllvm_context.cpp
	cgllvm_info.cpp
)

ADD_LIBRARY( sasl_cgllvm STATIC	${HEADER_FILES} ${SOURCE_FILES} )

config_output_paths( sasl_cgllvm )
