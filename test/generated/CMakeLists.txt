if (LIMIT_TESTING_FOR_CI)
    file(GLOB grapheme_break_tests grapheme_break_00.cpp)
    file(GLOB grapheme_iterator_tests grapheme_iterator_00.cpp)
    file(GLOB word_break_tests word_break_00.cpp)
    file(GLOB sentence_break_tests sentence_break_00.cpp)
    file(GLOB line_break_tests line_break_00.cpp)
    file(GLOB normalize_to_nfd_tests normalize_to_nfd_000.cpp)
    file(GLOB normalize_to_nfkd_tests normalize_to_nfkd_000.cpp)
    file(GLOB normalize_to_nfc_tests normalize_to_nfc_000.cpp)
    file(GLOB normalize_to_nfkc_tests normalize_to_nfkc_000.cpp)
    file(GLOB tailoring_rule_tests tailoring_rule_test_gu_standard_000.cpp)
    file(GLOB bidi_tests_a bidi_test_000.cpp)
    file(GLOB bidi_character_tests bidi_character_test_000.cpp)
else()
    file(GLOB grapheme_break_tests grapheme_break_??.cpp)
    file(GLOB grapheme_iterator_tests grapheme_iterator_??.cpp)
    file(GLOB word_break_tests word_break_??.cpp)
    file(GLOB sentence_break_tests sentence_break_??.cpp)
    file(GLOB line_break_tests line_break_??.cpp)
    file(GLOB bidi_tests_a bidi_test_[0-4]??.cpp)
    file(GLOB bidi_tests_b bidi_test_[5-9]??.cpp)
    file(GLOB bidi_character_tests bidi_character_test_???.cpp)
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
)

foreach(test ${globbed_tests})
    get_filename_component(test_name ${test} NAME_WE)
    add_test_executable(${test_name} 1)
endforeach()

foreach(test ${bidi_tests_a})
    get_filename_component(test_name ${test} NAME_WE)
    add_test_executable(${test_name} 2)
endforeach()

foreach(test ${bidi_tests_b})
    get_filename_component(test_name ${test} NAME_WE)
    add_test_executable(${test_name} 3)
endforeach()

foreach(test ${bidi_character_tests})
    get_filename_component(test_name ${test} NAME_WE)
    add_test_executable(${test_name} 4)
endforeach()

set(globbed_tests
    ${normalize_to_nfd_tests}
    ${normalize_to_nfkd_tests}
    ${normalize_to_nfc_tests}
    ${normalize_to_nfkc_tests}
)

foreach(test ${globbed_tests})
    get_filename_component(test_name ${test} NAME_WE)
    add_test_executable(${test_name} 5)
endforeach()

foreach(test ${tailoring_rule_tests})
    get_filename_component(test_name ${test} NAME_WE)
    add_test_executable(${test_name} 6)
endforeach()
