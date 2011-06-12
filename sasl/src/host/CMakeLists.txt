SALVIA_CHECK_BUILD_WITH_UNICODE()

INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${SALVIA_BOOST_HOME_DIR}
	${SALVIA_HOME_DIR}
)
	
set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/host/host.h
	${SALVIA_HOME_DIR}/salviar/include/shader_enums.h
	${SALVIA_HOME_DIR}/salviar/include/shader_unit.h
)

set( SOURCE_FILES
	host.cpp
	${SALVIA_HOME_DIR}/salviar/src/shader_unit.cpp
)

ADD_LIBRARY( sasl_host STATIC
	${HEADER_FILES} ${SOURCE_FILES}
)

SALVIA_CONFIG_OUTPUT_PATHS( sasl_host )

