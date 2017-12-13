#include <boost/text/text.hpp>

#include "strings.hpp"


namespace {

    boost::text::text_view make_text_view(int i)
    {
        return boost::text::text_view(strings[i]);
    }

    boost::text::text_view text_views[14] = {make_text_view(0),
                                             make_text_view(1),
                                             make_text_view(2),
                                             make_text_view(3),
                                             make_text_view(4),
                                             make_text_view(5),
                                             make_text_view(6),
                                             make_text_view(7),
                                             make_text_view(8),
                                             make_text_view(9),
                                             make_text_view(10),
                                             make_text_view(11),
                                             make_text_view(12),
                                             make_text_view(13)};

    boost::text::text make_text(int i) { return boost::text::text(strings[i]); }

    boost::text::text texts[14] = {make_text(0),
                                   make_text(1),
                                   make_text(2),
                                   make_text(3),
                                   make_text(4),
                                   make_text(5),
                                   make_text(6),
                                   make_text(7),
                                   make_text(8),
                                   make_text(9),
                                   make_text(10),
                                   make_text(11),
                                   make_text(12),
                                   make_text(13)};

    boost::text::unencoded_rope make_rope(int i)
    {
        boost::text::unencoded_rope retval;
        auto size = strings[i].size();
        auto const chunk_size = 512;
        while (size) {
            auto const this_chunk_size = chunk_size <= size ? chunk_size : size;
            retval.insert(
                retval.size(),
                boost::text::text(boost::text::repeat(".", this_chunk_size)));
            size -= this_chunk_size;
        }
        return retval;
    }

    boost::text::unencoded_rope ropes[14] = {make_rope(0),
                                             make_rope(1),
                                             make_rope(2),
                                             make_rope(3),
                                             make_rope(4),
                                             make_rope(5),
                                             make_rope(6),
                                             make_rope(7),
                                             make_rope(8),
                                             make_rope(9),
                                             make_rope(10),
                                             make_rope(11),
                                             make_rope(12),
                                             make_rope(13)};

    boost::text::unencoded_rope_view make_rope_view(int i)
    {
        return boost::text::unencoded_rope_view(ropes[i]);
    }

    boost::text::unencoded_rope_view rope_views[14] = {make_rope_view(0),
                                                       make_rope_view(1),
                                                       make_rope_view(2),
                                                       make_rope_view(3),
                                                       make_rope_view(4),
                                                       make_rope_view(5),
                                                       make_rope_view(6),
                                                       make_rope_view(7),
                                                       make_rope_view(8),
                                                       make_rope_view(9),
                                                       make_rope_view(10),
                                                       make_rope_view(11),
                                                       make_rope_view(12),
                                                       make_rope_view(13)};
}

#define BENCHMARK_ARGS_NONEMPTY()                                              \
    ->Arg(1)                                                                   \
        ->Arg(2)                                                               \
        ->Arg(3)                                                               \
        ->Arg(4)                                                               \
        ->Arg(5)                                                               \
        ->Arg(6)                                                               \
        ->Arg(7)                                                               \
        ->Arg(8)                                                               \
        ->Arg(9)                                                               \
        ->Arg(10)                                                              \
        ->Arg(11)                                                              \
        ->Arg(12)                                                              \
        ->Arg(13)
