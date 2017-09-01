#include "curses_interface.hpp"

extern "C" {
#include <ncurses.h>
}


curses_interface_t::curses_interface_t() : win_(initscr())
{
    if (win_ != stdscr)
        throw std::runtime_error("ncurses initscr() failed.");

    raw();
    noecho();
    keypad(stdscr, true);
    start_color();
    use_default_colors();
}

curses_interface_t::~curses_interface_t() { endwin(); }

screen_pos_t curses_interface_t::screen_size() const
{
    return {getmaxy(stdscr), getmaxx(stdscr)};
}

event_t curses_interface_t::next_event() const
{
    wint_t k = 0;
    int const mod = wget_wch(win_, &k);
    return {key_code_t(mod, (int)k), screen_size()};
}

namespace {

    void render_text(snapshot_t const & snapshot, screen_pos_t screen_size)
    {
        int row = 0;
        char buf[1 << 10]; // Assume lines are <= 1k.
        std::ptrdiff_t pos = snapshot.first_char_index_;
        auto line_first = snapshot.first_row_;
        auto const line_last = (std::min)(
            line_first + screen_size.row_ - 2,
            (int)snapshot.line_sizes_.size());
        for (; line_first != line_last; ++line_first) {
            auto const line = snapshot.line_sizes_[line_first];
            auto first = snapshot.content_.begin() + pos;
            auto const last = first + line.code_units_;
            move(row, 0);
            auto it = std::copy(first, last, buf);
            if (buf < it && *it == '\n')
                --it;
            if (buf < it && *it == '\r')
                --it;
            *it = '\0';
            addstr(buf);
            pos += line.code_units_;
            ++row;
        }
    }
}

void render(buffer_t const & buffer, screen_pos_t screen_size)
{
    erase();

    auto const size = screen_pos_t{screen_size.row_ - 2, screen_size.col_};
    render_text(buffer.snapshot_, screen_size);

    // render the info line
    move(size.row_, 0);
    attron(A_REVERSE);
    printw(
        " %s %s  (%d, %d)",
#if USE_ROPES
        dirty(buffer) ? "**" : "--",
#else
        "  ",
#endif
        buffer.path_.c_str(),
        buffer.snapshot_.first_row_ + buffer.snapshot_.cursor_pos_.row_ + 1,
        buffer.snapshot_.cursor_pos_.col_);
    attroff(A_REVERSE);
    hline(' ', size.col_);

    move(buffer.snapshot_.cursor_pos_.row_, buffer.snapshot_.cursor_pos_.col_);
    curs_set(true);

    refresh();
}
