include_directories(${CMAKE_HOME_DIRECTORY})

include(CTest)

enable_testing()

add_custom_target(
    check
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/*.table
    COMMAND ${CMAKE_CTEST_COMMAND} -j4 -C ${CMAKE_CFG_INTDIR}
)

set(warnings_flag)
if (NOT MSVC)
    set(warnings_flag -Wall)
endif ()

set(coverage_gcda_files)

# This is used to generate some of the test files.  It is not a test.
add_executable(generate_tailoring_rule_tests generate_tailoring_rule_tests.cpp)
target_compile_options(generate_tailoring_rule_tests PRIVATE ${warnings_flag})
target_link_libraries(generate_tailoring_rule_tests text gtest gtest_main)
set_property(TARGET generate_tailoring_rule_tests PROPERTY CXX_STANDARD ${CXX_STD})
if (clang_on_linux)
    target_link_libraries(generate_tailoring_rule_tests c++)
endif ()

macro(add_test_executable name)
    add_executable(${name} ${name}.cpp)
    target_compile_options(${name} PRIVATE ${warnings_flag})
    target_link_libraries(${name} text gtest gtest_main)
    if (NOT MSVC) # Boost auto-linking handles this with MSVC.
        target_link_libraries(${name} boost_system boost_filesystem)
    endif ()
    set_property(TARGET ${name} PROPERTY CXX_STANDARD ${CXX_STD})
    add_test(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name} --gtest_catch_exceptions=1)
    if (clang_on_linux)
        target_link_libraries(${name} c++)
    endif ()
    list(APPEND coverage_gcda_files ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${name}.dir/${name}.cpp.gcda)
    if (NOT "${ARGN}" STREQUAL "")
        foreach(arg ${ARGN})
            add_custom_command(
                TARGET ${name}
                POST_BUILD
                COMMAND
                    ${CMAKE_COMMAND} -E copy
                        ${CMAKE_CURRENT_SOURCE_DIR}/${arg}
                        ${CMAKE_CURRENT_BINARY_DIR}
            )
        endforeach()
    endif ()
endmacro()

add_test_executable(detail_utility)
add_test_executable(detail_iterator)
add_test_executable(utf8)
add_test_executable(string_view)
add_test_executable(string)
add_test_executable(text_)
add_test_executable(rope)
add_test_executable(detail_btree_util)
add_test_executable(detail_btree)
add_test_executable(detail_btree_2)
add_test_executable(detail_rope)
add_test_executable(detail_rope_btree)
add_test_executable(detail_rope_iterator)
add_test_executable(detail_vector_iterator)
add_test_executable(unencoded_rope_view)
add_test_executable(unencoded_rope)
add_test_executable(segmented_vector)
add_test_executable(common_op)
add_test_executable(algorithm)
add_test_executable(trie_map_0)
add_test_executable(trie_map_1)
add_test_executable(detail_bidi)

set(LIMIT_TESTING_FOR_TRAVIS false CACHE BOOL "Travis kinda sucks, so...")
if (LIMIT_TESTING_FOR_TRAVIS)
    file(GLOB grapheme_break_tests grapheme_break_00.cpp)
    file(GLOB grapheme_iterator_tests grapheme_iterator_00.cpp)
    file(GLOB word_break_tests word_break_00.cpp)
    file(GLOB sentence_break_tests sentence_break_00.cpp)
    file(GLOB line_break_tests line_break_00.cpp)
    file(GLOB bidi_tests bidi_test_000.cpp)
    file(GLOB normalize_to_nfd_tests normalize_to_nfd_000.cpp)
    file(GLOB normalize_to_nfkd_tests normalize_to_nfkd_000.cpp)
    file(GLOB normalize_to_nfc_tests normalize_to_nfc_000.cpp)
    file(GLOB normalize_to_nfkc_tests normalize_to_nfkc_000.cpp)
    file(GLOB tailoring_rule_test_gu_standard_000.cpp)
else()
    file(GLOB grapheme_break_tests grapheme_break_??.cpp)
    file(GLOB grapheme_iterator_tests grapheme_iterator_??.cpp)
    file(GLOB word_break_tests word_break_??.cpp)
    file(GLOB sentence_break_tests sentence_break_??.cpp)
    file(GLOB line_break_tests line_break_??.cpp)
    file(GLOB bidi_tests bidi_test_???.cpp)
    file(GLOB normalize_to_nfd_tests normalize_to_nfd_???.cpp)
    file(GLOB normalize_to_nfkd_tests normalize_to_nfkd_???.cpp)
    file(GLOB normalize_to_nfc_tests normalize_to_nfc_???.cpp)
    file(GLOB normalize_to_nfkc_tests normalize_to_nfkc_???.cpp)
    file(GLOB tailoring_rule_tests tailoring_rule_test_*.cpp)
endif()
set(globbed_tests
    ${grapheme_break_tests}
    ${grapheme_iterator_tests}
    ${word_break_tests}
    ${sentence_break_tests}
    ${line_break_tests}
    ${bidi_tests}
    ${normalize_to_nfd_tests}
    ${normalize_to_nfkd_tests}
    ${normalize_to_nfc_tests}
    ${normalize_to_nfkc_tests}
    ${tailoring_rule_tests}
)

foreach(test ${globbed_tests})
    get_filename_component(test_name ${test} NAME_WE)
    add_test_executable(${test_name})
endforeach()

add_test_executable(break_api) 
add_test_executable(normalization_idempotence)
add_test_executable(relative_collation_test_non_ignorable_0)
add_test_executable(relative_collation_test_non_ignorable_1)
add_test_executable(relative_collation_test_non_ignorable_2)
add_test_executable(relative_collation_test_non_ignorable_3)
add_test_executable(relative_collation_test_non_ignorable_4)
add_test_executable(relative_collation_test_non_ignorable_5)
add_test_executable(relative_collation_test_non_ignorable_6)
add_test_executable(relative_collation_test_shifted_0)
add_test_executable(relative_collation_test_shifted_1)
add_test_executable(relative_collation_test_shifted_2)
add_test_executable(relative_collation_test_shifted_3)
add_test_executable(relative_collation_test_shifted_4)
add_test_executable(relative_collation_test_shifted_5)
add_test_executable(relative_collation_test_shifted_6)
add_test_executable(relative_collation_test_shifted_7)

add_test_executable(lexer)
add_test_executable(lexer_data)
add_test_executable(parser)
add_test_executable(parser_data)
add_test_executable(tailoring riwords.txt)
add_test_executable(tailoring_reorderings)
add_test_executable(tailoring_suppressions)
add_test_executable(tailoring_data)

if (BUILD_COVERAGE AND UNIX)
    if (APPLE)
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
                lcov -e lcov-all.info ${CMAKE_SOURCE_DIR}/include/boost/text/* ${CMAKE_SOURCE_DIR}/include/boost/text/detail/* -o lcov.info
            COMMAND
                genhtml lcov.info -o output
        )
    else ()
        add_custom_target(
            coverage
            rm -rf ${coverage_gcda_files} lcov-all.info lcov.info output
            COMMAND
                ${CMAKE_CTEST_COMMAND} -j4 -VV -C ${CMAKE_CFG_INTDIR}
            COMMAND
                cd ${CMAKE_BINARY_DIR}
            COMMAND
                gcov -f -b ${coverage_gcda_files}
            COMMAND
                lcov --directory . --base-directory . --capture -o lcov-all.info
            COMMAND
                lcov -e lcov-all.info ${CMAKE_SOURCE_DIR}/include/boost/text/* ${CMAKE_SOURCE_DIR}/include/boost/text/detail/* -o lcov.info
            COMMAND
                genhtml lcov.info -o output
        )
    endif ()
endif ()

set(compile_sources
    compile_tests_main.cpp
    compile_include_string_view_1.cpp
    compile_include_string_view_2.cpp
    compile_include_string_view_3.cpp
    compile_include_string_1.cpp
    compile_include_string_2.cpp
    compile_include_string_3.cpp
    compile_include_unencoded_rope_view_1.cpp
    compile_include_unencoded_rope_view_2.cpp
    compile_include_unencoded_rope_view_3.cpp
    compile_include_unencoded_rope_1.cpp
    compile_include_unencoded_rope_2.cpp
    compile_include_unencoded_rope_3.cpp
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
    compile_detail_is_cp_iter.cpp
)

# Set to true to compile the headers with BOOST_TEXT_DOXYGEN defined
if (false)
    set(compile_sources
      ${compile_sources}
      compile_include_string_view_doxygen.cpp
      compile_include_string_doxygen.cpp
      compile_include_unencoded_rope_view_doxygen.cpp
      compile_include_unencoded_rope_doxygen.cpp
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

    file(GLOB fuzzer_sources ${CMAKE_CURRENT_SOURCE_DIR}/Fuzzer/*.cpp)
    add_library(Fuzzer STATIC ${fuzzer_sources})
    target_compile_options(Fuzzer PRIVATE -g -O2 -fno-omit-frame-pointer -std=c++11)

    add_executable(fuzz_rope fuzz_rope.cpp)
    target_compile_options(fuzz_rope PRIVATE ${warnings_flag} -fsanitize-coverage=trace-pc -fsanitize=address)
    target_link_libraries(fuzz_rope text Fuzzer -fsanitize-coverage=trace-pc -fsanitize=address)
    set_property(TARGET fuzz_rope PROPERTY CXX_STANDARD ${CXX_STD})
    add_test(fuzz_rope
             ${CMAKE_CURRENT_BINARY_DIR}/fuzz_rope
             -max_total_time=30)
    if (clang_on_linux)
        target_link_libraries(fuzz_rope c++)
    endif ()

    add_executable(fuzz_segmented_vector fuzz_segmented_vector.cpp)
    target_compile_options(fuzz_segmented_vector PRIVATE ${warnings_flag} -fsanitize-coverage=trace-pc -fsanitize=address)
    target_link_libraries(fuzz_segmented_vector text Fuzzer -fsanitize-coverage=trace-pc -fsanitize=address)
    set_property(TARGET fuzz_segmented_vector PROPERTY CXX_STANDARD ${CXX_STD})
    add_test(fuzz_segmented_vector
             ${CMAKE_CURRENT_BINARY_DIR}/fuzz_segmented_vector
             -max_total_time=30)
    if (clang_on_linux)
        target_link_libraries(fuzz_segmented_vector c++)
    endif ()

    add_executable(fuzz_trie fuzz_trie.cpp)
    target_compile_options(fuzz_trie PRIVATE ${warnings_flag} -fsanitize-coverage=trace-pc -fsanitize=address)
    target_link_libraries(fuzz_trie text Fuzzer -fsanitize-coverage=trace-pc -fsanitize=address)
    set_property(TARGET fuzz_trie PROPERTY CXX_STANDARD ${CXX_STD})
    add_test(fuzz_trie
             ${CMAKE_CURRENT_BINARY_DIR}/fuzz_trie
             -max_len=16 -only_ascii=1 -max_total_time=30)
    if (clang_on_linux)
        target_link_libraries(fuzz_trie c++)
    endif ()
endif ()
