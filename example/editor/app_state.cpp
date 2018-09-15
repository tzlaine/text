#include "app_state.hpp"

#include <boost/text/utf8.hpp>

#include <boost/algorithm/cxx14/mismatch.hpp>


namespace {

    void up_one_row(snapshot_t & snapshot)
    {
        snapshot.first_row_ -= 1;
        snapshot.first_char_index_ -=
            snapshot.lines_[snapshot.first_row_].code_units_;
    }

    boost::optional<app_state_t>
    move_up(app_state_t state, screen_pos_t, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        if (cursor_line(s) == 0)
            return state;
        else if (s.cursor_pos_.row_ == 0)
            up_one_row(s);
        else
            --s.cursor_pos_.row_;
        if (s.cursor_pos_.col_ != s.desired_col_)
            s.cursor_pos_.col_ = s.desired_col_;
        int const line = cursor_line(s) == s.lines_.size()
                             ? 0
                             : s.lines_[cursor_line(s)].graphemes_;
        if (line - 1 < s.cursor_pos_.col_)
            s.cursor_pos_.col_ = line;
        return state;
    }

    // -2 for two bottom rows
    int nonstatus_height(screen_pos_t screen_size)
    {
        return screen_size.row_ - 2;
    }

    void down_one_row(snapshot_t & snapshot)
    {
        snapshot.first_char_index_ +=
            snapshot.lines_[snapshot.first_row_].code_units_;
        snapshot.first_row_ += 1;
    }

    boost::optional<app_state_t>
    move_down(app_state_t state, screen_pos_t screen_size, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        if (cursor_line(s) == s.lines_.size())
            return state;
        else if (s.cursor_pos_.row_ == nonstatus_height(screen_size) - 1)
            down_one_row(s);
        else
            ++s.cursor_pos_.row_;
        if (s.cursor_pos_.col_ != s.desired_col_)
            s.cursor_pos_.col_ = s.desired_col_;
        int const line = cursor_line(s) == s.lines_.size()
                             ? 0
                             : s.lines_[cursor_line(s)].graphemes_;
        if (line - 1 < s.cursor_pos_.col_)
            s.cursor_pos_.col_ = line;
        return state;
    }

    void set_desired_col(snapshot_t & snapshot)
    {
        snapshot.desired_col_ = snapshot.cursor_pos_.col_;
    }

    boost::optional<app_state_t>
    move_left(app_state_t state, screen_pos_t, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        if (s.cursor_pos_.col_ == 0) {
            if (cursor_line(s) == 0) {
                return state;
            } else if (s.cursor_pos_.row_ == 0) {
                s.first_row_ -= 1;
                s.first_char_index_ -= s.lines_[s.first_row_].graphemes_;
            } else {
                --s.cursor_pos_.row_;
            }
            s.cursor_pos_.col_ = s.lines_[cursor_line(s)].graphemes_;
        } else {
            s.cursor_pos_.col_ -= 1;
        }
        set_desired_col(s);
        return state;
    }

