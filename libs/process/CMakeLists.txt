# Copyright (c) 2016 Alexander Lamaison
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

option(BUILD_TESTING "Build the Boost.Process tests" ON)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()

option(BOOST_PROCESS_BUILD_EXAMPLES "Build the Boost.Process examples" ON)
if(BOOST_PROCESS_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()
