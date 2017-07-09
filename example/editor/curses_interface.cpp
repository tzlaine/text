#include "curses_interface.hpp"

extern "C" {
#include <ncurses.h>
}


curses_interface_t::curses_interface_t () :
    win_ (initscr())
{
    if (win_ != stdscr)
        throw std::runtime_error("ncurses initscr() failed.");

    raw();
    noecho();
    keypad(stdscr, true);
    start_color();
    use_default_colors();
}

curses_interface_t::~curses_interface_t ()
{ endwin(); }

event_t curses_interface_t::next_event () const
{
    int k = 0;
    int const mod = wget_wch(win_, &k);
    return {
        key_code_t(mod, k),
        screen_pos_t{getmaxy(stdscr), getmaxx(stdscr)}
    };
}
