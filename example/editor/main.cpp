#include "app_state.hpp"
#include "curses_interface.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <iostream>


int main (int argc, char * argv[])
{
    if (argc < 2) {
        std::cerr << "error: You must supply at least a filename.\n";
        return 1;
    }

    boost::filesystem::path path(argv[1]);
    if (!exists(path)) {
        std::cerr << "error: Could not access filename " << argv[1] << ".\n";
        return 1;
    }

    std::locale::global(std::locale(""));

    curses_interface_t curses_interface;
    app_state_t app_state = {
        load(path, curses_interface.screen_size().col_),
        emacs_lite()
    };
    render(app_state.buffer_, curses_interface.screen_size());

    boost::optional<app_state_t> next_app_state;
    while (next_app_state = update(app_state, curses_interface.next_event())) {
        app_state = *next_app_state;
        render(app_state.buffer_, curses_interface.screen_size());
    }
}
