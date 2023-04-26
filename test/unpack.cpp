// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/unpack.hpp>

#include <boost/algorithm/cxx14/equal.hpp>

#include <vector>

#include <gtest/gtest.h>

#include "ill_formed.hpp"


TEST(unpack, base_case)
{
    {
        std::vector<uint32_t> utf32(3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            utf32.begin(), utf32.end());
        static_assert(unpacked.format_tag == boost::text::format::utf32, "");
        static_assert(
            std::is_same<
                decltype(unpacked.first),
                std::vector<uint32_t>::iterator>::value,
            "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                std::vector<uint32_t>::iterator>::value,
            "");
        EXPECT_EQ(unpacked.first, utf32.begin());
        EXPECT_EQ(unpacked.last, utf32.end());
        EXPECT_EQ(unpacked.repack(unpacked.first), utf32.begin());
    }
    {
        std::vector<uint16_t> utf16(3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            utf16.begin(), utf16.end());
        static_assert(unpacked.format_tag == boost::text::format::utf16, "");
        static_assert(
            std::is_same<
                decltype(unpacked.first),
                std::vector<uint16_t>::iterator>::value,
            "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                std::vector<uint16_t>::iterator>::value,
            "");
        EXPECT_EQ(unpacked.first, utf16.begin());
        EXPECT_EQ(unpacked.last, utf16.end());
        EXPECT_EQ(unpacked.repack(unpacked.first), utf16.begin());
    }
    {
        std::vector<unsigned char> utf8(3);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(utf8.begin(), utf8.end());
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<
                decltype(unpacked.first),
                std::vector<unsigned char>::iterator>::value,
            "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                std::vector<unsigned char>::iterator>::value,
            "");
        EXPECT_EQ(unpacked.first, utf8.begin());
        EXPECT_EQ(unpacked.last, utf8.end());
        EXPECT_EQ(unpacked.repack(unpacked.first), utf8.begin());
    }
    {
        std::vector<char> utf8(3);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(utf8.begin(), utf8.end());
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<
                decltype(unpacked.first),
                std::vector<char>::iterator>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.last), std::vector<char>::iterator>::
                value,
            "");
        EXPECT_EQ(unpacked.first, utf8.begin());
        EXPECT_EQ(unpacked.last, utf8.end());
        EXPECT_EQ(unpacked.repack(unpacked.first), utf8.begin());
    }
    {
        char const * utf8 = "foo";
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            utf8, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                boost::text::null_sentinel_t>::value,
            "");
        EXPECT_EQ(unpacked.first, utf8);
        EXPECT_EQ(unpacked.repack(unpacked.first), utf8);
    }
    {
        char const * utf8 = "foo";
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(utf8, utf8 + 3);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), char const *>::value, "");
        EXPECT_EQ(unpacked.first, utf8);
        EXPECT_EQ(unpacked.last, utf8 + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), utf8);
    }
}

template<typename Iter>
using bad_unpack_t = decltype(boost::text::unpack_iterator_and_sentinel(
    std::declval<Iter>(), std::declval<Iter>()));
static_assert(ill_formed<bad_unpack_t, uint64_t *>::value, "");

