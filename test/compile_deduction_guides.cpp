// Copyright (C) 2022 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/grapheme.hpp>
#include <boost/text/grapheme_view.hpp>
#include <boost/text/segmented_vector.hpp>
#include <boost/text/subrange.hpp>


#if defined(__cpp_deduction_guides)

using namespace boost::text;

void grapheme_guides()
{
    {
        char32_t code_points[2] = {'a', '\0'};
        grapheme_ref gr{std::begin(code_points), std::end(code_points)};
        static_assert(std::is_same_v<decltype(gr), grapheme_ref<char32_t *>>);
    }

    {
        char32_t const code_points[2] = {'a', '\0'};
        utf32_view v = code_points | as_utf32;
        grapheme_ref gr{v};
        static_assert(
            std::is_same_v<
                decltype(gr),
                grapheme_ref<
                    utf_iterator<format::utf32, format::utf32, char32_t const *>>>);
    }

#if !defined(_MSC_VER)
    {
        grapheme_ref gr{grapheme{}};
        static_assert(std::is_same_v<
                      decltype(gr),
                      grapheme_ref<grapheme::const_iterator>>);
    }
#endif
}

#if 0 // TODO
void segmented_vector_guides()
{
    {
        segmented_vector sv{13, 42};
        static_assert(std::is_same_v<decltype(sv), segmented_vector<int>>);
    }

    {
        int const ints[] = {4, 5, 6};
        segmented_vector sv(std::begin(ints), std::end(ints));
        static_assert(std::is_same_v<decltype(sv), segmented_vector<int>>);
    }

    {
        int const utf32[] = {'s', 't', 'r', '\0'};
        segmented_vector sv{std::begin(utf32), null_sentinel};
        static_assert(std::is_same_v<decltype(sv), segmented_vector<int>>);
    }

    {
        segmented_vector sv = {3.2, 3.1, 3.0};
        static_assert(std::is_same_v<decltype(sv), segmented_vector<double>>);
    }
}
#endif

void subrange_guides()
{
    {
        char32_t const utf32[] = {'s', 't', 'r', '\0'};
        subrange s{std::begin(utf32), std::begin(utf32)};
        static_assert(std::is_same_v<decltype(s), subrange<char32_t const *>>);
    }
    {
        char32_t utf32[] = {'s', 't', 'r', '\0'};
        subrange s{std::begin(utf32), null_sentinel};
        static_assert(
            std::is_same_v<decltype(s), subrange<char32_t *, null_sentinel_t>>);
    }
    {
        char32_t const utf32[] = {'s', 't', 'r', '\0'};
        subrange s{utf32};
        static_assert(std::is_same_v<decltype(s), subrange<char32_t const *>>);
    }
    {
        std::vector<char32_t> const utf32 = {'s', 't', 'r', '\0'};
        subrange s{utf32};
        static_assert(std::is_same_v<
                      decltype(s),
                      subrange<std::vector<char32_t>::const_iterator>>);
    }
}

#endif
