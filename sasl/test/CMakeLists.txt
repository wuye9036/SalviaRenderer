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
	make_tree.h
)

set( SOURCE_FILES
	llvm_test.cpp
	semantic.cpp
	syntax_tree.cpp
	test_utility.cpp
	compinfos.cpp
	make_tree.cpp
)

ADD_EXECUTABLE( sasl_test ${HEADER_FILES} ${SOURCE_FILES} )
TARGET_LINK_LIBRARIES( sasl_test
	sasl_enums sasl_common sasl_semantic sasl_syntaxtree
)

config_output_paths( sasl_test )