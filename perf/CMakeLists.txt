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

add_custom_target(perf
    COMMAND ctor_dtor_perf --benchmark_out=ctor_dtor_perf.json --benchmark_out_format=json
    COMMAND copy_perf --benchmark_out=copy_perf.json --benchmark_out_format=json
    COMMAND insert_erase_perf --benchmark_out=insert_erase_perf.json --benchmark_out_format=json
    COMMAND for_find_perf --benchmark_out=for_find_perf.json --benchmark_out_format=json
    COMMAND compare_boyer_moore_perf --benchmark_out=compare_boyer_moore_perf.json --benchmark_out_format=json
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
)


################################
# text_insert_max perf testing #
################################
macro(add_text_insert_max_perf_executable name size)
    add_executable(${name}_${size} ${name}.cpp)
    target_compile_options(${name}_${size} PRIVATE ${warnings_flag} -DBOOST_TEXT_TEXT_INSERT_MAX=${size})
    target_link_libraries(${name}_${size} text benchmark)
    set_property(TARGET ${name}_${size} PROPERTY CXX_STANDARD ${CXX_STD})
    if (clang_on_linux)
        target_link_libraries(${name}_${size} c++)
    endif ()
endmacro()

add_text_insert_max_perf_executable(text_insert_max_perf 128)
add_text_insert_max_perf_executable(text_insert_max_perf 256)
add_text_insert_max_perf_executable(text_insert_max_perf 512)
add_text_insert_max_perf_executable(text_insert_max_perf 1024)
add_text_insert_max_perf_executable(text_insert_max_perf 2048)
add_text_insert_max_perf_executable(text_insert_max_perf 4096)
add_text_insert_max_perf_executable(text_insert_max_perf 8192)
add_text_insert_max_perf_executable(text_insert_max_perf 16384)
add_text_insert_max_perf_executable(text_insert_max_perf 32768)
add_text_insert_max_perf_executable(text_insert_max_perf 65536)

add_custom_target(find_text_insert_max
    COMMAND text_insert_max_perf_128
    COMMAND text_insert_max_perf_256
    COMMAND text_insert_max_perf_512
    COMMAND text_insert_max_perf_1024
    COMMAND text_insert_max_perf_2048
    COMMAND text_insert_max_perf_4096
    COMMAND text_insert_max_perf_8192
    COMMAND text_insert_max_perf_16384
    COMMAND text_insert_max_perf_32768
    COMMAND text_insert_max_perf_65536
)
