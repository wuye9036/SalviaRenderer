set ( SALVIA_HOME_DIR "${CMAKE_HOME_DIRECTORY}" )
set ( SASL_HOME_DIR	"${CMAKE_HOME_DIRECTORY}" )

set( SALVIA_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/bin/${SALVIA_PLATFORM_NAME}" )
set( SALVIA_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_HOME_DIRECTORY}/lib/${SALVIA_PLATFORM_NAME}" )

################ configurate 3rd party library paths ###############
set( SALVIA_3RD_PARTY_PATH 
	"${SALVIA_HOME_DIR}/3rd_party/"
	)
set( SALVIA_3RD_PARTY_INCLUDES
    ${SALVIA_3RD_PARTY_PATH}/threadpool
    ${SALVIA_3RD_PARTY_PATH}/FreeImage/include
  )

  set(SALVIA_3RD_PARTY_LIBS "${SALVIA_3RD_PARTY_PATH}/FreeImage/lib/${SALVIA_PLATFORM_NAME}")

################ Set WTL Path ################
set( SALVIA_WTL_INCLUDE_PATH ${SALVIA_3RD_PARTY_PATH}/wtl/include )  
 
################ Set LLVM path ################
set( SALVIA_LLVM_PATH "${SALVIA_LLVM_INSTALL_PATH}" )
set( SALVIA_LLVM_INCLUDE_PATH "${SALVIA_LLVM_PATH}/include/")
set( SALVIA_LLVM_LIB_PATH "${SALVIA_LLVM_PATH}/lib" )

############ Set FreeType2 path #############
set( SALVIA_FREETYPE_INCLUDE_DIR "${SALVIA_3RD_PARTY_PATH}/freetype2/include/")
#SALVIA_FREE_TYPE_LIB_DIR
