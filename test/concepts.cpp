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

static_assert(utf8_code_unit<char>);
static_assert(utf8_code_unit<signed char>);
static_assert(utf8_code_unit<unsigned char>);
static_assert(utf8_code_unit<char8_t>);
static_assert(utf8_code_unit<int8_t>);

static_assert(!utf8_code_unit<wchar_t>);
static_assert(!utf8_code_unit<char16_t>);
static_assert(!utf8_code_unit<char32_t>);
static_assert(!utf8_code_unit<std::byte>);

static_assert(utf16_code_unit<char16_t>);
static_assert(utf16_code_unit<int16_t>);

#if !defined(_MSC_VER)
static_assert(!utf16_code_unit<wchar_t>);
#endif
static_assert(!utf16_code_unit<char8_t>);
static_assert(!utf16_code_unit<char32_t>);

static_assert(utf32_code_unit<char32_t>);
static_assert(utf32_code_unit<int32_t>);
#if defined(_MSC_VER)
static_assert(utf32_code_unit<wchar_t>);
#endif

static_assert(!utf32_code_unit<char8_t>);
static_assert(!utf32_code_unit<char16_t>);


// uN_iter

static_assert(utf8_iter<char *>);
static_assert(utf8_iter<char const *>);
static_assert(utf8_iter<char8_t *>);
static_assert(utf8_iter<char8_t const *>);

static_assert(utf8_iter<std::string::iterator>);
static_assert(utf8_iter<std::string::const_iterator>);
static_assert(utf8_iter<std::vector<char>::iterator>);

static_assert(!utf8_iter<wchar_t *>);
static_assert(!utf8_iter<int32_t *>);
static_assert(!utf8_iter<std::vector<uint16_t>::iterator>);

static_assert(utf16_iter<uint16_t *>);
static_assert(utf16_iter<uint16_t const *>);
static_assert(utf16_iter<char16_t *>);
static_assert(utf16_iter<char16_t const *>);

static_assert(utf16_iter<std::u16string::iterator>);
static_assert(utf16_iter<std::u16string::const_iterator>);
static_assert(utf16_iter<std::vector<char16_t>::iterator>);

static_assert(!utf16_iter<char *>);
static_assert(!utf16_iter<int32_t *>);
static_assert(!utf16_iter<std::vector<uint32_t>::iterator>);

static_assert(utf32_iter<uint32_t *>);
static_assert(utf32_iter<uint32_t const *>);
static_assert(utf32_iter<char32_t *>);
static_assert(utf32_iter<char32_t const *>);

static_assert(utf32_iter<std::u32string::iterator>);
static_assert(utf32_iter<std::u32string::const_iterator>);
static_assert(utf32_iter<std::vector<char32_t>::iterator>);

static_assert(!utf32_iter<char *>);
static_assert(!utf32_iter<int16_t *>);
static_assert(!utf32_iter<std::vector<uint16_t>::iterator>);


// uN_pointer

static_assert(utf8_pointer<char *>);
static_assert(
    !utf8_pointer<std::string::iterator> || std::is_pointer_v<std::string::iterator>);

static_assert(utf16_pointer<char16_t *>);
static_assert(
    !utf16_pointer<std::u16string::iterator> ||
    std::is_pointer_v<std::u16string::iterator>);

static_assert(utf32_pointer<char32_t *>);
static_assert(
    !utf32_pointer<std::u32string::iterator> ||
    std::is_pointer_v<std::u32string::iterator>);


// uN_range

static_assert(utf8_range<std::string>);
static_assert(utf8_range<std::u8string>);
static_assert(utf8_range<std::vector<char>>);
static_assert(utf8_range<std::list<char>>);
static_assert(utf8_range<std::deque<char>>);
static_assert(utf16_range<std::u16string>);
#if defined(_MSC_VER)
static_assert(utf16_range<std::wstring>);
#else
static_assert(utf32_range<std::wstring>);
#endif
static_assert(utf32_range<std::u32string>);


// contiguous_uN_range

static_assert(contiguous_utf8_range<std::string>);
static_assert(contiguous_utf8_range<std::u8string>);
static_assert(contiguous_utf8_range<std::vector<char>>);
static_assert(!contiguous_utf8_range<std::list<char>>);
static_assert(!contiguous_utf8_range<std::deque<char>>);
static_assert(contiguous_utf16_range<std::u16string>);
#if defined(_MSC_VER)
static_assert(contiguous_utf16_range<std::wstring>);
#else
static_assert(contiguous_utf32_range<std::wstring>);
#endif
static_assert(contiguous_utf32_range<std::u32string>);


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


// grapheme_iter

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


// grapheme_iter_code_unit

static_assert(!grapheme_iter_code_unit<
              decltype(graphemes_0.begin()),
              boost::text::format::utf8>);
static_assert(!grapheme_iter_code_unit<
              decltype(graphemes_0.end()),
              boost::text::format::utf8>);
static_assert(!grapheme_iter_code_unit<
              decltype(graphemes_1.begin()),
              boost::text::format::utf8>);
static_assert(!grapheme_iter_code_unit<
              decltype(graphemes_1.end()),
              boost::text::format::utf8>);
static_assert(grapheme_iter_code_unit<
              decltype(graphemes_2.begin()),
              boost::text::format::utf8>);
static_assert(grapheme_iter_code_unit<
              decltype(graphemes_2.end()),
              boost::text::format::utf8>);


// grapheme_range_code_unit

static_assert(!grapheme_range_code_unit<
              decltype(graphemes_0),
              boost::text::format::utf8>);
static_assert(!grapheme_range_code_unit<
              decltype(graphemes_1),
              boost::text::format::utf8>);
static_assert(
    grapheme_range_code_unit<decltype(graphemes_2), boost::text::format::utf8>);


// utf8_string

static_assert(utf8_string<std::vector<char>>);
static_assert(utf8_string<std::string>);
static_assert(utf8_string<std::u8string>);
static_assert(!utf8_string<std::vector<int>>);
static_assert(!utf8_string<std::wstring>);
static_assert(!utf8_string<std::u32string>);


// utf16_string

static_assert(utf16_string<std::vector<char16_t>>);
#if defined(_MSC_VER)
static_assert(utf16_string<std::wstring>);
#endif
static_assert(utf16_string<std::u16string>);
static_assert(!utf16_string<std::vector<int>>);
static_assert(!utf16_string<std::u8string>);
static_assert(!utf16_string<std::u32string>);

#endif

TEST(concepts, test) {}
