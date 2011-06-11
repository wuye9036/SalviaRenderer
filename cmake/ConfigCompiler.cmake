
if(MSVC)
	include( ${SALVIA_HOME_DIR}/cmake/MSVC.cmake )
endif(MSVC)
if(MINGW)
	include( ${SALVIA_HOME_DIR}/cmake/GCC.cmake )
endif(MINGW)