# TODO: Perf tests for:
# for-loop iteration
# std::find
# boyer_moore
# compare

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

#add_custom_target(perf
#    COMMAND ctor_dtor_perf
#    COMMAND copy_perf
#    COMMAND insert_erase_perf
#    COMMAND for_find_perf
#    COMMAND boyer_moore_compare_perf
#)
