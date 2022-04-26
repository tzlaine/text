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

TEST(transcode_view, detail_unpack_base_case)
{
    {
        std::vector<uint32_t> utf32(3);
        auto unpacked =
            detail::unpack_iterator_and_sentinel(utf32.begin(), utf32.end());
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value,
            "");
        static_assert(
            std::is_same<
                decltype(unpacked.f_),
                std::vector<uint32_t>::iterator>::value,
            "");
        static_assert(
            std::is_same<
                decltype(unpacked.l_),
                std::vector<uint32_t>::iterator>::value,
            "");
        EXPECT_EQ(unpacked.f_, utf32.begin());
        EXPECT_EQ(unpacked.l_, utf32.end());
        EXPECT_EQ(unpacked.repack_(unpacked.f_), utf32.begin());
    }
    {
        std::vector<uint16_t> utf16(3);
        auto unpacked =
            detail::unpack_iterator_and_sentinel(utf16.begin(), utf16.end());
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value,
            "");
        static_assert(
            std::is_same<
                decltype(unpacked.f_),
                std::vector<uint16_t>::iterator>::value,
            "");
        static_assert(
            std::is_same<
                decltype(unpacked.l_),
                std::vector<uint16_t>::iterator>::value,
            "");
        EXPECT_EQ(unpacked.f_, utf16.begin());
        EXPECT_EQ(unpacked.l_, utf16.end());
        EXPECT_EQ(unpacked.repack_(unpacked.f_), utf16.begin());
    }
    {
        std::vector<unsigned char> utf8(3);
        auto unpacked =
            detail::unpack_iterator_and_sentinel(utf8.begin(), utf8.end());
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<
                decltype(unpacked.f_),
                std::vector<unsigned char>::iterator>::value,
            "");
        static_assert(
            std::is_same<
                decltype(unpacked.l_),
                std::vector<unsigned char>::iterator>::value,
            "");
        EXPECT_EQ(unpacked.f_, utf8.begin());
        EXPECT_EQ(unpacked.l_, utf8.end());
        EXPECT_EQ(unpacked.repack_(unpacked.f_), utf8.begin());
    }
    {
        std::vector<char> utf8(3);
        auto unpacked =
            detail::unpack_iterator_and_sentinel(utf8.begin(), utf8.end());
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), std::vector<char>::iterator>::
                value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.l_), std::vector<char>::iterator>::
                value,
            "");
        EXPECT_EQ(unpacked.f_, utf8.begin());
        EXPECT_EQ(unpacked.l_, utf8.end());
        EXPECT_EQ(unpacked.repack_(unpacked.f_), utf8.begin());
    }
    {
        char const * utf8 = "foo";
        auto unpacked =
            detail::unpack_iterator_and_sentinel(utf8, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(unpacked.f_, utf8);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), utf8);
    }
    {
        char const * utf8 = "foo";
        auto unpacked = detail::unpack_iterator_and_sentinel(utf8, utf8 + 3);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), char const *>::value, "");
        EXPECT_EQ(unpacked.f_, utf8);
        EXPECT_EQ(unpacked.l_, utf8 + 3);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), utf8);
    }
}

template<typename Iter>
using bad_unpack_t = decltype(detail::unpack_iterator_and_sentinel(
    std::declval<Iter>(), std::declval<Iter>()));
static_assert(ill_formed<bad_unpack_t, uint64_t *>::value, "");

