// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/unencoded_rope.hpp>

#include <string>

#include "strings.hpp"


namespace {

    boost::text::string_view make_string_view(int i)
    {
        return boost::text::string_view(std_strings[i]);
    }

    boost::text::string_view string_views[14] = {
        make_string_view(0),
        make_string_view(1),
        make_string_view(2),
        make_string_view(3),
        make_string_view(4),
        make_string_view(5),
        make_string_view(6),
        make_string_view(7),
        make_string_view(8),
        make_string_view(9),
        make_string_view(10),
        make_string_view(11),
        make_string_view(12),
        make_string_view(13)};

    std::string make_string(int i) { return std::string(std_strings[i]); }

    std::string strings[14] = {
        make_string(0),
        make_string(1),
        make_string(2),
        make_string(3),
        make_string(4),
        make_string(5),
        make_string(6),
        make_string(7),
        make_string(8),
        make_string(9),
        make_string(10),
        make_string(11),
        make_string(12),
        make_string(13)};

    boost::text::unencoded_rope make_unencoded_rope(int i)
    {
        boost::text::unencoded_rope retval;
        auto size = strings[i].size();
        auto const chunk_size = 512;
        while (size) {
            auto const this_chunk_size = chunk_size <= size ? chunk_size : size;
            retval.insert(
                retval.end(),
                std::string(
                    ".........................................................."
                    ".........................................................."
                    ".........................................................."
                    ".........................................................."
                    ".........................................................."
                    ".........................................................."
                    ".........................................................."
                    ".........................................................."
                    "................................................"));
            size -= this_chunk_size;
        }
        return retval;
    }

    boost::text::unencoded_rope unencoded_ropes[14] = {
        make_unencoded_rope(0),
        make_unencoded_rope(1),
        make_unencoded_rope(2),
        make_unencoded_rope(3),
        make_unencoded_rope(4),
        make_unencoded_rope(5),
        make_unencoded_rope(6),
        make_unencoded_rope(7),
        make_unencoded_rope(8),
        make_unencoded_rope(9),
        make_unencoded_rope(10),
        make_unencoded_rope(11),
        make_unencoded_rope(12),
        make_unencoded_rope(13)};

    boost::text::unencoded_rope_view make_unencoded_rope_view(int i)
    {
        return boost::text::unencoded_rope_view(unencoded_ropes[i]);
    }

    boost::text::unencoded_rope_view unencoded_rope_views[14] = {
        make_unencoded_rope_view(0),
        make_unencoded_rope_view(1),
        make_unencoded_rope_view(2),
        make_unencoded_rope_view(3),
        make_unencoded_rope_view(4),
        make_unencoded_rope_view(5),
        make_unencoded_rope_view(6),
        make_unencoded_rope_view(7),
        make_unencoded_rope_view(8),
        make_unencoded_rope_view(9),
        make_unencoded_rope_view(10),
        make_unencoded_rope_view(11),
        make_unencoded_rope_view(12),
        make_unencoded_rope_view(13)};
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
