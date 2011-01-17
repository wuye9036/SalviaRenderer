check_build_with_unicode()

INCLUDE_DIRECTORIES(
	${EFLIB_HOME_PATH}
)

set ( SOURCE_FILES logrout.cpp )
ADD_EXECUTABLE( logrout ${SOURCE_FILES} )
TARGET_LINK_LIBRARIES( logrout EFLIB )

config_output_paths(logrout)