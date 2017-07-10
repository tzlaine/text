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

screen_pos_t curses_interface_t::screen_size () const
{ return {getmaxy(stdscr), getmaxx(stdscr)}; }

event_t curses_interface_t::next_event () const
{
    int k = 0;
    int const mod = wget_wch(win_, &k);
    return {key_code_t(mod, k), screen_size()};
}

namespace {

    void render_text (buffer_t const & buffer)
    {
        int col = 0;
        int row = 0;
        move(row, col);

        auto first = buffer.content_.begin();
        auto last = buffer.content_.end();

        char buf[1 << 10]; // Assume lines are <= 1k.
        char * buf_end = buf;
        while (first != last) {
            char const c = *first;
            if (c != '\n') { // TODO: CRLF
                *buf_end++ = c;
            } else {
                *buf_end = '\0';
                move(row, col);
                addstr(buf);
                buf_end = buf;
                ++row;
            }
            ++first;
        }
        *buf_end = '\0';
        move(row, col);
        addstr(buf);
    }
}

void render (buffer_t const & buffer, screen_pos_t screen_size)
{
    erase();

    auto const size = screen_pos_t{screen_size.row_ - 2, screen_size.col_};
    render_text(buffer);

    // render the info line
    move(size.row_, 0);
    attrset(A_REVERSE);
    printw(
        " %s %s  (%d, %d)",
        dirty(buffer) ? "**" : "--",
        buffer.path_.c_str(),
        0, // cursor.col_
        0  // cursor.row_
    );
    hline(' ', size.col_);

    refresh();
}
