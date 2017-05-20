include_directories(${CMAKE_HOME_DIRECTORY})

include(CTest)

enable_testing()

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -VV -C ${CMAKE_CFG_INTDIR})

macro(add_test_executable name)
    add_executable(${name} ${name}.cpp)
    target_compile_options(${name} PRIVATE ${std_flag} -Wall)
    target_link_libraries(${name} text gtest gtest_main)
    add_test(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name} --gtest_catch_exceptions=1)
    if (clang_on_linux)
        target_link_libraries(${name} c++)
    endif ()
endmacro()

add_test_executable(text_view)
