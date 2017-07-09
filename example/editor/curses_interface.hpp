#ifndef EDITOR_CURSES_INTERFACE
#define EDITOR_CURSES_INTERFACE

#include "event.hpp"


struct _win_st;

struct curses_interface_t
{
    curses_interface_t ();
    ~curses_interface_t ();

    event_t next_event () const;

private:
    _win_st * win_;
};

#endif
