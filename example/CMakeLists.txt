include_directories(${CMAKE_HOME_DIRECTORY})

macro(add_sample name)
    add_executable(${name} ${name}.cpp)
    target_compile_options(${name} PRIVATE -Wall)
    target_link_libraries(${name} text)
    set_property(TARGET ${name} PROPERTY CXX_STANDARD ${CXX_STD})
    if (clang_on_linux)
        target_link_libraries(${name} c++)
    endif ()
endmacro()

add_sample(utf8_guarantee)
add_sample(slicing)

add_sample(text_view_snippets)

add_sample(hello_world)
