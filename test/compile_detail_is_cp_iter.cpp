// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/algorithm.hpp>
#include <boost/text/string_view.hpp>
#include <boost/text/transcode_iterator.hpp>
#include <boost/text/unencoded_rope.hpp>

#include <array>
#include <deque>
#include <list>
#include <string>
#include <vector>

using namespace boost;

// positive tests

static_assert(text::detail::is_cp_iter<uint32_t *>{}, "");
static_assert(text::detail::is_cp_iter<uint32_t const *>{}, "");

static_assert(
    text::detail::is_cp_iter<std::array<uint32_t, 10>::iterator>{}, "");
static_assert(
    text::detail::is_cp_iter<std::array<uint32_t, 10>::const_iterator>{}, "");
static_assert(
    text::detail::is_cp_iter<std::array<uint32_t, 10>::reverse_iterator>{}, "");
static_assert(
    text::detail::is_cp_iter<
        std::array<uint32_t, 10>::const_reverse_iterator>{},
    "");

static_assert(text::detail::is_cp_iter<std::list<uint32_t>::iterator>{}, "");
static_assert(
    text::detail::is_cp_iter<std::list<uint32_t>::const_iterator>{}, "");
static_assert(
    text::detail::is_cp_iter<std::list<uint32_t>::reverse_iterator>{}, "");
static_assert(
    text::detail::is_cp_iter<std::list<uint32_t>::const_reverse_iterator>{},
    "");

static_assert(
    text::detail::is_cp_iter<text::utf_8_to_32_iterator<char const *>>{}, "");



// negative tests

static_assert(!text::detail::is_cp_iter<char *>{}, "");
static_assert(!text::detail::is_cp_iter<char const *>{}, "");

static_assert(!text::detail::is_cp_iter<int *>{}, "");
static_assert(!text::detail::is_cp_iter<int const *>{}, "");

static_assert(!text::detail::is_cp_iter<text::string_view::iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<text::string_view::const_iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<text::string_view::reverse_iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<text::string_view::const_reverse_iterator>{}, "");

static_assert(!text::detail::is_cp_iter<std::string::iterator>{}, "");
static_assert(!text::detail::is_cp_iter<std::string::const_iterator>{}, "");
static_assert(!text::detail::is_cp_iter<std::string::reverse_iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<std::string::const_reverse_iterator>{}, "");

static_assert(
    !text::detail::is_cp_iter<text::unencoded_rope_view::iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<text::unencoded_rope_view::const_iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<text::unencoded_rope_view::reverse_iterator>{},
    "");
static_assert(
    !text::detail::is_cp_iter<
        text::unencoded_rope_view::const_reverse_iterator>{},
    "");

static_assert(!text::detail::is_cp_iter<text::unencoded_rope::iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<text::unencoded_rope::const_iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<text::unencoded_rope::reverse_iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<text::unencoded_rope::const_reverse_iterator>{},
    "");

static_assert(!text::detail::is_cp_iter<std::string::iterator>{}, "");
static_assert(!text::detail::is_cp_iter<std::string::const_iterator>{}, "");
static_assert(!text::detail::is_cp_iter<std::string::reverse_iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<std::string::const_reverse_iterator>{}, "");

static_assert(!text::detail::is_cp_iter<std::vector<char>::iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<std::vector<char>::const_iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<std::vector<char>::reverse_iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<std::vector<char>::const_reverse_iterator>{}, "");

static_assert(!text::detail::is_cp_iter<std::array<char, 5>::iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<std::array<char, 5>::const_iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<std::array<char, 5>::reverse_iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<std::array<char, 5>::const_reverse_iterator>{},
    "");

static_assert(!text::detail::is_cp_iter<std::list<char>::iterator>{}, "");
static_assert(!text::detail::is_cp_iter<std::list<char>::const_iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<std::list<char>::reverse_iterator>{}, "");
static_assert(
    !text::detail::is_cp_iter<std::list<char>::const_reverse_iterator>{}, "");

static_assert(!text::detail::is_cp_iter<char>{}, "");
static_assert(!text::detail::is_cp_iter<int>{}, "");
static_assert(!text::detail::is_cp_iter<wchar_t *>{}, "");
static_assert(!text::detail::is_cp_iter<std::vector<int>::iterator>{}, "");
static_assert(!text::detail::is_cp_iter<wchar_t[5]>{}, "");
static_assert(!text::detail::is_cp_iter<int[5]>{}, "");
