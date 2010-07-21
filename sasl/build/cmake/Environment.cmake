macro ( set_environment )

set ( boost_home_dir 
	"C:/code/library/boost_1_43_0/"
)
set ( softart_home_dir
	"${CMAKE_HOME_DIRECTORY}/../softart/"
)
set ( sasl_home_dir
	"${CMAKE_HOME_DIRECTORY}/../"
)

if(SOFTART_PLATFORM_NAME STREQUAL "x64")
	set( boost_lib_dir ${boost_home_dir}/bin/x64)
else(SOFTART_PLATFORM_NAME STREQUAL "x64")
	set( boost_lib_dir ${boost_home_dir}/bin/x86)
endif(SOFTART_PLATFORM_NAME STREQUAL "x64")

endmacro( set_environment )