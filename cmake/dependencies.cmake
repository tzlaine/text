# Copyright Louis Dionne 2016
# Copyright Zach Laine 2016-2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

###############################################################################
# Boost
###############################################################################
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.64.0 COMPONENTS ${boost_components})
if (Boost_INCLUDE_DIR)
  add_library(boost INTERFACE)
  target_include_directories(boost INTERFACE ${Boost_INCLUDE_DIR})
else ()
  if (NOT BOOST_BRANCH)
    set(BOOST_BRANCH master)
  endif()
  message("-- Boost was not found; it will be cloned locally from ${BOOST_BRANCH}.")
  add_custom_target(
    boost_root_clone
    git clone --depth 100 -b ${BOOST_BRANCH}
      https://github.com/boostorg/boost.git boost_root
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  if (MSVC)
    set(bootstrap_cmd ./bootstrap.bat)
  else()
    set(bootstrap_cmd ./bootstrap.sh)
  endif()
  add_custom_target(
    boost_clone
    COMMAND git submodule init libs/test
    COMMAND git submodule init libs/algorithm
    COMMAND git submodule init libs/align
    COMMAND git submodule init libs/bind
    COMMAND git submodule init libs/concept_check
    COMMAND git submodule init libs/container
    COMMAND git submodule init libs/container_hash
    COMMAND git submodule init libs/exception
    COMMAND git submodule init libs/function
    COMMAND git submodule init libs/integer
    COMMAND git submodule init libs/intrusive
    COMMAND git submodule init libs/io
    COMMAND git submodule init libs/iterator
    COMMAND git submodule init libs/move
    COMMAND git submodule init libs/mpl
    COMMAND git submodule init libs/numeric
    COMMAND git submodule init libs/optional
    COMMAND git submodule init libs/preprocessor
    COMMAND git submodule init libs/range
    COMMAND git submodule init libs/smart_ptr
    COMMAND git submodule init libs/static_assert
    COMMAND git submodule init libs/type_index
    COMMAND git submodule init libs/type_traits
    COMMAND git submodule init libs/throw_exception
    COMMAND git submodule init libs/utility
    COMMAND git submodule init libs/assert
    COMMAND git submodule init libs/config
    COMMAND git submodule init libs/core
    COMMAND git submodule init libs/predef
    COMMAND git submodule init libs/detail
    COMMAND git submodule init tools/build
    COMMAND git submodule init libs/headers
    COMMAND git submodule init tools/boost_install
    COMMAND git submodule update
    COMMAND ${bootstrap_cmd}
    COMMAND ./b2 headers
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root
    DEPENDS boost_root_clone)
  add_library(boost INTERFACE)
  add_dependencies(boost boost_clone)
  target_include_directories(boost INTERFACE ${CMAKE_BINARY_DIR}/boost_root)
endif ()
#set(Boost_USE_STATIC_LIBS ON)
#find_package(Boost 1.64.0 COMPONENTS ${boost_components})
#if (Boost_INCLUDE_DIR)
#  add_library(boost INTERFACE)
#  target_include_directories(boost INTERFACE ${Boost_INCLUDE_DIR})
#else ()
#  message("-- Boost was not found; attempting to download it if we haven't already...")
#  include(ExternalProject)
#  ExternalProject_Add(install-Boost
#    PREFIX ${CMAKE_BINARY_DIR}/dependencies/boost_1_64_0
#    URL https://dl.bintray.com/boostorg/release/1.64.0/source/boost_1_64_0.tar.bz2
#    CONFIGURE_COMMAND ""
#    BUILD_COMMAND ""
#    INSTALL_COMMAND ""
#    LOG_DOWNLOAD ON
#  )
#
#  ExternalProject_Get_Property(install-Boost SOURCE_DIR)
#  add_library(boost INTERFACE)
#  target_include_directories(boost INTERFACE ${SOURCE_DIR})
#  add_dependencies(boost install-Boost)
#  unset(SOURCE_DIR)
#endif ()


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
