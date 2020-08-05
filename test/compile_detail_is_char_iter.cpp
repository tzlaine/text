// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/algorithm.hpp>
#include <boost/text/string_view.hpp>
#include <boost/text/unencoded_rope.hpp>
#include <boost/text/rope_view.hpp>
#include <boost/text/detail/algorithm.hpp>

#include <array>
#include <list>
#include <string>
#include <vector>

using namespace boost;

static_assert(text::detail::is_char_iter<char *>{}, "");
static_assert(text::detail::is_char_iter<char const *>{}, "");

static_assert(text::detail::is_char_iter<text::basic_string_view<char>::iterator>{}, "");
static_assert(
    text::detail::is_char_iter<text::basic_string_view<char>::const_iterator>{}, "");
static_assert(
    text::detail::is_char_iter<text::basic_string_view<char>::reverse_iterator>{}, "");
static_assert(
    text::detail::is_char_iter<text::basic_string_view<char>::const_reverse_iterator>{}, "");

static_assert(text::detail::is_char_iter<std::string::iterator>{}, "");
static_assert(text::detail::is_char_iter<std::string::const_iterator>{}, "");
static_assert(text::detail::is_char_iter<std::string::reverse_iterator>{}, "");
static_assert(
    text::detail::is_char_iter<std::string::const_reverse_iterator>{}, "");

static_assert(text::detail::is_char_iter<text::unencoded_rope_view::iterator>{}, "");
static_assert(text::detail::is_char_iter<text::unencoded_rope_view::const_iterator>{}, "");
static_assert(text::detail::is_char_iter<text::unencoded_rope_view::reverse_iterator>{}, "");
static_assert(
    text::detail::is_char_iter<text::unencoded_rope_view::const_reverse_iterator>{}, "");

static_assert(std::is_same<int *, text::detail::char_iter_ret_t<int *, text::unencoded_rope_view::iterator>>::value, "");
static_assert(std::is_same<int *, text::detail::char_iter_ret_t<int *, text::unencoded_rope_view::const_iterator>>::value, "");
static_assert(std::is_same<int *, text::detail::char_iter_ret_t<int *, text::unencoded_rope_view::reverse_iterator>>::value, "");
static_assert(std::is_same<int *, text::detail::char_iter_ret_t<int *, text::unencoded_rope_view::const_reverse_iterator>>::value, "");

static_assert(text::detail::is_char_iter<text::unencoded_rope::iterator>{}, "");
static_assert(text::detail::is_char_iter<text::unencoded_rope::const_iterator>{}, "");
static_assert(text::detail::is_char_iter<text::unencoded_rope::reverse_iterator>{}, "");
static_assert(
    text::detail::is_char_iter<text::unencoded_rope::const_reverse_iterator>{}, "");

static_assert(std::is_same<int *, text::detail::char_iter_ret_t<int *, text::unencoded_rope::iterator>>::value, "");
static_assert(std::is_same<int *, text::detail::char_iter_ret_t<int *, text::unencoded_rope::const_iterator>>::value, "");
static_assert(std::is_same<int *, text::detail::char_iter_ret_t<int *, text::unencoded_rope::reverse_iterator>>::value, "");
static_assert(std::is_same<int *, text::detail::char_iter_ret_t<int *, text::unencoded_rope::const_reverse_iterator>>::value, "");

static_assert(text::detail::is_char_iter<std::string::iterator>{}, "");
static_assert(text::detail::is_char_iter<std::string::const_iterator>{}, "");
static_assert(text::detail::is_char_iter<std::string::reverse_iterator>{}, "");
static_assert(
    text::detail::is_char_iter<std::string::const_reverse_iterator>{}, "");

static_assert(text::detail::is_char_iter<std::vector<char>::iterator>{}, "");
static_assert(
    text::detail::is_char_iter<std::vector<char>::const_iterator>{}, "");
static_assert(
    text::detail::is_char_iter<std::vector<char>::reverse_iterator>{}, "");
static_assert(
    text::detail::is_char_iter<std::vector<char>::const_reverse_iterator>{},
    "");

static_assert(text::detail::is_char_iter<std::array<char, 5>::iterator>{}, "");
static_assert(
    text::detail::is_char_iter<std::array<char, 5>::const_iterator>{}, "");
static_assert(
    text::detail::is_char_iter<std::array<char, 5>::reverse_iterator>{}, "");
static_assert(
    text::detail::is_char_iter<std::array<char, 5>::const_reverse_iterator>{},
    "");

static_assert(text::detail::is_char_iter<std::list<char>::iterator>{}, "");
static_assert(
    text::detail::is_char_iter<std::list<char>::const_iterator>{}, "");
static_assert(
    text::detail::is_char_iter<std::list<char>::reverse_iterator>{}, "");
static_assert(
    text::detail::is_char_iter<std::list<char>::const_reverse_iterator>{}, "");

// Does not work because int narrows when converted to char.
static_assert(!text::detail::is_char_iter<std::vector<int>::iterator>{}, "");

static_assert(!text::detail::is_char_iter<char>{}, "");
static_assert(!text::detail::is_char_iter<int>{}, "");
static_assert(!text::detail::is_char_iter<wchar_t *>{}, "");
static_assert(!text::detail::is_char_iter<wchar_t[5]>{}, "");
static_assert(!text::detail::is_char_iter<int[5]>{}, "");
