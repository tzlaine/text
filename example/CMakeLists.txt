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
add_sample(text_snippets)

add_sample(hello_world)


if (BUILD_EDITOR)
    add_executable(
        editor
        editor/main.cpp
        editor/app_state.cpp
        editor/curses_interface.cpp
        editor/key_mappings.cpp
    )
    target_compile_options(editor PRIVATE -Wall -D_XOPEN_SOURCE_EXTENDED)
    if (CMAKE_COMPILER_IS_GNUCXX)
        target_compile_options(editor PRIVATE -Wno-terminate)
    endif ()
    target_link_libraries(editor text ${Boost_LIBRARIES} ncurses)
    set_property(TARGET editor PROPERTY CXX_STANDARD ${CXX_STD})
    if (clang_on_linux)
        target_link_libraries(editor c++)
    endif ()
endif ()
