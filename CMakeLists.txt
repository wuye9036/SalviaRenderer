PROJECT( salvia )

CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

option(SALVIA_BUILD_WITH_UNICODE "Build with unicode(wide-byte) or ansi(multi-byte)." TRUE)

#set(CMAKE_USE_RELATIVE_PATHS TRUE)
set( SALVIA_HOME_PATH ${CMAKE_HOME_DIRECTORY} )

# Configuring
ADD_SUBDIRECTORY( cmake )

# Add projects
ADD_SUBDIRECTORY( eflib )
ADD_SUBDIRECTORY( sasl )
ADD_SUBDIRECTORY( salviar )
ADD_SUBDIRECTORY( salviax )
ADD_SUBDIRECTORY( samples )
