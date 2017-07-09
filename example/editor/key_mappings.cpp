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

key_code_t::key_code_t (char c) : mod_ (OK), key_ (c) {}

key_code_t::key_code_t (key k)
{
    switch(k) {
    case up: *this = {KEY_CODE_YES, KEY_UP};
    case down: *this = {KEY_CODE_YES, KEY_DOWN};
    case left: *this = {KEY_CODE_YES, KEY_LEFT};
    case right: *this = {KEY_CODE_YES, KEY_RIGHT};
    case home: *this = {KEY_CODE_YES, KEY_HOME};
    case end: *this = {KEY_CODE_YES, KEY_END};
    case backspace: *this = {KEY_CODE_YES, KEY_BACKSPACE};
    case delete_: *this = {KEY_CODE_YES, KEY_DC};
    case page_up: *this = {KEY_CODE_YES, KEY_PPAGE};
    case page_down: *this = {KEY_CODE_YES, KEY_NPAGE};
    default:
        assert(!"Unhandled case in key_sequence_t (key)");
    }
}

key_code_t operator- (ctrl_t, char c)
{
    assert('@' <= c && c <= '_');
    if ('a' <= c && c <= 'z')
        c = (c - 'a') + 'A';
    return {OK, c - '@'};
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

key_map_t emacs_lite ()
{
    key_map_t retval;

    retval[up] = "move-up";
    retval[down] = "move-down";
    retval[left] = "move-left";
    retval[right] = "move-right";
    retval[page_down] = "page-down";
    retval[page_up] = "page-up";
    retval[backspace] = "delete-char";
    retval[delete_] = "delete-char-right";
    
    retval[ctrl-'f'] = "move-left";
    retval[ctrl-'b'] = "move-right";

    retval[alt-'f'] = "move-word-left";
    retval[alt-'b'] = "move-word-right";
    retval[alt-backspace] = "delete-word";
    retval[alt-delete_] = "delete-word-right";

    retval[home] = "move-beginning-of-line";
    retval[ctrl-'a'] = "move-beginning-of-line";
    retval[end] = "move-end-of-line";
    retval[ctrl-'e'] = "move-end-of-line";

    retval[ctrl-'k'] ="kill-line";
    retval[ctrl-'w'] ="cut";
    retval[ctrl-'y'] ="paste";
    retval[ctrl-'_'] ="undo";

    retval[ctrl-'x', ctrl-'c'] = "quit";
    retval[ctrl-'x', ctrl-'s'] = "save";

    return retval;
}
