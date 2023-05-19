// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/transcode_view.hpp>

#include <boost/algorithm/cxx14/equal.hpp>

#include <vector>

#include <gtest/gtest.h>

#include "ill_formed.hpp"


using namespace boost::text;

TEST(transcode_view, adaptor_semantics)
{
    static_assert(std::is_same_v<
                  decltype(u8"text" | boost::text::as_utf16),
                  boost::text::utf16_view<std::ranges::ref_view<const char8_t [5]>>>);

    std::u8string str = u8"text";

    static_assert(std::is_same_v<
                  decltype(std::views::all(str) | boost::text::as_utf16),
                  boost::text::utf16_view<std::ranges::ref_view<std::u8string>>>);

    static_assert(std::is_same_v<
                  decltype(str.c_str() | boost::text::as_utf16),
                  boost::text::utf16_view<std::ranges::subrange<const char8_t *, boost::text::null_sentinel_t>>>);

    static_assert(std::is_same_v<
                  decltype(std::ranges::empty_view<int>{} | boost::text::as_utf16),
                  std::ranges::empty_view<int>>);

    std::u16string str2 = u"text";

    static_assert(std::is_same_v<
                  decltype(std::views::all(str2) | boost::text::as_utf16),
                  std::ranges::ref_view<std::u16string>>);

    static_assert(std::is_same_v<
                  decltype(str2.c_str() | boost::text::as_utf16),
                  std::ranges::subrange<const char16_t *, boost::text::null_sentinel_t>>);
}

