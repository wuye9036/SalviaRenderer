check_build_with_unicode()
set_environment()

INCLUDE_DIRECTORIES(../
	${boost_home_dir}
	${softart_home_dir}
)

include( compiler_infos_proj.cmake )
set( HEADERS_ALL ${HEADER_FILES} )
set( SOURCES_ALL ${SOURCE_FILES} )
	
include( enum_gen_config.cmake )
set( HEADERS_ALL ${HEADERS_ALL} ${HEADER_FILES} )
set( SOURCES_ALL ${SOURCES_ALL} ${SOURCE_FILES} )

ADD_CUSTOM_COMMAND( TARGET sasl_enums PRE_BUILD
	COMMAND gen.bat
	DEPENDS gen.bat compiler_infos.xml compiler_infos_proj.xml enum_defs.xml enum_gen_config.xml
)

ADD_LIBRARY( sasl_enums STATIC ${HEADERS_ALL} ${SOURCES_ALL} )

config_output_paths(sasl_enums)