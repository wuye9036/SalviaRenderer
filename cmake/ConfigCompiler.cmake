function(append value)
  foreach(variable ${ARGN})
    set(${variable} "${${variable}} ${value}" PARENT_SCOPE)
  endforeach(variable)
endfunction()

function(append_if condition value)
  if (${condition})
    foreach(variable ${ARGN})
	  set(${variable} "${${variable}} ${value}" PARENT_SCOPE)
    endforeach(variable)
  endif()
endfunction()

set(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UP)

if(MSVC)
  include(${CMAKE_CURRENT_LIST_DIR}/MSVC.cmake)
endif(MSVC)

if(MINGW OR UNIX)
  include(${CMAKE_CURRENT_LIST_DIR}/GCC.cmake)
endif(MINGW OR UNIX)