TEST(transcode_view, detail_unpack_8_N)
{
    char const * str = "foo";

    // N = 32
    {
        utf_8_to_32_iterator<char const *> it1(str, str, str + 3);
        utf_8_to_32_iterator<char const *> it2(str, str + 3, str + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), char const *>::value, "");
        EXPECT_EQ(unpacked.f_, str);
        EXPECT_EQ(unpacked.l_, str + 3);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_8_to_32_iterator<char const *, null_sentinel> it(
            str, str, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(unpacked.f_, str);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // N = 16
    {
        utf_8_to_16_iterator<char const *> it1(str, str, str + 3);
        utf_8_to_16_iterator<char const *> it2(str, str + 3, str + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), char const *>::value, "");
        EXPECT_EQ(unpacked.f_, str);
        EXPECT_EQ(unpacked.l_, str + 3);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_8_to_16_iterator<char const *, null_sentinel> it(
            str, str, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(unpacked.f_, str);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }
}

TEST(transcode_view, detail_unpack_16_N)
{
    uint16_t const str[4] = {'f', 'o', 'o', 0};

    // N = 32
    {
        utf_16_to_32_iterator<uint16_t const *> it1(str, str, str + 3);
        utf_16_to_32_iterator<uint16_t const *> it2(str, str + 3, str + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), uint16_t const *>::value, "");
        EXPECT_EQ(unpacked.f_, str);
        EXPECT_EQ(unpacked.l_, str + 3);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_16_to_32_iterator<uint16_t const *, null_sentinel> it(
            str, str, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(unpacked.f_, str);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // N = 8
    {
        utf_16_to_8_iterator<uint16_t const *> it1(str, str, str + 3);
        utf_16_to_8_iterator<uint16_t const *> it2(str, str + 3, str + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), uint16_t const *>::value, "");
        EXPECT_EQ(unpacked.f_, str);
        EXPECT_EQ(unpacked.l_, str + 3);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_16_to_8_iterator<uint16_t const *, null_sentinel> it(
            str, str, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(unpacked.f_, str);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }
}

TEST(transcode_view, detail_unpack_32_N)
{
    uint32_t const str[4] = {'f', 'o', 'o', 0};

    // N = 16
    {
        utf_32_to_16_iterator<uint32_t const *> it1(str, str, str + 3);
        utf_32_to_16_iterator<uint32_t const *> it2(str, str + 3, str + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), uint32_t const *>::value, "");
        EXPECT_EQ(unpacked.f_, str);
        EXPECT_EQ(unpacked.l_, str + 3);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_32_to_16_iterator<uint32_t const *, null_sentinel> it(
            str, str, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(unpacked.f_, str);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // N = 8
    {
        utf_32_to_8_iterator<uint32_t const *> it1(str, str, str + 3);
        utf_32_to_8_iterator<uint32_t const *> it2(str, str + 3, str + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), uint32_t const *>::value, "");
        EXPECT_EQ(unpacked.f_, str);
        EXPECT_EQ(unpacked.l_, str + 3);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_32_to_8_iterator<uint32_t const *, null_sentinel> it(
            str, str, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(unpacked.f_, str);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }
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
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        auto r =
            detail::make_utf8_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), char const *>::value, "");
        EXPECT_EQ(unpacked.f_, str8);
        EXPECT_EQ(unpacked.l_, str8 + 3);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_8_to_32_iterator<char const *, null_sentinel> it(
            str8, str8, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        auto r =
            detail::make_utf8_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(unpacked.f_, str8);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // 8 -> 16 -> 8
    {
        utf_8_to_16_iterator<char const *> it1(str8, str8, str8 + 3);
        utf_8_to_16_iterator<char const *> it2(str8, str8 + 3, str8 + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        auto r =
            detail::make_utf8_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), char const *>::value, "");
        EXPECT_EQ(unpacked.f_, str8);
        EXPECT_EQ(unpacked.l_, str8 + 3);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_8_to_16_iterator<char const *, null_sentinel> it(
            str8, str8, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        auto r =
            detail::make_utf8_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(unpacked.f_, str8);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // 16 -> 32 -> 8
    {
        utf_16_to_32_iterator<uint16_t const *> it1(str16, str16, str16 + 3);
        utf_16_to_32_iterator<uint16_t const *> it2(
            str16, str16 + 3, str16 + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value,
            "");
        auto r =
            detail::make_utf8_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), uint16_t const *>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_16_to_8_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(
            unpacked.l_,
            utf_16_to_8_iterator<uint16_t const *>(
                str16, str16 + 3, str16 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_16_to_32_iterator<uint16_t const *, null_sentinel> it(
            str16, str16, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value,
            "");
        auto r =
            detail::make_utf8_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_16_to_8_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // 16 -> 8 -> 8
    {
        utf_16_to_8_iterator<uint16_t const *> it1(str16, str16, str16 + 3);
        utf_16_to_8_iterator<uint16_t const *> it2(str16, str16 + 3, str16 + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value,
            "");
        auto r =
            detail::make_utf8_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), uint16_t const *>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_16_to_8_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(
            unpacked.l_,
            utf_16_to_8_iterator<uint16_t const *>(
                str16, str16 + 3, str16 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_16_to_8_iterator<uint16_t const *, null_sentinel> it(
            str16, str16, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value,
            "");
        auto r =
            detail::make_utf8_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_16_to_8_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // 32 -> 16 -> 8
    {
        utf_32_to_16_iterator<uint32_t const *> it1(str32, str32, str32 + 3);
        utf_32_to_16_iterator<uint32_t const *> it2(
            str32, str32 + 3, str32 + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value,
            "");
        auto r =
            detail::make_utf8_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), uint32_t const *>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_32_to_8_iterator<uint32_t const *>(str32, str32, str32 + 3));
        EXPECT_EQ(
            unpacked.l_,
            utf_32_to_8_iterator<uint32_t const *>(
                str32, str32 + 3, str32 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_32_to_16_iterator<uint32_t const *, null_sentinel> it(
            str32, str32, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value,
            "");
        auto r =
            detail::make_utf8_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_32_to_8_iterator<uint32_t const *>(str32, str32, str32 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // 32 -> 8 -> 8
    {
        utf_32_to_8_iterator<uint32_t const *> it1(str32, str32, str32 + 3);
        utf_32_to_8_iterator<uint32_t const *> it2(str32, str32 + 3, str32 + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value,
            "");
        auto r =
            detail::make_utf8_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), uint32_t const *>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_32_to_8_iterator<uint32_t const *>(str32, str32, str32 + 3));
        EXPECT_EQ(
            unpacked.l_,
            utf_32_to_8_iterator<uint32_t const *>(
                str32, str32 + 3, str32 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_32_to_8_iterator<uint32_t const *, null_sentinel> it(
            str32, str32, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value,
            "");
        auto r =
            detail::make_utf8_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf8_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_32_to_8_iterator<uint32_t const *>(str32, str32, str32 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
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
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        auto r =
            detail::make_utf16_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf16_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), char const *>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_8_to_16_iterator<char const *>(str8, str8, str8 + 3));
        EXPECT_EQ(
            unpacked.l_,
            utf_8_to_16_iterator<char const *>(str8, str8 + 3, str8 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_8_to_32_iterator<char const *, null_sentinel> it(
            str8, str8, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        auto r =
            detail::make_utf16_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf16_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_8_to_16_iterator<char const *>(str8, str8, str8 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // 8 -> 16 -> 16
    {
        utf_8_to_16_iterator<char const *> it1(str8, str8, str8 + 3);
        utf_8_to_16_iterator<char const *> it2(str8, str8 + 3, str8 + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        auto r =
            detail::make_utf16_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf16_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), char const *>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_8_to_16_iterator<char const *>(str8, str8, str8 + 3));
        EXPECT_EQ(
            unpacked.l_,
            utf_8_to_16_iterator<char const *>(str8, str8 + 3, str8 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_8_to_16_iterator<char const *, null_sentinel> it(
            str8, str8, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        auto r =
            detail::make_utf16_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf16_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_8_to_16_iterator<char const *>(str8, str8, str8 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // 16 -> 32 -> 16
    {
        utf_16_to_32_iterator<uint16_t const *> it1(str16, str16, str16 + 3);
        utf_16_to_32_iterator<uint16_t const *> it2(
            str16, str16 + 3, str16 + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value,
            "");
        auto r =
            detail::make_utf16_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf16_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), uint16_t const *>::value, "");
        EXPECT_EQ(unpacked.f_, str16);
        EXPECT_EQ(unpacked.l_, str16 + 3);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_16_to_32_iterator<uint16_t const *, null_sentinel> it(
            str16, str16, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value,
            "");
        auto r =
            detail::make_utf16_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf16_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_16_to_8_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(unpacked.f_, str16);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // 16 -> 8 -> 16
    {
        utf_16_to_8_iterator<uint16_t const *> it1(str16, str16, str16 + 3);
        utf_16_to_8_iterator<uint16_t const *> it2(str16, str16 + 3, str16 + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value,
            "");
        auto r =
            detail::make_utf16_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf16_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), uint16_t const *>::value, "");
        EXPECT_EQ(unpacked.f_, str16);
        EXPECT_EQ(unpacked.l_, str16 + 3);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_16_to_8_iterator<uint16_t const *, null_sentinel> it(
            str16, str16, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value,
            "");
        auto r =
            detail::make_utf16_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf16_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(unpacked.f_, str16);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // 32 -> 16 -> 16
    {
        utf_32_to_16_iterator<uint32_t const *> it1(str32, str32, str32 + 3);
        utf_32_to_16_iterator<uint32_t const *> it2(
            str32, str32 + 3, str32 + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value,
            "");
        auto r =
            detail::make_utf16_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf16_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), uint32_t const *>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_32_to_16_iterator<uint32_t const *>(str32, str32, str32 + 3));
        EXPECT_EQ(
            unpacked.l_,
            utf_32_to_16_iterator<uint32_t const *>(
                str32, str32 + 3, str32 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_32_to_16_iterator<uint32_t const *, null_sentinel> it(
            str32, str32, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value,
            "");
        auto r =
            detail::make_utf16_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf16_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_32_to_16_iterator<uint32_t const *>(str32, str32, str32 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // 32 -> 8 -> 16
    {
        utf_32_to_8_iterator<uint32_t const *> it1(str32, str32, str32 + 3);
        utf_32_to_8_iterator<uint32_t const *> it2(str32, str32 + 3, str32 + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value,
            "");
        auto r =
            detail::make_utf16_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf16_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), uint32_t const *>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_32_to_16_iterator<uint32_t const *>(str32, str32, str32 + 3));
        EXPECT_EQ(
            unpacked.l_,
            utf_32_to_16_iterator<uint32_t const *>(
                str32, str32 + 3, str32 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_32_to_8_iterator<uint32_t const *, null_sentinel> it(
            str32, str32, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value,
            "");
        auto r =
            detail::make_utf16_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf16_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_32_to_16_iterator<uint32_t const *>(str32, str32, str32 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
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
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        auto r =
            detail::make_utf32_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf32_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), char const *>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_8_to_32_iterator<char const *>(str8, str8, str8 + 3));
        EXPECT_EQ(
            unpacked.l_,
            utf_8_to_32_iterator<char const *>(str8, str8 + 3, str8 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_8_to_32_iterator<char const *, null_sentinel> it(
            str8, str8, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        auto r =
            detail::make_utf32_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf32_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_8_to_32_iterator<char const *>(str8, str8, str8 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // 8 -> 16 -> 32
    {
        utf_8_to_16_iterator<char const *> it1(str8, str8, str8 + 3);
        utf_8_to_16_iterator<char const *> it2(str8, str8 + 3, str8 + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        auto r =
            detail::make_utf32_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf32_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), char const *>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_8_to_32_iterator<char const *>(str8, str8, str8 + 3));
        EXPECT_EQ(
            unpacked.l_,
            utf_8_to_32_iterator<char const *>(str8, str8 + 3, str8 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_8_to_16_iterator<char const *, null_sentinel> it(
            str8, str8, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value, "");
        auto r =
            detail::make_utf32_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf32_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_8_to_32_iterator<char const *>(str8, str8, str8 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // 16 -> 32 -> 32
    {
        utf_16_to_32_iterator<uint16_t const *> it1(str16, str16, str16 + 3);
        utf_16_to_32_iterator<uint16_t const *> it2(
            str16, str16 + 3, str16 + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value,
            "");
        auto r =
            detail::make_utf32_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf32_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), uint16_t const *>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_16_to_32_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(
            unpacked.l_,
            utf_16_to_32_iterator<uint16_t const *>(
                str16, str16 + 3, str16 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_16_to_32_iterator<uint16_t const *, null_sentinel> it(
            str16, str16, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value,
            "");
        auto r =
            detail::make_utf32_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf32_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_16_to_8_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(
            unpacked.f_,
            utf_16_to_8_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // 16 -> 8 -> 32
    {
        utf_16_to_8_iterator<uint16_t const *> it1(str16, str16, str16 + 3);
        utf_16_to_8_iterator<uint16_t const *> it2(str16, str16 + 3, str16 + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value,
            "");
        auto r =
            detail::make_utf32_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf32_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), uint16_t const *>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_16_to_32_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(
            unpacked.l_,
            utf_16_to_32_iterator<uint16_t const *>(
                str16, str16 + 3, str16 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_16_to_8_iterator<uint16_t const *, null_sentinel> it(
            str16, str16, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value,
            "");
        auto r =
            detail::make_utf32_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf32_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(
            unpacked.f_,
            utf_16_to_32_iterator<uint16_t const *>(str16, str16, str16 + 3));
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // 32 -> 16 -> 32
    {
        utf_32_to_16_iterator<uint32_t const *> it1(str32, str32, str32 + 3);
        utf_32_to_16_iterator<uint32_t const *> it2(
            str32, str32 + 3, str32 + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value,
            "");
        auto r =
            detail::make_utf32_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf32_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), uint32_t const *>::value, "");
        EXPECT_EQ(unpacked.f_, str32);
        EXPECT_EQ(unpacked.l_, str32 + 3);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_32_to_16_iterator<uint32_t const *, null_sentinel> it(
            str32, str32, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value,
            "");
        auto r =
            detail::make_utf32_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf32_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(unpacked.f_, str32);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }

    // 32 -> 8 -> 32
    {
        utf_32_to_8_iterator<uint32_t const *> it1(str32, str32, str32 + 3);
        utf_32_to_8_iterator<uint32_t const *> it2(str32, str32 + 3, str32 + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value,
            "");
        auto r =
            detail::make_utf32_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf32_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), uint32_t const *>::value, "");
        EXPECT_EQ(unpacked.f_, str32);
        EXPECT_EQ(unpacked.l_, str32 + 3);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it1);
    }
    {
        utf_32_to_8_iterator<uint32_t const *, null_sentinel> it(
            str32, str32, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value,
            "");
        auto r =
            detail::make_utf32_range_(unpacked.tag_, unpacked.f_, unpacked.l_);
        static_assert(
            std::is_same<decltype(r.tag_), detail::utf32_tag>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value, "");
        EXPECT_EQ(unpacked.f_, str32);
        EXPECT_EQ(unpacked.repack_(unpacked.f_), it);
    }
}

// Unicode 9, 3.9/D90-D92
uint32_t const utf32_[4] = {0x004d, 0x0430, 0x4e8c, 0x10302};
uint16_t const utf16_[5] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02};
char const utf8_[10] = {
    0x4d,
    char(0xd0),
    char(0xb0),
    char(0xe4),
    char(0xba),
    char(0x8c),
    char(0xf0),
    char(0x90),
    char(0x8c),
    char(0x82)};

uint32_t const utf32_null[5] = {0x004d, 0x0430, 0x4e8c, 0x10302, 0};
uint16_t const utf16_null[6] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02, 0};
char const utf8_null[11] = {
    0x4d,
    char(0xd0),
    char(0xb0),
    char(0xe4),
    char(0xba),
    char(0x8c),
    char(0xf0),
    char(0x90),
    char(0x8c),
    char(0x82),
    0};

TEST(transcode_view, as_utfN)
{
    // array
    {
        auto r = as_utf8(utf32_);
        EXPECT_TRUE(boost::algorithm::equal(
            r.begin(), r.end(), std::begin(utf8_), std::end(utf8_)));
    }
    {
        auto r = as_utf8(utf16_);
        EXPECT_TRUE(boost::algorithm::equal(
            r.begin(), r.end(), std::begin(utf8_), std::end(utf8_)));
    }
    {
        auto r = as_utf8(utf8_);
        static_assert(
            std::is_same<decltype(r.begin()), char const *>::value, "");
        static_assert(std::is_same<decltype(r.end()), char const *>::value, "");
        EXPECT_TRUE(boost::algorithm::equal(
            r.begin(), r.end(), std::begin(utf8_), std::end(utf8_)));
    }
    {
        auto r = utf32_ | as_utf8;
        EXPECT_TRUE(boost::algorithm::equal(
            r.begin(), r.end(), std::begin(utf8_), std::end(utf8_)));
    }
    {
        auto r = utf16_ | as_utf8;
        EXPECT_TRUE(boost::algorithm::equal(
            r.begin(), r.end(), std::begin(utf8_), std::end(utf8_)));
    }
    {
        auto r = utf8_ | as_utf8;
        static_assert(
            std::is_same<decltype(r.begin()), char const *>::value, "");
        static_assert(std::is_same<decltype(r.end()), char const *>::value, "");
        EXPECT_TRUE(boost::algorithm::equal(
            r.begin(), r.end(), std::begin(utf8_), std::end(utf8_)));
    }

    // ptr/sentinel
    {
        auto r = as_utf8(utf32_null, null_sentinel{});
        auto truth = utf8_;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        auto r = as_utf8(utf16_null, null_sentinel{});
        auto truth = utf8_;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        auto r = as_utf8(utf8_null, null_sentinel{});
        auto truth = utf8_;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }

    // single pointers
    {
        char const * ptr = utf8_null;
        auto r = as_utf8(ptr);
        static_assert(
            std::is_same<decltype(r.begin()), char const *>::value, "");
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
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
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
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
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
        auto truth = utf8_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        char const * ptr = utf8_null;
        auto r = as_utf16(ptr);
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
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
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
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
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
        auto truth = utf16_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        char const * ptr = utf8_null;
        auto r = as_utf32(ptr);
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
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
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
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
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
        auto truth = utf32_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        char const * ptr = utf8_null;
        auto r = ptr | as_utf8;
        static_assert(
            std::is_same<decltype(r.begin()), char const *>::value, "");
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
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
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
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
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
        auto truth = utf8_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        char const * ptr = utf8_null;
        auto r = ptr | as_utf16;
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
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
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
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
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
        auto truth = utf16_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        char const * ptr = utf8_null;
        auto r = ptr | as_utf32;
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
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
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
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
            std::is_same<decltype(r.end()), null_sentinel>::value, "");
        auto truth = utf32_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }



    // funkyzeit
    {
        auto r = as_utf8(as_utf16(as_utf8(as_utf32(as_utf16(as_utf8(utf8_))))));
        EXPECT_TRUE(boost::algorithm::equal(
            r.begin(), r.end(), std::begin(utf8_), std::end(utf8_)));
        static_assert(
            std::is_same<decltype(r.begin()), char const *>::value, "");
        static_assert(std::is_same<decltype(r.end()), char const *>::value, "");
    }
    {
        auto r = utf8_ | as_utf8 | as_utf16 | as_utf32 | as_utf8 | as_utf16 |
                 as_utf8;
        EXPECT_TRUE(boost::algorithm::equal(
            r.begin(), r.end(), std::begin(utf8_), std::end(utf8_)));
        static_assert(
            std::is_same<decltype(r.begin()), char const *>::value, "");
        static_assert(std::is_same<decltype(r.end()), char const *>::value, "");
    }
}

TEST(transcode_view, stream_insertion)
{
    std::string const truth = utf8_null;
    {
        char const * ptr = utf8_null;
        auto r = as_utf8(ptr);
        std::stringstream ss;
        ss << r;
        EXPECT_EQ(ss.str(), truth);
    }
    {
        char const * ptr = utf8_null;
        auto r = as_utf16(ptr);
        std::stringstream ss;
        ss << r;
        EXPECT_EQ(ss.str(), truth);
    }
    {
        char const * ptr = utf8_null;
        auto r = as_utf32(ptr);
        std::stringstream ss;
        ss << r;
        EXPECT_EQ(ss.str(), truth);
    }
}
