#include "app_state.hpp"

#include <boost/text/utf8.hpp>

#include <boost/algorithm/cxx14/mismatch.hpp>


namespace {

    boost::optional<app_state_t> move_up(app_state_t state, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        if (s.first_row_ + s.cursor_pos_.row_ == 0) {
            return state;
        } else if (s.cursor_pos_.row_ == 0) {
            s.first_row_ -= 1;
            s.first_char_index_ -= s.lines_[s.first_row_].code_units_;
        } else {
            --s.cursor_pos_.row_;
        }
        if (s.cursor_pos_.col_ != s.desired_col_)
            s.cursor_pos_.col_ = s.desired_col_;
        int const line =
            s.first_row_ + s.cursor_pos_.row_ == s.lines_.size()
                ? 0
                : s.lines_[s.first_row_ + s.cursor_pos_.row_].graphemes_;
        if (line - 1 < s.cursor_pos_.col_)
            s.cursor_pos_.col_ = line;
        return state;
    }

    boost::optional<app_state_t>
    move_down(app_state_t state, screen_pos_t screen_size)
    {
        auto & s = state.buffer_.snapshot_;
        if (s.first_row_ + s.cursor_pos_.row_ == s.lines_.size()) {
            return state;
        } else if (s.cursor_pos_.row_ == screen_size.row_ - 1 - 2) {
            // -2 for two bottom rows
            s.first_char_index_ += s.lines_[s.first_row_].code_units_;
            s.first_row_ += 1;
        } else {
            ++s.cursor_pos_.row_;
        }
        if (s.cursor_pos_.col_ != s.desired_col_)
            s.cursor_pos_.col_ = s.desired_col_;
        int const line =
            s.first_row_ + s.cursor_pos_.row_ == s.lines_.size()
                ? 0
                : s.lines_[s.first_row_ + s.cursor_pos_.row_].graphemes_;
        if (line - 1 < s.cursor_pos_.col_)
            s.cursor_pos_.col_ = line;
        return state;
    }

    void set_desired_col(snapshot_t & snapshot)
    {
        snapshot.desired_col_ = snapshot.cursor_pos_.col_;
    }

    boost::optional<app_state_t> move_left(app_state_t state, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        if (s.cursor_pos_.col_ == 0) {
            if (s.first_row_ + s.cursor_pos_.row_ == 0) {
                return state;
            } else if (s.cursor_pos_.row_ == 0) {
                s.first_row_ -= 1;
                s.first_char_index_ -= s.lines_[s.first_row_].graphemes_;
            } else {
                --s.cursor_pos_.row_;
            }
            s.cursor_pos_.col_ =
                s.lines_[s.first_row_ + s.cursor_pos_.row_].graphemes_;
        } else {
            s.cursor_pos_.col_ -= 1;
        }
        set_desired_col(s);
        return state;
    }

    boost::optional<app_state_t>
    move_right(app_state_t state, screen_pos_t screen_size)
    {
        auto & s = state.buffer_.snapshot_;
        int const line =
            s.first_row_ + s.cursor_pos_.row_ == s.lines_.size()
                ? 0
                : s.lines_[s.first_row_ + s.cursor_pos_.row_].graphemes_;
        if (s.cursor_pos_.col_ == line) {
            if (s.first_row_ + s.cursor_pos_.row_ == s.lines_.size()) {
                return state;
            } else if (
                s.cursor_pos_.row_ ==
                screen_size.row_ - 1 - 2) { // -2 for two bottom rows
                s.first_char_index_ += s.lines_[s.first_row_].graphemes_;
                s.first_row_ += 1;
            } else {
                ++s.cursor_pos_.row_;
            }
            s.cursor_pos_.col_ = 0;
        } else {
            s.cursor_pos_.col_ += 1;
        }
        set_desired_col(s);
        return state;
    }

    struct edit_deltas
    {
        line_t prev_;
        line_t next_;
    };

    template<typename CPIter>
    edit_deltas grapheme_insertion_deltas(
        boost::text::grapheme_view<CPIter> prev_grapheme,
        boost::text::grapheme insertion,
        boost::text::grapheme_view<CPIter> next_grapheme)
    {
        boost::text::text t;
        t.insert(t.end(), prev_grapheme);
        auto it = t.insert(t.end(), next_grapheme);
        auto const initial_distance = t.distance();
        t.insert(it, insertion);
        auto const distance = t.distance();
        if (distance == initial_distance + 1) {
            return {{}, {boost::text::storage_bytes(insertion), 1}};
        } else {
            assert(distance == initial_distance);
            int const prev_sizes[2] = {
                boost::text::storage_bytes(prev_grapheme),
                boost::text::storage_bytes(next_grapheme)};
            int const sizes[2] = {
                boost::text::storage_bytes(*t.begin()),
                boost::text::storage_bytes(*std::next(t.begin()))};
            return {{sizes[0] - prev_sizes[0], 0},
                    {sizes[1] - prev_sizes[1], 0}};
        }
    }

