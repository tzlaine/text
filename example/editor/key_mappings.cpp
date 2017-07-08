#include "key_mappings.hpp"


key_map_t emacs_lite ()
{
    key_map_t retval;

#if 0
    retval[up] = "move-up";
    retval[down] = "move-down";
    retval[left] = "move-left";
    retval[right] = "move-right";
    retval[page_down] = "page-down";
    retval[page_up] = "page-up";
    retval[backspace] = "delete-char";
    retval[delete_] = "delete-char-right";
    
    retval[ctrl+'f'] = "move-left";
    retval[ctrl+'b'] = "move-right";

    retval[alt+'f'] = "move-word-left";
    retval[alt+'b'] = "move-word-right";
    retval[alt+backspace] = "delete-word";
    retval[alt+delete_] = "delete-word-right";

    retval[home] = "move-beginning-of-line";
    retval[ctrl+'a'] = "move-beginning-of-line";
    retval[end] = "move-end-of-line";
    retval[ctrl+'e'] = "move-end-of-line";

    retval[ctrl+'k'] ="kill-line";
    retval[ctrl+'w'] ="cut";
    retval[ctrl+'y'] ="paste";
    retval[ctrl+'_'] ="undo";

    retval[ctrl+'x', ctrl+'c'] = "quit";
    retval[ctrl+'x', ctrl+'s'] = "save";
#endif

    return retval;
}
