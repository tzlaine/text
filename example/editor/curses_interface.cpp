#include "curses_interface.hpp"

extern "C" {
#include <ncurses.h>
}

#include <fstream> // TODO


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

// TODO
std::ofstream ofs("log");

event_t curses_interface_t::next_event () const
{
    int k = 0;
    int const mod = wget_wch(win_, &k);

    ofs << "mod=" << mod << " k=" << k << std::endl;
    
    return {key_code_t(mod, k), screen_size()};
}

namespace {

    void render_text (snapshot_t const & snapshot)
    {
        int row = 0;
        char buf[1 << 10]; // Assume lines are <= 1k.
        std::ptrdiff_t pos = 0;
        for (auto line : snapshot.line_sizes_) {
            auto first = snapshot.content_.begin() + pos;
            auto const last = first + line;
            move(row, 0);
            *std::copy(first, last, buf) = '\0';
            addstr(buf);
            pos += line;
            ++row;
        }
    }

}

void render (buffer_t const & buffer, screen_pos_t screen_size)
{
    erase();

    auto const size = screen_pos_t{screen_size.row_ - 2, screen_size.col_};
    render_text(buffer.snapshot_);

    // render the info line
    move(size.row_, 0);
    attrset(A_REVERSE);
    printw(
        " %s %s  (%d, %d)",
        dirty(buffer) ? "**" : "--",
        buffer.path_.c_str(),
        buffer.snapshot_.cursor_pos_.row_,
        buffer.snapshot_.cursor_pos_.col_
    );
    hline(' ', size.col_);

    move(
        buffer.snapshot_.cursor_pos_.row_ - buffer.snapshot_.first_row_,
        buffer.snapshot_.cursor_pos_.col_
    );
    curs_set(true);

    refresh();
}
