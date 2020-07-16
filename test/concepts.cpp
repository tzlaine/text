// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <gtest/gtest.h>

#if defined(__cpp_lib_concepts)

#include <boost/text/concepts.hpp>
#include <boost/text/grapheme_view.hpp>

#include <deque>
#include <list>
#include <string>
#include <vector>

using namespace boost::text::v2;

// uN_cone_unit

static_assert(u8_code_unit<char>);
static_assert(u8_code_unit<signed char>);
static_assert(u8_code_unit<unsigned char>);
static_assert(u8_code_unit<char8_t>);
static_assert(u8_code_unit<int8_t>);

static_assert(!u8_code_unit<wchar_t>);
static_assert(!u8_code_unit<char16_t>);
static_assert(!u8_code_unit<char32_t>);
static_assert(!u8_code_unit<std::byte>);

static_assert(u16_code_unit<char16_t>);
static_assert(u16_code_unit<int16_t>);

#if !defined(_MSC_VER)
static_assert(!u16_code_unit<wchar_t>);
#endif
static_assert(!u16_code_unit<char8_t>);
static_assert(!u16_code_unit<char32_t>);

static_assert(u32_code_unit<char32_t>);
static_assert(u32_code_unit<int32_t>);
#if defined(_MSC_VER)
static_assert(u32_code_unit<wchar_t>);
#endif

static_assert(!u32_code_unit<char8_t>);
static_assert(!u32_code_unit<char16_t>);


// uN_iter

static_assert(u8_iter<char *>);
static_assert(u8_iter<char const *>);
static_assert(u8_iter<char8_t *>);
static_assert(u8_iter<char8_t const *>);

static_assert(u8_iter<std::string::iterator>);
static_assert(u8_iter<std::string::const_iterator>);
static_assert(u8_iter<std::vector<char>::iterator>);

static_assert(!u8_iter<wchar_t *>);
static_assert(!u8_iter<int32_t *>);
static_assert(!u8_iter<std::vector<uint16_t>::iterator>);

static_assert(u16_iter<uint16_t *>);
static_assert(u16_iter<uint16_t const *>);
static_assert(u16_iter<char16_t *>);
static_assert(u16_iter<char16_t const *>);

static_assert(u16_iter<std::u16string::iterator>);
static_assert(u16_iter<std::u16string::const_iterator>);
static_assert(u16_iter<std::vector<char16_t>::iterator>);

static_assert(!u16_iter<char *>);
static_assert(!u16_iter<int32_t *>);
static_assert(!u16_iter<std::vector<uint32_t>::iterator>);

static_assert(u32_iter<uint32_t *>);
static_assert(u32_iter<uint32_t const *>);
static_assert(u32_iter<char32_t *>);
static_assert(u32_iter<char32_t const *>);

static_assert(u32_iter<std::u32string::iterator>);
static_assert(u32_iter<std::u32string::const_iterator>);
static_assert(u32_iter<std::vector<char32_t>::iterator>);

static_assert(!u32_iter<char *>);
static_assert(!u32_iter<int16_t *>);
static_assert(!u32_iter<std::vector<uint16_t>::iterator>);


// uN_ptr

static_assert(u8_ptr<char *>);
static_assert(
    !u8_ptr<std::string::iterator> || std::is_pointer_v<std::string::iterator>);

static_assert(u16_ptr<char16_t *>);
static_assert(
    !u16_ptr<std::u16string::iterator> ||
    std::is_pointer_v<std::u16string::iterator>);

static_assert(u32_ptr<char32_t *>);
static_assert(
    !u32_ptr<std::u32string::iterator> ||
    std::is_pointer_v<std::u32string::iterator>);


// uN_range

static_assert(u8_range<std::string>);
static_assert(u8_range<std::u8string>);
static_assert(u8_range<std::vector<char>>);
static_assert(u8_range<std::list<char>>);
static_assert(u8_range<std::deque<char>>);
static_assert(u16_range<std::u16string>);
#if defined(_MSC_VER)
static_assert(u16_range<std::wstring>);
#else
static_assert(u32_range<std::wstring>);
#endif
static_assert(u32_range<std::u32string>);


// contig_uN_range

static_assert(contig_u8_range<std::string>);
static_assert(contig_u8_range<std::u8string>);
static_assert(contig_u8_range<std::vector<char>>);
static_assert(!contig_u8_range<std::list<char>>);
static_assert(!contig_u8_range<std::deque<char>>);
static_assert(contig_u16_range<std::u16string>);
#if defined(_MSC_VER)
static_assert(contig_u16_range<std::wstring>);
#else
static_assert(contig_u32_range<std::wstring>);
#endif
static_assert(contig_u32_range<std::u32string>);


// grapheme_iter

uint32_t cps[2] = {1, 0};
auto graphemes_0 = boost::text::as_graphemes(cps);
auto graphemes_1 = boost::text::as_graphemes((uint32_t *)cps);

std::string str;
auto graphemes_2 = boost::text::as_graphemes(str);

static_assert(grapheme_iter<decltype(graphemes_0.begin())>);
static_assert(grapheme_iter<decltype(graphemes_0.end())>);
static_assert(grapheme_iter<decltype(graphemes_1.begin())>);
static_assert(!grapheme_iter<decltype(graphemes_1.end())>);
static_assert(grapheme_iter<decltype(graphemes_2.begin())>);
static_assert(grapheme_iter<decltype(graphemes_2.end())>);


// grapheme_range

static_assert(grapheme_range<decltype(graphemes_0)>);
static_assert(grapheme_range<decltype(graphemes_1)>);
static_assert(grapheme_range<decltype(graphemes_2)>);


// grapheme_char_iter

static_assert(!grapheme_char_iter<decltype(graphemes_0.begin())>);
static_assert(!grapheme_char_iter<decltype(graphemes_0.end())>);
static_assert(!grapheme_char_iter<decltype(graphemes_1.begin())>);
static_assert(!grapheme_char_iter<decltype(graphemes_1.end())>);
static_assert(grapheme_char_iter<decltype(graphemes_2.begin())>);
static_assert(grapheme_char_iter<decltype(graphemes_2.end())>);


// grapheme_char_range

static_assert(!grapheme_char_range<decltype(graphemes_0)>);
static_assert(!grapheme_char_range<decltype(graphemes_1)>);
static_assert(grapheme_char_range<decltype(graphemes_2)>);

#endif

TEST(concepts, test) {}
