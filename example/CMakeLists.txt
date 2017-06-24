include_directories(${CMAKE_HOME_DIRECTORY})

macro(add_sample name)
    add_executable(${name} ${name}.cpp)
    target_compile_options(${name} PRIVATE ${std_flag} -Wall)
    target_link_libraries(${name} text)
    if (clang_on_linux)
        target_link_libraries(${name} c++)
    endif ()
endmacro()

add_sample(hello_world)
