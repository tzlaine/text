# Copyright Louis Dionne 2016
# Copyright Zach Laine 2016-2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

###############################################################################
# Boost
###############################################################################
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.71.0 COMPONENTS ${boost_components})
if (Boost_INCLUDE_DIR)
  add_library(boost INTERFACE)
  target_include_directories(boost INTERFACE ${Boost_INCLUDE_DIR})
else ()
  if (NOT EXISTS ${CMAKE_BINARY_DIR}/boost_root)
    if (NOT BOOST_BRANCH)
      set(BOOST_BRANCH master)
    endif()
    message("-- Boost was not found; it will be cloned locally from ${BOOST_BRANCH}.")
    execute_process(
      COMMAND
      git clone --depth 100 -b ${BOOST_BRANCH}
        https://github.com/boostorg/boost.git boost_root
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    if (MSVC)
      set(bootstrap_cmd ./bootstrap.bat)
    else()
      set(bootstrap_cmd ./bootstrap.sh)
    endif()
    execute_process(
      COMMAND git submodule init libs/test
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/algorithm
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/align
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/array
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/bind
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/concept_check
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/container
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/container_hash
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/exception
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/filesystem
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/function
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/integer
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/intrusive
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/io
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/iterator
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/move
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/mpl
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/numeric
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/optional
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/preprocessor
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/range
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/smart_ptr
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/static_assert
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/system
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/type_index
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/type_traits
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/throw_exception
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/utility
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/assert
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/config
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/core
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/predef
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/detail
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init tools/build
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init libs/headers
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule init tools/boost_install
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND git submodule update --jobs 3
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND ${bootstrap_cmd}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
    execute_process(
      COMMAND ./b2 headers
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root)
  endif()
  add_library(boost INTERFACE)
  target_include_directories(boost INTERFACE ${CMAKE_BINARY_DIR}/boost_root)
  set(Boost_INCLUDE_DIR ${CMAKE_BINARY_DIR}/boost_root)
endif ()


###############################################################################
# GoogleTest
###############################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/googletest-release-1.8.0)
target_include_directories(gtest      INTERFACE ${CMAKE_HOME_DIRECTORY}/googletest-release-1.8.0/googletest/include)
target_include_directories(gtest_main INTERFACE ${CMAKE_HOME_DIRECTORY}/googletest-release-1.8.0/googletest/include)


###############################################################################
# Google Benchmark
###############################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/benchmark-v1.2.0)


###############################################################################
# ICU, for perf comparisons
###############################################################################
if (MSVC)
    find_package(ICU 60 COMPONENTS in dt uc)
else()
    find_package(ICU 60 COMPONENTS i18n data uc)
endif()
