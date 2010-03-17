PROJECT( SoftArt )

CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

option(SOFTART_BUILD_WITH_UNICODE "Build with unicode(wide-byte) or ansi(multi-byte)." TRUE)

#set(CMAKE_USE_RELATIVE_PATHS TRUE)

ADD_SUBDIRECTORY( cmake )

ADD_SUBDIRECTORY( eflib )
ADD_SUBDIRECTORY( softart )
ADD_SUBDIRECTORY( softartx )
ADD_SUBDIRECTORY( samples )
