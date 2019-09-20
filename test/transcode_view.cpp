#include <boost/text/transcode_view.hpp>

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
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value);
        static_assert(std::is_same<
                      decltype(unpacked.f_),
                      std::vector<uint32_t>::iterator>::value);
        static_assert(std::is_same<
                      decltype(unpacked.l_),
                      std::vector<uint32_t>::iterator>::value);
        EXPECT_EQ(unpacked.f_, utf32.begin());
        EXPECT_EQ(unpacked.l_, utf32.end());
    }
    {
        std::vector<uint16_t> utf16(3);
        auto unpacked =
            detail::unpack_iterator_and_sentinel(utf16.begin(), utf16.end());
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value);
        static_assert(std::is_same<
                      decltype(unpacked.f_),
                      std::vector<uint16_t>::iterator>::value);
        static_assert(std::is_same<
                      decltype(unpacked.l_),
                      std::vector<uint16_t>::iterator>::value);
        EXPECT_EQ(unpacked.f_, utf16.begin());
        EXPECT_EQ(unpacked.l_, utf16.end());
    }
    {
        std::vector<unsigned char> utf8(3);
        auto unpacked =
            detail::unpack_iterator_and_sentinel(utf8.begin(), utf8.end());
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value);
        static_assert(std::is_same<
                      decltype(unpacked.f_),
                      std::vector<unsigned char>::iterator>::value);
        static_assert(std::is_same<
                      decltype(unpacked.l_),
                      std::vector<unsigned char>::iterator>::value);
        EXPECT_EQ(unpacked.f_, utf8.begin());
        EXPECT_EQ(unpacked.l_, utf8.end());
    }
    {
        std::vector<char> utf8(3);
        auto unpacked =
            detail::unpack_iterator_and_sentinel(utf8.begin(), utf8.end());
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), std::vector<char>::iterator>::
                value);
        static_assert(
            std::is_same<decltype(unpacked.l_), std::vector<char>::iterator>::
                value);
        EXPECT_EQ(unpacked.f_, utf8.begin());
        EXPECT_EQ(unpacked.l_, utf8.end());
    }
    {
        char const * utf8 = "foo";
        auto unpacked =
            detail::unpack_iterator_and_sentinel(utf8, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value);
        static_assert(std::is_same<decltype(unpacked.f_), char const *>::value);
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value);
        EXPECT_EQ(unpacked.f_, utf8);
    }
    {
        char const * utf8 = "foo";
        auto unpacked = detail::unpack_iterator_and_sentinel(utf8, utf8 + 3);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value);
        static_assert(std::is_same<decltype(unpacked.f_), char const *>::value);
        static_assert(std::is_same<decltype(unpacked.l_), char const *>::value);
        EXPECT_EQ(unpacked.f_, utf8);
        EXPECT_EQ(unpacked.l_, utf8 + 3);
    }
}

template<typename Iter>
using bad_unpack_t = decltype(detail::unpack_iterator_and_sentinel(
    std::declval<Iter>(), std::declval<Iter>()));
static_assert(!ill_formed<bad_unpack_t, uint64_t *>::value_t::value, "");

