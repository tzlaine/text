cmake_minimum_required(VERSION 3.5)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# This is a standalone CMake build for building the icu_caniter C library, for
# the Python scripts to use.

set(ICU_INCLUDE_DIR "" CACHE PATH "Path to ICU headers.")
set(ICU_LIBRARY_DIR "" CACHE PATH "Path to built ICU libs.")

link_directories(${ICU_LIBRARY_DIR})

add_library(
    icu_caniter
    SHARED
    icu_caniter.cpp
)
target_include_directories(icu_caniter PRIVATE ${ICU_INCLUDE_DIR})  
target_link_libraries(icu_caniter PRIVATE icuuc)  
set_property(TARGET icu_caniter PROPERTY CXX_STANDARD 11)  
if (NOT MSVC)
    target_compile_options(icu_caniter PRIVATE -Wall)
endif ()

add_definitions(-DTEST)
add_executable(icu_caniter_test icu_caniter.cpp)
target_include_directories(icu_caniter_test PRIVATE ${ICU_INCLUDE_DIR})
target_link_libraries(icu_caniter_test PRIVATE icuuc)
set_property(TARGET icu_caniter_test PROPERTY CXX_STANDARD 11)
if (NOT MSVC)
    target_compile_options(icu_caniter_test PRIVATE -Wall)
endif ()
