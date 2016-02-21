# Copyright (c) 2016 Alexander Lamaison
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(EXAMPLES
  args
  cleanup
  cmd_line
  env
  error_handling
  execute
  intro
  streams
  sync_io
  terminate
  work_dir)
list (FIND CMAKE_CXX_COMPILE_FEATURES "cxx_lambdas" _index)
if (${_index} GREATER -1)
  list(APPEND EXAMPLES wait async_io)
endif()
if(WIN32)
  if (${_index} GREATER -1)
    list(APPEND EXAMPLES windows)
  endif()
  list(APPEND EXAMPLES windows_unicode)
else()
  list(APPEND EXAMPLES posix)
endif()

foreach(EXAMPLE ${EXAMPLES})
  add_executable(example-${EXAMPLE} ${EXAMPLE}.cpp)
  target_link_libraries(example-${EXAMPLE} boost_process)
endforeach()
