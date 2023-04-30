// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#include <boost/text/string_view.hpp>
#include <boost/text/transcode_iterator.hpp>
#include <boost/text/transcode_view.hpp>

#include <gtest/gtest.h>

#include <forward_list>
#include <iso646.h> // For "not" "token" on MSVC

#include "ill_formed.hpp"


using namespace boost;

template<typename T>
using has_prefix_decrement = decltype(--std::declval<T>());
template<typename T>
using has_postfix_decrement = decltype(std::declval<T>()--);

template<typename T>
struct input_iter
    : stl_interfaces::
          iterator_interface<input_iter<T>, std::input_iterator_tag, T>
{
    input_iter() = default;
    input_iter(T * ptr) : it_{ptr} {}

private:
    friend boost::stl_interfaces::access;
    T *& base_reference() noexcept { return it_; }
    T * base_reference() const noexcept { return it_; }
    T * it_;
};


TEST(transcode_non_bidi, forward)
{
    // Unicode 3.9/D90-D92
    uint32_t const utf32[4] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    uint16_t const utf16[5] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02};
    char8_t const utf8[] = {
        0x4d, 0xd0, 0xb0, 0xe4, 0xba, 0x8c, 0xf0, 0x90, 0x8c, 0x82};

    // UTF-8 -> UTF-32
    {
        std::forward_list<char> list(std::begin(utf8), std::end(utf8));

        auto it = text::utf_8_to_32_iterator(
            list.begin(), list.begin(), list.end());

        auto const end = text::utf_8_to_32_iterator(
            list.begin(), list.end(), list.end());

        EXPECT_EQ(*it, utf32[0]);
        ++it;
        EXPECT_EQ(*it, utf32[1]);
        ++it;
        EXPECT_EQ(*it, utf32[2]);
        ++it;
        EXPECT_EQ(*it, utf32[3]);
        ++it;

        EXPECT_EQ(it, end);

        static_assert(ill_formed<has_prefix_decrement, decltype(it)>::value);
        static_assert(ill_formed<has_postfix_decrement, decltype(it)>::value);
    }
    {
        auto const first = input_iter(std::begin(utf8));
        auto const last = input_iter(std::end(utf8));

        auto it = text::utf_8_to_32_iterator(first, first, last);

        auto const end = text::utf_8_to_32_iterator(first, last, last);

        EXPECT_EQ(*it, utf32[0]);
        ++it;
        EXPECT_EQ(*it, utf32[1]);
        ++it;
        EXPECT_EQ(*it, utf32[2]);
        ++it;
        EXPECT_EQ(*it, utf32[3]);
        ++it;

        EXPECT_EQ(it, end);

        static_assert(ill_formed<has_prefix_decrement, decltype(it)>::value);
        static_assert(ill_formed<has_postfix_decrement, decltype(it)>::value);
    }

    // UTF-32 -> UTF-8
    {
        auto const first = input_iter(std::begin(utf32));
        auto const last = input_iter(std::end(utf32));

        auto it = text::utf_32_to_8_iterator(first, first, last);

        auto const end = text::utf_32_to_8_iterator(first, last, last);

        EXPECT_EQ(*it, utf8[0]);
        it++;
        EXPECT_EQ(*it, utf8[1]);
        it++;
        EXPECT_EQ(*it, utf8[2]);
        it++;
        EXPECT_EQ(*it, utf8[3]);
        it++;
        EXPECT_EQ(*it, utf8[4]);
        it++;
        EXPECT_EQ(*it, utf8[5]);
        it++;
        EXPECT_EQ(*it, utf8[6]);
        it++;
        EXPECT_EQ(*it, utf8[7]);
        it++;
        EXPECT_EQ(*it, utf8[8]);
        it++;
        EXPECT_EQ(*it, utf8[9]);
        it++;

        EXPECT_EQ(it, end);

        static_assert(ill_formed<has_prefix_decrement, decltype(it)>::value);
        static_assert(ill_formed<has_postfix_decrement, decltype(it)>::value);
    }
    {
        auto const first = input_iter(std::begin(utf32));
        auto const last = input_iter(std::end(utf32));

        auto it = text::utf_32_to_8_iterator(first, first, last);

        auto const end = text::utf_32_to_8_iterator(first, last, last);

        EXPECT_EQ(*it, utf8[0]);
        it++;
        EXPECT_EQ(*it, utf8[1]);
        it++;
        EXPECT_EQ(*it, utf8[2]);
        it++;
        EXPECT_EQ(*it, utf8[3]);
        it++;
        EXPECT_EQ(*it, utf8[4]);
        it++;
        EXPECT_EQ(*it, utf8[5]);
        it++;
        EXPECT_EQ(*it, utf8[6]);
        it++;
        EXPECT_EQ(*it, utf8[7]);
        it++;
        EXPECT_EQ(*it, utf8[8]);
        it++;
        EXPECT_EQ(*it, utf8[9]);
        it++;

        EXPECT_EQ(it, end);

        static_assert(ill_formed<has_prefix_decrement, decltype(it)>::value);
        static_assert(ill_formed<has_postfix_decrement, decltype(it)>::value);
    }

    // UTF-8 -> UTF-16
    {
        std::forward_list<char> list(std::begin(utf8), std::end(utf8));

        auto it = text::utf_8_to_16_iterator(
            list.begin(), list.begin(), list.end());

        auto const end = text::utf_8_to_16_iterator(
            list.begin(), list.end(), list.end());

        EXPECT_EQ(*it, utf16[0]);
        ++it;
        EXPECT_EQ(*it, utf16[1]);
        ++it;
        EXPECT_EQ(*it, utf16[2]);
        ++it;
        EXPECT_EQ(*it, utf16[3]);
        ++it;
        EXPECT_EQ(*it, utf16[4]);
        ++it;

        EXPECT_EQ(it, end);

        static_assert(ill_formed<has_prefix_decrement, decltype(it)>::value);
        static_assert(ill_formed<has_postfix_decrement, decltype(it)>::value);
    }
    {
        auto const first = input_iter(std::begin(utf8));
        auto const last = input_iter(std::end(utf8));

        auto it = text::utf_8_to_16_iterator(first, first, last);

        auto const end = text::utf_8_to_16_iterator(first, last, last);

        EXPECT_EQ(*it, utf16[0]);
        ++it;
        EXPECT_EQ(*it, utf16[1]);
        ++it;
        EXPECT_EQ(*it, utf16[2]);
        ++it;
        EXPECT_EQ(*it, utf16[3]);
        ++it;
        EXPECT_EQ(*it, utf16[4]);
        ++it;

        EXPECT_EQ(it, end);

        static_assert(ill_formed<has_prefix_decrement, decltype(it)>::value);
        static_assert(ill_formed<has_postfix_decrement, decltype(it)>::value);
    }

    // UTF-16 -> UTF-8
    {
        std::forward_list<uint16_t> list(std::begin(utf16), std::end(utf16));

        auto it =
            text::utf_16_to_8_iterator(list.begin(), list.begin(), list.end());

        auto const end =
            text::utf_16_to_8_iterator(list.begin(), list.end(), list.end());

        EXPECT_EQ(*it, utf8[0]);
        ++it;
        EXPECT_EQ(*it, utf8[1]);
        ++it;
        EXPECT_EQ(*it, utf8[2]);
        ++it;
        EXPECT_EQ(*it, utf8[3]);
        ++it;
        EXPECT_EQ(*it, utf8[4]);
        ++it;
        EXPECT_EQ(*it, utf8[5]);
        ++it;
        EXPECT_EQ(*it, utf8[6]);
        ++it;
        EXPECT_EQ(*it, utf8[7]);
        ++it;
        EXPECT_EQ(*it, utf8[8]);
        ++it;
        EXPECT_EQ(*it, utf8[9]);
        ++it;

        EXPECT_EQ(it, end);

        static_assert(ill_formed<has_prefix_decrement, decltype(it)>::value);
        static_assert(ill_formed<has_postfix_decrement, decltype(it)>::value);
    }
