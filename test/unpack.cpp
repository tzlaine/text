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
        std::vector<char32_t> utf32(3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            utf32.begin(), utf32.end());
        static_assert(unpacked.format_tag == boost::text::format::utf32, "");
        static_assert(
            std::is_same<
                decltype(unpacked.first),
                std::vector<char32_t>::iterator>::value,
            "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                std::vector<char32_t>::iterator>::value,
            "");
        EXPECT_EQ(unpacked.first, utf32.begin());
        EXPECT_EQ(unpacked.last, utf32.end());
        EXPECT_EQ(unpacked.repack(unpacked.first), utf32.begin());
    }
    {
        std::vector<char16_t> utf16(3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            utf16.begin(), utf16.end());
        static_assert(unpacked.format_tag == boost::text::format::utf16, "");
        static_assert(
            std::is_same<
                decltype(unpacked.first),
                std::vector<char16_t>::iterator>::value,
            "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                std::vector<char16_t>::iterator>::value,
            "");
        EXPECT_EQ(unpacked.first, utf16.begin());
        EXPECT_EQ(unpacked.last, utf16.end());
        EXPECT_EQ(unpacked.repack(unpacked.first), utf16.begin());
    }
    {
        std::vector<char8_t> utf8(3);
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(utf8.begin(), utf8.end());
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<
                decltype(unpacked.first),
                std::vector<char8_t>::iterator>::value,
            "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                std::vector<char8_t>::iterator>::value,
            "");
        EXPECT_EQ(unpacked.first, utf8.begin());
        EXPECT_EQ(unpacked.last, utf8.end());
        EXPECT_EQ(unpacked.repack(unpacked.first), utf8.begin());
    }
    {
        char8_t const * utf8 = u8"foo";
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            utf8, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char8_t const *>::value, "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                boost::text::null_sentinel_t>::value,
            "");
        EXPECT_EQ(unpacked.first, utf8);
        EXPECT_EQ(unpacked.repack(unpacked.first), utf8);
    }
    {
        char8_t const * utf8 = u8"foo";
        auto unpacked =
            boost::text::unpack_iterator_and_sentinel(utf8, utf8 + 3);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char8_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), char8_t const *>::value, "");
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
    char8_t const * str = u8"foo";

    // N = 32
    {
        boost::text::utf_8_to_32_iterator<char8_t const *> it1(str, str, str + 3);
        boost::text::utf_8_to_32_iterator<char8_t const *> it2(
            str, str + 3, str + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char8_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), char8_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.last, str + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        boost::text::
            utf_8_to_32_iterator<char8_t const *, boost::text::null_sentinel_t>
                it(str, str, boost::text::null_sentinel);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char8_t const *>::value, "");
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
        boost::text::utf_8_to_16_iterator<char8_t const *> it1(str, str, str + 3);
        boost::text::utf_8_to_16_iterator<char8_t const *> it2(
            str, str + 3, str + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char8_t const *>::value, "");
        static_assert(
            std::is_same<decltype(unpacked.last), char8_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.last, str + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        boost::text::
            utf_8_to_16_iterator<char8_t const *, boost::text::null_sentinel_t>
                it(str, str, boost::text::null_sentinel);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char8_t const *>::value, "");
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
    char16_t const str[4] = {'f', 'o', 'o', 0};

    // N = 32
    {
        boost::text::utf_16_to_32_iterator<char16_t const *> it1(
            str, str, str + 3);
        boost::text::utf_16_to_32_iterator<char16_t const *> it2(
            str, str + 3, str + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == boost::text::format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char16_t const *>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.last), char16_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.last, str + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        boost::text::utf_16_to_32_iterator<
            char16_t const *,
            boost::text::null_sentinel_t>
            it(str, str, boost::text::null_sentinel);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char16_t const *>::value,
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
        boost::text::utf_16_to_8_iterator<char16_t const *> it1(
            str, str, str + 3);
        boost::text::utf_16_to_8_iterator<char16_t const *> it2(
            str, str + 3, str + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == boost::text::format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char16_t const *>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.last), char16_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.last, str + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        boost::text::
            utf_16_to_8_iterator<char16_t const *, boost::text::null_sentinel_t>
                it(str, str, boost::text::null_sentinel);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf16, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char16_t const *>::value,
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
    char32_t const str[4] = {'f', 'o', 'o', 0};

    // N = 16
    {
        boost::text::utf_32_to_16_iterator<char32_t const *> it1(
            str, str, str + 3);
        boost::text::utf_32_to_16_iterator<char32_t const *> it2(
            str, str + 3, str + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == boost::text::format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char32_t const *>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.last), char32_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.last, str + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        boost::text::utf_32_to_16_iterator<
            char32_t const *,
            boost::text::null_sentinel_t>
            it(str, str, boost::text::null_sentinel);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char32_t const *>::value,
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
        boost::text::utf_32_to_8_iterator<char32_t const *> it1(
            str, str, str + 3);
        boost::text::utf_32_to_8_iterator<char32_t const *> it2(
            str, str + 3, str + 3);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(it1, it2);
        static_assert(unpacked.format_tag == boost::text::format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char32_t const *>::value,
            "");
        static_assert(
            std::is_same<decltype(unpacked.last), char32_t const *>::value, "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.last, str + 3);
        EXPECT_EQ(unpacked.repack(unpacked.first), it1);
    }
    {
        boost::text::
            utf_32_to_8_iterator<char32_t const *, boost::text::null_sentinel_t>
                it(str, str, boost::text::null_sentinel);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf32, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char32_t const *>::value,
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
              iterator_interface<_8_to_32, std::bidirectional_iterator_tag, char32_t>
    {
        _8_to_32() = default;
        _8_to_32(
            char8_t const * first, char8_t const * it, boost::text::null_sentinel_t) :
            it_{first, it, boost::text::null_sentinel}
        {}

        friend bool
        operator==(_8_to_32 first, boost::text::null_sentinel_t last)
        {
            return first.it_ == last;
        }

        auto base() const { return it_.base(); }
        auto begin() const { return it_.begin(); }
        auto end() const { return it_.end(); }

    private:
        friend boost::stl_interfaces::access;
        using iterator_t = boost::text::
            utf_8_to_32_iterator<char8_t const *, boost::text::null_sentinel_t>;
        iterator_t & base_reference() noexcept { return it_; }
        iterator_t base_reference() const noexcept { return it_; }
        iterator_t it_;
    };

    template<typename Repack = boost::text::no_op_repacker>
    auto unpack_iterator_and_sentinel(
        _8_to_32 it, boost::text::null_sentinel_t, Repack repack_ = Repack())
    {
        boost::text::detail::repacker<
            _8_to_32,
            char8_t const *,
            boost::text::null_sentinel_t,
            Repack,
            true>
            repack(it.begin(), it.end(), repack_);
        return boost::text::unpack_result<
            boost::text::format::utf8,
            char8_t const *,
            boost::text::null_sentinel_t,
            decltype(repack)>{it.begin(), boost::text::null_sentinel, repack};
    }

    template<typename I>
    struct template_8_to_32 : boost::stl_interfaces::iterator_interface<
                                  template_8_to_32<I>,
                                  std::bidirectional_iterator_tag,
                                  char32_t>
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

        auto base() const { return it_.base(); }
        auto begin() const { return it_.begin(); }
        auto end() const { return it_.end(); }

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
        template_8_to_32<I> first,
        boost::text::null_sentinel_t last,
        Repack repack = Repack())
    {
        return boost::text::unpack_iterator_and_sentinel(
            first.base(),
            last,
            boost::text::detail::repacker<
                template_8_to_32<I>,
                I,
                boost::text::null_sentinel_t,
                Repack,
                true>(first.begin(), first.end(), repack));
    }

}

TEST(unpack, user_type)
{
    char8_t str[4] = {'f', 'o', 'o', 0};

    {
        my::_8_to_32 it(str, str, boost::text::null_sentinel);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char8_t const *>::value, "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                boost::text::null_sentinel_t>::value,
            "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    {
        my::template_8_to_32<char8_t const *> it(
            str, str, boost::text::null_sentinel);
        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char8_t const *>::value, "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                boost::text::null_sentinel_t>::value,
            "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }
}

TEST(unpack, user_type_interop)
{
    char8_t str[4] = {'f', 'o', 'o', 0};

    {
        my::template_8_to_32<char8_t const *> bottom(
            str, str, boost::text::null_sentinel);

        boost::text::utf_32_to_8_iterator<
            my::template_8_to_32<char8_t const *>,
            boost::text::null_sentinel_t>
            it(bottom, bottom, boost::text::null_sentinel);

        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char8_t const *>::value, "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                boost::text::null_sentinel_t>::value,
            "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }

    {
        my::template_8_to_32<char8_t const *> bottom(
            str, str, boost::text::null_sentinel);
        boost::text::utf_32_to_8_iterator<
            my::template_8_to_32<char8_t const *>,
            boost::text::null_sentinel_t>
            middle(bottom, bottom, boost::text::null_sentinel);

        my::template_8_to_32<boost::text::utf_32_to_8_iterator<
            my::template_8_to_32<char8_t const *>,
            boost::text::null_sentinel_t>>
            it(middle, middle, boost::text::null_sentinel);

        auto unpacked = boost::text::unpack_iterator_and_sentinel(
            it, boost::text::null_sentinel);
        static_assert(unpacked.format_tag == boost::text::format::utf8, "");
        static_assert(
            std::is_same<decltype(unpacked.first), char8_t const *>::value, "");
        static_assert(
            std::is_same<
                decltype(unpacked.last),
                boost::text::null_sentinel_t>::value,
            "");
        EXPECT_EQ(unpacked.first, str);
        EXPECT_EQ(unpacked.repack(unpacked.first), it);
    }
}