TEST(unpack, _8_N)
{
    char const * str = "foo";

    // N = 32
    {
        boost::text::utf_8_to_32_iterator<char const *> it1(str, str, str + 3);
        boost::text::utf_8_to_32_iterator<char const *> it2(
            str, str + 3, str + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), char const *>::value, "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.last, str + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        boost::text::
            utf_8_to_32_iterator<char const *, boost::text::null_sentinel_t>
                it(str, str, boost::text::null_sentinel);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                boost::text::null_sentinel_t>::value,
            "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // N = 16
    {
        boost::text::utf_8_to_16_iterator<char const *> it1(str, str, str + 3);
        boost::text::utf_8_to_16_iterator<char const *> it2(
            str, str + 3, str + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), char const *>::value, "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.last, str + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        boost::text::
            utf_8_to_16_iterator<char const *, boost::text::null_sentinel_t>
                it(str, str, boost::text::null_sentinel);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                boost::text::null_sentinel_t>::value,
            "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }
}

TEST(unpack, _16_N)
{
    uint16_t const str[4] = {'f', 'o', 'o', 0};

    // N = 32
    {
        boost::text::utf_16_to_32_iterator<uint16_t const *> it1(
            str, str, str + 3);
        boost::text::utf_16_to_32_iterator<uint16_t const *> it2(
            str, str + 3, str + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == boost::text::format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint16_t const *>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.last), uint16_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.last, str + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        boost::text::utf_16_to_32_iterator<
            uint16_t const *,
            boost::text::null_sentinel_t>
            it(str, str, boost::text::null_sentinel);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint16_t const *>::value,
            "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                boost::text::null_sentinel_t>::value,
            "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // N = 8
    {
        boost::text::utf_16_to_8_iterator<uint16_t const *> it1(
            str, str, str + 3);
        boost::text::utf_16_to_8_iterator<uint16_t const *> it2(
            str, str + 3, str + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == boost::text::format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint16_t const *>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.last), uint16_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.last, str + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        boost::text::
            utf_16_to_8_iterator<uint16_t const *, boost::text::null_sentinel_t>
                it(str, str, boost::text::null_sentinel);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint16_t const *>::value,
            "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                boost::text::null_sentinel_t>::value,
            "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }
}

TEST(unpack, _32_N)
{
    uint32_t const str[4] = {'f', 'o', 'o', 0};

    // N = 16
    {
        boost::text::utf_32_to_16_iterator<uint32_t const *> it1(
            str, str, str + 3);
        boost::text::utf_32_to_16_iterator<uint32_t const *> it2(
            str, str + 3, str + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == boost::text::format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint32_t const *>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.last), uint32_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.last, str + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        boost::text::utf_32_to_16_iterator<
            uint32_t const *,
            boost::text::null_sentinel_t>
            it(str, str, boost::text::null_sentinel);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint32_t const *>::value,
            "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                boost::text::null_sentinel_t>::value,
            "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    // N = 8
    {
        boost::text::utf_32_to_8_iterator<uint32_t const *> it1(
            str, str, str + 3);
        boost::text::utf_32_to_8_iterator<uint32_t const *> it2(
            str, str + 3, str + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == boost::text::format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint32_t const *>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.last), uint32_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.last, str + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        boost::text::
            utf_32_to_8_iterator<uint32_t const *, boost::text::null_sentinel_t>
                it(str, str, boost::text::null_sentinel);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), uint32_t const *>::value,
            "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                boost::text::null_sentinel_t>::value,
            "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }
}

namespace my {

    struct _8_to_32
        : boost::stl_interfaces::
              iterator_interface<_8_to_32, std::bidirectional_iterator_tag, int>
    {
        _8_to_32() = default;
        _8_to_32(
            char const * first, char const * it, boost::text::null_sentinel_t) :
            it_{first, it, boost::text::null_sentinel}
        {}

        friend bool
        operator==(_8_to_32 first, boost::text::null_sentinel_t last)
        {
            return first.it_ == last;
        }

        auto base() const { return it_; }

    private:
        friend boost::stl_interfaces::access;
        using iterator_t = boost::text::
            utf_8_to_32_iterator<char const *, boost::text::null_sentinel_t>;
        iterator_t & base_reference() noexcept { return it_; }
        iterator_t base_reference() const noexcept { return it_; }
        iterator_t it_;
    };

    template<typename Repack = boost::text::no_op_repacker>
    auto unpack_iterator_and_sentinel(
        _8_to_32 it, boost::text::null_sentinel_t, Repack repack_ = Repack())
    {
        boost::text::repacker<
            _8_to_32,
            char const *,
            boost::text::null_sentinel_t,
            Repack>
            repack(it.base().begin(), it.base().end(), repack_);
        return boost::text::utf_tagged_range<
            boost::text::format::utf8,
            char const *,
            boost::text::null_sentinel_t,
            decltype(repack)>{
            it.base().begin(), boost::text::null_sentinel, repack};
    }

    template<typename I>
    struct template_8_to_32 : boost::stl_interfaces::iterator_interface<
                                  template_8_to_32<I>,
                                  std::bidirectional_iterator_tag,
                                  int>
    {
        template_8_to_32() = default;
        template_8_to_32(I first, I it, boost::text::null_sentinel_t) :
            it_{first, it, boost::text::null_sentinel}
        {}

        friend bool
        operator==(template_8_to_32<I> first, boost::text::null_sentinel_t last)
        {
            return first.it_ == last;
        }

        auto base() const { return it_; }

    private:
        friend boost::stl_interfaces::access;
        using iterator_t =
            boost::text::utf_8_to_32_iterator<I, boost::text::null_sentinel_t>;
        iterator_t & base_reference() noexcept { return it_; }
        iterator_t base_reference() const noexcept { return it_; }
        iterator_t it_;
    };

    template<typename I, typename Repack = boost::text::no_op_repacker>
    auto unpack_iterator_and_sentinel(
        template_8_to_32<I> it,
        boost::text::null_sentinel_t,
        Repack repack_ = Repack())
    {
        boost::text::repacker<
            template_8_to_32<I>,
            char const *,
            boost::text::null_sentinel_t,
            Repack>
            repack(it.base().begin(), it.base().end(), repack_);
        return boost::text::utf_tagged_range<
            boost::text::format::utf8,
            char const *,
            boost::text::null_sentinel_t,
            decltype(repack)>{
            it.base().begin(), boost::text::null_sentinel, repack};
    }

}

TEST(unpack, user_type)
{
    char str[4] = {'f', 'o', 'o', 0};

    {
        my::_8_to_32 it(str, str, boost::text::null_sentinel);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                boost::text::null_sentinel_t>::value,
            "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    {
        my::template_8_to_32<char const *> it(
            str, str, boost::text::null_sentinel);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char const *>::value, "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                boost::text::null_sentinel_t>::value,
            "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }
}
