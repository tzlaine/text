// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/algorithm.hpp>

#include <boost/text/string_view.hpp>
#include <boost/text/unencoded_rope.hpp>
#include <boost/text/unencoded_rope_view.hpp>
#include <boost/text/text.hpp>
#include <boost/text/text_view.hpp>
#include <boost/text/rope.hpp>
#include <boost/text/rope_view.hpp>

#include <boost/range/iterator_range_core.hpp>

#include <array>
#include <list>
#include <string>
#include <vector>


struct inline_t
{
    using iterator = std::array<char, 4>::const_iterator;

    std::array<char, 4>::const_iterator begin() const { return chars_.begin(); }
    std::array<char, 4>::const_iterator end() const { return chars_.end(); }

    std::array<char, 4> chars_;
};


using namespace boost;

static_assert(text::detail::is_char_range_v<text::basic_string_view<char>>, "");
static_assert(text::detail::is_char_range_v<text::basic_string_view<char> const>, "");
static_assert(text::detail::is_char_range_v<std::string>, "");
static_assert(text::detail::is_char_range_v<std::string const>, "");
static_assert(text::detail::is_char_range_v<text::unencoded_rope>, "");
static_assert(text::detail::is_char_range_v<text::unencoded_rope_view>, "");
static_assert(text::detail::is_char_range_v<std::string>, "");
static_assert(text::detail::is_char_range_v<std::string const>, "");
static_assert(text::detail::is_char_range_v<std::vector<char>>, "");
static_assert(text::detail::is_char_range_v<std::vector<char> const>, "");
static_assert(text::detail::is_char_range_v<std::array<char, 5>>, "");
static_assert(text::detail::is_char_range_v<std::array<char, 5> const>, "");
static_assert(text::detail::is_char_range_v<iterator_range<char *>>, "");
static_assert(text::detail::is_char_range_v<iterator_range<char const *>>, "");
static_assert(
    text::detail::is_char_range_v<iterator_range<std::vector<char>::iterator>>,
    "");
static_assert(
    text::detail::is_char_range_v<
        iterator_range<std::vector<char>::iterator const>>,
    "");

static_assert(text::detail::is_char_range_v<inline_t>, "");
static_assert(text::detail::is_char_range_v<inline_t const>, "");

// These don't work because their value types narrow when converted to char.
static_assert(!text::detail::is_char_range_v<std::vector<wchar_t>>, "");
static_assert(!text::detail::is_char_range_v<std::vector<int>>, "");
static_assert(!text::detail::is_char_range_v<std::array<float, 5>>, "");
static_assert(
    !text::detail::is_char_range_v<iterator_range<wchar_t *>>, "");
static_assert(
    !text::detail::is_char_range_v<
        iterator_range<std::vector<int>::iterator>>,
    "");

static_assert(text::detail::is_char_range_v<std::list<char>>, "");

static_assert(!text::detail::is_char_range_v<char>, "");
static_assert(!text::detail::is_char_range_v<int>, "");

static_assert(!text::detail::is_char_range_v<text::text>, "");
static_assert(!text::detail::is_char_range_v<text::text_view>, "");
static_assert(!text::detail::is_char_range_v<text::rope>, "");
static_assert(!text::detail::is_char_range_v<text::rope_view>, "");


static_assert(!text::detail::is_contig_char_range_v<text::unencoded_rope>, "");
static_assert(!text::detail::is_contig_char_range_v<text::unencoded_rope_view>, "");


static_assert(text::detail::is_grapheme_range<text::text>::value, "");
static_assert(text::detail::is_grapheme_range<text::text_view>::value, "");
static_assert(text::detail::is_grapheme_range<text::rope>::value, "");
static_assert(text::detail::is_grapheme_range<text::rope_view>::value, "");

static_assert(text::detail::is_contig_grapheme_range<text::text>::value, "");
static_assert(text::detail::is_contig_grapheme_range<text::text_view>::value, "");
static_assert(!text::detail::is_contig_grapheme_range<text::rope>::value, "");
static_assert(!text::detail::is_contig_grapheme_range<text::rope_view>::value, "");

static_assert(!text::detail::is_char16_range_v<text::basic_string_view<char>>, "");
static_assert(!text::detail::is_char16_range_v<std::string>, "");
static_assert(text::detail::is_char16_range_v<std::array<uint16_t, 1>>, "");