TEST(transcode_view, detail_unpack_8_N)
{
    char const * str = "full";

    // N = 32
    {
        utf_8_to_32_iterator<char const *> it1(str, str, str + 3);
        utf_8_to_32_iterator<char const *> it2(str, str + 3, str + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value);
        static_assert(std::is_same<decltype(unpacked.f_), char const *>::value);
        static_assert(std::is_same<decltype(unpacked.l_), char const *>::value);
    }
    {
        utf_8_to_32_iterator<char const *> it(str, str, str + 3);
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value);
        static_assert(std::is_same<decltype(unpacked.f_), char const *>::value);
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value);
    }
    {
        utf_8_to_32_iterator<char const *, null_sentinel> it(
            str, str, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value);
        static_assert(std::is_same<decltype(unpacked.f_), char const *>::value);
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value);
    }

    // N = 16
    {
        utf_8_to_16_iterator<char const *> it1(str, str, str + 3);
        utf_8_to_16_iterator<char const *> it2(str, str + 3, str + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value);
        static_assert(std::is_same<decltype(unpacked.f_), char const *>::value);
        static_assert(std::is_same<decltype(unpacked.l_), char const *>::value);
    }
    {
        utf_8_to_16_iterator<char const *> it(str, str, str + 3);
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value);
        static_assert(std::is_same<decltype(unpacked.f_), char const *>::value);
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value);
    }
    {
        utf_8_to_16_iterator<char const *, null_sentinel> it(
            str, str, null_sentinel{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sentinel{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf8_tag>::value);
        static_assert(std::is_same<decltype(unpacked.f_), char const *>::value);
        static_assert(
            std::is_same<decltype(unpacked.l_), null_sentinel>::value);
    }
}

struct null_sent_16
{};
inline bool operator==(uint16_t const * p, null_sent_16) { return !*p; }
inline bool operator!=(uint16_t const * p, null_sent_16) { return *p; }

TEST(transcode_view, detail_unpack_16_N)
{
    uint16_t const str[4] = {'f', 'o', 'o', 0};

    // N = 32
    {
        utf_16_to_32_iterator<uint16_t const *> it1(str, str, str + 3);
        utf_16_to_32_iterator<uint16_t const *> it2(str, str + 3, str + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value);
        static_assert(
            std::is_same<decltype(unpacked.l_), uint16_t const *>::value);
    }
    {
        utf_16_to_32_iterator<uint16_t const *> it(str, str, str + 3);
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sent_16{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value);
        static_assert(std::is_same<decltype(unpacked.l_), null_sent_16>::value);
    }
    {
        utf_16_to_32_iterator<uint16_t const *, null_sent_16> it(
            str, str, null_sent_16{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sent_16{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value);
        static_assert(std::is_same<decltype(unpacked.l_), null_sent_16>::value);
    }

    // N = 8
    {
        utf_16_to_8_iterator<uint16_t const *> it1(str, str, str + 3);
        utf_16_to_8_iterator<uint16_t const *> it2(str, str + 3, str + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value);
        static_assert(
            std::is_same<decltype(unpacked.l_), uint16_t const *>::value);
    }
    {
        utf_16_to_8_iterator<uint16_t const *> it(str, str, str + 3);
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sent_16{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value);
        static_assert(std::is_same<decltype(unpacked.l_), null_sent_16>::value);
    }
    {
        utf_16_to_8_iterator<uint16_t const *, null_sent_16> it(
            str, str, null_sent_16{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sent_16{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf16_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint16_t const *>::value);
        static_assert(std::is_same<decltype(unpacked.l_), null_sent_16>::value);
    }
}

struct null_sent_32
{};
inline bool operator==(uint32_t const * p, null_sent_32) { return !*p; }
inline bool operator!=(uint32_t const * p, null_sent_32) { return *p; }

TEST(transcode_view, detail_unpack_32_N)
{
    uint32_t const str[4] = {'f', 'o', 'o', 0};

    // N = 16
    {
        utf_32_to_16_iterator<uint32_t const *> it1(str, str, str + 3);
        utf_32_to_16_iterator<uint32_t const *> it2(str, str + 3, str + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value);
        static_assert(
            std::is_same<decltype(unpacked.l_), uint32_t const *>::value);
    }
    {
        utf_32_to_16_iterator<uint32_t const *> it(str, str, str + 3);
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sent_32{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value);
        static_assert(std::is_same<decltype(unpacked.l_), null_sent_32>::value);
    }
    {
        utf_32_to_16_iterator<uint32_t const *, null_sent_32> it(
            str, str, null_sent_32{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sent_32{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value);
        static_assert(std::is_same<decltype(unpacked.l_), null_sent_32>::value);
    }

    // N = 8
    {
        utf_32_to_8_iterator<uint32_t const *> it1(str, str, str + 3);
        utf_32_to_8_iterator<uint32_t const *> it2(str, str + 3, str + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value);
        static_assert(
            std::is_same<decltype(unpacked.l_), uint32_t const *>::value);
    }
    {
        utf_32_to_8_iterator<uint32_t const *> it(str, str, str + 3);
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sent_32{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value);
        static_assert(std::is_same<decltype(unpacked.l_), null_sent_32>::value);
    }
    {
        utf_32_to_8_iterator<uint32_t const *, null_sent_32> it(
            str, str, null_sent_32{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sent_32{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value);
        static_assert(std::is_same<decltype(unpacked.l_), null_sent_32>::value);
    }
}

#if 0
TEST(transcode_view, detail_make_utf8)
{
    char const * str = "foo";

    // N = 16
    {
        utf_32_to_16_iterator<uint32_t const *> it1(str, str, str + 3);
        utf_32_to_16_iterator<uint32_t const *> it2(str, str + 3, str + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value);
        static_assert(
            std::is_same<decltype(unpacked.l_), uint32_t const *>::value);
    }
    {
        utf_32_to_16_iterator<uint32_t const *> it(str, str, str + 3);
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sent_32{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value);
        static_assert(std::is_same<decltype(unpacked.l_), null_sent_32>::value);
    }
    {
        utf_32_to_16_iterator<uint32_t const *, null_sent_32> it(
            str, str, null_sent_32{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sent_32{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value);
        static_assert(std::is_same<decltype(unpacked.l_), null_sent_32>::value);
    }

    // N = 8
    {
        utf_32_to_8_iterator<uint32_t const *> it1(str, str, str + 3);
        utf_32_to_8_iterator<uint32_t const *> it2(str, str + 3, str + 3);
        auto unpacked = detail::unpack_iterator_and_sentinel(it1, it2);
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value);
        static_assert(
            std::is_same<decltype(unpacked.l_), uint32_t const *>::value);
    }
    {
        utf_32_to_8_iterator<uint32_t const *> it(str, str, str + 3);
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sent_32{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value);
        static_assert(std::is_same<decltype(unpacked.l_), null_sent_32>::value);
    }
    {
        utf_32_to_8_iterator<uint32_t const *, null_sent_32> it(
            str, str, null_sent_32{});
        auto unpacked =
            detail::unpack_iterator_and_sentinel(it, null_sent_32{});
        static_assert(
            std::is_same<decltype(unpacked.tag_), detail::utf32_tag>::value);
        static_assert(
            std::is_same<decltype(unpacked.f_), uint32_t const *>::value);
        static_assert(std::is_same<decltype(unpacked.l_), null_sent_32>::value);
    }
}
#endif
