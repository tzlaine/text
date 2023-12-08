// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/transcode_view.hpp>
#include <boost/text/formatter.hpp> // TODO

#include <boost/algorithm/cxx14/equal.hpp>

#include <vector>

#include <gtest/gtest.h>

#include "ill_formed.hpp"


using namespace boost::text;

#if defined(__cpp_char8_t)
using u8string_type = std::u8string;
#else
using u8string_type = std::string;
#endif

struct my_text_type
{
    my_text_type() = default;
    my_text_type(u8string_type utf8) : utf8_(std::move(utf8)) {}

    auto begin() const
    {
        return utf_iterator<
            format::utf8,
            format::utf32,
            u8string_type::const_iterator>(
            utf8_.begin(), utf8_.begin(), utf8_.end());
    }
    auto end() const
    {
        return utf_iterator<
            format::utf8,
            format::utf32,
            u8string_type::const_iterator>(
            utf8_.begin(), utf8_.end(), utf8_.end());
    }

private:
    u8string_type utf8_;
};

TEST(transcode_view, paper_example)
{
    {
        u8string_type str = u8"some text";

        auto utf16_str = str | boost::text::as_utf16;

        static_assert(std::is_same_v<
                      decltype(utf16_str.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf8,
                          boost::text::format::utf16,
                          u8string_type::iterator>>);

        auto utf32_str = str | boost::text::as_utf32;

        static_assert(std::is_same_v<
                      decltype(utf32_str.begin()),
                      boost::text::utf_iterator<
                          boost::text::format::utf8,
                          boost::text::format::utf32,
                          u8string_type::iterator>>);
    }
    {
        u8string_type str = u8"";
        auto first = str.begin();
        auto it = first;
        auto last = str.end();
        utf_iterator<format::utf8, format::utf32, u8string_type::iterator>(
            first, it, last);
    }
}

#if BOOST_TEXT_USE_CONCEPTS
TEST(transcode_view, adaptor_semantics)
{
    static_assert(std::is_same_v<
                  decltype(my_text_type(u8"text") | boost::text::as_utf16),
                  boost::text::utf16_view<
                      boost::text::unpacking_view<std::ranges::owning_view<my_text_type>>>>);

    static_assert(
        std::is_same_v<
            decltype(u8"text" | boost::text::as_utf16),
            boost::text::utf16_view<std::ranges::subrange<const char8_t *>>>);

    static_assert(
        std::is_same_v<
            decltype(std::u8string(u8"text") | boost::text::as_utf16),
            boost::text::utf16_view<std::ranges::owning_view<std::u8string>>>);

    std::u8string const str = u8"text";

    static_assert(std::is_same_v<
                  decltype(str | boost::text::as_utf16),
                  boost::text::utf16_view<std::ranges::ref_view<std::u8string const>>>);

    static_assert(std::is_same_v<
                  decltype(str.c_str() | boost::text::as_utf16),
                  boost::text::utf16_view<std::ranges::subrange<
                      const char8_t *,
                      boost::text::null_sentinel_t>>>);

    static_assert(
        std::is_same_v<
            decltype(std::ranges::empty_view<int>{} | boost::text::as_char16_t),
            std::ranges::empty_view<char16_t>>);

    std::u16string str2 = u"text";

    static_assert(std::is_same_v<
                  decltype(str2 | boost::text::as_utf16),
                  boost::text::utf16_view<
                      std::ranges::ref_view<std::u16string>>>);

    static_assert(std::is_same_v<
                  decltype(str2.c_str() | boost::text::as_utf16),
                  boost::text::utf16_view<std::ranges::subrange<
                      const char16_t *,
                      boost::text::null_sentinel_t>>>);
}
#endif

#if defined(__cpp_lib_format) && !defined(__clang__)
TEST(formatter, basic) // TODO
{
    {
        auto v = u"text" | boost::text::as_utf8;
        std::string result = std::format("{}", v);
    }
    {
        auto v = u"text" | boost::text::as_utf8;
        std::wstring result = std::format(L"{}", v);
    }

    {
        auto v = u8"text" | boost::text::as_utf16;
        std::string result = std::format("{}", v);
    }
    {
        auto v = u8"text" | boost::text::as_utf16;
        std::wstring result = std::format(L"{}", v);
    }

    {
        auto v = u8"text" | boost::text::as_utf32;
        std::string result = std::format("{}", v);
    }
    {
        auto v = u8"text" | boost::text::as_utf32;
        std::wstring result = std::format(L"{}", v);
    }
}
#endif

TEST(transcode_view, from_utf8)
{
    char8_type const * str8 = u8"foo";

    {
        utf_iterator<format::utf8, format::utf32, char8_type const *> it1(str8, str8, str8 + 3);
        utf_iterator<format::utf8, format::utf32, char8_type const *> it2(str8, str8 + 3, str8 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char8_type const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), char8_type const *>::value, "");
        EXPECT_EQ(unpacked.first, str8);
        EXPECT_EQ(unpacked.last, str8 + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
        EXPECT_TRUE(std::equal(unpacked.repack(unpacked.first), it2, it1, it2));
    }
    {
        utf_iterator<format::utf8, format::utf32, char8_type const *, null_sentinel_t> it(
            str8, str8, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char8_type const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(unpacked.first, str8);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
        auto it_1 = unpacked.repack(unpacked.first);
        for (auto it_2 = it; it_1 != null_sentinel && it_2 != null_sentinel;
             ++it_1, ++it_2) {
            EXPECT_EQ(*it_1, *it_2);
        }
    }

    {
        utf_iterator<format::utf8, format::utf16, char8_type const *> it1(str8, str8, str8 + 3);
        utf_iterator<format::utf8, format::utf16, char8_type const *> it2(str8, str8 + 3, str8 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char8_type const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), char8_type const *>::value, "");
        EXPECT_EQ(unpacked.first, str8);
        EXPECT_EQ(unpacked.last, str8 + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
        EXPECT_TRUE(std::equal(unpacked.repack(unpacked.first), it2, it1, it2));
    }
    {
        utf_iterator<format::utf8, format::utf16, char8_type const *, null_sentinel_t> it(
            str8, str8, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char8_type const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(unpacked.first, str8);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
        auto it_1 = unpacked.repack(unpacked.first);
        for (auto it_2 = it; it_1 != null_sentinel && it_2 != null_sentinel;
             ++it_1, ++it_2) {
            EXPECT_EQ(*it_1, *it_2);
        }
    }
}

TEST(transcode_view, from_utf16)
{
    char16_t const str16[4] = {'f', 'o', 'o', 0};

    {
        utf_iterator<format::utf16, format::utf8, char16_t const *> it1(str16, str16, str16 + 3);
        utf_iterator<format::utf16, format::utf8, char16_t const *> it2(str16, str16 + 3, str16 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char16_t const *>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.last), char16_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str16);
        EXPECT_EQ(unpacked.last, str16 + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
        EXPECT_TRUE(std::equal(unpacked.repack(unpacked.first), it2, it1, it2));
    }
    {
        utf_iterator<format::utf16, format::utf8, char16_t const *, null_sentinel_t> it(
            str16, str16, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char16_t const *>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(unpacked.first, str16);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
        auto it_1 = unpacked.repack(unpacked.first);
        for (auto it_2 = it; it_1 != null_sentinel && it_2 != null_sentinel;
             ++it_1, ++it_2) {
            EXPECT_EQ(*it_1, *it_2);
        }
    }

    {
        utf_iterator<format::utf16, format::utf32, char16_t const *> it1(str16, str16, str16 + 3);
        utf_iterator<format::utf16, format::utf32, char16_t const *> it2(str16, str16 + 3, str16 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char16_t const *>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.last), char16_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str16);
        EXPECT_EQ(unpacked.last, str16 + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
        EXPECT_TRUE(std::equal(unpacked.repack(unpacked.first), it2, it1, it2));
    }
    {
        utf_iterator<format::utf16, format::utf32, char16_t const *, null_sentinel_t> it(
            str16, str16, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char16_t const *>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(unpacked.first, str16);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
        auto it_1 = unpacked.repack(unpacked.first);
        for (auto it_2 = it; it_1 != null_sentinel && it_2 != null_sentinel;
             ++it_1, ++it_2) {
            EXPECT_EQ(*it_1, *it_2);
        }
    }
}

TEST(transcode_view, from_utf32)
{
    char32_t const str32[4] = {'f', 'o', 'o', 0};

    {
        utf_iterator<format::utf32, format::utf16, char32_t const *> it1(str32, str32, str32 + 3);
        utf_iterator<format::utf32, format::utf16, char32_t const *> it2(
            str32, str32 + 3, str32 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char32_t const *>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.last), char32_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str32);
        EXPECT_EQ(unpacked.last, str32 + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
        EXPECT_TRUE(std::equal(unpacked.repack(unpacked.first), it2, it1, it2));
    }
    {
        utf_iterator<format::utf32, format::utf16, char32_t const *, null_sentinel_t> it(
            str32, str32, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char32_t const *>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(unpacked.first, str32);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
        auto it_1 = unpacked.repack(unpacked.first);
        for (auto it_2 = it; it_1 != null_sentinel && it_2 != null_sentinel;
             ++it_1, ++it_2) {
            EXPECT_EQ(*it_1, *it_2);
        }
    }

    {
        utf_iterator<format::utf32, format::utf8, char32_t const *> it1(str32, str32, str32 + 3);
        utf_iterator<format::utf32, format::utf8, char32_t const *> it2(str32, str32 + 3, str32 + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char32_t const *>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.last), char32_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str32);
        EXPECT_EQ(unpacked.last, str32 + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
        EXPECT_TRUE(std::equal(unpacked.repack(unpacked.first), it2, it1, it2));
    }
    {
        utf_iterator<format::utf32, format::utf8, char32_t const *, null_sentinel_t> it(
            str32, str32, null_sentinel);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(it, null_sentinel);
        static_assert(unpacked.format_tag == format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char32_t const *>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.last), null_sentinel_t>::value, "");
        EXPECT_EQ(unpacked.first, str32);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
        auto it_1 = unpacked.repack(unpacked.first);
        for (auto it_2 = it; it_1 != null_sentinel && it_2 != null_sentinel;
             ++it_1, ++it_2) {
            EXPECT_EQ(*it_1, *it_2);
        }
    }
}

TEST(transcode_view, null_terminated_arrays)
{
    char8_type const utf8_[10] = {
        (char8_type)0x4d,
        (char8_type)0xd0,
        (char8_type)0xb0,
        (char8_type)0xe4,
        (char8_type)0xba,
        (char8_type)0x8c,
        (char8_type)0xf0,
        (char8_type)0x90,
        (char8_type)0x8c,
        (char8_type)0x82};
    char8_type const utf8_null[] = {
        (char8_type)0x4d,
        (char8_type)0xd0,
        (char8_type)0xb0,
        (char8_type)0xe4,
        (char8_type)0xba,
        (char8_type)0x8c,
        (char8_type)0xf0,
        (char8_type)0x90,
        (char8_type)0x8c,
        (char8_type)0x82,
        0};

    {
        auto a = utf8_ | as_utf32;
        auto b = utf8_null | as_utf32;
        EXPECT_TRUE(std::equal(a.begin(), a.end(), b.begin(), b.end()));
    }
    {
        auto a = utf8_ | as_utf32;
        auto b = u8"\x4d\xd0\xb0\xe4\xba\x8c\xf0\x90\x8c\x82" | as_utf32;
        auto it_1 = a.begin();
        for (auto it_2 = b.begin(); it_1 != a.end() && it_2 != b.end();
             ++it_1, ++it_2) {
            EXPECT_EQ(*it_1, *it_2);
        }
    }
}

// Unicode 9, 3.9/D90-D92
char32_t const utf32_[4] = {0x004d, 0x0430, 0x4e8c, 0x10302};
char16_t const utf16_[5] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02};
char8_type const utf8_[10] = {
    (char8_type)0x4d,
    (char8_type)0xd0,
    (char8_type)0xb0,
    (char8_type)0xe4,
    (char8_type)0xba,
    (char8_type)0x8c,
    (char8_type)0xf0,
    (char8_type)0x90,
    (char8_type)0x8c,
    (char8_type)0x82};

char32_t const utf32_null[5] = {0x004d, 0x0430, 0x4e8c, 0x10302, 0};
char16_t const utf16_null[6] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02, 0};
char8_type const utf8_null[11] = {
    (char8_type)0x4d,
    (char8_type)0xd0,
    (char8_type)0xb0,
    (char8_type)0xe4,
    (char8_type)0xba,
    (char8_type)0x8c,
    (char8_type)0xf0,
    (char8_type)0x90,
    (char8_type)0x8c,
    (char8_type)0x82,
    (char8_type)0};

TEST(transcode_view, as_utfN)
{
    // array
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
            std::is_same<
                decltype(r.begin()),
                utf_iterator<format::utf8, format::utf8, char8_type const *>>::
                value,
            "");
        static_assert(
            std::is_same<
                decltype(r.begin()),
                utf_iterator<format::utf8, format::utf8, char8_type const *>>::
                value,
            "");
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
            std::is_same<
                decltype(r.begin()),
                utf_iterator<format::utf8, format::utf8, char8_type const *>>::
                value,
            "");
        static_assert(
            std::is_same<
                decltype(r.end()),
                utf_iterator<format::utf8, format::utf8, char8_type const *>>::
                value,
            "");
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
        char8_type const * ptr = utf8_null;
        auto r = as_utf8(ptr);
        static_assert(
            std::is_same<
                decltype(r.begin()),
                utf_iterator<
                    format::utf8,
                    format::utf8,
                    char8_type const *,
                    null_sentinel_t>>::value,
            "");
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf8_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        char16_t const * ptr = utf16_null;
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
        char32_t const * ptr = utf32_null;
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
        char8_type const * ptr = utf8_null;
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
        char16_t const * ptr = utf16_null;
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
        char32_t const * ptr = utf32_null;
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
        char8_type const * ptr = utf8_null;
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
        char16_t const * ptr = utf16_null;
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
        char32_t const * ptr = utf32_null;
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
        char8_type const * ptr = utf8_null;
        auto r = ptr | as_utf8;
        static_assert(
            std::is_same<
                decltype(r.begin()),
                utf_iterator<
                    format::utf8,
                    format::utf8,
                    char8_type const *,
                    null_sentinel_t>>::value,
            "");
        static_assert(
            std::is_same<decltype(r.end()), null_sentinel_t>::value, "");
        auto truth = utf8_null;
        int i = 0;
        for (auto it = r.begin(); it != r.end(); ++it, ++i, ++truth) {
            EXPECT_EQ(*it, *truth) << "iteration " << i;
        }
    }
    {
        char16_t const * ptr = utf16_null;
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
        char32_t const * ptr = utf32_null;
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
        char8_type const * ptr = utf8_null;
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
        char16_t const * ptr = utf16_null;
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
        char32_t const * ptr = utf32_null;
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
        char8_type const * ptr = utf8_null;
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
        char16_t const * ptr = utf16_null;
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
        char32_t const * ptr = utf32_null;
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
        auto r = utf8_ | as_utf8 | as_utf16 | as_utf32 | as_utf8 | as_utf16 |
                 as_utf8;
        EXPECT_TRUE(boost::algorithm::equal(
            r.begin(), r.end(), std::begin(utf8_), std::end(utf8_)));
        static_assert(
            std::is_same<
                decltype(r.begin()),
                utf_iterator<format::utf8, format::utf8, char8_type const *>>::
                value,
            "");
        static_assert(
            std::is_same<
                decltype(r.end()),
                utf_iterator<format::utf8, format::utf8, char8_type const *>>::
                value,
            "");
    }
}

TEST(transcode_view, stream_insertion)
{
    std::string const truth(std::begin(utf8_null), std::end(utf8_null) - 1);
    {
        char8_type const * ptr = utf8_null;
        auto r = ptr | as_utf16;
        std::stringstream ss;
        ss << r;
        EXPECT_EQ(ss.str(), truth);
    }
    {
        char8_type const * ptr = utf8_null;
        auto r = ptr | as_utf32;
        std::stringstream ss;
        ss << r;
        EXPECT_EQ(ss.str(), truth);
    }
}
