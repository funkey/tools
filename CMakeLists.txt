cmake_minimum_required(VERSION 2.8.12)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)
include(${CMAKE_SOURCE_DIR}/cmake/include/define_module.cmake)

#######################
# project information #
#######################

project(tools)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)

#####################
# compiler settings #
#####################

if(WIN32)
  set(SYSTEM_WINDOWS 1)
else()
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-deprecated-declarations -fomit-frame-pointer -fPIC -std=c++11 -DWITH_BOOST_GRAPH")
  set(CMAKE_CXX_FLAGS_DEBUG   "-g -Wall -Wextra -fPIC -std=c++11 -DWITH_BOOST_GRAPH")
  set(SYSTEM_UNIX 1)
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Release or Debug" FORCE)
endif()

#######################
# project directories #
#######################

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(modules)
add_subdirectory(io)
add_subdirectory(gui)
add_subdirectory(binaries)

###############
# config file #
###############

configure_file(
  ${PROJECT_SOURCE_DIR}/cmake/config.h.in
  ${PROJECT_BINARY_DIR}/config.h
)
