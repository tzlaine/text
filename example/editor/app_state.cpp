#include "app_state.hpp"

#include <boost/text/utf8.hpp>

#include <boost/algorithm/cxx14/mismatch.hpp>


namespace {

    boost::optional<app_state_t> move_up (app_state_t state, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        if (s.first_row_ + s.cursor_pos_.row_ == 0) {
            return state;
        } else if (s.cursor_pos_.row_ == 0) {
            s.first_row_ -= 1;
            s.first_char_index_ -= s.line_sizes_[s.first_row_].code_units_;
        } else {
            --s.cursor_pos_.row_;
        }
        if (s.cursor_pos_.col_ != s.desired_col_)
            s.cursor_pos_.col_ = s.desired_col_;
        int const line_size = s.first_row_ + s.cursor_pos_.row_ == s.line_sizes_.size() ?
            0 :
            s.line_sizes_[s.first_row_ + s.cursor_pos_.row_].code_points_;
        if (line_size - 1 < s.cursor_pos_.col_)
            s.cursor_pos_.col_ = line_size;
        return state;
    }

    boost::optional<app_state_t> move_down (app_state_t state, screen_pos_t screen_size)
    {
        auto & s = state.buffer_.snapshot_;
        if (s.first_row_ + s.cursor_pos_.row_ == s.line_sizes_.size()) {
            return state;
        } else if (s.cursor_pos_.row_ == screen_size.row_ - 1 - 2) { // -2 for two bottom rows
            s.first_char_index_ += s.line_sizes_[s.first_row_].code_units_;
            s.first_row_ += 1;
        } else {
            ++s.cursor_pos_.row_;
        }
        if (s.cursor_pos_.col_ != s.desired_col_)
            s.cursor_pos_.col_ = s.desired_col_;
        int const line_size = s.first_row_ + s.cursor_pos_.row_ == s.line_sizes_.size() ?
            0 :
            s.line_sizes_[s.first_row_ + s.cursor_pos_.row_].code_points_;
        if (line_size - 1 < s.cursor_pos_.col_)
            s.cursor_pos_.col_ = line_size;
        return state;
    }

    boost::optional<app_state_t> move_left (app_state_t state, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        if (s.cursor_pos_.col_ == 0) {
            if (s.first_row_ + s.cursor_pos_.row_ == 0) {
                return state;
            } else if (s.cursor_pos_.row_ == 0) {
                s.first_row_ -= 1;
                s.first_char_index_ -= s.line_sizes_[s.first_row_].code_units_;
            } else {
                --s.cursor_pos_.row_;
            }
            s.cursor_pos_.col_ = s.line_sizes_[s.first_row_ + s.cursor_pos_.row_].code_points_;
        } else {
            s.cursor_pos_.col_ -= 1;
        }
        s.desired_col_ = s.cursor_pos_.col_;
        return state;
    }

    boost::optional<app_state_t> move_right (app_state_t state, screen_pos_t screen_size)
    {
        auto & s = state.buffer_.snapshot_;
        int const line_size = s.first_row_ + s.cursor_pos_.row_ == s.line_sizes_.size() ?
            0 :
            s.line_sizes_[s.first_row_ + s.cursor_pos_.row_].code_points_;
        if (s.cursor_pos_.col_ == line_size) {
            if (s.first_row_ + s.cursor_pos_.row_ == s.line_sizes_.size()) {
                return state;
            } else if (s.cursor_pos_.row_ == screen_size.row_ - 1 - 2) { // -2 for two bottom rows
                s.first_char_index_ += s.line_sizes_[s.first_row_].code_units_;
                s.first_row_ += 1;
            } else {
                ++s.cursor_pos_.row_;
            }
            s.cursor_pos_.col_ = 0;
        } else {
            s.cursor_pos_.col_ += 1;
        }
        s.desired_col_ = s.cursor_pos_.col_;
        return state;
    }

