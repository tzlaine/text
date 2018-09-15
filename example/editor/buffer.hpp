#ifndef EDITOR_BUFFER_HPP
#define EDITOR_BUFFER_HPP

#include "event.hpp"

#include <boost/text/line_break.hpp>
#include <boost/text/rope.hpp>
#include <boost/text/text.hpp>
#include <boost/text/segmented_vector.hpp>
#include <boost/filesystem/fstream.hpp>

#include <vector>


using content_t = boost::text::rope;

struct line_t
{
    int code_units_ = 0;
    int graphemes_ = 0;
    bool hard_break_ = false;
};

struct snapshot_t
{
    content_t content_;
    boost::text::segmented_vector<line_t> lines_;
    int first_row_ = 0;
    int desired_col_ = 0;
    screen_pos_t cursor_pos_;
    std::ptrdiff_t first_char_index_ = 0;
};

struct buffer_t
{
    snapshot_t snapshot_;
    boost::filesystem::path path_;
    std::vector<snapshot_t> history_;
};

inline bool dirty(buffer_t const & b)
{
    return !b.snapshot_.content_.equal_root(b.history_.front().content_);
}

inline int cursor_line(snapshot_t const & snapshot)
{
    return snapshot.first_row_ + snapshot.cursor_pos_.row_;
}

inline int cursor_at_last_line(snapshot_t const & snapshot)
{
    return cursor_line(snapshot) == snapshot.lines_.size();
}

struct cursor_iterators_t
{
    content_t::iterator first_;
    content_t::iterator cursor_;
    content_t::iterator last_;
};

inline cursor_iterators_t cursor_iterators(snapshot_t const & snapshot)
{
    std::ptrdiff_t offset = snapshot.first_char_index_;
    for (int i = snapshot.first_row_, end = cursor_line(snapshot); i < end;
         ++i) {
        offset += snapshot.lines_[i].code_units_;
    }
    auto const first = snapshot.content_.begin().base().base();
    auto const it = first + offset;
    auto const last = snapshot.content_.end().base().base();
    auto const line_grapheme_first = content_t::iterator{
        content_t::iterator::iterator_type{first, first, last},
        content_t::iterator::iterator_type{first, it, last},
        content_t::iterator::iterator_type{first, last, last}};
    return {line_grapheme_first,
            std::next(line_grapheme_first, snapshot.cursor_pos_.col_),
            std::next(
                line_grapheme_first,
                snapshot.lines_[snapshot.cursor_pos_.row_].graphemes_)};
}

using content_cp_iter_t = content_t::const_iterator::iterator_type;

struct cp_extent
{
    int operator()(content_cp_iter_t first, content_cp_iter_t last) const
        noexcept
    {
        boost::text::grapheme_range<content_cp_iter_t> range(first, last);
        return std::distance(range.begin(), range.end());
    }
};

inline buffer_t load(boost::filesystem::path path, int screen_width)
{
    boost::filesystem::ifstream ifs(path);

    buffer_t retval;
    retval.path_ = std::move(path);

    int const chunk_size = 1 << 16;
    while (ifs.good()) {
        boost::text::string chunk;
        chunk.resize(chunk_size, ' ');

        ifs.read(chunk.begin(), chunk_size);
        if (!ifs.good())
            chunk.resize(ifs.gcount(), ' ');

        retval.snapshot_.content_ += std::move(chunk);
    }

    for (auto line : boost::text::lines(
             retval.snapshot_.content_, screen_width - 1, cp_extent{})) {
        line_t const line_{
            int(line.end().base().base() - line.begin().base().base()),
            (int)std::distance(line.begin(), line.end()) -
                (line.hard_break() ? 1 : 0),
            line.hard_break()};
        retval.snapshot_.lines_.push_back(line_);
    }

    retval.history_.push_back(retval.snapshot_);

    return retval;
}

#endif
