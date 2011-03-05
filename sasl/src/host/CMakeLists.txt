check_build_with_unicode()
set_environment()

INCLUDE_DIRECTORIES(
	${SASL_HOME_DIR}
	${BOOST_HOME_DIR}
	${SOFTART_HOME_DIR}
)
	
set( HEADER_FILES
	${SASL_HOME_DIR}/sasl/include/host/host.h
)

set( SOURCE_FILES
	host.cpp
)

ADD_LIBRARY( sasl_host STATIC
	${HEADER_FILES} ${SOURCE_FILES}
)

config_output_paths( sasl_host )