    boost::optional<app_state_t>
    move_right(app_state_t state, screen_pos_t screen_size, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        int const line = cursor_line(s) == s.lines_.size()
                             ? 0
                             : s.lines_[cursor_line(s)].graphemes_;
        if (s.cursor_pos_.col_ == line) {
            if (cursor_line(s) == s.lines_.size()) {
                return state;
            } else if (
                s.cursor_pos_.row_ == nonstatus_height(screen_size) - 1) {
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

    // Returns true if 'it' starts with a code point that has a property that
    // is not a letter or number -- which is what the user intuitively expects
    // a word to be.  Note that this only needs to be checked at word
    // beginnings, so we leave out the "extend" properties below.
    bool between_words(content_t::iterator it)
    {
        auto const prop = boost::text::word_prop(*it->begin());
        return !(
            prop == boost::text::word_property::Katakana ||
            prop == boost::text::word_property::ALetter ||
            prop == boost::text::word_property::Numeric ||
            prop == boost::text::word_property::Regional_Indicator ||
            prop == boost::text::word_property::Hebrew_Letter);
    }

    boost::optional<app_state_t> word_move_left(
        app_state_t state, screen_pos_t screen_width, screen_pos_t xy)
    {
        auto & s = state.buffer_.snapshot_;
        auto const word_and_it = cursor_word(s);
        auto it = word_and_it.cursor_;

        auto content_first = s.content_.begin();
        if (it == word_and_it.word_.begin()) {
            // We're already at the beginning of a word; back up to the
            // previous one.
            if (it != content_first)
                it = boost::text::prev_word_break(s.content_, std::prev(it));
        } else {
            it = word_and_it.word_.begin();
        }

        // The word break algorithm finds anything bounded by word breaks to
        // be a word, even whitespace sequences; keep backing up until we hit
        // a "word" that a starts with a letter, number, etc.
        while (it != content_first && between_words(it)) {
            it = boost::text::prev_word_break(s.content_, std::prev(it));
        }

        std::ptrdiff_t graphemes_to_move =
            std::distance(it, word_and_it.cursor_);
        for (std::ptrdiff_t i = 0; i < graphemes_to_move; ++i) {
            state = *move_left(state, screen_width, xy);
        }

        return state;
    }

    bool after_a_word(content_t::iterator it)
    {
        return !between_words(std::prev(it));
    }

    boost::optional<app_state_t> word_move_right(
        app_state_t state, screen_pos_t screen_width, screen_pos_t xy)
    {
        auto & s = state.buffer_.snapshot_;
        auto const word_and_it = cursor_word(s);
        auto it = word_and_it.cursor_;

        it = word_and_it.word_.end();

        auto content_last = s.content_.end();
        while (it != content_last && !after_a_word(it)) {
            it = boost::text::next_word_break(s.content_, it);
        }

        std::ptrdiff_t graphemes_to_move =
            std::distance(word_and_it.cursor_, it);
        for (std::ptrdiff_t i = 0; i < graphemes_to_move; ++i) {
            state = *move_right(state, screen_width, xy);
        }

        return state; // TODO: Moves!
    }

    boost::optional<app_state_t>
    move_home(app_state_t state, screen_pos_t, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        s.cursor_pos_.col_ = 0;
        set_desired_col(s);
        return state;
    }

    boost::optional<app_state_t>
    move_end(app_state_t state, screen_pos_t, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        if (cursor_at_last_line(s)) {
            s.cursor_pos_.col_ = 0;
        } else {
            auto const line = s.lines_[cursor_line(s)];
            s.cursor_pos_.col_ = line.graphemes_;
        }
        set_desired_col(s);
        return state;
    }

    boost::optional<app_state_t>
    click(app_state_t state, screen_pos_t, screen_pos_t xy)
    {
        auto & s = state.buffer_.snapshot_;
        s.cursor_pos_ = xy;
        s.cursor_pos_.row_ =
            (std::min)(s.lines_.size(), (ptrdiff_t)s.cursor_pos_.row_);
        if (cursor_at_last_line(s)) {
            s.cursor_pos_.col_ = 0;
            return state;
        }
        auto const line = s.lines_[cursor_line(s)];
        s.cursor_pos_.col_ = (std::min)(line.graphemes_, s.cursor_pos_.col_);
        set_desired_col(s);
        return state;
    }

    boost::optional<app_state_t>
    double_click(app_state_t state, screen_pos_t screen_size, screen_pos_t xy)
    {
        state = *click(state, screen_size, xy);
        // TODO: Select word.
        return state;
    }

    boost::optional<app_state_t>
    triple_click(app_state_t state, screen_pos_t screen_size, screen_pos_t xy)
    {
        state = *click(state, screen_size, xy);
        // TODO: Select sentence.
        return state;
    }

    boost::optional<app_state_t>
    move_page_up(app_state_t state, screen_pos_t screen_size, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        auto const page = nonstatus_height(screen_size);
        if (s.lines_.size() <= page)
            return state;
        for (ptrdiff_t i = 0; i < page; ++i) {
            if (!s.first_row_)
                break;
            up_one_row(s);
        }
        return *click(state, screen_size, s.cursor_pos_);
    }

    boost::optional<app_state_t>
    move_page_down(app_state_t state, screen_pos_t screen_size, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        auto const page = nonstatus_height(screen_size);
        if (s.lines_.size() <= page || s.lines_.size() < s.first_row_ + page)
            return state;
        for (ptrdiff_t i = 0; i < page; ++i) {
            down_one_row(s);
        }
        return *click(state, screen_size, s.cursor_pos_);
    }

    boost::optional<app_state_t>
    erase_at(app_state_t state, screen_pos_t, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        state.buffer_.history_.push_back(s);

        auto const cursor_its = cursor_iterators(s);
        if (cursor_at_last_line(s) || cursor_its.cursor_ == s.content_.end())
            return state;

        auto const cursor_grapheme_cus =
            boost::text::storage_bytes(*cursor_its.cursor_);
        auto const line_index = cursor_line(s);

        if (cursor_its.cursor_ == cursor_its.last_) {
            auto const line = s.lines_[cursor_line(s)];
            if (line.hard_break_) {
                auto line = s.lines_[line_index];
                auto const next_line = s.lines_[line_index + 1];
                line.code_units_ += next_line.code_units_ - cursor_grapheme_cus;
                line.graphemes_ += next_line.graphemes_;
                s.lines_.replace(s.lines_.begin() + line_index, line);
                s.lines_.erase(s.lines_.begin() + line_index + 1);
            } else {
                auto next_line = s.lines_[line_index + 1];
                next_line.code_units_ -= cursor_grapheme_cus;
                next_line.graphemes_ -= 1;
                s.lines_.replace(s.lines_.begin() + line_index + 1, next_line);
            }
        } else {
            auto line = s.lines_[line_index];
            line.code_units_ -= cursor_grapheme_cus;
            line.graphemes_ -= 1;
            s.lines_.replace(s.lines_.begin() + line_index, line);
        }

        s.content_.erase(cursor_its.cursor_, std::next(cursor_its.cursor_));

        return state;
    }

    boost::optional<app_state_t>
    erase_before(app_state_t state, screen_pos_t screen_size, screen_pos_t xy)
    {
        auto const initial_pos = state.buffer_.snapshot_.cursor_pos_;
        state = *move_left(state, screen_size, xy);
        if (state.buffer_.snapshot_.cursor_pos_ != initial_pos)
            return *erase_at(state, screen_size, xy);
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
                   screen_pos_t screen_size,
                   screen_pos_t) -> boost::optional<app_state_t> {
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

                line_t line;
                if (!cursor_at_last_line(snapshot))
                    line = snapshot.lines_[line_index];
                line.code_units_ += curr_line_delta.code_units_;
                line.graphemes_ += curr_line_delta.graphemes_;
                if (cursor_at_last_line(snapshot)) {
                    snapshot.lines_.insert(snapshot.lines_.end(), line);
                } else {
                    snapshot.lines_.replace(
                        snapshot.lines_.begin() + line_index, line);
                }

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

    boost::optional<app_state_t>
    undo(app_state_t state, screen_pos_t, screen_pos_t)
    {
        state.buffer_.snapshot_ = state.buffer_.history_.back();
        if (1 < state.buffer_.history_.size())
            state.buffer_.history_.pop_back();
        return state;
    }

    boost::optional<app_state_t> quit(app_state_t, screen_pos_t, screen_pos_t)
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
            if (key_code.key_ <= 256) {
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
        key_map_entry_t{ctrl-'p', move_up},
        key_map_entry_t{up, move_up},
        key_map_entry_t{ctrl-'n', move_down},
        key_map_entry_t{down, move_down},
        key_map_entry_t{ctrl-'b', move_left},
        key_map_entry_t{left, move_left},
        key_map_entry_t{ctrl-'f', move_right},
        key_map_entry_t{right, move_right},

        key_map_entry_t{ctrl-'a', move_home},
        key_map_entry_t{home, move_home},
        key_map_entry_t{ctrl-'e', move_end},
        key_map_entry_t{end, move_end},

        key_map_entry_t{page_up, move_page_up},
        key_map_entry_t{page_down, move_page_down},

        key_map_entry_t{left_click, click},
        key_map_entry_t{left_double_click, double_click},
        key_map_entry_t{left_triple_click, triple_click},

        key_map_entry_t{backspace, erase_before},
        key_map_entry_t{ctrl-'d', erase_at},
        key_map_entry_t{delete_, erase_at},

        key_map_entry_t{alt-'f', word_move_right},
//        key_map_entry_t{alt-right, word_move_right}, // TODO: These crash!
        key_map_entry_t{alt-'b', word_move_left},
//        key_map_entry_t{alt-left, word_move_left},

        key_map_entry_t{ctrl-'_', undo},

        key_map_entry_t{(ctrl-'x', ctrl-'c'), quit},
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
    if (input_evaluation.command_) {
        screen_pos_t const xy{event.key_code_.y_, event.key_code_.x_};
        return input_evaluation.command_(state, event.screen_size_, xy);
    }
    return state;
}