    command_t insert(boost::text::grapheme grapheme)
    {
        return [grapheme](
                   app_state_t state,
                   screen_pos_t screen_size) -> boost::optional<app_state_t> {
            auto & snapshot = state.buffer_.snapshot_;
            state.buffer_.history_.push_back(snapshot);

            auto const line_index = cursor_line(snapshot);
            auto const cursor_its = cursor_iterators(snapshot);
            int const code_unit_offset = cursor_its.cursor_.base().base() -
                                         cursor_its.first_.base().base();
            int const grapheme_offset = snapshot.cursor_pos_.col_;

            if (boost::text::storage_bytes(grapheme) == 1 &&
                *grapheme.begin() == '\n') {
                snapshot.content_.insert(cursor_its.cursor_, grapheme);
                line_t line;
                if (line_index < snapshot.lines_.size())
                    line = snapshot.lines_[line_index];
                line_t const new_line{line.code_units_ - code_unit_offset,
                                      line.graphemes_ - grapheme_offset};
                auto insert_it = line_index + 1 <= snapshot.lines_.size()
                                     ? snapshot.lines_.begin() + line_index + 1
                                     : snapshot.lines_.end();
                snapshot.lines_.insert(insert_it, new_line);
                line.code_units_ = code_unit_offset + 1;
                line.graphemes_ = grapheme_offset;
                snapshot.lines_.replace(
                    snapshot.lines_.begin() + line_index, line);
                ++snapshot.cursor_pos_.row_;
                snapshot.cursor_pos_.col_ = 0;
            } else {
                using graphme_view = decltype(*cursor_its.first_);
                graphme_view prev_grapheme;
                if (cursor_its.cursor_ != snapshot.content_.begin())
                    prev_grapheme = *std::prev(cursor_its.cursor_);
                graphme_view next_grapheme;
                if (cursor_its.cursor_ != snapshot.content_.end())
                    next_grapheme = *cursor_its.cursor_;
                auto const deltas = grapheme_insertion_deltas(
                    prev_grapheme, grapheme, next_grapheme);

                auto curr_line_delta = deltas.next_;
                if (!snapshot.cursor_pos_.col_ && deltas.prev_.code_units_) {
                    assert(0 < deltas.prev_.code_units_);
                    assert(!deltas.prev_.graphemes_);
                    line_t prev_line = snapshot.lines_[line_index - 1];
                    prev_line.code_units_ += deltas.prev_.code_units_;
                    prev_line.graphemes_ += deltas.prev_.graphemes_;
                    snapshot.lines_.replace(
                        snapshot.lines_.begin() + line_index - 1, prev_line);
                } else {
                    curr_line_delta.code_units_ += deltas.prev_.code_units_;
                    curr_line_delta.graphemes_ += deltas.prev_.graphemes_;
                }

                line_t line = snapshot.lines_[line_index];
                line.code_units_ += curr_line_delta.code_units_;
                line.graphemes_ += curr_line_delta.graphemes_;
                snapshot.lines_.replace(
                    snapshot.lines_.begin() + line_index, line);

                snapshot.content_.insert(cursor_its.cursor_, grapheme);
                snapshot.cursor_pos_.col_ += 1;
                if (screen_size.col_ <= snapshot.cursor_pos_.col_) {
                    ++snapshot.cursor_pos_.row_;
                    snapshot.cursor_pos_.col_ = 0;
                }
            }

            set_desired_col(snapshot);

            return state;
        };
    }

    boost::optional<app_state_t> undo(app_state_t state, screen_pos_t)
    {
        state.buffer_.snapshot_ = state.buffer_.history_.back();
        if (1 < state.buffer_.history_.size())
            state.buffer_.history_.pop_back();
        return state;
    }

    boost::optional<app_state_t> quit(app_state_t, screen_pos_t)
    {
        return boost::none;
    }

    struct eval_input_t
    {
        command_t command_;
        bool reset_input_;
    };

    eval_input_t eval_input(key_map_t const & key_map, key_sequence_t input_seq)
    {
        bool input_greater_than_all = true;
        for (auto const & key_map_entry : key_map) {
            auto const iters = boost::algorithm::mismatch(
                input_seq.begin(),
                input_seq.end(),
                key_map_entry.key_seq_.begin(),
                key_map_entry.key_seq_.end());

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
                    return eval_input_t{insert(boost::text::grapheme('\n')),
                                        true};
                } else if (
                    ' ' <= key_code.key_ && key_code.key_ <= '~' &&
                    boost::text::utf8::valid_code_point(key_code.key_)) {
                    return eval_input_t{
                        insert(boost::text::grapheme(key_code.key_)), true};
                }
            }
        }

        return eval_input_t{{}, input_greater_than_all};
    }
}

key_map_t emacs_lite()
{
    key_map_t retval = {
        key_map_entry_t{up, move_up},
        key_map_entry_t{down, move_down},
        key_map_entry_t{left, move_left},
        key_map_entry_t{right, move_right},

        key_map_entry_t{ctrl - '_', undo},

        key_map_entry_t{(ctrl - 'x', ctrl - 'c'), quit},
    };

    return retval;
}

boost::optional<app_state_t> update(app_state_t state, event_t event)
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