TEST(transcode_view, detail_make_utf8)
{
    char const * str8 = "foo";
    uint16_t const str16[4] = {'f', 'o', 'o', 0};
    uint32_t const str32[4] = {'f', 'o', 'o', 0};

    // 8 -> 32 -> 8
    {
        utf_8_to_32_iterator<char const *> it1(str8, str8, str8 + 3);
        utf_8_to_32_iterator<char const *> it2(str8, str8 + 3, str8 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf8, "");
        auto r = detail::make_utf8_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), char const *>::value, "");
        EXPECT_EQ(unpacked.first, str8);
        EXPECT_EQ(unpacked.last, str8 + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_8_to_32_iterator<char const *, null_sentinel_t> it(
            str8, str8, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf8, "");
        auto r = detail::make_utf8_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(unpacked.first, str8);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // 8 -> 16 -> 8
    {
        utf_8_to_16_iterator<char const *> it1(str8, str8, str8 + 3);
        utf_8_to_16_iterator<char const *> it2(str8, str8 + 3, str8 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf8, "");
        auto r = detail::make_utf8_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), char const *>::value, "");
        EXPECT_EQ(unpacked.first, str8);
        EXPECT_EQ(unpacked.last, str8 + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_8_to_16_iterator<char const *, null_sentinel_t> it(
            str8, str8, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf8, "");
        auto r = detail::make_utf8_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(unpacked.first, str8);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // 16 -> 32 -> 8
    {
        utf_16_to_32_iterator<uint16_t const *> it1(str16, str16, str16 + 3);
        utf_16_to_32_iterator<uint16_t const *> it2(
            str16, str16 + 3, str16 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf16, "");
        auto r = detail::make_utf8_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), uint16_t const *>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_16_to_8_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(
            unpacked.last,
            utf_16_to_8_iterator<uint16_t const *>(
                str16, str16 + 3, str16 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_16_to_32_iterator<uint16_t const *, null_sentinel_t> it(
            str16, str16, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf16, "");
        auto r = detail::make_utf8_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_16_to_8_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // 16 -> 8 -> 8
    {
        utf_16_to_8_iterator<uint16_t const *> it1(str16, str16, str16 + 3);
        utf_16_to_8_iterator<uint16_t const *> it2(str16, str16 + 3, str16 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf16, "");
        auto r = detail::make_utf8_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), uint16_t const *>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_16_to_8_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(
            unpacked.last,
            utf_16_to_8_iterator<uint16_t const *>(
                str16, str16 + 3, str16 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_16_to_8_iterator<uint16_t const *, null_sentinel_t> it(
            str16, str16, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf16, "");
        auto r = detail::make_utf8_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_16_to_8_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // 32 -> 16 -> 8
    {
        utf_32_to_16_iterator<uint32_t const *> it1(str32, str32, str32 + 3);
        utf_32_to_16_iterator<uint32_t const *> it2(
            str32, str32 + 3, str32 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf32, "");
        auto r = detail::make_utf8_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), uint32_t const *>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_32_to_8_iterator<uint32_t const *>(str32, str32, str32 + 3));
        EXPECT_EQ(
            unpacked.last,
            utf_32_to_8_iterator<uint32_t const *>(
                str32, str32 + 3, str32 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_32_to_16_iterator<uint32_t const *, null_sentinel_t> it(
            str32, str32, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf32, "");
        auto r = detail::make_utf8_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_32_to_8_iterator<uint32_t const *>(str32, str32, str32 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // 32 -> 8 -> 8
    {
        utf_32_to_8_iterator<uint32_t const *> it1(str32, str32, str32 + 3);
        utf_32_to_8_iterator<uint32_t const *> it2(str32, str32 + 3, str32 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf32, "");
        auto r = detail::make_utf8_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), uint32_t const *>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_32_to_8_iterator<uint32_t const *>(str32, str32, str32 + 3));
        EXPECT_EQ(
            unpacked.last,
            utf_32_to_8_iterator<uint32_t const *>(
                str32, str32 + 3, str32 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_32_to_8_iterator<uint32_t const *, null_sentinel_t> it(
            str32, str32, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf32, "");
        auto r = detail::make_utf8_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_32_to_8_iterator<uint32_t const *>(str32, str32, str32 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }
}

TEST(transcode_view, detail_make_utf16)
{
    char const * str8 = "foo";
    uint16_t const str16[4] = {'f', 'o', 'o', 0};
    uint32_t const str32[4] = {'f', 'o', 'o', 0};

    // 8 -> 32 -> 16
    {
        utf_8_to_32_iterator<char const *> it1(str8, str8, str8 + 3);
        utf_8_to_32_iterator<char const *> it2(str8, str8 + 3, str8 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf8, "");
        auto r = detail::make_utf16_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), char const *>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_8_to_16_iterator<char const *>(str8, str8, str8 + 3));
        EXPECT_EQ(
            unpacked.last,
            utf_8_to_16_iterator<char const *>(str8, str8 + 3, str8 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_8_to_32_iterator<char const *, null_sentinel_t> it(
            str8, str8, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf8, "");
        auto r = detail::make_utf16_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_8_to_16_iterator<char const *>(str8, str8, str8 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // 8 -> 16 -> 16
    {
        utf_8_to_16_iterator<char const *> it1(str8, str8, str8 + 3);
        utf_8_to_16_iterator<char const *> it2(str8, str8 + 3, str8 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf8, "");
        auto r = detail::make_utf16_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), char const *>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_8_to_16_iterator<char const *>(str8, str8, str8 + 3));
        EXPECT_EQ(
            unpacked.last,
            utf_8_to_16_iterator<char const *>(str8, str8 + 3, str8 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_8_to_16_iterator<char const *, null_sentinel_t> it(
            str8, str8, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf8, "");
        auto r = detail::make_utf16_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_8_to_16_iterator<char const *>(str8, str8, str8 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // 16 -> 32 -> 16
    {
        utf_16_to_32_iterator<uint16_t const *> it1(str16, str16, str16 + 3);
        utf_16_to_32_iterator<uint16_t const *> it2(
            str16, str16 + 3, str16 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf16, "");
        auto r = detail::make_utf16_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), uint16_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str16);
        EXPECT_EQ(unpacked.last, str16 + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_16_to_32_iterator<uint16_t const *, null_sentinel_t> it(
            str16, str16, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf16, "");
        auto r = detail::make_utf16_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_16_to_8_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(unpacked.first, str16);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // 16 -> 8 -> 16
    {
        utf_16_to_8_iterator<uint16_t const *> it1(str16, str16, str16 + 3);
        utf_16_to_8_iterator<uint16_t const *> it2(str16, str16 + 3, str16 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf16, "");
        auto r = detail::make_utf16_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), uint16_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str16);
        EXPECT_EQ(unpacked.last, str16 + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_16_to_8_iterator<uint16_t const *, null_sentinel_t> it(
            str16, str16, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf16, "");
        auto r = detail::make_utf16_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(unpacked.first, str16);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // 32 -> 16 -> 16
    {
        utf_32_to_16_iterator<uint32_t const *> it1(str32, str32, str32 + 3);
        utf_32_to_16_iterator<uint32_t const *> it2(
            str32, str32 + 3, str32 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf32, "");
        auto r = detail::make_utf16_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), uint32_t const *>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_32_to_16_iterator<uint32_t const *>(str32, str32, str32 + 3));
        EXPECT_EQ(
            unpacked.last,
            utf_32_to_16_iterator<uint32_t const *>(
                str32, str32 + 3, str32 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_32_to_16_iterator<uint32_t const *, null_sentinel_t> it(
            str32, str32, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf32, "");
        auto r = detail::make_utf16_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_32_to_16_iterator<uint32_t const *>(str32, str32, str32 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // 32 -> 8 -> 16
    {
        utf_32_to_8_iterator<uint32_t const *> it1(str32, str32, str32 + 3);
        utf_32_to_8_iterator<uint32_t const *> it2(str32, str32 + 3, str32 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf32, "");
        auto r = detail::make_utf16_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), uint32_t const *>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_32_to_16_iterator<uint32_t const *>(str32, str32, str32 + 3));
        EXPECT_EQ(
            unpacked.last,
            utf_32_to_16_iterator<uint32_t const *>(
                str32, str32 + 3, str32 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_32_to_8_iterator<uint32_t const *, null_sentinel_t> it(
            str32, str32, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf32, "");
        auto r = detail::make_utf16_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_32_to_16_iterator<uint32_t const *>(str32, str32, str32 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }
}

TEST(transcode_view, detail_make_utf32)
{
    char const * str8 = "foo";
    uint16_t const str16[4] = {'f', 'o', 'o', 0};
    uint32_t const str32[4] = {'f', 'o', 'o', 0};

    // 8 -> 32 -> 32
    {
        utf_8_to_32_iterator<char const *> it1(str8, str8, str8 + 3);
        utf_8_to_32_iterator<char const *> it2(str8, str8 + 3, str8 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf8, "");
        auto r = detail::make_utf32_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), char const *>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_8_to_32_iterator<char const *>(str8, str8, str8 + 3));
        EXPECT_EQ(
            unpacked.last,
            utf_8_to_32_iterator<char const *>(str8, str8 + 3, str8 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_8_to_32_iterator<char const *, null_sentinel_t> it(
            str8, str8, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf8, "");
        auto r = detail::make_utf32_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_8_to_32_iterator<char const *>(str8, str8, str8 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // 8 -> 16 -> 32
    {
        utf_8_to_16_iterator<char const *> it1(str8, str8, str8 + 3);
        utf_8_to_16_iterator<char const *> it2(str8, str8 + 3, str8 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf8, "");
        auto r = detail::make_utf32_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), char const *>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_8_to_32_iterator<char const *>(str8, str8, str8 + 3));
        EXPECT_EQ(
            unpacked.last,
            utf_8_to_32_iterator<char const *>(str8, str8 + 3, str8 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_8_to_16_iterator<char const *, null_sentinel_t> it(
            str8, str8, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf8, "");
        auto r = detail::make_utf32_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_8_to_32_iterator<char const *>(str8, str8, str8 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // 16 -> 32 -> 32
    {
        utf_16_to_32_iterator<uint16_t const *> it1(str16, str16, str16 + 3);
        utf_16_to_32_iterator<uint16_t const *> it2(
            str16, str16 + 3, str16 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf16, "");
        auto r = detail::make_utf32_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), uint16_t const *>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_16_to_32_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(
            unpacked.last,
            utf_16_to_32_iterator<uint16_t const *>(
                str16, str16 + 3, str16 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_16_to_32_iterator<uint16_t const *, null_sentinel_t> it(
            str16, str16, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf16, "");
        auto r = detail::make_utf32_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_16_to_8_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(
            unpacked.first,
            utf_16_to_8_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // 16 -> 8 -> 32
    {
        utf_16_to_8_iterator<uint16_t const *> it1(str16, str16, str16 + 3);
        utf_16_to_8_iterator<uint16_t const *> it2(str16, str16 + 3, str16 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf16, "");
        auto r = detail::make_utf32_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), uint16_t const *>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_16_to_32_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(
            unpacked.last,
            utf_16_to_32_iterator<uint16_t const *>(
                str16, str16 + 3, str16 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_16_to_8_iterator<uint16_t const *, null_sentinel_t> it(
            str16, str16, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf16, "");
        auto r = detail::make_utf32_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(
            unpacked.first,
            utf_16_to_32_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // 32 -> 16 -> 32
    {
        utf_32_to_16_iterator<uint32_t const *> it1(str32, str32, str32 + 3);
        utf_32_to_16_iterator<uint32_t const *> it2(
            str32, str32 + 3, str32 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf32, "");
        auto r = detail::make_utf32_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), uint32_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str32);
        EXPECT_EQ(unpacked.last, str32 + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_32_to_16_iterator<uint32_t const *, null_sentinel_t> it(
            str32, str32, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf32, "");
        auto r = detail::make_utf32_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(unpacked.first, str32);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // 32 -> 8 -> 32
    {
        utf_32_to_8_iterator<uint32_t const *> it1(str32, str32, str32 + 3);
        utf_32_to_8_iterator<uint32_t const *> it2(str32, str32 + 3, str32 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf32, "");
        auto r = detail::make_utf32_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), uint32_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str32);
        EXPECT_EQ(unpacked.last, str32 + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        utf_32_to_8_iterator<uint32_t const *, null_sentinel_t> it(
            str32, str32, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf32, "");
        auto r = detail::make_utf32_range_(
            detail::tag_t<unpacked.format_tag>{},
            unpacked.first,
            unpacked.last);
        static_assert(r.format_tag == format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(unpacked.first, str32);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }
}

// Unicode 9, 3.9/D90-D92
uint32_t const utf32_[4] = {0x004d, 0x0430, 0x4e8c, 0x10302};
uint16_t const utf16_[5] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02};
char8_t const utf8_[10] = {
    0x4d, 0xd0, 0xb0, 0xe4, 0xba, 0x8c, 0xf0, 0x90, 0x8c, 0x82};

uint32_t const utf32_null[5] = {0x004d, 0x0430, 0x4e8c, 0x10302, 0};
uint16_t const utf16_null[6] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02, 0};
char8_t const utf8_null[11] = {
    0x4d, 0xd0, 0xb0, 0xe4, 0xba, 0x8c, 0xf0, 0x90, 0x8c, 0x82, 0};

TEST(transcode_view, as_utfN)
{
    // array
    {
        auto r = std::views::all(utf32_) | as_utf8;
        EXPECT_TRUE(boost::algorithm::equal(
            r.begin(), r.end(), std::begin(utf8_), std::end(utf8_)));
    }
    {
        auto r = std::views::all(utf16_) | as_utf8;
        EXPECT_TRUE(boost::algorithm::equal(
            r.begin(), r.end(), std::begin(utf8_), std::end(utf8_)));
    }
    {
        auto r = std::views::all(utf8_) | as_utf8;
        static_assert(
            std::is_same<decltype(r.begin()), char8_t const *>::value, "");
        static_assert(
            std::is_same<decltype(r.end()), char8_t const *>::value, "");
        EXPECT_TRUE(boost::algorithm::equal(
            r.begin(), r.end(), std::begin(utf8_), std::end(utf8_)));
    }
    {
        auto r = std::views::all(utf32_) | as_utf8;
        EXPECT_TRUE(boost::algorithm::equal(
            r.begin(), r.end(), std::begin(utf8_), std::end(utf8_)));
    }
    {
        auto r = std::views::all(utf16_) | as_utf8;
        EXPECT_TRUE(boost::algorithm::equal(
            r.begin(), r.end(), std::begin(utf8_), std::end(utf8_)));
    }
    {
        auto r = std::views::all(utf8_) | as_utf8;
        static_assert(
            std::is_same<decltype(r.begin()), char8_t const *>::value, "");
        static_assert(
            std::is_same<decltype(r.end()), char8_t const *>::value, "");
        EXPECT_TRUE(boost::algorithm::equal(
            r.begin(), r.end(), std::begin(utf8_), std::end(utf8_)));
    }

    // ptr/sentinel
    {
        auto r = (utf32_null + 0) | as_utf8;
        auto truth = utf8_;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        auto r = (utf16_null + 0) | as_utf8;
        auto truth = utf8_;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        auto r = (utf8_null + 0) | as_utf8;
        auto truth = utf8_;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }

    // single pointers
    {
        char8_t const * ptr = utf8_null;
        auto r = as_utf8(ptr);
        static_assert(
            std::is_same<decltype(r.begin()), char8_t const *>::value, "");
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf8_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        uint16_t const * ptr = utf16_null;
        auto r = as_utf8(ptr);
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf8_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        uint32_t const * ptr = utf32_null;
        auto r = as_utf8(ptr);
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf8_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        char8_t const * ptr = utf8_null;
        auto r = as_utf16(ptr);
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf16_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        uint16_t const * ptr = utf16_null;
        auto r = as_utf16(ptr);
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf16_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        uint32_t const * ptr = utf32_null;
        auto r = as_utf16(ptr);
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf16_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        char8_t const * ptr = utf8_null;
        auto r = as_utf32(ptr);
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf32_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        uint16_t const * ptr = utf16_null;
        auto r = as_utf32(ptr);
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf32_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        uint32_t const * ptr = utf32_null;
        auto r = as_utf32(ptr);
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf32_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        char8_t const * ptr = utf8_null;
        auto r = ptr | as_utf8;
        static_assert(
            std::is_same<decltype(r.begin()), char8_t const *>::value, "");
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf8_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        uint16_t const * ptr = utf16_null;
        auto r = ptr | as_utf8;
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf8_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        uint32_t const * ptr = utf32_null;
        auto r = ptr | as_utf8;
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf8_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        char8_t const * ptr = utf8_null;
        auto r = ptr | as_utf16;
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf16_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        uint16_t const * ptr = utf16_null;
        auto r = ptr | as_utf16;
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf16_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        uint32_t const * ptr = utf32_null;
        auto r = ptr | as_utf16;
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf16_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        char8_t const * ptr = utf8_null;
        auto r = ptr | as_utf32;
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf32_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        uint16_t const * ptr = utf16_null;
        auto r = ptr | as_utf32;
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf32_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        uint32_t const * ptr = utf32_null;
        auto r = ptr | as_utf32;
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf32_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }



    // funkyzeit
    {
        auto r = std::views::all(utf8_) | as_utf8 | as_utf16 | as_utf32 |
                 as_utf8 | as_utf16 | as_utf8;
        EXPECT_TRUE(boost::algorithm::equal(
            r.begin(), r.end(), std::begin(utf8_), std::end(utf8_)));
        static_assert(
            std::is_same<decltype(r.begin()), char8_t const *>::value, "");
        static_assert(
            std::is_same<decltype(r.end()), char8_t const *>::value, "");
    }
}

TEST(transcode_view, stream_insertion)
{
    std::string const truth(std::begin(utf8_null), std::end(utf8_null) - 1);
    {
        char8_t const * ptr = utf8_null;
        auto r = ptr | as_utf16;
        std::stringstream ss;
        ss << r;
        EXPECT_EQ(ss.str(), truth);
    }
    {
        char8_t const * ptr = utf8_null;
        auto r = ptr | as_utf32;
        std::stringstream ss;
        ss << r;
        EXPECT_EQ(ss.str(), truth);
    }
}
