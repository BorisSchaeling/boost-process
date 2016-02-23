# Copyright (c) 2016 Alexander Lamaison
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

add_executable(sparring_partner sparring_partner.cpp)

hunter_add_package(Boost COMPONENTS program_options)
find_package(Boost 1.48 CONFIG REQUIRED program_options)
target_link_libraries(sparring_partner Boost::program_options boost_process)

set(TESTS
  bind_stderr
  bind_stdin
  bind_stdin_stdout
  bind_stdout
  bind_stdout_stderr
  close_stderr
  close_stdin
  close_stdout
  exit_code
  extensions
  inherit_env
  run_exe
  run_exe_path
  run_exe_wstring
  search_path
  set_args
  set_cmd_line
  set_env
  set_on_error
  shell_path
  start_in_dir
  terminate
  throw_on_error
  wait)
if(WIN32)
  list(APPEND TESTS windows_specific search_path_wstring set_args_wstring
    set_cmd_line_wstring set_env_wstring shell_path_wstring show_window
    start_in_dir_wstring)
else()
  list(APPEND TESTS posix_specific)
endif()

foreach(TEST ${TESTS})
  add_executable(test-${TEST} ${TEST}.cpp)
  target_link_libraries(test-${TEST} boost_process)
  add_test(NAME test-${TEST} COMMAND "$<TARGET_FILE:test-${TEST}>" "$<TARGET_FILE:sparring_partner>")
endforeach()
