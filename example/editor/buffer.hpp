#ifndef EDITOR_BUFFER_HPP
#define EDITOR_BUFFER_HPP

#include "event.hpp"

#include <boost/text/rope.hpp>
#include <boost/text/segmented_vector.hpp>
#include <boost/filesystem/fstream.hpp>

#include <vector>


struct line_size_t
{
    int code_units_;
    int code_points_;
};

struct snapshot_t
{
    boost::text::rope content_;
    boost::text::segmented_vector<line_size_t> line_sizes_;
    int first_row_ = 0;
    screen_pos_t cursor_pos_;
};

struct buffer_t
{
    snapshot_t snapshot_;
    boost::filesystem::path path_;
    std::vector<snapshot_t> history_;
    int desired_col_; // TODO: Set this when changing history.
};

inline bool dirty (buffer_t const & b)
{ return !b.snapshot_.content_.equal_root(b.history_.back().content_); }

inline char const * advance_by_code_point (char const * str, int code_points)
{
    while (code_points) {
        int bytes = boost::text::utf8::code_point_bytes(*str);
        assert(0 < bytes);
        str += bytes;
        --code_points;
    }
    return str;
}

inline buffer_t load (boost::filesystem::path path, int screen_width)
{
    boost::filesystem::ifstream ifs(path);

    snapshot_t snapshot;
    int line_size = 0;
    int line_cps = 0;
    while (ifs.good()) {
        boost::text::text chunk;
        int const chunk_size = 1 << 16;
        chunk.resize(chunk_size, ' ');
        ifs.read(chunk.begin(), chunk_size);
        if (!ifs.good())
            chunk.resize(ifs.gcount(), ' ');

        auto prev_it = chunk.cbegin();
        auto it = std::find(chunk.cbegin(), chunk.cend(), '\n');
        if (it != chunk.end())
            ++it;
        line_size += it - prev_it;
        line_cps += std::distance(
            boost::text::utf8::to_utf32_iterator(prev_it),
            boost::text::utf8::to_utf32_iterator(it)
        );
        while (it != chunk.end()) {
            auto prev_width_end = prev_it;
            while (screen_width < line_cps) {
                line_cps -= screen_width;
                auto const width_end = advance_by_code_point(prev_width_end, screen_width);
                int const code_units = width_end - prev_width_end;
                line_size -= code_units;
                snapshot.line_sizes_.push_back({code_units, screen_width});
                prev_width_end = width_end;
            }
            snapshot.line_sizes_.push_back({line_size, line_cps});
            line_size = 0;
            line_cps = 0;
            prev_it = it;
            it = std::find(it, chunk.cend(), '\n');
            if (it != chunk.end())
                ++it;
            line_size += it - prev_it;
            line_cps += std::distance(
                boost::text::utf8::to_utf32_iterator(prev_it),
                boost::text::utf8::to_utf32_iterator(it)
            );
        }
        snapshot.content_ += std::move(chunk);
    }

    if (line_size)
        snapshot.line_sizes_.push_back({line_size, line_cps});

    // TODO
    std::ofstream ofs("lines.txt");
    for (auto const width : snapshot.line_sizes_) {
        ofs << width.code_units_ << " " << width.code_points_ << std::endl;
    }

    return buffer_t{snapshot, path, {1, snapshot}, 0};
}

#endif
