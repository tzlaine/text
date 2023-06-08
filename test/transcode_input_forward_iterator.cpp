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
#include <iterator>
#include <sstream>
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


TEST(transcode_non_bidi, forward_and_input)
{
    // Unicode 3.9/D90-D92
    char32_t const utf32[4] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    char16_t const utf16[5] = {0x004d, 0x0430, 0x4e8c, 0xd800, 0xdf02};
    char8_t const utf8[] = {
        0x4d, 0xd0, 0xb0, 0xe4, 0xba, 0x8c, 0xf0, 0x90, 0x8c, 0x82};

    // UTF-8 -> UTF-32
    {
        std::forward_list<char8_t> list(std::begin(utf8), std::end(utf8));

        auto it = text::utf_8_to_32_iterator(list.begin(), list.end());

        auto const end = text::utf_8_to_32_iterator(list.end(), list.end());

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

        auto it = text::utf_8_to_32_iterator(first, last);

        auto const end = text::utf_8_to_32_iterator(last, last);

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
#if BOOST_TEXT_CODE_UNIT_CONCEPT_OPTION_2
    {
        std::string str(std::begin(utf8), std::end(utf8));
        std::istringstream iss(str);
        std::istreambuf_iterator<char> first(iss);
        std::istreambuf_iterator<char> last;

        auto it = text::utf_8_to_32_iterator(first, last);

        auto const end = text::utf_8_to_32_iterator(last, last);

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
#endif

    // UTF-32 -> UTF-8
    {
        auto const first = input_iter(std::begin(utf32));
        auto const last = input_iter(std::end(utf32));

        auto it = text::utf_32_to_8_iterator(first, last);

        auto const end = text::utf_32_to_8_iterator(last, last);

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

        auto it = text::utf_32_to_8_iterator(first, last);

        auto const end = text::utf_32_to_8_iterator(last, last);

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
        std::forward_list<char8_t> list(std::begin(utf8), std::end(utf8));

        auto it = text::utf_8_to_16_iterator(list.begin(), list.end());

        auto const end = text::utf_8_to_16_iterator(list.end(), list.end());

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

        auto it = text::utf_8_to_16_iterator(first, last);

        auto const end = text::utf_8_to_16_iterator(last, last);

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
        std::forward_list<char16_t> list(std::begin(utf16), std::end(utf16));

        auto it = text::utf_16_to_8_iterator(list.begin(), list.end());

        auto const end = text::utf_16_to_8_iterator(list.end(), list.end());

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
    {
        auto const first = input_iter(std::begin(utf16));
        auto const last = input_iter(std::end(utf16));

        auto it = text::utf_16_to_8_iterator(first, last);

        auto const end = text::utf_16_to_8_iterator(last, last);

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

    // UTF-16 -> UTF-32
    {
        std::forward_list<char16_t> list(std::begin(utf16), std::end(utf16));

        auto it = text::utf_16_to_32_iterator(list.begin(), list.end());

        auto const end = text::utf_16_to_32_iterator(list.end(), list.end());

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

        auto it = text::utf_16_to_32_iterator(first, last);

        auto const end = text::utf_16_to_32_iterator(last, last);

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
        std::forward_list<char32_t> list(std::begin(utf32), std::end(utf32));

        auto it = text::utf_32_to_16_iterator(list.begin(), list.end());

        auto const end = text::utf_32_to_16_iterator(list.end(), list.end());

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

        auto it = text::utf_32_to_16_iterator(first, last);

        auto const end = text::utf_32_to_16_iterator(last, last);

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

TEST(transcode_non_bidi, range_adaptors)
{
    // Unicode 3.9/D90-D92
    char32_t const utf32[4] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    char8_t const utf8[] = {
        0x4d, 0xd0, 0xb0, 0xe4, 0xba, 0x8c, 0xf0, 0x90, 0x8c, 0x82};

    {
        std::forward_list<char8_t> list(std::begin(utf8), std::end(utf8));

        auto r = list | text::as_utf32;
        auto it = r.begin();
        auto const end = r.end();

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

        auto r = std::ranges::subrange(first, last) | text::as_utf32;
        auto it = r.begin();
        auto const end = r.end();

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
}

TEST(transcode_non_bidi, chained_range_adaptors)
{
    // Unicode 3.9/D90-D92
    char32_t const utf32[4] = {0x004d, 0x0430, 0x4e8c, 0x10302};
    char8_t const utf8[] = {
        0x4d, 0xd0, 0xb0, 0xe4, 0xba, 0x8c, 0xf0, 0x90, 0x8c, 0x82};

    {
        std::forward_list<char8_t> list(std::begin(utf8), std::end(utf8));

        auto simple = list | text::as_utf32;

        auto r = list | text::as_utf32 | text::as_utf16 | text::as_utf32;
        auto it = r.begin();
        auto end = r.end();

        static_assert(std::same_as<decltype(it), decltype(simple.begin())>);
        static_assert(std::same_as<decltype(end), decltype(simple.end())>);

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

        auto simple = std::ranges::subrange(first, last) | text::as_utf32;

        // Like simple, but this is a subrange, rather than a utf_view.
        auto almost_simple =
            std::ranges::subrange(simple.begin(), simple.end());

        auto r = almost_simple | text::as_utf16 | text::as_utf32;
        auto it = r.begin();
        auto end = r.end();

        // These types should not match for input ranges -- no unpacking.
        static_assert(!std::same_as<decltype(it), decltype(simple.begin())>);
        static_assert(!std::same_as<decltype(end), decltype(simple.end())>);

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

        auto simple = std::ranges::subrange(first, last) | text::as_utf32;

        auto r = std::ranges::subrange(first, last) | text::as_utf32 |
                 text::as_utf16 | text::as_utf32;
        auto it = r.begin();
        auto end = r.end();

        // These types *should*, even for input ranges -- due to the use of
        // utf_view::base() in view adaptors called on utf_views.
        static_assert(std::same_as<decltype(it), decltype(simple.begin())>);
        static_assert(std::same_as<decltype(end), decltype(simple.end())>);

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
}
