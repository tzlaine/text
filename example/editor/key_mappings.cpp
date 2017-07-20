#include "key_mappings.hpp"

extern "C" {
#include <ncurses.h>
}


namespace {

    key_code_t to_key_code (boost::text::text_view name)
    {
        assert(0 < (intptr_t)tigetstr((char *)name.begin()));
        int const k = key_defined(name.begin());
        assert(0 < k);
        return {KEY_CODE_YES, k};
    }

}

ctrl_t ctrl;
alt_t alt;

key_code_t::key_code_t (char c) : mod_ (OK), key_ (c) {}

key_code_t::key_code_t (key k)
{
    switch(k) {
    case up: *this = {KEY_CODE_YES, KEY_UP}; break;
    case down: *this = {KEY_CODE_YES, KEY_DOWN}; break;
    case left: *this = {KEY_CODE_YES, KEY_LEFT}; break;
    case right: *this = {KEY_CODE_YES, KEY_RIGHT}; break;
    case home: *this = {KEY_CODE_YES, KEY_HOME}; break;
    case end: *this = {KEY_CODE_YES, KEY_END}; break;
    case backspace: *this = {KEY_CODE_YES, KEY_BACKSPACE}; break;
    case delete_: *this = {KEY_CODE_YES, KEY_DC}; break;
    case page_up: *this = {KEY_CODE_YES, KEY_PPAGE}; break;
    case page_down: *this = {KEY_CODE_YES, KEY_NPAGE}; break;
    default:
        assert(!"Unhandled case in key_sequence_t (key)");
    }
}

key_code_t operator- (ctrl_t, char c)
{
    assert(' ' <= c && c <= '~');
    if ('a' <= c && c <= 'z')
        c -= 96;
    return {OK, c};
}

key_code_t operator- (ctrl_t, key k)
{
    switch (k) {
    case up: return to_key_code("kUP5");
    case down: return to_key_code("kDN5");
    case left: return to_key_code("kLFT5");
    case right: return to_key_code("kRIT5");
    default: assert(!"Unhandled case in operator- (ctrl_t, key)");
    }
    return {}; // This should never execute.
}

key_sequence_t operator- (alt_t, char c)
{ return ctrl-'[', c; }

key_code_t operator- (alt_t, key k)
{
    switch (k) {
    case up: return to_key_code("kUP3");
    case down: return to_key_code("kDN3");
    case left: return to_key_code("kLFT3");
    case right: return to_key_code("kRIT3");
    default: assert(!"Unhandled case in operator- (alt_t, key)");
    }
    return {}; // This should never execute.
}
