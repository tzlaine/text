cmake_minimum_required(VERSION 3.5)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# TODO: MSVC
set(std_flag -std=c++14)

##################################################
# Sanitizers
##################################################
set(USE_ASAN false CACHE BOOL "Set to true to enable -fsanitize=address when building tests.") 
set(USE_UBSAN false CACHE BOOL "Set to true to enable -fsanitize=undefined when building tests.")
if (USE_ASAN AND USE_UBSAN)
    message(FATAL_ERROR "USE_ASAN and USE_UBSAN must not be enabled at the same time")
elseif (USE_ASAN)
    set(link_flags -fsanitize=address)
elseif (USE_UBSAN)
    set(link_flags -fsanitize=undefined)
endif()
 
add_library(text INTERFACE)
target_include_directories(text INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(text INTERFACE boost)
target_link_libraries(text INTERFACE ${link_flags})

include(dependencies)

add_subdirectory(test)
