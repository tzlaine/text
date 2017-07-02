include_directories(${CMAKE_HOME_DIRECTORY})

include(CTest)

enable_testing()

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -VV -C ${CMAKE_CFG_INTDIR})

set(warnings_flag)
if (NOT MSVC)
    set(warnings_flag -Wall)
endif ()

set(coverage_gcda_files)

macro(add_test_executable name)
    add_executable(${name} ${name}.cpp)
    target_compile_options(${name} PRIVATE ${warnings_flag})
    target_link_libraries(${name} text gtest gtest_main)
    set_property(TARGET ${name} PROPERTY CXX_STANDARD ${CXX_STD})
    add_test(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name} --gtest_catch_exceptions=1)
    if (clang_on_linux)
        target_link_libraries(${name} c++)
    endif ()
    list(APPEND coverage_gcda_files ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${name}.dir/${name}.cpp.gcda)
endmacro()

add_test_executable(detail_utility)
add_test_executable(detail_iterator)
add_test_executable(utf8)
add_test_executable(text_view)
add_test_executable(text_)
add_test_executable(detail_rope)
add_test_executable(detail_rope_btree)
add_test_executable(detail_rope_iterator)
add_test_executable(rope_view)
add_test_executable(rope)
add_test_executable(common_op)
add_test_executable(algorithm)

if (BUILD_COVERAGE)
    add_custom_target(
        coverage
        ${CMAKE_CTEST_COMMAND} -VV -C ${CMAKE_CFG_INTDIR}
        COMMAND
            cd ${CMAKE_BINARY_DIR}
        COMMAND
            llvm-cov gcov -f -b ${coverage_gcda_files}
        COMMAND
            lcov --directory . --base-directory . --gcov-tool ${CMAKE_SOURCE_DIR}/llvm-gcov.sh --capture -o lcov-all.info
        COMMAND
            lcov -e lcov-all.info ${CMAKE_SOURCE_DIR}/boost/text/* -o lcov.info
        COMMAND
            genhtml lcov.info -o output
    )
endif ()

set(compile_sources
    compile_tests_main.cpp
    compile_include_text_view_1.cpp
    compile_include_text_view_2.cpp
    compile_include_text_view_3.cpp
    compile_include_text_1.cpp
    compile_include_text_2.cpp
    compile_include_text_3.cpp
    compile_include_rope_view_1.cpp
    compile_include_rope_view_2.cpp
    compile_include_rope_view_3.cpp
    compile_include_rope_1.cpp
    compile_include_rope_2.cpp
    compile_include_rope_3.cpp
    compile_include_algorithm_1.cpp
    compile_include_algorithm_2.cpp
    compile_detail_is_char_iter.cpp
    compile_detail_is_char_range.cpp
)

# Set to true to compile the headers with BOOST_TEXT_DOXYGEN defiend
if (false)
    set(compile_sources
      ${compile_sources}
      compile_include_text_view_doxygen.cpp
      compile_include_text_doxygen.cpp
      compile_include_rope_view_doxygen.cpp
      compile_include_rope_doxygen.cpp
      compile_include_algorithm_doxygen.cpp
      compile_include_utf8_doxygen.cpp
    )
endif ()

add_executable(compile_tests ${compile_sources})
target_compile_options(compile_tests PRIVATE ${warnings_flag})
target_link_libraries(compile_tests text)
set_property(TARGET compile_tests PROPERTY CXX_STANDARD ${CXX_STD})
if (clang_on_linux)
    target_link_libraries(compile_tests c++)
endif ()
