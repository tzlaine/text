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
add_test_executable(detail_btree_util)
add_test_executable(detail_btree)
add_test_executable(detail_btree_2)
add_test_executable(detail_rope)
add_test_executable(detail_rope_btree)
add_test_executable(detail_rope_iterator)
add_test_executable(rope_view)
add_test_executable(rope)
add_test_executable(common_op)
add_test_executable(algorithm)

add_test_executable(grapheme_prop_lookup_00)
add_test_executable(grapheme_prop_lookup_01)
add_test_executable(grapheme_prop_lookup_02)
add_test_executable(grapheme_prop_lookup_03)
add_test_executable(grapheme_prop_lookup_04)
add_test_executable(grapheme_prop_lookup_05)
add_test_executable(grapheme_prop_lookup_06)
add_test_executable(grapheme_prop_lookup_07)
add_test_executable(grapheme_prop_lookup_08)
add_test_executable(word_prop_lookup_00)
add_test_executable(word_prop_lookup_01)
add_test_executable(word_prop_lookup_02)
add_test_executable(word_prop_lookup_03)
add_test_executable(word_prop_lookup_04)
add_test_executable(word_prop_lookup_05)
add_test_executable(word_prop_lookup_06)
add_test_executable(word_prop_lookup_07)
add_test_executable(word_prop_lookup_08)
add_test_executable(word_prop_lookup_09)
add_test_executable(word_prop_lookup_10)
add_test_executable(word_prop_lookup_11)
add_test_executable(word_prop_lookup_12)
add_test_executable(word_prop_lookup_13)
add_test_executable(word_prop_lookup_14)
add_test_executable(word_prop_lookup_15)
add_test_executable(word_prop_lookup_16)
add_test_executable(word_prop_lookup_17)
add_test_executable(word_prop_lookup_18)
add_test_executable(word_prop_lookup_19)
add_test_executable(word_prop_lookup_20)

add_test_executable(grapheme_break_00)
add_test_executable(grapheme_break_01)
add_test_executable(grapheme_break_02)
add_test_executable(grapheme_break_03)
add_test_executable(grapheme_break_04)
add_test_executable(grapheme_break_05)
add_test_executable(grapheme_break_06)
add_test_executable(grapheme_break_07)
add_test_executable(grapheme_break_08)

# TODO: A totally different approach is required for word breaks.  Word break
# parsing is LL(*).
#add_test_executable(word_break_00)
#add_test_executable(word_break_01)
#add_test_executable(word_break_02)
#add_test_executable(word_break_03)
#add_test_executable(word_break_04)
#add_test_executable(word_break_05)
#add_test_executable(word_break_06)
#add_test_executable(word_break_07)
#add_test_executable(word_break_08)
#add_test_executable(word_break_09)
#add_test_executable(word_break_10)
#add_test_executable(word_break_10)
#add_test_executable(word_break_11)
#add_test_executable(word_break_12)
#add_test_executable(word_break_13)
#add_test_executable(word_break_14)
#add_test_executable(word_break_15)
#add_test_executable(word_break_16)
#add_test_executable(word_break_17)
#add_test_executable(word_break_18)
#add_test_executable(word_break_19)
#add_test_executable(word_break_20)

add_test_executable(grapheme_iterator_00)
add_test_executable(grapheme_iterator_01)
add_test_executable(grapheme_iterator_02)
add_test_executable(grapheme_iterator_03)
add_test_executable(grapheme_iterator_04)
add_test_executable(grapheme_iterator_05)
add_test_executable(grapheme_iterator_06)
add_test_executable(grapheme_iterator_07)
add_test_executable(grapheme_iterator_08)

if (BUILD_COVERAGE)
    add_custom_target(
        coverage
        rm -rf ${coverage_gcda_files} lcov-all.info lcov.info output
        COMMAND
            ${CMAKE_CTEST_COMMAND} -VV -C ${CMAKE_CFG_INTDIR}
        COMMAND
            cd ${CMAKE_BINARY_DIR}
        COMMAND
            llvm-cov gcov -f -b ${coverage_gcda_files}
        COMMAND
            lcov --directory . --base-directory . --gcov-tool ${CMAKE_SOURCE_DIR}/llvm-gcov.sh --capture -o lcov-all.info
        COMMAND
            lcov -e lcov-all.info ${CMAKE_SOURCE_DIR}/boost/text/* ${CMAKE_SOURCE_DIR}/boost/text/detail/* -o lcov.info
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
    compile_include_vector_1.cpp
    compile_include_vector_2.cpp
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

if (BUILD_ROPE_THREADSAFETY_TEST)
    if (USE_ASAN OR USE_UBSAN)
        message(FATAL_ERROR "BUILD_ROPE_THREADSAFETY_TEST must not be enabled at the same time as USE_ASAN or USE_UBSAN")
    endif ()
    add_executable(rope_threadsafety rope_threadsafety.cpp)
    target_compile_options(rope_threadsafety PRIVATE ${warnings_flag} -fsanitize=thread)
    target_link_libraries(rope_threadsafety text -fsanitize=thread)
    if (UNIX AND NOT APPLE) # Linux
        target_compile_options(rope_threadsafety PRIVATE -pthread)
        target_link_libraries(rope_threadsafety -pthread)
    endif ()
    set_property(TARGET rope_threadsafety PROPERTY CXX_STANDARD ${CXX_STD})
    add_test(rope_threadsafety ${CMAKE_CURRENT_BINARY_DIR}/rope_threadsafety)
    if (clang_on_linux)
        target_link_libraries(rope_threadsafety c++)
    endif ()
endif ()

if (BUILD_FUZZ_TESTS)
    if (NOT USE_ASAN AND NOT USE_UBSAN)
        message("BUILD_FUZZ_TEST would greatly benefit from also turning on USE_ASAN or USE_UBSAN")
    endif ()

    add_executable(fuzz_rope fuzz_rope.cpp)
    target_compile_options(fuzz_rope PRIVATE ${warnings_flag} -fsanitize-coverage=trace-pc -fsanitize=address)
    target_link_libraries(fuzz_rope text ${CMAKE_CURRENT_BINARY_DIR}/libFuzzer.a -fsanitize-coverage=trace-pc -fsanitize=address)
    set_property(TARGET fuzz_rope PROPERTY CXX_STANDARD ${CXX_STD})
    add_test(fuzz_rope ${CMAKE_CURRENT_BINARY_DIR}/fuzz_rope)
    if (clang_on_linux)
        target_link_libraries(fuzz_rope c++)
    endif ()

    add_custom_target(build_libfuzzer ${CMAKE_CURRENT_SOURCE_DIR}/Fuzzer/build.sh)
    add_dependencies(fuzz_rope build_libfuzzer)
endif ()