#if 0 // TODO
    {
        auto const first = input_iter(std::begin(utf16));
        auto const last = input_iter(std::end(utf16));

        auto it = text::utf_16_to_8_iterator(first, first, last);

        auto const end = text::utf_16_to_8_iterator(first, last, last);

        EXPECT_EQ(*it, utf16[0]);
        ++it;
        EXPECT_EQ(*it, utf16[1]);
        ++it;
        EXPECT_EQ(*it, utf16[2]);
        ++it;
        EXPECT_EQ(*it, utf16[3]);
        ++it;
        EXPECT_EQ(*it, utf16[4]);
        ++it;
        EXPECT_EQ(*it, utf16[5]);
        ++it;
        EXPECT_EQ(*it, utf16[6]);
        ++it;
        EXPECT_EQ(*it, utf16[7]);
        ++it;
        EXPECT_EQ(*it, utf16[16]);
        ++it;
        EXPECT_EQ(*it, utf16[9]);
        ++it;

        EXPECT_EQ(it, end);

        static_assert(ill_formed<has_prefix_decrement, decltype(it)>::value);
        static_assert(ill_formed<has_postfix_decrement, decltype(it)>::value);
    }
#endif

    // UTF-16 -> UTF-32
    {
        std::forward_list<uint16_t> list(std::begin(utf16), std::end(utf16));

        auto it = text::utf_16_to_32_iterator(
            list.begin(), list.begin(), list.end());

        auto const end = text::utf_16_to_32_iterator(
            list.begin(), list.end(), list.end());

        EXPECT_EQ(*it, utf32[0]);
        ++it;
        EXPECT_EQ(*it, utf32[1]);
        ++it;
        EXPECT_EQ(*it, utf32[2]);
        ++it;
        EXPECT_EQ(*it, utf32[3]);
        ++it;

        EXPECT_EQ(it, end);

        static_assert(ill_formed<has_prefix_decrement, decltype(it)>::value);
        static_assert(ill_formed<has_postfix_decrement, decltype(it)>::value);
    }
    {
        auto const first = input_iter(std::begin(utf16));
        auto const last = input_iter(std::end(utf16));

        auto it = text::utf_16_to_32_iterator(first, first, last);

        auto const end = text::utf_16_to_32_iterator(first, last, last);

        EXPECT_EQ(*it, utf32[0]);
        ++it;
        EXPECT_EQ(*it, utf32[1]);
        ++it;
        EXPECT_EQ(*it, utf32[2]);
        ++it;
        EXPECT_EQ(*it, utf32[3]);
        ++it;

        EXPECT_EQ(it, end);

        static_assert(ill_formed<has_prefix_decrement, decltype(it)>::value);
        static_assert(ill_formed<has_postfix_decrement, decltype(it)>::value);
    }

    // UTF-32 -> UTF-16
    {
        std::forward_list<uint32_t> list(std::begin(utf32), std::end(utf32));

        auto it = text::utf_32_to_16_iterator(
            list.begin(), list.begin(), list.end());

        auto const end = text::utf_32_to_16_iterator(
            list.begin(), list.end(), list.end());

        EXPECT_EQ(*it, utf16[0]);
        it++;
        EXPECT_EQ(*it, utf16[1]);
        it++;
        EXPECT_EQ(*it, utf16[2]);
        it++;
        EXPECT_EQ(*it, utf16[3]);
        it++;
        EXPECT_EQ(*it, utf16[4]);
        it++;

        EXPECT_EQ(it, end);

        static_assert(ill_formed<has_prefix_decrement, decltype(it)>::value);
        static_assert(ill_formed<has_postfix_decrement, decltype(it)>::value);
    }
    {
        auto const first = input_iter(std::begin(utf32));
        auto const last = input_iter(std::end(utf32));

        auto it = text::utf_32_to_16_iterator(first, first, last);

        auto const end = text::utf_32_to_16_iterator(first, last, last);

        EXPECT_EQ(*it, utf16[0]);
        it++;
        EXPECT_EQ(*it, utf16[1]);
        it++;
        EXPECT_EQ(*it, utf16[2]);
        it++;
        EXPECT_EQ(*it, utf16[3]);
        it++;
        EXPECT_EQ(*it, utf16[4]);
        it++;

        EXPECT_EQ(it, end);

        static_assert(ill_formed<has_prefix_decrement, decltype(it)>::value);
        static_assert(ill_formed<has_postfix_decrement, decltype(it)>::value);
    }
}
