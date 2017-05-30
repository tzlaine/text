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

add_test_executable(detail_utility)
add_test_executable(detail_iterator)
add_test_executable(utf8)
add_test_executable(text_view)
add_test_executable(text_)
add_test_executable(rope)
add_test_executable(algorithm)

add_executable(
    compile_tests
    compile_tests_main.cpp
    compile_include_text_view_1.cpp
    compile_include_text_view_2.cpp
    compile_include_text_view_3.cpp
    compile_include_text_1.cpp
    compile_include_text_2.cpp
    compile_include_text_3.cpp
    compile_include_algorithm_1.cpp
    compile_include_algorithm_2.cpp
    compile_detail_is_char_range.cpp
)
target_compile_options(compile_tests PRIVATE ${std_flag} -Wall)
target_link_libraries(compile_tests text)
if (clang_on_linux)
    target_link_libraries(compile_tests c++)
endif ()
