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
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py ctor_dtor_perf.json  ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/ctor_dtor_perf.json
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py copy_perf.json  ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/copy_perf.json
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py insert_erase_perf.json  ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/insert_erase_perf.json
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py for_find_perf.json  ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/for_find_perf.json
    COMMAND ${CMAKE_SOURCE_DIR}/benchmark-v1.2.0/tools/compare_bench.py compare_boyer_moore_perf.json  ${CMAKE_SOURCE_DIR}/perf/latest_snapshot/compare_boyer_moore_perf.json
)
