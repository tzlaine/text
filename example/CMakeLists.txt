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

# TODO add_sample(utf8_guarantee)
add_sample(slicing)

add_sample(text_view_snippets)
# TODO add_sample(text_snippets)

add_sample(hello_world)
# TODO add_sample(encoding)
add_sample(param_conventions)
add_sample(code_point_to_utf8)


if (BUILD_EDITOR)
    find_package(Curses)
    if (NOT CURSES_FOUND)
        message("-- Could not find libcurses; editor will not be built.")
        set(BUILD_EDITOR false)
    endif ()
endif ()


if (BUILD_EDITOR)
    add_executable(
        rope_editor
        editor/main.cpp
        editor/app_state.cpp
        editor/curses_interface.cpp
        editor/key_mappings.cpp
    )
    target_compile_options(rope_editor PRIVATE -Wall -D_XOPEN_SOURCE_EXTENDED -DUSE_ROPES)
    if (CMAKE_COMPILER_IS_GNUCXX)
        target_compile_options(rope_editor PRIVATE -Wno-terminate)
    endif ()
    target_link_libraries(rope_editor text ${Boost_LIBRARIES} ${CURSES_LIBRARIES})
    set_property(TARGET rope_editor PROPERTY CXX_STANDARD ${CXX_STD})
    if (clang_on_linux)
        target_link_libraries(rope_editor c++)
    endif ()

    add_executable(
        text_editor
        editor/main.cpp
        editor/app_state.cpp
        editor/curses_interface.cpp
        editor/key_mappings.cpp
    )
    target_compile_options(text_editor PRIVATE -Wall -D_XOPEN_SOURCE_EXTENDED)
    if (CMAKE_COMPILER_IS_GNUCXX)
        target_compile_options(text_editor PRIVATE -Wno-terminate)
    endif ()
    target_link_libraries(text_editor text ${Boost_LIBRARIES} ${CURSES_LIBRARIES})
    set_property(TARGET text_editor PROPERTY CXX_STANDARD ${CXX_STD})
    if (clang_on_linux)
        target_link_libraries(text_editor c++)
    endif ()

    if (UNIX AND NOT APPLE) # Linux
        target_include_directories(rope_editor PRIVATE ${CURSES_INCLUDE_DIR}/ncursesw)
        target_include_directories(text_editor PRIVATE ${CURSES_INCLUDE_DIR}/ncursesw)
        target_link_libraries(rope_editor ncursesw)
        target_link_libraries(text_editor ncursesw)
    endif ()
endif ()
