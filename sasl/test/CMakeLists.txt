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
	test_utility.h
)

set( SOURCE_FILES
	test_main.cpp
	semantic.cpp
	syntax_tree.cpp
	test_utility.cpp
	compinfos.cpp
)

ADD_EXECUTABLE( sasl_test ${HEADER_FILES} ${SOURCE_FILES} )
TARGET_LINK_LIBRARIES( sasl_test
	sasl_enums sasl_common sasl_semantic sasl_syntaxtree
)

config_output_paths( sasl_test )

if( SOFTART_BUILD_WITH_LLVM )
ADD_SUBDIRECTORY( cgllvm_test )
endif( SOFTART_BUILD_WITH_LLVM )