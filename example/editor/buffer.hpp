#ifndef EDITOR_BUFFER_HPP
#define EDITOR_BUFFER_HPP

#include <boost/text/rope.hpp>
#include <boost/filesystem/fstream.hpp>

#include <vector>


struct buffer_t
{
    boost::text::rope content_;
    boost::filesystem::path path_;
    std::vector<boost::text::rope> history_;
};

inline bool dirty (buffer_t const & b)
{ return !b.content_.equal_root(b.history_.back()); }

inline buffer_t load (boost::filesystem::path path)
{
    boost::filesystem::ifstream ifs(path);
    boost::text::rope content;
    while (ifs.good()) {
        boost::text::text chunk;
        int const chunk_size = 1 << 16;
        chunk.resize(chunk_size, ' ');
        ifs.read(chunk.begin(), chunk_size);
        if (!ifs.good())
            chunk.resize(ifs.gcount(), ' ');
        content += std::move(chunk);
    }

    return buffer_t{content, path, {1, content}};
}

#endif
