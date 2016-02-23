# Copyright (c) 2016 Alexander Lamaison
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.0)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.12.49.tar.gz"
    SHA1 "bf7620ff95a5e6abfa6046154bbe7c1a85972d79"
)

project(BoostProcess VERSION 0.5)

include(CTest)

list(APPEND CMAKE_MODULE_PATH "${BoostProcess_SOURCE_DIR}/cmake")

set(include_install_dir "include")
set(config_install_dir "lib/cmake/${PROJECT_NAME}")

add_library(boost_process INTERFACE)

hunter_add_package(Boost COMPONENTS system filesystem iostreams)

find_package(Boost 1.48 CONFIG REQUIRED system filesystem iostreams)
target_link_libraries(boost_process INTERFACE
  Boost::system
  Boost::filesystem
  Boost::iostreams)
target_include_directories(boost_process INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${include_install_dir}>)
if(WIN32)
  target_compile_definitions(
    boost_process INTERFACE _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN)
endif()

add_subdirectory(libs)

set(config_export_name "${PROJECT_NAME}Config")
set(targets_export_name "${PROJECT_NAME}Targets")
set(project_config "${CMAKE_CURRENT_BINARY_DIR}/${config_export_name}.cmake")
set(version_config "${CMAKE_CURRENT_BINARY_DIR}/${config_export_name}Version.cmake")
set(namespace "${PROJECT_NAME}::")

include(CMakePackageConfigHelpers)
configure_package_config_file( # Uses target_exports_name
    "cmake/Config.cmake.in"
    "${project_config}"
    INSTALL_DESTINATION "${config_install_dir}"
)
write_basic_package_version_file(
  "${version_config}" COMPATIBILITY SameMajorVersion)

install(
  TARGETS boost_process
  EXPORT "${targets_export_name}"
  INCLUDES DESTINATION "${include_install_dir}")

install(
  DIRECTORY "boost"
  DESTINATION "${include_install_dir}")

install(
  EXPORT "${targets_export_name}"
  NAMESPACE "${namespace}"
  DESTINATION "${config_install_dir}")
install(
  FILES "${project_config}" "${version_config}"
  DESTINATION "${config_install_dir}")
