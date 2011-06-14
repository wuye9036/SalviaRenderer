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
)

set (HEADER_FILES
	AboutDlg.h
	MainFrm.h
	resource.h
	SRComplexMesh.h
	SRComplexMeshView.h
	Timer.h
	stdafx.h
)

set (SOURCE_FILES
	SRComplexMesh.cpp
	Timer.cpp
	stdafx.cpp
)

set (RESOURCE_FILES
	res/SRComplexMesh.ico
	res/toolbar.bmp
	SRComplexMesh.rc
)

ADD_EXECUTABLE(ComplexMesh
${HEADER_FILES}
${SOURCE_FILES}
${RESOURCE_FILES}
)

TARGET_LINK_LIBRARIES(ComplexMesh
	EFLIB
	salviar
	salviax
	sasl_enums
	sasl_host
	sasl_syntaxtree
	sasl_semantic
	sasl_common
)

SALVIA_CONFIG_OUTPUT_PATHS(ComplexMesh)
SALVIA_CREATE_VCPROJ_USERFILE(ComplexMesh)