    void fixup_lines (
        boost::text::segmented_vector<line_size_t> & line_sizes,
        int line,
        boost::text::rope::const_iterator line_it,
        int cols
    ) {
        while (line < (int)line_sizes.size() &&
               cols < line_sizes[line].code_points_) {
            auto line_size = line_sizes[line];
            auto const line_end = advance_by_code_point(line_it, cols);
            auto const excess_units = line_size.code_units_ - int(line_end - line_it);
            auto const excess_points = std::distance(
                boost::text::utf8::to_utf32_iterator<boost::text::rope::const_iterator>(line_end),
                boost::text::utf8::to_utf32_iterator<boost::text::rope::const_iterator>(line_end + excess_units)
            );
            line_size.code_units_ -= excess_units;
            line_size.code_points_ -= excess_points;
            line_sizes.replace(line_sizes.begin() + line, line_size);

            if (line_end[excess_units - 1] == '\n') {
                auto insert_it = line + 1 <= line_sizes.size() ?
                    line_sizes.begin() + line + 1 :
                    line_sizes.end();
                line_sizes.insert(insert_it, line_size_t{excess_units, excess_points});
            } else {
                line_size = line_sizes[line + 1];
                line_size.code_units_ += excess_units;
                line_size.code_points_ += excess_points;
                line_sizes.replace(line_sizes.begin() + line + 1, line_size);
            }

            line_it = line_end;
            ++line;
        }
    }

    command_t insert (boost::text::text_view tv)
    {
        return [tv] (app_state_t state, screen_pos_t screen_size)
            -> boost::optional<app_state_t>
        {
            auto & s = state.buffer_.snapshot_;
            state.buffer_.history_.push_back(s);
            auto const offset = cursor_offset(s);
            if (tv == "\n") {
                s.content_.insert(offset.rope_offset_, tv);
                auto const line = cursor_line(s);
                line_size_t line_size;
                if (line < s.line_sizes_.size())
                    line_size = s.line_sizes_[line];
                line_size_t const new_line_size{
                    line_size.code_units_ - offset.line_offset_.code_units_,
                    line_size.code_points_ - offset.line_offset_.code_points_
                };
                auto insert_it = line + 1 <= s.line_sizes_.size() ?
                    s.line_sizes_.begin() + line + 1 :
                    s.line_sizes_.end();
                s.line_sizes_.insert(insert_it, new_line_size);
                line_size.code_units_ = offset.line_offset_.code_units_ + 1;
                line_size.code_points_ = offset.line_offset_.code_points_ + 1;
                s.line_sizes_.replace(s.line_sizes_.begin() + line, line_size);
                ++s.cursor_pos_.row_;
                s.cursor_pos_.col_ = 0;
            } else {
                auto const cols = screen_size.col_;
                s.content_.insert(offset.rope_offset_, tv);
                auto const line = cursor_line(s);
                line_size_t line_size;
                if (line < s.line_sizes_.size())
                    line_size = s.line_sizes_[line];
                line_size.code_units_ += tv.size();
                line_size.code_points_ += 1;
                s.cursor_pos_.col_ += 1;
                if (cols <= s.cursor_pos_.col_) {
                    ++s.cursor_pos_.row_;
                    s.cursor_pos_.col_ = 0;
                }

                if (line < s.line_sizes_.size()) {
                    s.line_sizes_.replace(s.line_sizes_.begin() + line, line_size);
                    fixup_lines(
                        s.line_sizes_,
                        line,
                        s.content_.begin() + offset.rope_offset_ - offset.line_offset_.code_units_,
                        cols
                    );
                } else {
                    s.line_sizes_.insert(s.line_sizes_.end(), line_size);
                }
            }
            return state;
        };
    }

    boost::optional<app_state_t> undo (app_state_t state, screen_pos_t)
    {
        state.buffer_.snapshot_ = state.buffer_.history_.back();
        if (1 < state.buffer_.history_.size())
            state.buffer_.history_.pop_back();
        return state;
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

        if (input_seq.single_key()) {
            auto const key_code = input_seq.get_single_key();
            if (key_code.mod_ == 0) {
                if (key_code.key_ == '\n') {
                    return eval_input_t{insert("\n"), true};
                } else if(' ' <= key_code.key_ && key_code.key_ <= '~' &&
                          boost::text::utf8::valid_code_point(key_code.key_)) {
                    static char buf[4];
                    int const * const key_ptr = &key_code.key_;
                    auto const last = std::copy(
                        boost::text::utf8::from_utf32_iterator<int const *>(key_ptr),
                        boost::text::utf8::from_utf32_iterator<int const *>(key_ptr + 1),
                        buf
                    );
                    return eval_input_t{insert(boost::text::text_view(buf, last - buf)), true};
                }
            }
        }

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

        key_map_entry_t{ctrl-'_', undo},

        key_map_entry_t{(ctrl-'x', ctrl-'c'), quit},
    };

#if 0
    retval[backspace] = "delete-before";
    retval[delete_] = "delete-after";
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
        return input_evaluation.command_(state, event.screen_size_);
    return state;
}
