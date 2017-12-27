set(warnings_flag)
if (NOT MSVC)
    set(warnings_flag -Wall)
endif ()

macro(add_perf_executable name)
    add_executable(${name} ${name}.cpp)
    target_compile_options(${name} PRIVATE ${warnings_flag})
    target_link_libraries(${name} text benchmark)
    set_property(TARGET ${name} PROPERTY CXX_STANDARD ${CXX_STD})
    if (clang_on_linux)
        target_link_libraries(${name} c++)
    endif ()
endmacro()

add_perf_executable(ctor_dtor_perf)
add_perf_executable(copy_perf)
add_perf_executable(insert_erase_perf)
add_perf_executable(for_find_perf) 
add_perf_executable(compare_boyer_moore_perf)
add_perf_executable(grapheme_prop_lookup_perf)
add_perf_executable(normalize_perf)
add_perf_executable(collation_element_lookup_perf_000)
add_perf_executable(collation_element_lookup_perf_001)
add_perf_executable(collation_element_lookup_perf_002)
add_perf_executable(collation_element_lookup_perf_003)
add_perf_executable(collation_element_lookup_perf_004)
add_perf_executable(collation_element_lookup_perf_005)
add_perf_executable(collation_element_lookup_perf_006)

add_custom_target(perf
    COMMAND ctor_dtor_perf --benchmark_out=ctor_dtor_perf.json --benchmark_out_format=json
    COMMAND copy_perf --benchmark_out=copy_perf.json --benchmark_out_format=json
    COMMAND insert_erase_perf --benchmark_out=insert_erase_perf.json --benchmark_out_format=json
    COMMAND for_find_perf --benchmark_out=for_find_perf.json --benchmark_out_format=json
    COMMAND compare_boyer_moore_perf --benchmark_out=compare_boyer_moore_perf.json --benchmark_out_format=json
    COMMAND grapheme_prop_lookup_perf --benchmark_out=grapheme_prop_lookup_perf.json --benchmark_out_format=json
    COMMAND normalize_perf --benchmark_out=normalize_perf.json --benchmark_out_format=json
    COMMAND collation_element_lookup_perf_000 --benchmark_out=collation_element_lookup_perf_000.json --benchmark_out_format=json 
    COMMAND collation_element_lookup_perf_001 --benchmark_out=collation_element_lookup_perf_001.json --benchmark_out_format=json 
    COMMAND collation_element_lookup_perf_002 --benchmark_out=collation_element_lookup_perf_002.json --benchmark_out_format=json 
    COMMAND collation_element_lookup_perf_003 --benchmark_out=collation_element_lookup_perf_003.json --benchmark_out_format=json 
    COMMAND collation_element_lookup_perf_004 --benchmark_out=collation_element_lookup_perf_004.json --benchmark_out_format=json 
    COMMAND collation_element_lookup_perf_005 --benchmark_out=collation_element_lookup_perf_005.json --benchmark_out_format=json 
    COMMAND collation_element_lookup_perf_006 --benchmark_out=collation_element_lookup_perf_006.json --benchmark_out_format=json 
)

add_custom_target(perf_snapshot
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/snapshot.py --output-root ${CMAKE_SOURCE_DIR}/perf
)

add_custom_target(perf_comparison
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/ctor_dtor_perf.json ctor_dtor_perf.json
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/copy_perf.json copy_perf.json
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/insert_erase_perf.json insert_erase_perf.json
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/for_find_perf.json for_find_perf.json
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/compare_boyer_moore_perf.json compare_boyer_moore_perf.json
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/grapheme_prop_lookup_perf.json grapheme_prop_lookup_perf.json
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/normalize_perf.json normalize_perf.json
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/collation_element_lookup_perf__000.json collation_element_lookup_perf_000.json
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/collation_element_lookup_perf__001.json collation_element_lookup_perf_001.json
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/collation_element_lookup_perf__002.json collation_element_lookup_perf_002.json
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/collation_element_lookup_perf__003.json collation_element_lookup_perf_003.json
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/collation_element_lookup_perf__004.json collation_element_lookup_perf_004.json
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/collation_element_lookup_perf__005.json collation_element_lookup_perf_005.json
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/collation_element_lookup_perf__006.json collation_element_lookup_perf_006.json
)


##################################
# string_insert_max perf testing #
##################################
macro(add_string_insert_max_perf_executable name size)
    add_executable(${name}_${size} ${name}.cpp)
    target_compile_options(${name}_${size} PRIVATE ${warnings_flag} -DBOOST_TEXT_STRING_INSERT_MAX=${size})
    target_link_libraries(${name}_${size} text benchmark)
    set_property(TARGET ${name}_${size} PROPERTY CXX_STANDARD ${CXX_STD})
    if (clang_on_linux)
        target_link_libraries(${name}_${size} c++)
    endif ()
endmacro()

add_string_insert_max_perf_executable(string_insert_max_perf 128)
add_string_insert_max_perf_executable(string_insert_max_perf 256)
add_string_insert_max_perf_executable(string_insert_max_perf 512)
add_string_insert_max_perf_executable(string_insert_max_perf 1024)
add_string_insert_max_perf_executable(string_insert_max_perf 2048)
add_string_insert_max_perf_executable(string_insert_max_perf 4096)
add_string_insert_max_perf_executable(string_insert_max_perf 8192)
add_string_insert_max_perf_executable(string_insert_max_perf 16384)
add_string_insert_max_perf_executable(string_insert_max_perf 32768)
add_string_insert_max_perf_executable(string_insert_max_perf 65536)

add_custom_target(find_string_insert_max
    COMMAND string_insert_max_perf_128
    COMMAND string_insert_max_perf_256
    COMMAND string_insert_max_perf_512
    COMMAND string_insert_max_perf_1024
    COMMAND string_insert_max_perf_2048
    COMMAND string_insert_max_perf_4096
    COMMAND string_insert_max_perf_8192
    COMMAND string_insert_max_perf_16384
    COMMAND string_insert_max_perf_32768
    COMMAND string_insert_max_perf_65536
)
