SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SALVIA_HOME_DIR}
	${SALVIA_3RD_PARTY_INCLUDES}
	${SALVIA_BOOST_HOME_DIR}
	${SALVIA_WTL_INCLUDE_PATH}
)
LINK_DIRECTORIES(
	${SALVIA_3RD_PARTY_LIBS}
	${SALVIA_BOOST_LIB_DIR}
	${SALVIA_LLVM_LIB_PATH}
	)

ADD_EXECUTABLE( ObjLoader
	ObjLoader.cpp
)

TARGET_LINK_LIBRARIES( ObjLoader
	EFLIB
	salviar
	salviax
	salviau
	sasl_enums
	sasl_host
	sasl_parser
	sasl_syntaxtree
	sasl_semantic
	sasl_common
	sasl_cgllvm
	${SASL_LLVM_LIBS}
)

SALVIA_CONFIG_OUTPUT_PATHS(ObjLoader)
SALVIA_CREATE_VCPROJ_USERFILE(ObjLoader)