check_build_with_unicode()
config_3rd_party()

INCLUDE_DIRECTORIES(../../
${3rd_party_includes}
)

LINK_DIRECTORIES(${3rd_party_libs})

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
	SoftArt
	SoftArtX
)
config_output_paths(ComplexMesh)


softart_create_vcproj_userfile(ComplexMesh)