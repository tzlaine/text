#include "app_state.hpp"

#include <boost/text/utf8.hpp>

#include <boost/algorithm/cxx14/mismatch.hpp>


namespace {

    std::ptrdiff_t max_row(snapshot_t & snapshot)
    {
        auto retval = snapshot.lines_.size();
        if (!snapshot.lines_.empty() && !snapshot.lines_[-1].hard_break_)
            --retval;
        return retval;
    }

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
            return std::move(state);
        else if (s.cursor_pos_.row_ == 0)
            up_one_row(s);
        else
            --s.cursor_pos_.row_;
        if (s.cursor_pos_.col_ != s.desired_col_)
            s.cursor_pos_.col_ = s.desired_col_;
        int const line =
            cursor_at_last_line(s) ? 0 : s.lines_[cursor_line(s)].graphemes_;
        if (line - 1 < s.cursor_pos_.col_)
            s.cursor_pos_.col_ = line;
        return std::move(state);
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
        if (cursor_line(s) == max_row(s))
            return std::move(state);
        else if (s.cursor_pos_.row_ == nonstatus_height(screen_size) - 1)
            down_one_row(s);
        else
            ++s.cursor_pos_.row_;
        if (s.cursor_pos_.col_ != s.desired_col_)
            s.cursor_pos_.col_ = s.desired_col_;
        int const line =
            cursor_at_last_line(s) ? 0 : s.lines_[cursor_line(s)].graphemes_;
        if (line - 1 < s.cursor_pos_.col_)
            s.cursor_pos_.col_ = line;
        return std::move(state);
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
            if (cursor_line(s) == 0)
                return std::move(state);
            if (s.cursor_pos_.row_ == 0)
                up_one_row(s);
            else
                --s.cursor_pos_.row_;
            s.cursor_pos_.col_ = s.lines_[cursor_line(s)].graphemes_;
        } else {
            s.cursor_pos_.col_ -= 1;
        }
        set_desired_col(s);
        return std::move(state);
    }

    boost::optional<app_state_t>
    move_right(app_state_t state, screen_pos_t screen_size, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        int const line =
            cursor_at_last_line(s) ? 0 : s.lines_[cursor_line(s)].graphemes_;
        if (s.cursor_pos_.col_ == line) {
            if (cursor_line(s) == max_row(s))
                return std::move(state);
            if (s.cursor_pos_.row_ == nonstatus_height(screen_size) - 1)
                down_one_row(s);
            else
                ++s.cursor_pos_.row_;
            s.cursor_pos_.col_ = 0;
        } else {
            s.cursor_pos_.col_ += 1;
        }
        set_desired_col(s);
        return std::move(state);
    }

    // Returns true if 'it' starts with a code point that has a property that
    // is not a letter or number -- which is what the user intuitively expects
    // a word to be.  Note that this only needs to be checked at word
    // beginnings, so we leave out the "extend" properties below.
    bool non_word_grapheme(content_t::iterator it)
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
        while (it != content_first && non_word_grapheme(it)) {
            it = boost::text::prev_word_break(s.content_, std::prev(it));
        }

        std::ptrdiff_t graphemes_to_move =
            std::distance(it, word_and_it.cursor_);
        for (std::ptrdiff_t i = 0; i < graphemes_to_move; ++i) {
            state = *move_left(std::move(state), screen_width, xy);
        }

        return std::move(state);
    }

    bool after_a_word(content_t::iterator it)
    {
        return !non_word_grapheme(std::prev(it));
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
            state = *move_right(std::move(state), screen_width, xy);
        }

        return std::move(state);
    }

    boost::optional<app_state_t>
    move_home(app_state_t state, screen_pos_t, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        s.cursor_pos_.col_ = 0;
        set_desired_col(s);
        return std::move(state);
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
        return std::move(state);
    }

    boost::optional<app_state_t>
    click(app_state_t state, screen_pos_t, screen_pos_t xy)
    {
        auto & s = state.buffer_.snapshot_;
        auto max_row_ = max_row(s);
        s.cursor_pos_ = xy;
        s.cursor_pos_.row_ = (std::min)(max_row_, (ptrdiff_t)s.cursor_pos_.row_);
        if (cursor_at_last_line(s)) {
            s.cursor_pos_.col_ = 0;
            return std::move(state);
        }
        auto const line = s.lines_[cursor_line(s)];
        s.cursor_pos_.col_ = (std::min)(line.graphemes_, s.cursor_pos_.col_);
        set_desired_col(s);
        return std::move(state);
    }

    boost::optional<app_state_t>
    double_click(app_state_t state, screen_pos_t screen_size, screen_pos_t xy)
    {
        state = *click(std::move(state), screen_size, xy);
        // TODO: Select word; requires changing the cursor to a range.
        return std::move(state);
    }

    boost::optional<app_state_t>
    triple_click(app_state_t state, screen_pos_t screen_size, screen_pos_t xy)
    {
        state = *click(std::move(state), screen_size, xy);
        // TODO: Select sentence; requires changing the cursor to a range.
        return std::move(state);
    }

    boost::optional<app_state_t>
    move_page_up(app_state_t state, screen_pos_t screen_size, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        auto const page = nonstatus_height(screen_size);
        if (s.lines_.size() <= page)
            return std::move(state);
        for (ptrdiff_t i = 0; i < page; ++i) {
            if (!s.first_row_)
                break;
            up_one_row(s);
        }
        return *click(std::move(state), screen_size, s.cursor_pos_);
    }

    boost::optional<app_state_t>
    move_page_down(app_state_t state, screen_pos_t screen_size, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;
        auto const page = nonstatus_height(screen_size);
        if (s.lines_.size() <= page || s.lines_.size() < s.first_row_ + page)
            return std::move(state);
        for (ptrdiff_t i = 0; i < page; ++i) {
            down_one_row(s);
        }
        return *click(std::move(state), screen_size, s.cursor_pos_);
    }

    // When an insertion or erasure happens on line line_index, and line_index
    // does not end in a hard break, we need to re-break the lines from the
    // line of the edit to the next hard-break-line, or the end if there is no
    // later hard break.
    void rebreak_wrapped_line(
        app_state_t & state,
        std::ptrdiff_t line_index,
        screen_pos_t screen_size)
    {
        auto & s = state.buffer_.snapshot_;
        assert(line_index < s.lines_.size());

        auto const lines_it = s.lines_.begin() + line_index;
        auto lines_last =
            std::find_if(lines_it, s.lines_.end(), [](line_t line) {
                return line.hard_break_;
            });
        if (lines_last != s.lines_.end())
            ++lines_last;

        auto total = std::accumulate(
            lines_it, lines_last, line_t{}, [](line_t result, line_t line) {
                result.code_units_ += line.code_units_;
                result.graphemes_ += line.graphemes_;
                return result;
            });
        // Include the hard line break grapheme, if any.
        int const hard_break_grapheme = int(lines_last != s.lines_.end());

        auto const grapheme_first = iterator_at_start_of_line(s, line_index);
        auto const grapheme_last =
            std::next(grapheme_first, total.graphemes_ + hard_break_grapheme);
        boost::text::grapheme_range<content_t::iterator::iterator_type> const
            graphemes(grapheme_first, grapheme_last);

        std::vector<line_t> replacements;
        get_lines(graphemes, screen_size.col_, replacements);

        if (!std::prev(lines_last)->code_units_)
            replacements.push_back(*std::prev(lines_last));

        s.lines_.replace(lines_it, lines_last, std::move(replacements));

        state = *click(std::move(state), screen_size, s.cursor_pos_);
    }

    boost::optional<app_state_t>
    erase_at(app_state_t state, screen_pos_t screen_size, screen_pos_t)
    {
        auto & s = state.buffer_.snapshot_;

        auto const cursor_its = cursor_iterators(s);
        if (cursor_at_last_line(s) || cursor_its.cursor_ == s.content_.end())
            return std::move(state);

        state.buffer_.history_.push_back(s);

        auto const cursor_grapheme_cus =
            boost::text::storage_bytes(*cursor_its.cursor_);
        auto line_index = cursor_line(s);

        auto line = s.lines_[line_index];
        if (cursor_its.cursor_ == cursor_its.last_ && line.hard_break_) {
            line.hard_break_ = false;
            line.code_units_ -= 1;
        } else {
            line.code_units_ -= cursor_grapheme_cus;
            line.graphemes_ -= 1;
        }

        if (!line.hard_break_ && line_index + 1 < s.lines_.size()) {
            auto const next_line = s.lines_[line_index + 1];
            line.code_units_ += next_line.code_units_;
            line.graphemes_ += next_line.graphemes_;
            line.hard_break_ = next_line.hard_break_;
            s.lines_.erase(s.lines_.begin() + line_index + 1);
        }

        s.lines_.replace(s.lines_.begin() + line_index, line);
        s.content_.erase(cursor_its.cursor_, std::next(cursor_its.cursor_));
        rebreak_wrapped_line(state, line_index, screen_size);

        return std::move(state);
    }

    boost::optional<app_state_t>
    erase_before(app_state_t state, screen_pos_t screen_size, screen_pos_t xy)
    {
        auto const initial_pos = state.buffer_.snapshot_.cursor_pos_;
        state = *move_left(std::move(state), screen_size, xy);
        if (state.buffer_.snapshot_.cursor_pos_ != initial_pos)
            return *erase_at(std::move(state), screen_size, xy);
        return std::move(state);
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
                                      line.graphemes_ - grapheme_offset,
                                      line.hard_break_};
                snapshot.lines_.insert(
                    snapshot.lines_.begin() + line_index + 1, new_line);

                line.code_units_ = code_unit_offset + 1;
                line.graphemes_ = grapheme_offset;
                line.hard_break_ = true;
                snapshot.lines_.replace(
                    snapshot.lines_.begin() + line_index, line);

                ++snapshot.cursor_pos_.row_;
                snapshot.cursor_pos_.col_ = 0;

                rebreak_wrapped_line(state, line_index + 1, screen_size);
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

                auto rebreak_line_index = line_index;
                auto curr_line_delta = deltas.next_;
                if (!snapshot.cursor_pos_.col_ && deltas.prev_.code_units_) {
                    assert(0 < deltas.prev_.code_units_);
                    assert(!deltas.prev_.graphemes_);
                    line_t prev_line = snapshot.lines_[line_index - 1];
                    prev_line.code_units_ += deltas.prev_.code_units_;
                    prev_line.graphemes_ += deltas.prev_.graphemes_;
                    snapshot.lines_.replace(
                        snapshot.lines_.begin() + line_index - 1, prev_line);
                    --rebreak_line_index;
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

                rebreak_wrapped_line(state, rebreak_line_index, screen_size);
            }

            set_desired_col(snapshot);

            return std::move(state);
        };
    }

    boost::optional<app_state_t>
    undo(app_state_t state, screen_pos_t, screen_pos_t)
    {
        state.buffer_.snapshot_ = state.buffer_.history_.back();
        if (1 < state.buffer_.history_.size())
            state.buffer_.history_.pop_back();
        return std::move(state);
    }

    boost::optional<app_state_t> save(app_state_t state, screen_pos_t, screen_pos_t)
    {
        save_buffer(state.buffer_.path_, state.buffer_);
        state.buffer_.latest_save_ = state.buffer_.snapshot_.content_;
        return std::move(state);
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
        key_map_entry_t{delete_, erase_at},

        key_map_entry_t{alt-'f', word_move_right},
        key_map_entry_t{alt-'b', word_move_left},

        key_map_entry_t{ctrl-'_', undo},

        key_map_entry_t{(ctrl-'x', ctrl-'s'), save},
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
        return input_evaluation.command_(
            std::move(state), event.screen_size_, xy);
    }
    return std::move(state);
}
