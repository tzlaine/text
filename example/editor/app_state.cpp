#include "app_state.hpp"

#include <boost/algorithm/cxx14/mismatch.hpp>


namespace {

    boost::optional<app_state_t> move_up (app_state_t state, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        s.cursor_pos_.row_ = (std::max)(s.cursor_pos_.row_ - 1, 0);
        if (s.cursor_pos_.col_ != state.buffer_.desired_col_)
            s.cursor_pos_.col_ = state.buffer_.desired_col_;
        int line_size = s.cursor_pos_.row_ == s.line_sizes_.size() ?
            0 :
            s.line_sizes_[s.cursor_pos_.row_].code_points_ - 1;
        if (line_size - 1 < s.cursor_pos_.col_)
            s.cursor_pos_.col_ = line_size;
        return state;
    }

    boost::optional<app_state_t> move_down (app_state_t state, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        s.cursor_pos_.row_ =
            (std::min)(s.cursor_pos_.row_ + 1, (int)s.line_sizes_.size());
        if (s.cursor_pos_.col_ != state.buffer_.desired_col_)
            s.cursor_pos_.col_ = state.buffer_.desired_col_;
        int line_size = s.cursor_pos_.row_ == s.line_sizes_.size() ?
            0 :
            s.line_sizes_[s.cursor_pos_.row_].code_points_ - 1;
        if (line_size - 1 < s.cursor_pos_.col_)
            s.cursor_pos_.col_ = line_size;
        return state;
    }

    boost::optional<app_state_t> move_left (app_state_t state, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        if (s.cursor_pos_.col_ == 0) {
            if (s.cursor_pos_.row_ == 0)
                return state;
            s.cursor_pos_.row_ -= 1;
            s.cursor_pos_.col_ = s.line_sizes_[s.cursor_pos_.row_].code_points_ - 1;
        } else {
            s.cursor_pos_.col_ -= 1;
        }
        state.buffer_.desired_col_ = s.cursor_pos_.col_;
        return state;
    }

    boost::optional<app_state_t> move_right (app_state_t state, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        int line_size = s.cursor_pos_.row_ == s.line_sizes_.size() ?
            0 :
            s.line_sizes_[s.cursor_pos_.row_].code_points_ - 1;
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

    command_t insert (boost::text::text_view tv)
    {
        return [tv] (app_state_t state, screen_pos_t) -> boost::optional<app_state_t> {
            return state; // TODO
        };
    }

    boost::optional<app_state_t> quit (app_state_t, screen_pos_t)
    { return boost::none; }

    struct eval_input_t
    {
        command_t command_;
        bool reset_input_;
    };

    eval_input_t eval_input (key_map_t const & key_map, key_sequence_t input_seq)
    {
        bool input_greater_than_all = true;
        for (auto const & key_map_entry : key_map) {
            auto const iters = boost::algorithm::mismatch(
                input_seq.begin(), input_seq.end(),
                key_map_entry.key_seq_.begin(), key_map_entry.key_seq_.end()
            );

            if (iters.first == input_seq.end()) {
                if (iters.second == key_map_entry.key_seq_.end())
                    return eval_input_t{key_map_entry.command_, true};
                input_greater_than_all = false;
            }
        }

#if 0
        if (input_seq.single_key()) {
            auto const key_code = input_seq.get_single_key();
            if (key_code.mod_ == 0 &&
                ' ' <= key_code.key_ && key_code.key_ <= '~') {
                return eval_input_t{input(key_code.key_), true};
            }
        }
#endif

        return eval_input_t{{}, input_greater_than_all};
    }

}

key_map_t emacs_lite ()
{
    key_map_t retval = {
        key_map_entry_t{up, move_up},
        key_map_entry_t{down, move_down},
        key_map_entry_t{left, move_left},
        key_map_entry_t{right, move_right},

        key_map_entry_t{(ctrl-'x', ctrl-'c'), quit},
    };

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

    retval[ctrl-'x', ctrl-'s'] = "save";
#endif

    return retval;
}

boost::optional<app_state_t> update (app_state_t state, event_t event)
{
    state.input_seq_.append(event.key_code_);

    eval_input_t const input_evaluation =
        eval_input(state.key_map_, state.input_seq_);
    if (input_evaluation.reset_input_)
        state.input_seq_ = key_sequence_t();
    if (input_evaluation.command_)
        return input_evaluation.command_(state, screen_pos_t{});
    return state;
}
