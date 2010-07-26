check_build_with_unicode()
set_environment()

include (generate_file.cmake)

INCLUDE_DIRECTORIES(../
	${boost_home_dir}
	${softart_home_dir}
)

# sasl_generate_enum( 
	# ${CMAKE_CURRENT_SOURCE_DIR}/enums.cmake 
	# ${CMAKE_CURRENT_SOURCE_DIR}/enums.xpg
	# ${CMAKE_CURRENT_SOURCE_DIR}/enums.xpt
# )

# sasl_generate_enum( 
	# ${CMAKE_CURRENT_SOURCE_DIR}/compiler_infos.cmake 
	# ${CMAKE_CURRENT_SOURCE_DIR}/compiler_infos.xpg
	# ${CMAKE_CURRENT_SOURCE_DIR}/compiler_infos.xpt
# )

ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/enums.cmake
	COMMAND cd 
	ARGS \"${CMAKE_CURRENT_SOURCE_DIR}\"
	COMMAND python
	ARGS ${CMAKE_CURRENT_SOURCE_DIR}/enums_generator.py ${CMAKE_CURRENT_SOURCE_DIR}/enums.xgp
	DEPENDS
		${CMAKE_CURRENT_SOURCE_DIR}/enums_generator.py ${CMAKE_CURRENT_SOURCE_DIR}/enums.xgp ${CMAKE_CURRENT_SOURCE_DIR}/enums.xgt
)

ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/compiler_infos.cmake
	COMMAND cd
	ARGS \"${CMAKE_CURRENT_SOURCE_DIR}\"
	COMMAND python
	ARGS ${CMAKE_CURRENT_SOURCE_DIR}/enums_generator.py ${CMAKE_CURRENT_SOURCE_DIR}/compiler_infos.xgp
	DEPENDS
		${CMAKE_CURRENT_SOURCE_DIR}/enums_generator.py ${CMAKE_CURRENT_SOURCE_DIR}/compiler_infos.xgp ${CMAKE_CURRENT_SOURCE_DIR}/compiler_infos.xgt
)

SET_SOURCE_FILES_PROPERTIES( enums.cmake compiler_infos.cmake
	PROPERTIES GENERATED 1
)

include( compiler_infos.cmake )
set( HEADERS_ALL ${HEADER_FILES} )
set( SOURCES_ALL ${SOURCE_FILES} )
	
include( enums.cmake )
set( HEADERS_ALL ${HEADERS_ALL} ${HEADER_FILES} )
set( SOURCES_ALL ${SOURCES_ALL} ${SOURCE_FILES} )

SET_SOURCE_FILES_PROPERTIES(
	${HEADERS_ALL} ${SOURCES_ALL}
	PROPERTIES GENERATED 1
)

ADD_LIBRARY( sasl_enums STATIC enum_base.h default_hasher.h proxy.cpp ${HEADERS_ALL} ${SOURCES_ALL} compiler_infos.cmake enums.cmake )

config_output_paths(sasl_enums)