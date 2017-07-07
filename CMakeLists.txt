cmake_minimum_required(VERSION 3.5)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

################################################## 
# C++ standard version selection
################################################## 
set(CXX_STD 14 CACHE STRING "Set to 11 or 14 to enable C++11 or C++14 builds, respectively.")
message("-- Using -std=c++${CXX_STD}")


##################################################
# Sanitizers
##################################################
set(USE_ASAN false CACHE BOOL "Set to true to enable -fsanitize=address when building tests.") 
set(USE_UBSAN false CACHE BOOL "Set to true to enable -fsanitize=undefined when building tests.")
if (USE_ASAN AND USE_UBSAN)
    message(FATAL_ERROR "USE_ASAN and USE_UBSAN must not be enabled at the same time")
elseif (USE_ASAN)
    set(compile_flags -fsanitize=address)
    set(link_flags -fsanitize=address)
    message("-- Using -fsanitize=address") 
elseif (USE_UBSAN)
    set(compile_flags -fsanitize=undefined)
    set(link_flags -fsanitize=undefined)
    message("-- Using -fsanitize=undefined")
endif()


##################################################
# Code coverage
##################################################
if (APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL Clang)
    set(BUILD_COVERAGE false CACHE BOOL "Set to true to enable code coverage when building tests.")
    if (BUILD_COVERAGE)
        message("-- Building for code coverage; disabling any sanitizers")
        set(compile_flags -fprofile-arcs -ftest-coverage)
        set(CMAKE_BUILD_TYPE RelWithDebInfo)
        set(link_flags --coverage)
    endif ()
endif ()


##################################################
# text library
##################################################
add_library(text INTERFACE)
target_include_directories(text INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(text INTERFACE boost)
if (link_flags)
    target_link_libraries(text INTERFACE ${link_flags})
    target_compile_options(text INTERFACE ${compile_flags})
endif ()


##################################################
# Dependencies
##################################################
include(dependencies)


##################################################
# Tests and examples
##################################################
# Built conditionally, because it relies on TSan.
set(BUILD_ROPE_THREADSAFETY_TEST false CACHE BOOL "Set to true to build rope the threadsafety test.")
# Built conditionally, because it relies on libFuzzer.
set(BUILD_FUZZ_TESTS false CACHE BOOL "Set to true to build fuzz tests.")

if (BUILD_FUZZ_TESTS AND NOT CMAKE_CXX_COMPILER_ID STREQUAL Clang)
    message(FATAL_ERROR "BUILD_FUZZ_TESTS only works with Clang; it uses libFuzzer.")
endif ()

add_subdirectory(test)
add_subdirectory(example)
