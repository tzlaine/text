#include "app_state.hpp"


namespace {

    boost::optional<app_state_t> move_up (app_state_t state, screen_pos_t pos)
    {
        auto & s = state.buffer_.snapshot_;
        s.cursor_pos_.row_ = (std::max)(s.cursor_pos_.row_ - 1, 0);
        if (s.cursor_pos_.col_ != state.buffer_.desired_col_)
            s.cursor_pos_.col_ = state.buffer_.desired_col_;
        int line_size = s.cursor_pos_.row_ == s.line_sizes_.size() ?
            0 :
            s.line_sizes_[s.cursor_pos_.row_] - 1;
        if (line_size - 1 < s.cursor_pos_.col_)
            s.cursor_pos_.col_ = line_size;
        return state;
    }

    boost::optional<app_state_t> move_down (app_state_t state, screen_pos_t pos)
    {
        auto & s = state.buffer_.snapshot_;
        s.cursor_pos_.row_ =
            (std::min)(s.cursor_pos_.row_ + 1, (int)s.line_sizes_.size());
        if (s.cursor_pos_.col_ != state.buffer_.desired_col_)
            s.cursor_pos_.col_ = state.buffer_.desired_col_;
        int line_size = s.cursor_pos_.row_ == s.line_sizes_.size() ?
            0 :
            s.line_sizes_[s.cursor_pos_.row_] - 1;
        if (line_size - 1 < s.cursor_pos_.col_)
            s.cursor_pos_.col_ = line_size;
        return state;
    }

    boost::optional<app_state_t> move_left (app_state_t state, screen_pos_t pos)
    {
        auto & s = state.buffer_.snapshot_;
        if (s.cursor_pos_.col_ == 0) {
            if (s.cursor_pos_.row_ == 0)
                return state;
            s.cursor_pos_.row_ -= 1;
            s.cursor_pos_.col_ = s.line_sizes_[s.cursor_pos_.row_] - 1;
        } else {
            s.cursor_pos_.col_ -= 1;
        }
        state.buffer_.desired_col_ = s.cursor_pos_.col_;
        return state;
    }

    boost::optional<app_state_t> move_right (app_state_t state, screen_pos_t pos)
    {
        auto & s = state.buffer_.snapshot_;
        int line_size = s.cursor_pos_.row_ == s.line_sizes_.size() ?
            0 :
            s.line_sizes_[s.cursor_pos_.row_] - 1;
        if (s.cursor_pos_.col_ == line_size) {
            if (s.cursor_pos_.row_ == s.line_sizes_.size())
                return state;
            s.cursor_pos_.row_ += 1;
            s.cursor_pos_.col_ = 0;
        } else {
            s.cursor_pos_.col_ += 1;
        }
        state.buffer_.desired_col_ = s.cursor_pos_.col_;
        return state;
    }

}

key_map_t emacs_lite ()
{
    key_map_t retval;

    retval[up] = move_up;
    retval[down] = move_down;
    retval[left] = move_left;
    retval[right] = move_right;
#if 0
    retval[page_down] = "page-down";
    retval[page_up] = "page-up";
    retval[backspace] = "delete-char";
    retval[delete_] = "delete-char-right";
    
    retval[ctrl-'f'] = "move-left";
    retval[ctrl-'b'] = "move-right";

    retval[alt-'f'] = "move-word-left";
    retval[alt-'b'] = "move-word-right";

    retval[home] = "move-beginning-of-line";
    retval[ctrl-'a'] = "move-beginning-of-line";
    retval[end] = "move-end-of-line";
    retval[ctrl-'e'] = "move-end-of-line";

    retval[ctrl-'k'] = "kill-line";
    retval[ctrl-'w'] = "cut";
    retval[ctrl-'y'] = "paste";
    retval[ctrl-'_'] = "undo";

    retval[ctrl-'x', ctrl-'c'] = "quit";
    retval[ctrl-'x', ctrl-'s'] = "save";
#endif

    return retval;
}

boost::optional<app_state_t> update (app_state_t state, event_t event)
{
    state.input_seq_.append(event.key_code_);

    auto const it = state.key_map_.find(state.input_seq_);
    if (it == state.key_map_.end())
        return state;

    state.input_seq_ = key_sequence_t();
    return it->second(state, screen_pos_t{});
}
