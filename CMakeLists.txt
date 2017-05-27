cmake_minimum_required(VERSION 3.5)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# TODO: MSVC
set(std_flag -std=c++14)

add_library(text INTERFACE)
target_include_directories(text INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(text INTERFACE boost)

include(dependencies)

add_subdirectory(test)
