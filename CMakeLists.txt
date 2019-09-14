cmake_minimum_required(VERSION 3.5)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
project(boost_text)

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
if (UNIX)
    set(BUILD_COVERAGE false CACHE BOOL "Set to true to enable code coverage when building tests.  Only Linux and Mac are supported.")
    if (BUILD_COVERAGE)
        message("-- Building for code coverage; disabling any sanitizers")
        if (APPLE)
            set(compile_flags -fprofile-arcs -ftest-coverage)
            set(CMAKE_BUILD_TYPE Debug)
            set(link_flags --coverage)
        else ()
            set(compile_flags --coverage)
            set(CMAKE_BUILD_TYPE Debug)
            set(link_flags --coverage)
        endif ()
    endif ()
endif ()


##################################################
# Dependencies
##################################################
set(boost_components)

# Built conditionally, because it relies on curses, Boost.System, and
# Boost.Filesystem.  The Boost dependencies mean it must come before the
# include here.
set(BUILD_EDITOR false CACHE BOOL "Set to true to build ncurses-based text editor.")
if (BUILD_EDITOR)
    set(boost_components filesystem system)
endif()

include(dependencies)


##################################################
# text library
##################################################
add_library(text
    src/grapheme_break.cpp
    src/word_break.cpp
    src/sentence_break.cpp
    src/line_break.cpp
    src/bidi.cpp
    src/normalization_data_cp_props.cpp
    src/normalization_data_compose.cpp
    src/collation_data_0.cpp
    src/collation_data_1.cpp
    src/case_mapping.cpp
    src/data_versions.cpp
)

target_include_directories(text PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(text PUBLIC boost)
set_property(TARGET text PROPERTY CXX_STANDARD ${CXX_STD})
target_compile_options(text PRIVATE -DBOOST_TEXT_SOURCE)
if (link_flags)
    target_link_libraries(text PUBLIC ${link_flags})
    target_compile_options(text PUBLIC ${compile_flags})
endif ()
if (NOT MSVC)
    target_compile_options(text PRIVATE -Wall)
endif ()
#if (ICU_FOUND)
#    target_compile_options(text PUBLIC -DBOOST_TEXT_HAS_ICU -DBOOST_TEXT_USE_ICU)
#endif ()


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

# See above for editor example.

add_subdirectory(test)
add_subdirectory(perf)
add_subdirectory(example)
