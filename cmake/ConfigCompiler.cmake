include(CheckCXXCompilerFlag)

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

function(target_compile_options_if_applicable target flag)
  check_cxx_compiler_flag (${flag} ${target}_FLAG_COULD_BE_ENABLED)
  if(${target}_FLAG_COULD_BE_ENABLED)
    target_compile_options(${target} PUBLIC ${flag})
  endif()
endfunction()

set(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UP)

if(MSVC)
  include(${CMAKE_CURRENT_LIST_DIR}/MSVC.cmake)
endif(MSVC)

if(MINGW OR UNIX)
  include(${CMAKE_CURRENT_LIST_DIR}/GCC.cmake)
endif(MINGW OR UNIX)

function(deploy_dlls target)
  if (WIN32)
    add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${target}> $<TARGET_FILE_DIR:${target}>
            COMMAND_EXPAND_LISTS
            )
  endif ()
endfunction()
