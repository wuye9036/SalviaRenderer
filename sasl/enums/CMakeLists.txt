check_build_with_unicode()
set_environment()

include (generate_enum.cmake)

INCLUDE_DIRECTORIES(../
	${BOOST_HOME_DIR}
	${SASL_HOME_DIR}
	${SOFTART_HOME_DIR}
)

LIST(APPEND enum_confs enums.xgt )
sasl_generate_enum( 
	enums.cmake 
	enums.xgp
	${enum_confs}
)

LIST(APPEND ci_confs compiler_infos.xgt)
sasl_generate_enum( 
	compiler_infos.cmake 
	compiler_infos.xgp
	${ci_confs}
)

SET_SOURCE_FILES_PROPERTIES( enums.cmake compiler_infos.cmake
	PROPERTIES GENERATED 1
)

include( enums.cmake )
LIST(APPEND FILES_ALL ${HEADER_FILES} )
LIST(APPEND FILES_ALL ${SOURCE_FILES} )
	
include( compiler_infos.cmake )
LIST(APPEND FILES_ALL ${HEADER_FILES} )
LIST(APPEND FILES_ALL ${SOURCE_FILES} )

SET_SOURCE_FILES_PROPERTIES(
	${FILES_ALL}
	PROPERTIES GENERATED 1
)

foreach( code_file ${FILES_ALL} )
LIST(APPEND code_fullpath "${CMAKE_CURRENT_SOURCE_DIR}/${code_file}" )
endforeach( code_file )

SET_SOURCE_FILES_PROPERTIES(
	${code_fullpath}
	PROPERTIES GENERATED 1
)

ADD_LIBRARY( sasl_enums STATIC 
	enum_base.h default_hasher.h
	enums_helper.h enums_helper.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/enums.cmake 
	${CMAKE_CURRENT_SOURCE_DIR}/compiler_infos.cmake
	${code_fullpath}
	enums_generator.py
)

config_output_paths(sasl_enums)