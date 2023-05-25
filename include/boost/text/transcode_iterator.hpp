// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_TRANSCODE_ITERATOR_HPP
#define BOOST_TEXT_TRANSCODE_ITERATOR_HPP

#include <boost/text/transcode_iterator_fwd.hpp>
#include <boost/text/config.hpp>
#include <boost/text/concepts.hpp>
#include <boost/text/utf.hpp>
#include <boost/text/detail/algorithm.hpp>

#include <boost/assert.hpp>
#include <boost/throw_exception.hpp>
#include <boost/stl_interfaces/iterator_interface.hpp>

#include <array>
#include <iterator>
#include <type_traits>
#include <stdexcept>


namespace boost { namespace text {

    namespace {
        constexpr char16_t high_surrogate_base = 0xd7c0;
        constexpr char16_t low_surrogate_base = 0xdc00;
        constexpr char32_t high_surrogate_min = 0xd800;
        constexpr char32_t high_surrogate_max = 0xdbff;
        constexpr char32_t low_surrogate_min = 0xdc00;
        constexpr char32_t low_surrogate_max = 0xdfff;
    }

    namespace detail {
        constexpr bool in(unsigned char lo, unsigned char c, unsigned char hi)
        {
            return lo <= c && c <= hi;
        }

        struct throw_on_encoding_error
        {};

        template<typename OutIter>
        inline constexpr OutIter read_into_buf(char32_t cp, OutIter buf)
        {
            if (cp < 0x80) {
                *buf = static_cast<char>(cp);
                ++buf;
            } else if (cp < 0x800) {
                *buf = static_cast<char>(0xC0 + (cp >> 6));
                ++buf;
                *buf = static_cast<char>(0x80 + (cp & 0x3f));
                ++buf;
            } else if (cp < 0x10000) {
                *buf = static_cast<char>(0xe0 + (cp >> 12));
                ++buf;
                *buf = static_cast<char>(0x80 + ((cp >> 6) & 0x3f));
                ++buf;
                *buf = static_cast<char>(0x80 + (cp & 0x3f));
                ++buf;
            } else {
                *buf = static_cast<char>(0xf0 + (cp >> 18));
                ++buf;
                *buf = static_cast<char>(0x80 + ((cp >> 12) & 0x3f));
                ++buf;
                *buf = static_cast<char>(0x80 + ((cp >> 6) & 0x3f));
                ++buf;
                *buf = static_cast<char>(0x80 + (cp & 0x3f));
                ++buf;
            }
            return buf;
        }

        template<typename OutIter>
        constexpr OutIter write_cp_utf8(char32_t cp, OutIter out)
        {
            return detail::read_into_buf(cp, out);
        }

        template<typename OutIter>
        constexpr OutIter write_cp_utf16(char32_t cp, OutIter out)
        {
            if (cp < 0x10000) {
                *out = static_cast<char16_t>(cp);
                ++out;
            } else {
                *out = static_cast<char16_t>(cp >> 10) + high_surrogate_base;
                ++out;
                *out = static_cast<char16_t>(cp & 0x3ff) + low_surrogate_base;
                ++out;
            }
            return out;
        }

        inline constexpr char32_t surrogates_to_cp(char16_t hi, char16_t lo)
        {
            return char32_t((hi - high_surrogate_base) << 10) +
                   (lo - low_surrogate_base);
        }

        template<typename T, typename U>
        using enable_utf8_cp = std::enable_if<is_char_iter_v<T>, U>;
        template<typename T, typename U = T>
        using enable_utf8_cp_t = typename enable_utf8_cp<T, U>::type;

        template<typename T, typename U>
        using enable_utf16_cp = std::enable_if<is_16_iter_v<T>, U>;
        template<typename T, typename U = T>
        using enable_utf16_cp_t = typename enable_utf16_cp<T, U>::type;

        template<typename I>
        auto bidirectional_at_most()
        {
            if constexpr (std::bidirectional_iterator<I>) {
                return std::bidirectional_iterator_tag{};
            } else if constexpr (std::forward_iterator<I>) {
                return std::forward_iterator_tag{};
            } else if constexpr (std::input_iterator<I>) {
                return std::input_iterator_tag{};
            }
        }
    }

    /** The replacement character used to mark invalid portions of a Unicode
        sequence when converting between two encodings.

        \see Unicode 3.2/C10 */
    constexpr char32_t replacement_character() { return 0xfffd; }

    /** Returns true iff `c` is a Unicode surrogate. */
    inline constexpr bool surrogate(char32_t c)
    {
        return high_surrogate_min <= c && c <= low_surrogate_max;
    }

    /** Returns true iff `c` is a Unicode high surrogate. */
    inline constexpr bool high_surrogate(char32_t c)
    {
        return high_surrogate_min <= c && c <= high_surrogate_max;
    }

    /** Returns true iff `c` is a Unicode low surrogate. */
    inline constexpr bool low_surrogate(char32_t c)
    {
        return low_surrogate_min <= c && c <= low_surrogate_max;
    }

    /** Returns true iff `c` is a Unicode reserved noncharacter.

        \see Unicode 3.4/D14 */
    inline constexpr bool reserved_noncharacter(char32_t c)
    {
        bool const byte01_reserved = (c & 0xffff) >= 0xfffe;
        bool const byte2_at_most_0x10 = ((c & 0xff0000u) >> 16) <= 0x10;
        return (byte01_reserved && byte2_at_most_0x10) ||
               (0xfdd0 <= c && c <= 0xfdef);
    }

    /** Returns true iff `c` is a valid Unicode scalar value.

        \see Unicode 3.9/D90 */
    inline constexpr bool scalar_value(char32_t c)
    {
        return c <= 0x10ffff && !surrogate(c);
    }

    /** Returns true iff `c` is a Unicode scalar value not in the reserved
        range.

        \see Unicode 3.9/D90 */
    inline constexpr bool unreserved_scalar_value(char32_t c)
    {
        return scalar_value(c) && !reserved_noncharacter(c);
    }

    /** Returns true iff `c` is a UTF-8 lead code unit (which must be followed
        by 1-3 following units). */
    constexpr bool lead_code_unit(char8_t c)
    {
        return uint8_t((unsigned char)c - 0xc2) <= 0x32;
    }

    /** Returns true iff `c` is a UTF-8 continuation code unit. */
    constexpr bool continuation(char8_t c) { return (int8_t)c < -0x40; }

    /** Given the first (and possibly only) code unit of a UTF-8-encoded code
        point, returns the number of bytes occupied by that code point (in the
        range `[1, 4]`).  Returns a value < 0 if `first_unit` is not a valid
        initial UTF-8 code unit. */
    inline constexpr int utf8_code_units(char8_t first_unit_)
    {
        auto first_unit = (unsigned int)first_unit_;
        return first_unit <= 0x7f ? 1
               : boost::text::lead_code_unit(first_unit)
                   ? int(0xe0 <= first_unit) + int(0xf0 <= first_unit) + 2
                   : -1;
    }

    /** Given the first (and possibly only) code unit of a UTF-16-encoded code
        point, returns the number of code units occupied by that code point
        (in the range `[1, 2]`).  Returns a negative value if `first_unit` is
        not a valid initial UTF-16 code unit. */
    inline constexpr int utf16_code_units(char16_t first_unit)
    {
        if (boost::text::low_surrogate(first_unit))
            return -1;
        if (boost::text::high_surrogate(first_unit))
            return 2;
        return 1;
    }

    namespace detail {
        // optional is not constexpr friendly.
        template<typename Iter>
        struct optional_iter
        {
            constexpr optional_iter() : it_(), valid_(false) {}
            constexpr optional_iter(Iter it) : it_(it), valid_(true) {}

            constexpr operator bool() const { return valid_; }
            constexpr Iter operator*() const
            {
                BOOST_ASSERT(valid_);
                return it_;
            }
            Iter & operator*()
            {
                BOOST_ASSERT(valid_);
                return it_;
            }

            friend constexpr bool
            operator==(optional_iter lhs, optional_iter rhs)
            {
                return lhs.valid_ == rhs.valid_ &&
                       (!lhs.valid_ || lhs.it_ == rhs.it_);
            }
            friend constexpr bool
            operator!=(optional_iter lhs, optional_iter rhs)
            {
                return !(lhs == rhs);
            }

        private:
            Iter it_;
            bool valid_;
        };

        // Follow Table 3-7 in Unicode 3.9/D92
        template<typename Iter>
        constexpr optional_iter<Iter> end_of_invalid_utf8(Iter it)
        {
            BOOST_ASSERT(!boost::text::continuation(*it));

            if (detail::in(0, *it, 0x7f))
                return optional_iter<Iter>{};

            if (detail::in(0xc2, *it, 0xdf)) {
                auto next = it;
                if (!boost::text::continuation(*++next))
                    return next;
                return optional_iter<Iter>{};
            }

            if (detail::in(0xe0, *it, 0xe0)) {
                auto next = it;
                if (!detail::in(0xa0, *++next, 0xbf))
                    return next;
                if (!boost::text::continuation(*++next))
                    return next;
                return optional_iter<Iter>{};
            }
            if (detail::in(0xe1, *it, 0xec)) {
                auto next = it;
                if (!boost::text::continuation(*++next))
                    return next;
                if (!boost::text::continuation(*++next))
                    return next;
                return optional_iter<Iter>{};
            }
            if (detail::in(0xed, *it, 0xed)) {
                auto next = it;
                if (!detail::in(0x80, *++next, 0x9f))
                    return next;
                if (!boost::text::continuation(*++next))
                    return next;
                return optional_iter<Iter>{};
            }
            if (detail::in(0xee, *it, 0xef)) {
                auto next = it;
                if (!boost::text::continuation(*++next))
                    return next;
                if (!boost::text::continuation(*++next))
                    return next;
                return optional_iter<Iter>{};
            }

            if (detail::in(0xf0, *it, 0xf0)) {
                auto next = it;
                if (!detail::in(0x90, *++next, 0xbf))
                    return next;
                if (!boost::text::continuation(*++next))
                    return next;
                if (!boost::text::continuation(*++next))
                    return next;
                return optional_iter<Iter>{};
            }
            if (detail::in(0xf1, *it, 0xf3)) {
                auto next = it;
                if (!boost::text::continuation(*++next))
                    return next;
                if (!boost::text::continuation(*++next))
                    return next;
                if (!boost::text::continuation(*++next))
                    return next;
                return optional_iter<Iter>{};
            }
            if (detail::in(0xf4, *it, 0xf4)) {
                auto next = it;
                if (!detail::in(0x80, *++next, 0x8f))
                    return next;
                if (!boost::text::continuation(*++next))
                    return next;
                if (!boost::text::continuation(*++next))
                    return next;
                return optional_iter<Iter>{};
            }

            return it;
        }

        template<typename Iter>
        constexpr Iter decrement(Iter it)
        {
            Iter retval = it;

            int backup = 0;
            while (backup < 4 && boost::text::continuation(*--retval)) {
                ++backup;
            }
            backup = it - retval;

            if (boost::text::continuation(*retval))
                return it - 1;

            optional_iter<Iter> first_invalid = end_of_invalid_utf8(retval);
            if (first_invalid == retval)
                ++*first_invalid;
            while (first_invalid && (*first_invalid - retval) < backup) {
                backup -= *first_invalid - retval;
                retval = *first_invalid;
                first_invalid = end_of_invalid_utf8(retval);
                if (first_invalid == retval)
                    ++*first_invalid;
            }

            if (1 < backup) {
                int const cp_bytes = boost::text::utf8_code_units(*retval);
                if (cp_bytes < backup)
                    retval = it - 1;
            }

            return retval;
        }

        template<typename Iter>
        constexpr Iter decrement(Iter first, Iter it)
        {
            Iter retval = it;

            int backup = 0;
            while (backup < 4 && retval != first &&
                   boost::text::continuation(*--retval)) {
                ++backup;
            }
            backup = std::distance(retval, it);

            if (boost::text::continuation(*retval)) {
                if (it != first)
                    --it;
                return it;
            }

            optional_iter<Iter> first_invalid = end_of_invalid_utf8(retval);
            if (first_invalid == retval)
                ++*first_invalid;
            while (first_invalid &&
                   std::distance(retval, *first_invalid) < backup) {
                backup -= std::distance(retval, *first_invalid);
                retval = *first_invalid;
                first_invalid = end_of_invalid_utf8(retval);
                if (first_invalid == retval)
                    ++*first_invalid;
            }

            if (1 < backup) {
                int const cp_bytes = boost::text::utf8_code_units(*retval);
                if (cp_bytes < backup) {
                    if (it != first)
                        --it;
                    retval = it;
                }
            }

            return retval;
        }

        enum char_class : uint8_t {
            ill = 0,
            asc = 1,
            cr1 = 2,
            cr2 = 3,
            cr3 = 4,
            l2a = 5,
            l3a = 6,
            l3b = 7,
            l3c = 8,
            l4a = 9,
            l4b = 10,
            l4c = 11,
        };

        enum table_state : uint8_t {
            bgn = 0,
            e_d = bgn, // "end"
            err = 12,
            cs1 = 24,
            cs2 = 36,
            cs3 = 48,
            p3a = 60,
            p3b = 72,
            p4a = 84,
            p4b = 96,
            invalid_table_state = 200
        };

        struct first_cu
        {
            unsigned char initial_octet;
            table_state next;
        };

        namespace {
            constexpr first_cu first_cus[256] = {
                {0x00, bgn}, {0x01, bgn}, {0x02, bgn}, {0x03, bgn}, {0x04, bgn},
                {0x05, bgn}, {0x06, bgn}, {0x07, bgn}, {0x08, bgn}, {0x09, bgn},
                {0x0a, bgn}, {0x0b, bgn}, {0x0c, bgn}, {0x0d, bgn}, {0x0e, bgn},
                {0x0f, bgn}, {0x10, bgn}, {0x11, bgn}, {0x12, bgn}, {0x13, bgn},
                {0x14, bgn}, {0x15, bgn}, {0x16, bgn}, {0x17, bgn}, {0x18, bgn},
                {0x19, bgn}, {0x1a, bgn}, {0x1b, bgn}, {0x1c, bgn}, {0x1d, bgn},
                {0x1e, bgn}, {0x1f, bgn}, {0x20, bgn}, {0x21, bgn}, {0x22, bgn},
                {0x23, bgn}, {0x24, bgn}, {0x25, bgn}, {0x26, bgn}, {0x27, bgn},
                {0x28, bgn}, {0x29, bgn}, {0x2a, bgn}, {0x2b, bgn}, {0x2c, bgn},
                {0x2d, bgn}, {0x2e, bgn}, {0x2f, bgn}, {0x30, bgn}, {0x31, bgn},
                {0x32, bgn}, {0x33, bgn}, {0x34, bgn}, {0x35, bgn}, {0x36, bgn},
                {0x37, bgn}, {0x38, bgn}, {0x39, bgn}, {0x3a, bgn}, {0x3b, bgn},
                {0x3c, bgn}, {0x3d, bgn}, {0x3e, bgn}, {0x3f, bgn}, {0x40, bgn},
                {0x41, bgn}, {0x42, bgn}, {0x43, bgn}, {0x44, bgn}, {0x45, bgn},
                {0x46, bgn}, {0x47, bgn}, {0x48, bgn}, {0x49, bgn}, {0x4a, bgn},
                {0x4b, bgn}, {0x4c, bgn}, {0x4d, bgn}, {0x4e, bgn}, {0x4f, bgn},
                {0x50, bgn}, {0x51, bgn}, {0x52, bgn}, {0x53, bgn}, {0x54, bgn},
                {0x55, bgn}, {0x56, bgn}, {0x57, bgn}, {0x58, bgn}, {0x59, bgn},
                {0x5a, bgn}, {0x5b, bgn}, {0x5c, bgn}, {0x5d, bgn}, {0x5e, bgn},
                {0x5f, bgn}, {0x60, bgn}, {0x61, bgn}, {0x62, bgn}, {0x63, bgn},
                {0x64, bgn}, {0x65, bgn}, {0x66, bgn}, {0x67, bgn}, {0x68, bgn},
                {0x69, bgn}, {0x6a, bgn}, {0x6b, bgn}, {0x6c, bgn}, {0x6d, bgn},
                {0x6e, bgn}, {0x6f, bgn}, {0x70, bgn}, {0x71, bgn}, {0x72, bgn},
                {0x73, bgn}, {0x74, bgn}, {0x75, bgn}, {0x76, bgn}, {0x77, bgn},
                {0x78, bgn}, {0x79, bgn}, {0x7a, bgn}, {0x7b, bgn}, {0x7c, bgn},
                {0x7d, bgn}, {0x7e, bgn}, {0x7f, bgn}, {0x00, err}, {0x01, err},
                {0x02, err}, {0x03, err}, {0x04, err}, {0x05, err}, {0x06, err},
                {0x07, err}, {0x08, err}, {0x09, err}, {0x0a, err}, {0x0b, err},
                {0x0c, err}, {0x0d, err}, {0x0e, err}, {0x0f, err}, {0x10, err},
                {0x11, err}, {0x12, err}, {0x13, err}, {0x14, err}, {0x15, err},
                {0x16, err}, {0x17, err}, {0x18, err}, {0x19, err}, {0x1a, err},
                {0x1b, err}, {0x1c, err}, {0x1d, err}, {0x1e, err}, {0x1f, err},
                {0x20, err}, {0x21, err}, {0x22, err}, {0x23, err}, {0x24, err},
                {0x25, err}, {0x26, err}, {0x27, err}, {0x28, err}, {0x29, err},
                {0x2a, err}, {0x2b, err}, {0x2c, err}, {0x2d, err}, {0x2e, err},
                {0x2f, err}, {0x30, err}, {0x31, err}, {0x32, err}, {0x33, err},
                {0x34, err}, {0x35, err}, {0x36, err}, {0x37, err}, {0x38, err},
                {0x39, err}, {0x3a, err}, {0x3b, err}, {0x3c, err}, {0x3d, err},
                {0x3e, err}, {0x3f, err}, {0xc0, err}, {0xc1, err}, {0x02, cs1},
                {0x03, cs1}, {0x04, cs1}, {0x05, cs1}, {0x06, cs1}, {0x07, cs1},
                {0x08, cs1}, {0x09, cs1}, {0x0a, cs1}, {0x0b, cs1}, {0x0c, cs1},
                {0x0d, cs1}, {0x0e, cs1}, {0x0f, cs1}, {0x10, cs1}, {0x11, cs1},
                {0x12, cs1}, {0x13, cs1}, {0x14, cs1}, {0x15, cs1}, {0x16, cs1},
                {0x17, cs1}, {0x18, cs1}, {0x19, cs1}, {0x1a, cs1}, {0x1b, cs1},
                {0x1c, cs1}, {0x1d, cs1}, {0x1e, cs1}, {0x1f, cs1}, {0x00, p3a},
                {0x01, cs2}, {0x02, cs2}, {0x03, cs2}, {0x04, cs2}, {0x05, cs2},
                {0x06, cs2}, {0x07, cs2}, {0x08, cs2}, {0x09, cs2}, {0x0a, cs2},
                {0x0b, cs2}, {0x0c, cs2}, {0x0d, p3b}, {0x0e, cs2}, {0x0f, cs2},
                {0x00, p4a}, {0x01, cs3}, {0x02, cs3}, {0x03, cs3}, {0x04, p4b},
                {0xf5, err}, {0xf6, err}, {0xf7, err}, {0xf8, err}, {0xf9, err},
                {0xfa, err}, {0xfb, err}, {0xfc, err}, {0xfd, err}, {0xfe, err},
                {0xff, err},
            };

            constexpr char_class octet_classes[256] = {
                asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
                asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
                asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
                asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
                asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
                asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
                asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
                asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
                asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc,
                asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, asc, cr1, cr1,
                cr1, cr1, cr1, cr1, cr1, cr1, cr1, cr1, cr1, cr1, cr1, cr1, cr1,
                cr1, cr2, cr2, cr2, cr2, cr2, cr2, cr2, cr2, cr2, cr2, cr2, cr2,
                cr2, cr2, cr2, cr2, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3,
                cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3,
                cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, cr3, ill, ill, l2a,
                l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a,
                l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a, l2a,
                l2a, l2a, l2a, l3a, l3b, l3b, l3b, l3b, l3b, l3b, l3b, l3b, l3b,
                l3b, l3b, l3b, l3c, l3b, l3b, l4a, l4b, l4b, l4b, l4c, ill, ill,
                ill, ill, ill, ill, ill, ill, ill, ill, ill,
            };

            constexpr table_state transitions[108] = {
                err, e_d, err, err, err, cs1, p3a, cs2, p3b, p4a, cs3, p4b,
                err, err, err, err, err, err, err, err, err, err, err, err,
                err, err, e_d, e_d, e_d, err, err, err, err, err, err, err,
                err, err, cs1, cs1, cs1, err, err, err, err, err, err, err,
                err, err, cs2, cs2, cs2, err, err, err, err, err, err, err,
                err, err, err, err, cs1, err, err, err, err, err, err, err,
                err, err, cs1, cs1, err, err, err, err, err, err, err, err,
                err, err, err, cs2, cs2, err, err, err, err, err, err, err,
                err, err, cs2, err, err, err, err, err, err, err, err, err,
            };
        }

        template<typename InputIter, typename Sentinel>
        char32_t advance(InputIter & first, Sentinel last)
        {
            char32_t retval = 0;

            first_cu const info = first_cus[(unsigned char)*first];
            ++first;

            retval = info.initial_octet;
            int state = info.next;

            while (state != bgn) {
                if (first != last) {
                    unsigned char const cu = *first;
                    retval = (retval << 6) | (cu & 0x3f);
                    char_class const class_ = octet_classes[cu];
                    state = transitions[state + class_];
                    if (state == err)
                        return replacement_character();
                    ++first;
                } else {
                    return replacement_character();
                }
            }

            return retval;
        }

        template<typename Derived, typename Iter>
        struct trans_ins_iter
        {
            using value_type = void;
            using difference_type =
#if BOOST_TEXT_USE_CONCEPTS
                std::ptrdiff_t;
#else
                void;
#endif
            using pointer = void;
            using reference = void;
            using iterator_category = std::output_iterator_tag;

            constexpr trans_ins_iter() {}
            constexpr trans_ins_iter(Iter it) : it_(it) {}
            constexpr Derived & operator*() { return derived(); }
            constexpr Derived & operator++() { return derived(); }
            constexpr Derived operator++(int) { return derived(); }
            constexpr Iter base() const { return it_; }

        protected:
            constexpr Iter & iter() { return it_; }

        private:
            constexpr Derived & derived()
            {
                return static_cast<Derived &>(*this);
            }
            Iter it_;
        };

        template<typename Derived, typename I, typename ValueType>
        using trans_iter = stl_interfaces::iterator_interface<
            Derived,
            decltype(detail::bidirectional_at_most<I>()),
            ValueType,
            ValueType>;
    }

}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

#if defined(BOOST_TEXT_DOXYGEN)

    /** Returns the first code unit in `[r.begin(), r.end())` that is not
        properly UTF-8 encoded, or `r.begin() + std::distance(r)` if no such
        code unit is found. */
    template<utf8_range R>
        requires std::ranges::forward_range<R>
    constexpr std::ranges::borrowed_iterator_t<R> find_invalid_encoding(R && r);

    /** Returns the first code unit in `[r.begin(), r.end())` that is not
        properly UTF-16 encoded, or `r.begin() + std::distance(r)` if no such
        code unit is found. */
    template<utf16_range R>
        requires std::ranges::forward_range<R>
    constexpr std::ranges::borrowed_iterator_t<R> find_invalid_encoding(R && r);

    /** Returns true iff `r` is properly UTF-8 encoded. */
    template<utf8_range R>
        requires std::ranges::forward_range<R>
    constexpr bool encoded(R && r);

    /** Returns true iff `r` is properly UTF-16 encoded */
    template<utf16_range R>
        requires std::ranges::forward_range<R>
    constexpr bool encoded(R && r);

    /** Returns true iff `r` is empty or the initial UTF-8 code units in `r`
        form a valid Unicode code point. */
    template<utf8_range R>
        requires std::ranges::forward_range<R>
    constexpr bool starts_encoded(R && r);

    /** Returns true iff `r` is empty or the initial UTF-16 code units in `r`
        form a valid Unicode code point. */
    template<utf16_range R>
        requires std::ranges::forward_range<R>
    constexpr bool starts_encoded(R && r);

    /** Returns true iff `r` is empty or the final UTF-8 code units in `r`
        form a valid Unicode code point. */
    template<utf8_range R>
        requires std::ranges::bidirectional_range<R> &&
                 std::ranges::common_range<R>
    constexpr bool ends_encoded(R && r);

    /** Returns true iff `r` is empty or the final UTF-16 code units in `r`
        form a valid Unicode code point. */
    template<utf8_range R>
        requires std::ranges::bidirectional_range<R> &&
                 std::ranges::common_range<R>
    constexpr bool ends_encoded(R && r);

#endif

    template<typename Iter>
    constexpr detail::enable_utf8_cp_t<Iter>
    find_invalid_encoding(Iter first, Iter last)
    {
        while (first != last) {
            int const cp_bytes = boost::text::utf8_code_units(*first);
            if (cp_bytes == -1 || last - first < cp_bytes)
                return first;

            if (detail::end_of_invalid_utf8(first))
                return first;

            first += cp_bytes;
        }

        return last;
    }

    template<typename Iter>
    constexpr detail::enable_utf16_cp_t<Iter>
    find_invalid_encoding(Iter first, Iter last)
    {
        while (first != last) {
            int const cp_units = boost::text::utf16_code_units(*first);
            if (cp_units == -1 || last - first < cp_units)
                return first;

            if (cp_units == 2 && !boost::text::low_surrogate(*(first + 1)))
                return first;

            first += cp_units;
        }

        return last;
    }

    template<typename Iter>
    constexpr detail::enable_utf8_cp_t<Iter, bool> encoded(
        Iter first, Iter last)
    {
        return v1::find_invalid_encoding(first, last) == last;
    }

    template<typename Iter>
    constexpr detail::enable_utf16_cp_t<Iter, bool> encoded(
        Iter first, Iter last)
    {
        return v1::find_invalid_encoding(first, last) == last;
    }

    template<typename Iter>
    constexpr detail::enable_utf8_cp_t<Iter, bool>
    starts_encoded(Iter first, Iter last)
    {
        if (first == last)
            return true;

        int const cp_bytes = boost::text::utf8_code_units(*first);
        if (cp_bytes == -1 || last - first < cp_bytes)
            return false;

        return !detail::end_of_invalid_utf8(first);
    }

    template<typename Iter>
    constexpr detail::enable_utf16_cp_t<Iter, bool>
    starts_encoded(Iter first, Iter last)
    {
        if (first == last)
            return true;

        int const cp_units = boost::text::utf16_code_units(*first);
        if (cp_units == -1 || last - first < cp_units)
            return false;

        return cp_units == 1 || boost::text::low_surrogate(*(first + 1));
    }

    template<typename Iter>
    constexpr detail::enable_utf8_cp_t<Iter, bool>
    ends_encoded(Iter first, Iter last)
    {
        if (first == last)
            return true;

        auto it = last;
        while (first != --it && boost::text::continuation(*it))
            ;

        return v1::starts_encoded(it, last);
    }

    template<typename Iter>
    constexpr detail::enable_utf16_cp_t<Iter, bool>
    ends_encoded(Iter first, Iter last)
    {
        if (first == last)
            return true;

        auto it = last;
        if (boost::text::low_surrogate(*--it))
            --it;

        return v1::starts_encoded(it, last);
    }

}}}

#if BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    template<utf8_range R>
        requires std::ranges::forward_range<R>
    constexpr std::ranges::borrowed_iterator_t<R> find_invalid_encoding(R && r)
    {
        auto first = std::ranges::begin(r);
        auto last = std::ranges::end(r);

        while (first != last) {
            int const cp_bytes = boost::text::utf8_code_units(*first);
            if (cp_bytes == -1 || last - first < cp_bytes)
                return first;

            if (detail::end_of_invalid_utf8(first))
                return first;

            first += cp_bytes;
        }

        if constexpr (std::ranges::borrowed_range<R>) {
            return last;
        } else {
            return std::ranges::dangling{};
        }
    }

    template<utf16_range R>
        requires std::ranges::forward_range<R>
    constexpr std::ranges::borrowed_iterator_t<R> find_invalid_encoding(R && r)
    {
        auto first = std::ranges::begin(r);
        auto last = std::ranges::end(r);

        while (first != last) {
            int const cp_units = boost::text::utf16_code_units(*first);
            if (cp_units == -1 || last - first < cp_units)
                return first;

            if (cp_units == 2 && !boost::text::low_surrogate(*(first + 1)))
                return first;

            first += cp_units;
        }

        if constexpr (std::ranges::borrowed_range<R>) {
            return last;
        } else {
            return std::ranges::dangling{};
        }
    }

    template<utf8_range R>
        requires std::ranges::forward_range<R>
    constexpr bool encoded(R && r)
    {
        return boost::text::v1::find_invalid_encoding(r.begin(), r.end()) ==
               r.end();
    }

    template<utf16_range R>
        requires std::ranges::forward_range<R>
    constexpr bool encoded(R && r)
    {
        return boost::text::v1::find_invalid_encoding(r.begin(), r.end()) ==
               r.end();
    }

    template<utf8_range R>
        requires std::ranges::forward_range<R>
    constexpr bool starts_encoded(R && r)
    {
        auto first = std::ranges::begin(r);
        auto last = std::ranges::end(r);

        if (first == last)
            return true;

        int const cp_bytes = boost::text::utf8_code_units(*first);
        if (cp_bytes == -1 || last - first < cp_bytes)
            return false;

        return !detail::end_of_invalid_utf8(first);
    }

    template<utf16_range R>
        requires std::ranges::forward_range<R>
    constexpr bool starts_encoded(R && r)
    {
        auto first = std::ranges::begin(r);
        auto last = std::ranges::end(r);

        if (first == last)
            return true;

        int const cp_units = boost::text::utf16_code_units(*first);
        if (cp_units == -1 || last - first < cp_units)
            return false;

        return cp_units == 1 || boost::text::low_surrogate(*(first + 1));
    }

    template<utf8_range R>
        requires std::ranges::bidirectional_range<R> &&
                 std::ranges::common_range<R>
    constexpr bool ends_encoded(R && r)
    {
        auto first = std::ranges::begin(r);
        auto last = std::ranges::end(r);

        if (first == last)
            return true;

        auto it = last;
        while (first != --it && boost::text::continuation(*it))
            ;

        return boost::text::starts_encoded(it, last);
    }

    template<utf16_range R>
        requires std::ranges::bidirectional_range<R> &&
                 std::ranges::common_range<R>
    constexpr bool ends_encoded(R && r)
    {
        auto first = std::ranges::begin(r);
        auto last = std::ranges::end(r);

        if (first == last)
            return true;

        auto it = last;
        if (boost::text::low_surrogate(*--it))
            --it;

        return boost::text::starts_encoded(it, last);
    }

}}}

#endif

namespace boost { namespace text {

    /** An error handler type that can be used with the converting iterators;
        provides the Unicode replacement character on errors. */
    struct use_replacement_character
    {
        constexpr char32_t operator()(char const *) const noexcept
        {
            return replacement_character();
        }
    };

    /** An error handler type that can be used with the converting iterators;
        throws `std::logic_error` on errors. */
    struct throw_logic_error
    {
        char32_t operator()(char const * msg) const
        {
            boost::throw_exception(std::logic_error(msg));
            return 0;
        }
    };


    /** A sentinel type that compares equal to a pointer to a 1-, 2-, or
        4-byte integral value, iff the pointer is null. */
    struct null_sentinel_t
    {
        constexpr null_sentinel_t base() const { return {}; }

        template<typename T>
        friend constexpr auto operator==(T const * p, null_sentinel_t)
        {
            return *p == T{};
        }
    };

#if defined(__cpp_inline_variables)
    inline constexpr null_sentinel_t null_sentinel;
#else
    namespace {
        constexpr null_sentinel_t null_sentinel;
    }
#endif

    /** An out iterator that converts UTF-32 to UTF-8. */
#if BOOST_TEXT_USE_CONCEPTS
    template<std::output_iterator<char8_t> Iter>
#else
    template<typename Iter>
#endif
    struct utf_32_to_8_out_iterator
        : detail::trans_ins_iter<utf_32_to_8_out_iterator<Iter>, Iter>
    {
        constexpr utf_32_to_8_out_iterator() {}
        explicit constexpr utf_32_to_8_out_iterator(Iter it) :
            detail::trans_ins_iter<utf_32_to_8_out_iterator<Iter>, Iter>(it)
        {}

        constexpr utf_32_to_8_out_iterator & operator=(char32_t cp)
        {
            auto & out = this->iter();
            out = detail::write_cp_utf8(cp, out);
            return *this;
        }
    };

    /** An insert-iterator analogous to std::insert_iterator, that also
        converts UTF-32 to UTF-8. */
    template<typename Cont>
    struct utf_32_to_8_insert_iterator : detail::trans_ins_iter<
                                             utf_32_to_8_insert_iterator<Cont>,
                                             std::insert_iterator<Cont>>
    {
        constexpr utf_32_to_8_insert_iterator() {}
        constexpr utf_32_to_8_insert_iterator(
            Cont & c, typename Cont::iterator it) :
            detail::trans_ins_iter<
                utf_32_to_8_insert_iterator<Cont>,
                std::insert_iterator<Cont>>(std::insert_iterator<Cont>(c, it))
        {}

        constexpr utf_32_to_8_insert_iterator & operator=(char32_t cp)
        {
            auto & out = this->iter();
            out = detail::write_cp_utf8(cp, out);
            return *this;
        }
    };

    /** An insert-iterator analogous to std::front_insert_iterator, that also
        converts UTF-32 to UTF-8. */
    template<typename Cont>
    struct utf_32_to_8_front_insert_iterator
        : detail::trans_ins_iter<
              utf_32_to_8_front_insert_iterator<Cont>,
              std::front_insert_iterator<Cont>>
    {
        constexpr utf_32_to_8_front_insert_iterator() {}
        explicit constexpr utf_32_to_8_front_insert_iterator(Cont & c) :
            detail::trans_ins_iter<
                utf_32_to_8_front_insert_iterator<Cont>,
                std::front_insert_iterator<Cont>>(
                std::front_insert_iterator<Cont>(c))
        {}

        constexpr utf_32_to_8_front_insert_iterator & operator=(char32_t cp)
        {
            auto & out = this->iter();
            out = detail::write_cp_utf8(cp, out);
            return *this;
        }
    };

    /** An insert-iterator analogous to std::back_insert_iterator, that also
        converts UTF-32 to UTF-8. */
    template<typename Cont>
    struct utf_32_to_8_back_insert_iterator
        : detail::trans_ins_iter<
              utf_32_to_8_back_insert_iterator<Cont>,
              std::back_insert_iterator<Cont>>
    {
        constexpr utf_32_to_8_back_insert_iterator() {}
        explicit constexpr utf_32_to_8_back_insert_iterator(Cont & c) :
            detail::trans_ins_iter<
                utf_32_to_8_back_insert_iterator<Cont>,
                std::back_insert_iterator<Cont>>(
                std::back_insert_iterator<Cont>(c))
        {}

        constexpr utf_32_to_8_back_insert_iterator & operator=(char32_t cp)
        {
            auto & out = this->iter();
            out = detail::write_cp_utf8(cp, out);
            return *this;
        }
    };


    namespace detail {
        template<typename OutIter>
        OutIter assign_8_to_32_insert(
            unsigned char cu, char32_t & cp, int & state, OutIter out)
        {
            auto write = [&] {
                *out = cp;
                ++out;
                state = invalid_table_state;
            };
            auto start_cp = [&] {
                first_cu const info = first_cus[cu];
                state = info.next;
                cp = info.initial_octet;
                if (state == bgn)
                    write();
            };
            if (state == invalid_table_state) {
                start_cp();
            } else {
                cp = (cp << 6) | (cu & 0x3f);
                char_class const class_ = octet_classes[cu];
                state = transitions[state + class_];
                if (state == bgn) {
                    write();
                } else if (state == err) {
                    *out = replacement_character();
                    ++out;
                    start_cp();
                }
            }
            return out;
        }
    }

    /** An out iterator that converts UTF-8 to UTF-32. */
#if BOOST_TEXT_USE_CONCEPTS
    template<std::output_iterator<char32_t> Iter>
#else
    template<typename Iter>
#endif
    struct utf_8_to_32_out_iterator
        : detail::trans_ins_iter<utf_8_to_32_out_iterator<Iter>, Iter>
    {
        constexpr utf_8_to_32_out_iterator() {}
        explicit constexpr utf_8_to_32_out_iterator(Iter it) :
            detail::trans_ins_iter<utf_8_to_32_out_iterator<Iter>, Iter>(it),
            state_(detail::invalid_table_state)
        {}

        constexpr utf_8_to_32_out_iterator & operator=(char8_t cu)
        {
            auto & out = this->iter();
            out = detail::assign_8_to_32_insert(cu, cp_, state_, out);
            return *this;
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        int state_;
        char32_t cp_;
#endif
    };

    /** An insert-iterator analogous to std::insert_iterator, that also
        converts UTF-8 to UTF-32. */
    template<typename Cont>
    struct utf_8_to_32_insert_iterator : detail::trans_ins_iter<
                                             utf_8_to_32_insert_iterator<Cont>,
                                             std::insert_iterator<Cont>>
    {
        constexpr utf_8_to_32_insert_iterator() {}
        constexpr utf_8_to_32_insert_iterator(
            Cont & c, typename Cont::iterator it) :
            detail::trans_ins_iter<
                utf_8_to_32_insert_iterator<Cont>,
                std::insert_iterator<Cont>>(std::insert_iterator<Cont>(c, it)),
            state_(detail::invalid_table_state)
        {}

        constexpr utf_8_to_32_insert_iterator & operator=(char16_t cu)
        {
            auto & out = this->iter();
            out = detail::assign_8_to_32_insert(cu, cp_, state_, out);
            return *this;
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        int state_;
        char32_t cp_;
#endif
    };

    /** An insert-iterator analogous to std::front_insert_iterator, that also
        converts UTF-8 to UTF-32. */
    template<typename Cont>
    struct utf_8_to_32_front_insert_iterator
        : detail::trans_ins_iter<
              utf_8_to_32_front_insert_iterator<Cont>,
              std::front_insert_iterator<Cont>>
    {
        constexpr utf_8_to_32_front_insert_iterator() {}
        explicit constexpr utf_8_to_32_front_insert_iterator(Cont & c) :
            detail::trans_ins_iter<
                utf_8_to_32_front_insert_iterator<Cont>,
                std::front_insert_iterator<Cont>>(
                std::front_insert_iterator<Cont>(c)),
            state_(detail::invalid_table_state)
        {}

        constexpr utf_8_to_32_front_insert_iterator & operator=(char16_t cu)
        {
            auto & out = this->iter();
            out = detail::assign_8_to_32_insert(cu, cp_, state_, out);
            return *this;
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        int state_;
        char32_t cp_;
#endif
    };

    /** An insert-iterator analogous to std::back_insert_iterator, that also
        converts UTF-8 to UTF-32. */
    template<typename Cont>
    struct utf_8_to_32_back_insert_iterator
        : detail::trans_ins_iter<
              utf_8_to_32_back_insert_iterator<Cont>,
              std::back_insert_iterator<Cont>>
    {
        constexpr utf_8_to_32_back_insert_iterator() {}
        explicit constexpr utf_8_to_32_back_insert_iterator(Cont & c) :
            detail::trans_ins_iter<
                utf_8_to_32_back_insert_iterator<Cont>,
                std::back_insert_iterator<Cont>>(
                std::back_insert_iterator<Cont>(c)),
            state_(detail::invalid_table_state)
        {}

        constexpr utf_8_to_32_back_insert_iterator & operator=(char16_t cu)
        {
            auto & out = this->iter();
            out = detail::assign_8_to_32_insert(cu, cp_, state_, out);
            return *this;
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        int state_;
        char32_t cp_;
#endif
    };


    /** An out iterator that converts UTF-8 to UTF-16. */
#if BOOST_TEXT_USE_CONCEPTS
    template<std::output_iterator<char16_t> Iter>
#else
    template<typename Iter>
#endif
    struct utf_32_to_16_out_iterator
        : detail::trans_ins_iter<utf_32_to_16_out_iterator<Iter>, Iter>
    {
        constexpr utf_32_to_16_out_iterator() {}
        explicit constexpr utf_32_to_16_out_iterator(Iter it) :
            detail::trans_ins_iter<utf_32_to_16_out_iterator<Iter>, Iter>(it)
        {}

        constexpr utf_32_to_16_out_iterator & operator=(char32_t cp)
        {
            auto & out = this->iter();
            out = detail::write_cp_utf16(cp, out);
            return *this;
        }
    };

    /** An insert-iterator analogous to std::insert_iterator, that also
        converts UTF-32 to UTF-16. */
    template<typename Cont>
    struct utf_32_to_16_insert_iterator
        : detail::trans_ins_iter<
              utf_32_to_16_insert_iterator<Cont>,
              std::insert_iterator<Cont>>
    {
        constexpr utf_32_to_16_insert_iterator() {}
        constexpr utf_32_to_16_insert_iterator(
            Cont & c, typename Cont::iterator it) :
            detail::trans_ins_iter<
                utf_32_to_16_insert_iterator<Cont>,
                std::insert_iterator<Cont>>(std::insert_iterator<Cont>(c, it))
        {}

        constexpr utf_32_to_16_insert_iterator & operator=(char32_t cp)
        {
            auto & out = this->iter();
            out = detail::write_cp_utf16(cp, out);
            return *this;
        }
    };

    /** An insert-iterator analogous to std::front_insert_iterator, that also
        converts UTF-32 to UTF-16. */
    template<typename Cont>
    struct utf_32_to_16_front_insert_iterator
        : detail::trans_ins_iter<
              utf_32_to_16_front_insert_iterator<Cont>,
              std::front_insert_iterator<Cont>>
    {
        constexpr utf_32_to_16_front_insert_iterator() {}
        explicit constexpr utf_32_to_16_front_insert_iterator(Cont & c) :
            detail::trans_ins_iter<
                utf_32_to_16_front_insert_iterator<Cont>,
                std::front_insert_iterator<Cont>>(
                std::front_insert_iterator<Cont>(c))
        {}

        constexpr utf_32_to_16_front_insert_iterator & operator=(char32_t cp)
        {
            auto & out = this->iter();
            out = detail::write_cp_utf16(cp, out);
            return *this;
        }
    };

    /** An insert-iterator analogous to std::back_insert_iterator, that also
        converts UTF-32 to UTF-16. */
    template<typename Cont>
    struct utf_32_to_16_back_insert_iterator
        : detail::trans_ins_iter<
              utf_32_to_16_back_insert_iterator<Cont>,
              std::back_insert_iterator<Cont>>
    {
        constexpr utf_32_to_16_back_insert_iterator() {}
        explicit constexpr utf_32_to_16_back_insert_iterator(Cont & c) :
            detail::trans_ins_iter<
                utf_32_to_16_back_insert_iterator<Cont>,
                std::back_insert_iterator<Cont>>(
                std::back_insert_iterator<Cont>(c))
        {}

        constexpr utf_32_to_16_back_insert_iterator & operator=(char32_t cp)
        {
            auto & out = this->iter();
            out = detail::write_cp_utf16(cp, out);
            return *this;
        }
    };


    namespace detail {
        template<typename OutIter>
        OutIter
        assign_16_to_32_insert(char16_t & prev_cu, char16_t cu, OutIter out)
        {
            if (high_surrogate(cu)) {
                if (prev_cu) {
                    *out = replacement_character();
                    ++out;
                }
                prev_cu = cu;
            } else if (low_surrogate(cu)) {
                if (prev_cu) {
                    *out = detail::surrogates_to_cp(prev_cu, cu);
                    ++out;
                } else {
                    *out = replacement_character();
                    ++out;
                }
                prev_cu = 0;
            } else {
                if (prev_cu) {
                    *out = replacement_character();
                    ++out;
                }
                *out = cu;
                ++out;
                prev_cu = 0;
            }
            return out;
        }
    }

    /** An out iterator that converts UTF-16 to UTF-32. */
#if BOOST_TEXT_USE_CONCEPTS
    template<std::output_iterator<char32_t> Iter>
#else
    template<typename Iter>
#endif
    struct utf_16_to_32_out_iterator
        : detail::trans_ins_iter<utf_16_to_32_out_iterator<Iter>, Iter>
    {
        constexpr utf_16_to_32_out_iterator() {}
        explicit constexpr utf_16_to_32_out_iterator(Iter it) :
            detail::trans_ins_iter<utf_16_to_32_out_iterator<Iter>, Iter>(it),
            prev_cu_(0)
        {}

        constexpr utf_16_to_32_out_iterator & operator=(char16_t cu)
        {
            auto & out = this->iter();
            out = detail::assign_16_to_32_insert(prev_cu_, cu, out);
            return *this;
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        char16_t prev_cu_;
#endif
    };

    /** An insert-iterator analogous to std::insert_iterator, that also
        converts UTF-16 to UTF-32. */
    template<typename Cont>
    struct utf_16_to_32_insert_iterator
        : detail::trans_ins_iter<
              utf_16_to_32_insert_iterator<Cont>,
              std::insert_iterator<Cont>>
    {
        constexpr utf_16_to_32_insert_iterator() {}
        constexpr utf_16_to_32_insert_iterator(
            Cont & c, typename Cont::iterator it) :
            detail::trans_ins_iter<
                utf_16_to_32_insert_iterator<Cont>,
                std::insert_iterator<Cont>>(std::insert_iterator<Cont>(c, it)),
            prev_cu_(0)
        {}

        constexpr utf_16_to_32_insert_iterator & operator=(char16_t cu)
        {
            auto & out = this->iter();
            out = detail::assign_16_to_32_insert(prev_cu_, cu, out);
            return *this;
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        char16_t prev_cu_;
#endif
    };

    /** An insert-iterator analogous to std::front_insert_iterator, that also
        converts UTF-16 to UTF-32. */
    template<typename Cont>
    struct utf_16_to_32_front_insert_iterator
        : detail::trans_ins_iter<
              utf_16_to_32_front_insert_iterator<Cont>,
              std::front_insert_iterator<Cont>>
    {
        constexpr utf_16_to_32_front_insert_iterator() {}
        explicit constexpr utf_16_to_32_front_insert_iterator(Cont & c) :
            detail::trans_ins_iter<
                utf_16_to_32_front_insert_iterator<Cont>,
                std::front_insert_iterator<Cont>>(
                std::front_insert_iterator<Cont>(c)),
            prev_cu_(0)
        {}

        constexpr utf_16_to_32_front_insert_iterator & operator=(char16_t cu)
        {
            auto & out = this->iter();
            out = detail::assign_16_to_32_insert(prev_cu_, cu, out);
            return *this;
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        char16_t prev_cu_;
#endif
    };

    /** An insert-iterator analogous to std::back_insert_iterator, that also
        converts UTF-16 to UTF-32. */
    template<typename Cont>
    struct utf_16_to_32_back_insert_iterator
        : detail::trans_ins_iter<
              utf_16_to_32_back_insert_iterator<Cont>,
              std::back_insert_iterator<Cont>>
    {
        constexpr utf_16_to_32_back_insert_iterator() {}
        explicit constexpr utf_16_to_32_back_insert_iterator(Cont & c) :
            detail::trans_ins_iter<
                utf_16_to_32_back_insert_iterator<Cont>,
                std::back_insert_iterator<Cont>>(
                std::back_insert_iterator<Cont>(c)),
            prev_cu_(0)
        {}

        constexpr utf_16_to_32_back_insert_iterator & operator=(char16_t cu)
        {
            auto & out = this->iter();
            out = detail::assign_16_to_32_insert(prev_cu_, cu, out);
            return *this;
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        char16_t prev_cu_;
#endif
    };


    namespace detail {
        template<typename OutIter>
        OutIter
        assign_16_to_8_insert(char16_t & prev_cu, char16_t cu, OutIter out)
        {
            if (high_surrogate(cu)) {
                if (prev_cu)
                    out = detail::write_cp_utf8(replacement_character(), out);
                prev_cu = cu;
            } else if (low_surrogate(cu)) {
                if (prev_cu) {
                    auto const cp = detail::surrogates_to_cp(prev_cu, cu);
                    out = detail::write_cp_utf8(cp, out);
                } else {
                    out = detail::write_cp_utf8(replacement_character(), out);
                }
                prev_cu = 0;
            } else {
                if (prev_cu)
                    out = detail::write_cp_utf8(replacement_character(), out);
                out = detail::write_cp_utf8(cu, out);
                prev_cu = 0;
            }
            return out;
        }
    }

    /** An out iterator that converts UTF-16 to UTF-8. */
#if BOOST_TEXT_USE_CONCEPTS
    template<std::output_iterator<char8_t> Iter>
#else
    template<typename Iter>
#endif
    struct utf_16_to_8_out_iterator
        : detail::trans_ins_iter<utf_16_to_8_out_iterator<Iter>, Iter>
    {
        constexpr utf_16_to_8_out_iterator() {}
        explicit constexpr utf_16_to_8_out_iterator(Iter it) :
            detail::trans_ins_iter<utf_16_to_8_out_iterator<Iter>, Iter>(it),
            prev_cu_(0)
        {}

        constexpr utf_16_to_8_out_iterator & operator=(char16_t cu)
        {
            auto & out = this->iter();
            out = detail::assign_16_to_8_insert(prev_cu_, cu, out);
            return *this;
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        char16_t prev_cu_;
#endif
    };

    /** An insert-iterator analogous to std::insert_iterator, that also
        converts UTF-16 to UTF-8. */
    template<typename Cont>
    struct utf_16_to_8_insert_iterator : detail::trans_ins_iter<
                                             utf_16_to_8_insert_iterator<Cont>,
                                             std::insert_iterator<Cont>>
    {
        constexpr utf_16_to_8_insert_iterator() {}
        constexpr utf_16_to_8_insert_iterator(
            Cont & c, typename Cont::iterator it) :
            detail::trans_ins_iter<
                utf_16_to_8_insert_iterator<Cont>,
                std::insert_iterator<Cont>>(std::insert_iterator<Cont>(c, it)),
            prev_cu_(0)
        {}

        constexpr utf_16_to_8_insert_iterator & operator=(char16_t cu)
        {
            auto & out = this->iter();
            out = detail::assign_16_to_8_insert(prev_cu_, cu, out);
            return *this;
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        char16_t prev_cu_;
#endif
    };

    /** An insert-iterator analogous to std::front_insert_iterator, that also
        converts UTF-16 to UTF-8. */
    template<typename Cont>
    struct utf_16_to_8_front_insert_iterator
        : detail::trans_ins_iter<
              utf_16_to_8_front_insert_iterator<Cont>,
              std::front_insert_iterator<Cont>>
    {
        constexpr utf_16_to_8_front_insert_iterator() {}
        explicit constexpr utf_16_to_8_front_insert_iterator(Cont & c) :
            detail::trans_ins_iter<
                utf_16_to_8_front_insert_iterator<Cont>,
                std::front_insert_iterator<Cont>>(
                std::front_insert_iterator<Cont>(c)),
            prev_cu_(0)
        {}

        constexpr utf_16_to_8_front_insert_iterator & operator=(char16_t cu)
        {
            auto & out = this->iter();
            out = detail::assign_16_to_8_insert(prev_cu_, cu, out);
            return *this;
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        char16_t prev_cu_;
#endif
    };

    /** An insert-iterator analogous to std::back_insert_iterator, that also
        converts UTF-16 to UTF-8. */
    template<typename Cont>
    struct utf_16_to_8_back_insert_iterator
        : detail::trans_ins_iter<
              utf_16_to_8_back_insert_iterator<Cont>,
              std::back_insert_iterator<Cont>>
    {
        constexpr utf_16_to_8_back_insert_iterator() {}
        explicit constexpr utf_16_to_8_back_insert_iterator(Cont & c) :
            detail::trans_ins_iter<
                utf_16_to_8_back_insert_iterator<Cont>,
                std::back_insert_iterator<Cont>>(
                std::back_insert_iterator<Cont>(c)),
            prev_cu_(0)
        {}

        constexpr utf_16_to_8_back_insert_iterator & operator=(char16_t cu)
        {
            auto & out = this->iter();
            out = detail::assign_16_to_8_insert(prev_cu_, cu, out);
            return *this;
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        char16_t prev_cu_;
#endif
    };


    namespace detail {
        template<typename OutIter>
        OutIter assign_8_to_16_insert(
            unsigned char cu, char32_t & cp, int & state, OutIter out)
        {
            auto write = [&] {
                out = detail::write_cp_utf16(cp, out);
                state = invalid_table_state;
            };
            auto start_cp = [&] {
                first_cu const info = first_cus[cu];
                state = info.next;
                cp = info.initial_octet;
                if (state == bgn)
                    write();
            };
            if (state == invalid_table_state) {
                start_cp();
            } else {
                cp = (cp << 6) | (cu & 0x3f);
                char_class const class_ = octet_classes[cu];
                state = transitions[state + class_];
                if (state == bgn) {
                    write();
                } else if (state == err) {
                    out = detail::write_cp_utf16(replacement_character(), out);
                    start_cp();
                }
            }
            return out;
        }
    }

    /** An out iterator that converts UTF-8 to UTF-16. */
#if BOOST_TEXT_USE_CONCEPTS
    template<std::output_iterator<char16_t> Iter>
#else
    template<typename Iter>
#endif
    struct utf_8_to_16_out_iterator
        : detail::trans_ins_iter<utf_8_to_16_out_iterator<Iter>, Iter>
    {
        constexpr utf_8_to_16_out_iterator() {}
        explicit constexpr utf_8_to_16_out_iterator(Iter it) :
            detail::trans_ins_iter<utf_8_to_16_out_iterator<Iter>, Iter>(it),
            state_(detail::invalid_table_state)
        {}

        constexpr utf_8_to_16_out_iterator & operator=(char8_t cu)
        {
            auto & out = this->iter();
            out = detail::assign_8_to_16_insert(cu, cp_, state_, out);
            return *this;
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        int state_;
        char32_t cp_;
#endif
    };

    /** An insert-iterator analogous to std::insert_iterator, that also
        converts UTF-8 to UTF-16. */
    template<typename Cont>
    struct utf_8_to_16_insert_iterator : detail::trans_ins_iter<
                                             utf_8_to_16_insert_iterator<Cont>,
                                             std::insert_iterator<Cont>>
    {
        constexpr utf_8_to_16_insert_iterator() {}
        constexpr utf_8_to_16_insert_iterator(
            Cont & c, typename Cont::iterator it) :
            detail::trans_ins_iter<
                utf_8_to_16_insert_iterator<Cont>,
                std::insert_iterator<Cont>>(std::insert_iterator<Cont>(c, it)),
            state_(detail::invalid_table_state)
        {}

        constexpr utf_8_to_16_insert_iterator & operator=(char16_t cu)
        {
            auto & out = this->iter();
            out = detail::assign_8_to_16_insert(cu, cp_, state_, out);
            return *this;
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        int state_;
        char32_t cp_;
#endif
    };

    /** An insert-iterator analogous to std::front_insert_iterator, that also
        converts UTF-8 to UTF-16. */
    template<typename Cont>
    struct utf_8_to_16_front_insert_iterator
        : detail::trans_ins_iter<
              utf_8_to_16_front_insert_iterator<Cont>,
              std::front_insert_iterator<Cont>>
    {
        constexpr utf_8_to_16_front_insert_iterator() {}
        explicit constexpr utf_8_to_16_front_insert_iterator(Cont & c) :
            detail::trans_ins_iter<
                utf_8_to_16_front_insert_iterator<Cont>,
                std::front_insert_iterator<Cont>>(
                std::front_insert_iterator<Cont>(c)),
            state_(detail::invalid_table_state)
        {}

        constexpr utf_8_to_16_front_insert_iterator & operator=(char16_t cu)
        {
            auto & out = this->iter();
            out = detail::assign_8_to_16_insert(cu, cp_, state_, out);
            return *this;
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        int state_;
        char32_t cp_;
#endif
    };

    /** An insert-iterator analogous to std::back_insert_iterator, that also
        converts UTF-8 to UTF-16. */
    template<typename Cont>
    struct utf_8_to_16_back_insert_iterator
        : detail::trans_ins_iter<
              utf_8_to_16_back_insert_iterator<Cont>,
              std::back_insert_iterator<Cont>>
    {
        constexpr utf_8_to_16_back_insert_iterator() {}
        explicit constexpr utf_8_to_16_back_insert_iterator(Cont & c) :
            detail::trans_ins_iter<
                utf_8_to_16_back_insert_iterator<Cont>,
                std::back_insert_iterator<Cont>>(
                std::back_insert_iterator<Cont>(c)),
            state_(detail::invalid_table_state)
        {}

        constexpr utf_8_to_16_back_insert_iterator & operator=(char16_t cu)
        {
            auto & out = this->iter();
            out = detail::assign_8_to_16_insert(cu, cp_, state_, out);
            return *this;
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        int state_;
        char32_t cp_;
#endif
    };

}}

#include <boost/text/unpack.hpp>

namespace boost { namespace text { namespace detail {

    template<format Tag>
    struct make_utf8_dispatch;

    template<>
    struct make_utf8_dispatch<format::utf8>
    {
        template<typename Iter, typename Sentinel>
        static constexpr Iter call(Iter first, Iter it, Sentinel last)
        {
            return it;
        }
    };

    template<>
    struct make_utf8_dispatch<format::utf16>
    {
        template<typename Iter, typename Sentinel>
        static constexpr utf_16_to_8_iterator<Iter, Sentinel>
        call(Iter first, Iter it, Sentinel last)
        {
            return utf_16_to_8_iterator<Iter, Sentinel>(first, it, last);
        }
    };

    template<>
    struct make_utf8_dispatch<format::utf32>
    {
        template<typename Iter, typename Sentinel>
        static constexpr utf_32_to_8_iterator<Iter, Sentinel>
        call(Iter first, Iter it, Sentinel last)
        {
            return utf_32_to_8_iterator<Iter, Sentinel>(first, it, last);
        }
    };

    template<format Tag>
    struct make_utf16_dispatch;

    template<>
    struct make_utf16_dispatch<format::utf8>
    {
        template<typename Iter, typename Sentinel>
        static constexpr utf_8_to_16_iterator<Iter, Sentinel>
        call(Iter first, Iter it, Sentinel last)
        {
            return utf_8_to_16_iterator<Iter, Sentinel>(first, it, last);
        }
    };

    template<>
    struct make_utf16_dispatch<format::utf16>
    {
        template<typename Iter, typename Sentinel>
        static constexpr Iter call(Iter first, Iter it, Sentinel last)
        {
            return it;
        }
    };

    template<>
    struct make_utf16_dispatch<format::utf32>
    {
        template<typename Iter, typename Sentinel>
        static constexpr utf_32_to_16_iterator<Iter, Sentinel>
        call(Iter first, Iter it, Sentinel last)
        {
            return utf_32_to_16_iterator<Iter, Sentinel>(first, it, last);
        }
    };

    template<format Tag>
    struct make_utf32_dispatch;

    template<>
    struct make_utf32_dispatch<format::utf8>
    {
        template<typename Iter, typename Sentinel>
        static constexpr utf_8_to_32_iterator<Iter, Sentinel>
        call(Iter first, Iter it, Sentinel last)
        {
            return utf_8_to_32_iterator<Iter, Sentinel>(first, it, last);
        }
    };

    template<>
    struct make_utf32_dispatch<format::utf16>
    {
        template<typename Iter, typename Sentinel>
        static constexpr utf_16_to_32_iterator<Iter, Sentinel>
        call(Iter first, Iter it, Sentinel last)
        {
            return utf_16_to_32_iterator<Iter, Sentinel>(first, it, last);
        }
    };

    template<>
    struct make_utf32_dispatch<format::utf32>
    {
        template<typename Iter, typename Sentinel>
        static constexpr Iter call(Iter first, Iter it, Sentinel last)
        {
            return it;
        }
    };

    template<
        typename Cont,
        typename UTF8,
        typename UTF16,
        typename UTF32,
        int Bytes = sizeof(typename Cont::value_type)>
    struct from_utf8_dispatch
    {
        using type = UTF8;
    };

    template<typename Cont, typename UTF8, typename UTF16, typename UTF32>
    struct from_utf8_dispatch<Cont, UTF8, UTF16, UTF32, 2>
    {
        using type = UTF16;
    };

    template<typename Cont, typename UTF8, typename UTF16, typename UTF32>
    struct from_utf8_dispatch<Cont, UTF8, UTF16, UTF32, 4>
    {
        using type = UTF32;
    };

    template<typename Cont, typename UTF8, typename UTF16, typename UTF32>
    using from_utf8_dispatch_t =
        typename from_utf8_dispatch<Cont, UTF8, UTF16, UTF32>::type;

    template<
        typename Cont,
        typename UTF8,
        typename UTF16,
        typename UTF32,
        int Bytes = sizeof(typename Cont::value_type)>
    struct from_utf16_dispatch
    {
        using type = UTF16;
    };

    template<typename Cont, typename UTF8, typename UTF16, typename UTF32>
    struct from_utf16_dispatch<Cont, UTF8, UTF16, UTF32, 1>
    {
        using type = UTF8;
    };

    template<typename Cont, typename UTF8, typename UTF16, typename UTF32>
    struct from_utf16_dispatch<Cont, UTF8, UTF16, UTF32, 4>
    {
        using type = UTF32;
    };

    template<typename Cont, typename UTF8, typename UTF16, typename UTF32>
    using from_utf16_dispatch_t =
        typename from_utf16_dispatch<Cont, UTF8, UTF16, UTF32>::type;

    template<
        typename Cont,
        typename UTF8,
        typename UTF16,
        typename UTF32,
        int Bytes = sizeof(typename Cont::value_type)>
    struct from_utf32_dispatch
    {
        using type = UTF32;
    };

    template<typename Cont, typename UTF8, typename UTF16, typename UTF32>
    struct from_utf32_dispatch<Cont, UTF8, UTF16, UTF32, 1>
    {
        using type = UTF8;
    };

    template<typename Cont, typename UTF8, typename UTF16, typename UTF32>
    struct from_utf32_dispatch<Cont, UTF8, UTF16, UTF32, 2>
    {
        using type = UTF16;
    };

    template<typename Cont, typename UTF8, typename UTF16, typename UTF32>
    using from_utf32_dispatch_t =
        typename from_utf32_dispatch<Cont, UTF8, UTF16, UTF32>::type;

}}}

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V1 {

#if defined(BOOST_TEXT_DOXYGEN)

    /** Returns a `utf_32_to_8_out_iterator<O>` constructed from the given
        iterator. */
    template<std::output_iterator<char8_t> O>
    utf_32_to_8_out_iterator<O> utf_32_to_8_out(O it);

    /** Returns a `utf_8_to_32_out_iterator<O>` constructed from the given
        iterator. */
    template<std::output_iterator<char32_t> O>
    utf_8_to_32_out_iterator<O> utf_8_to_32_out(O it);

    /** Returns a `utf_32_to_16_out_iterator<O>` constructed from the given
        iterator. */
    template<std::output_iterator<char16_t> O>
    utf_32_to_16_out_iterator<O> utf_32_to_16_out(O it);

    /** Returns a `utf_16_to_32_out_iterator<O>` constructed from the given
        iterator. */
    template<std::output_iterator<char32_t> O>
    utf_16_to_32_out_iterator<O> utf_16_to_32_out(O it);

    /** Returns a `utf_16_to_8_out_iterator<O>` constructed from the given
        iterator. */
    template<std::output_iterator<char8_t> O>
    utf_16_to_8_out_iterator<O> utf_16_to_8_out(O it);

    /** Returns a `utf_8_to_16_out_iterator<O>` constructed from the given
        iterator. */
    template<std::output_iterator<char16_t> O>
    utf_8_to_16_out_iterator<O> utf_8_to_16_out(O it);

    /** Returns an iterator equivalent to `it` that transcodes `[first, last)`
        to UTF-8. */
    template<std::input_iterator I, std::sentinel_for<I> S>
    auto utf8_iterator(I first, I it, S last);

    /** Returns an iterator equivalent to `it` that transcodes `[first, last)`
        to UTF-16. */
    template<std::input_iterator I, std::sentinel_for<I> S>
    auto utf16_iterator(I first, I it, S last);

    /** Returns an iterator equivalent to `it` that transcodes `[first, last)`
        to UTF-32. */
    template<std::input_iterator I, std::sentinel_for<I> S>
    auto utf32_iterator(I first, I it, S last);

    /** Returns a inserting iterator that transcodes from UTF-8 to UTF-8,
        UTF-16, or UTF-32.  Which UTF the iterator transcodes to depends on
        `sizeof(Cont::value_type)`: `1` implies UTF-8; `2` implies UTF-16; and
        any other size implies UTF-32. */
    template<typename Cont>
    requires requires { typename Cont::value_type; } &&
             std::is_integral_v<typename Cont::value_type>
    auto from_utf8_inserter(Cont & c, typename Cont::iterator it);

    /** Returns a inserting iterator that transcodes from UTF-16 to UTF-8,
        UTF-16, or UTF-32.  Which UTF the iterator transcodes to depends on
        `sizeof(Cont::value_type)`: `1` implies UTF-8; `2` implies UTF-16; and
        any other size implies UTF-32. */
    template<typename Cont>
    requires requires { typename Cont::value_type; } &&
             std::is_integral_v<typename Cont::value_type>
    auto from_utf16_inserter(Cont & c, typename Cont::iterator it);

    /** Returns a inserting iterator that transcodes from UTF-32 to UTF-8,
        UTF-16, or UTF-32.  Which UTF the iterator transcodes to depends on
        `sizeof(Cont::value_type)`: `1` implies UTF-8; `2` implies UTF-16; and
        any other size implies UTF-32. */
    template<typename Cont>
    requires requires { typename Cont::value_type; } &&
             std::is_integral_v<typename Cont::value_type>
    auto from_utf32_inserter(Cont & c, typename Cont::iterator it);

    /** Returns a back-inserting iterator that transcodes from UTF-8 to UTF-8,
        UTF-16, or UTF-32.  Which UTF the iterator transcodes to depends on
        `sizeof(Cont::value_type)`: `1` implies UTF-8; `2` implies UTF-16; and
        any other size implies UTF-32. */
    template<typename Cont>
    requires requires { typename Cont::value_type; } &&
             std::is_integral_v<typename Cont::value_type>
    auto from_utf8_back_inserter(Cont & c);

    /** Returns a back-inserting iterator that transcodes from UTF-16 to UTF-8,
        UTF-16, or UTF-32.  Which UTF the iterator transcodes to depends on
        `sizeof(Cont::value_type)`: `1` implies UTF-8; `2` implies UTF-16; and
        any other size implies UTF-32. */
    template<typename Cont>
    requires requires { typename Cont::value_type; } &&
             std::is_integral_v<typename Cont::value_type>
    auto from_utf16_back_inserter(Cont & c);

    /** Returns a back-inserting iterator that transcodes from UTF-32 to UTF-8,
        UTF-16, or UTF-32.  Which UTF the iterator transcodes to depends on
        `sizeof(Cont::value_type)`: `1` implies UTF-8; `2` implies UTF-16; and
        any other size implies UTF-32. */
    template<typename Cont>
    requires requires { typename Cont::value_type; } &&
             std::is_integral_v<typename Cont::value_type>
    auto from_utf32_back_inserter(Cont & c);

    /** Returns a front-inserting iterator that transcodes from UTF-8 to UTF-8,
        UTF-16, or UTF-32.  Which UTF the iterator transcodes to depends on
        `sizeof(Cont::value_type)`: `1` implies UTF-8; `2` implies UTF-16; and
        any other size implies UTF-32. */
    template<typename Cont>
    requires requires { typename Cont::value_type; } &&
             std::is_integral_v<typename Cont::value_type>
    auto from_utf8_front_inserter(Cont & c);

    /** Returns a front-inserting iterator that transcodes from UTF-16 to UTF-8,
        UTF-16, or UTF-32.  Which UTF the iterator transcodes to depends on
        `sizeof(Cont::value_type)`: `1` implies UTF-8; `2` implies UTF-16; and
        any other size implies UTF-32. */
    template<typename Cont>
    requires requires { typename Cont::value_type; } &&
             std::is_integral_v<typename Cont::value_type>
    auto from_utf16_front_inserter(Cont & c);

    /** Returns a front-inserting iterator that transcodes from UTF-32 to UTF-8,
        UTF-16, or UTF-32.  Which UTF the iterator transcodes to depends on
        `sizeof(Cont::value_type)`: `1` implies UTF-8; `2` implies UTF-16; and
        any other size implies UTF-32. */
    template<typename Cont>
    requires requires { typename Cont::value_type; } &&
             std::is_integral_v<typename Cont::value_type>
    auto from_utf32_front_inserter(Cont & c);

#endif

    template<typename Iter>
    utf_32_to_8_out_iterator<Iter> utf_32_to_8_out(Iter it)
    {
        return utf_32_to_8_out_iterator<Iter>(it);
    }

    template<typename Iter>
    utf_8_to_32_out_iterator<Iter> utf_8_to_32_out(Iter it)
    {
        return utf_8_to_32_out_iterator<Iter>(it);
    }

    template<typename Iter>
    utf_32_to_16_out_iterator<Iter> utf_32_to_16_out(Iter it)
    {
        return utf_32_to_16_out_iterator<Iter>(it);
    }

    template<typename Iter>
    utf_16_to_32_out_iterator<Iter> utf_16_to_32_out(Iter it)
    {
        return utf_16_to_32_out_iterator<Iter>(it);
    }

    template<typename Iter>
    utf_16_to_8_out_iterator<Iter> utf_16_to_8_out(Iter it)
    {
        return utf_16_to_8_out_iterator<Iter>(it);
    }

    template<typename Iter>
    utf_8_to_16_out_iterator<Iter> utf_8_to_16_out(Iter it)
    {
        return utf_8_to_16_out_iterator<Iter>(it);
    }

    template<typename Iter, typename Sentinel>
    auto utf8_iterator(Iter first, Iter it, Sentinel last)
    {
        auto const unpacked = text::unpack_iterator_and_sentinel(first, last);
        auto const unpacked_it =
            text::unpack_iterator_and_sentinel(it, last).first;
        constexpr format tag = unpacked.format_tag;
        return detail::make_utf8_dispatch<tag>::call(
            unpacked.first, unpacked_it, unpacked.last);
    }

    template<typename Iter, typename Sentinel>
    auto utf16_iterator(Iter first, Iter it, Sentinel last)
    {
        auto const unpacked = text::unpack_iterator_and_sentinel(first, last);
        auto const unpacked_it =
            text::unpack_iterator_and_sentinel(it, last).first;
        constexpr format tag = unpacked.format_tag;
        return detail::make_utf16_dispatch<tag>::call(
            unpacked.first, unpacked_it, unpacked.last);
    }

    template<typename Iter, typename Sentinel>
    auto utf32_iterator(Iter first, Iter it, Sentinel last)
    {
        auto const unpacked = text::unpack_iterator_and_sentinel(first, last);
        auto const unpacked_it =
            text::unpack_iterator_and_sentinel(it, last).first;
        constexpr format tag = unpacked.format_tag;
        return detail::make_utf32_dispatch<tag>::call(
            unpacked.first, unpacked_it, unpacked.last);
    }

    template<typename Cont>
    auto from_utf8_inserter(Cont & c, typename Cont::iterator it)
    {
        using result_type = detail::from_utf8_dispatch_t<
            Cont,
            std::insert_iterator<Cont>,
            utf_8_to_16_insert_iterator<Cont>,
            utf_8_to_32_insert_iterator<Cont>>;
        return result_type(c, it);
    }

    template<typename Cont>
    auto from_utf16_inserter(Cont & c, typename Cont::iterator it)
    {
        using result_type = detail::from_utf16_dispatch_t<
            Cont,
            utf_16_to_8_insert_iterator<Cont>,
            std::insert_iterator<Cont>,
            utf_16_to_32_insert_iterator<Cont>>;
        return result_type(c, it);
    }

    template<typename Cont>
    auto from_utf32_inserter(Cont & c, typename Cont::iterator it)
    {
        using result_type = detail::from_utf32_dispatch_t<
            Cont,
            utf_32_to_8_insert_iterator<Cont>,
            utf_32_to_16_insert_iterator<Cont>,
            std::insert_iterator<Cont>>;
        return result_type(c, it);
    }

    template<typename Cont>
    auto from_utf8_back_inserter(Cont & c)
    {
        using result_type = detail::from_utf8_dispatch_t<
            Cont,
            std::back_insert_iterator<Cont>,
            utf_8_to_16_back_insert_iterator<Cont>,
            utf_8_to_32_back_insert_iterator<Cont>>;
        return result_type(c);
    }

    template<typename Cont>
    auto from_utf16_back_inserter(Cont & c)
    {
        using result_type = detail::from_utf16_dispatch_t<
            Cont,
            utf_16_to_8_back_insert_iterator<Cont>,
            std::back_insert_iterator<Cont>,
            utf_16_to_32_back_insert_iterator<Cont>>;
        return result_type(c);
    }

    template<typename Cont>
    auto from_utf32_back_inserter(Cont & c)
    {
        using result_type = detail::from_utf32_dispatch_t<
            Cont,
            utf_32_to_8_back_insert_iterator<Cont>,
            utf_32_to_16_back_insert_iterator<Cont>,
            std::back_insert_iterator<Cont>>;
        return result_type(c);
    }

    template<typename Cont>
    auto from_utf8_front_inserter(Cont & c)
    {
        using result_type = detail::from_utf8_dispatch_t<
            Cont,
            std::front_insert_iterator<Cont>,
            utf_8_to_16_front_insert_iterator<Cont>,
            utf_8_to_32_front_insert_iterator<Cont>>;
        return result_type(c);
    }

    template<typename Cont>
    auto from_utf16_front_inserter(Cont & c)
    {
        using result_type = detail::from_utf16_dispatch_t<
            Cont,
            utf_16_to_8_front_insert_iterator<Cont>,
            std::front_insert_iterator<Cont>,
            utf_16_to_32_front_insert_iterator<Cont>>;
        return result_type(c);
    }

    template<typename Cont>
    auto from_utf32_front_inserter(Cont & c)
    {
        using result_type = detail::from_utf32_dispatch_t<
            Cont,
            utf_32_to_8_front_insert_iterator<Cont>,
            utf_32_to_16_front_insert_iterator<Cont>,
            std::front_insert_iterator<Cont>>;
        return result_type(c);
    }

}}}

#if BOOST_TEXT_USE_CONCEPTS

namespace boost { namespace text { BOOST_TEXT_NAMESPACE_V2 {

    template<std::output_iterator<char8_t> O>
    constexpr utf_32_to_8_out_iterator<O> utf_32_to_8_out(O it)
    {
        return utf_32_to_8_out_iterator<O>(it);
    }

    template<std::output_iterator<char32_t> O>
    constexpr utf_8_to_32_out_iterator<O> utf_8_to_32_out(O it)
    {
        return utf_8_to_32_out_iterator<O>(it);
    }

    template<std::output_iterator<char16_t> O>
    constexpr utf_32_to_16_out_iterator<O> utf_32_to_16_out(O it)
    {
        return utf_32_to_16_out_iterator<O>(it);
    }

    template<std::output_iterator<char32_t> O>
    constexpr utf_16_to_32_out_iterator<O> utf_16_to_32_out(O it)
    {
        return utf_16_to_32_out_iterator<O>(it);
    }

    template<std::output_iterator<char8_t> O>
    constexpr utf_16_to_8_out_iterator<O> utf_16_to_8_out(O it)
    {
        return utf_16_to_8_out_iterator<O>(it);
    }

    template<std::output_iterator<char16_t> O>
    constexpr utf_8_to_16_out_iterator<O> utf_8_to_16_out(O it)
    {
        return utf_8_to_16_out_iterator<O>(it);
    }

    template<std::input_iterator I, std::sentinel_for<I> S>
    constexpr auto utf8_iterator(I first, I it, S last)
    {
        return v1::utf8_iterator(first, it, last);
    }

    template<std::input_iterator I, std::sentinel_for<I> S>
    constexpr auto utf16_iterator(I first, I it, S last)
    {
        return v1::utf16_iterator(first, it, last);
    }

    template<std::input_iterator I, std::sentinel_for<I> S>
    constexpr auto utf32_iterator(I first, I it, S last)
    {
        return v1::utf32_iterator(first, it, last);
    }

    template<typename Cont>
    // clang-format off
        requires requires { typename Cont::value_type; } &&
        utf_code_unit<typename Cont::value_type>
    constexpr auto from_utf8_inserter(Cont & c, typename Cont::iterator it)
    // clang-format on
    {
        if constexpr (sizeof(typename Cont::value_type) == 1) {
            return std::insert_iterator<Cont>(c, it);
        } else if constexpr (sizeof(typename Cont::value_type) == 2) {
            return utf_8_to_16_insert_iterator<Cont>(c, it);
        } else {
            return utf_8_to_32_insert_iterator<Cont>(c, it);
        }
    }

    template<typename Cont>
    // clang-format off
        requires requires { typename Cont::value_type; } &&
        utf_code_unit<typename Cont::value_type>
    constexpr auto from_utf16_inserter(Cont & c, typename Cont::iterator it)
    // clang-format on
    {
        if constexpr (sizeof(typename Cont::value_type) == 1) {
            return utf_16_to_8_insert_iterator<Cont>(c, it);
        } else if constexpr (sizeof(typename Cont::value_type) == 2) {
            return std::insert_iterator<Cont>(c, it);
        } else {
            return utf_16_to_32_insert_iterator<Cont>(c, it);
        }
    }

    template<typename Cont>
    // clang-format off
        requires requires { typename Cont::value_type; } &&
        utf_code_unit<typename Cont::value_type>
    constexpr auto from_utf32_inserter(Cont & c, typename Cont::iterator it)
    // clang-format on
    {
        if constexpr (sizeof(typename Cont::value_type) == 1) {
            return utf_32_to_8_insert_iterator<Cont>(c, it);
        } else if constexpr (sizeof(typename Cont::value_type) == 2) {
            return utf_32_to_16_insert_iterator<Cont>(c, it);
        } else {
            return std::insert_iterator<Cont>(c, it);
        }
    }

    template<typename Cont>
    // clang-format off
        requires requires { typename Cont::value_type; } &&
        utf_code_unit<typename Cont::value_type>
    constexpr auto from_utf8_back_inserter(Cont & c)
    // clang-format on
    {
        if constexpr (sizeof(typename Cont::value_type) == 1) {
            return std::back_insert_iterator<Cont>(c);
        } else if constexpr (sizeof(typename Cont::value_type) == 2) {
            return utf_8_to_16_back_insert_iterator<Cont>(c);
        } else {
            return utf_8_to_32_back_insert_iterator<Cont>(c);
        }
    }

    template<typename Cont>
    // clang-format off
        requires requires { typename Cont::value_type; } &&
        utf_code_unit<typename Cont::value_type>
    constexpr auto from_utf16_back_inserter(Cont & c)
    // clang-format on
    {
        if constexpr (sizeof(typename Cont::value_type) == 1) {
            return utf_16_to_8_back_insert_iterator<Cont>(c);
        } else if constexpr (sizeof(typename Cont::value_type) == 2) {
            return std::back_insert_iterator<Cont>(c);
        } else {
            return utf_16_to_32_back_insert_iterator<Cont>(c);
        }
    }

    template<typename Cont>
    // clang-format off
        requires requires { typename Cont::value_type; } &&
        utf_code_unit<typename Cont::value_type>
    constexpr auto from_utf32_back_inserter(Cont & c)
    // clang-format on
    {
        if constexpr (sizeof(typename Cont::value_type) == 1) {
            return utf_32_to_8_back_insert_iterator<Cont>(c);
        } else if constexpr (sizeof(typename Cont::value_type) == 2) {
            return utf_32_to_16_back_insert_iterator<Cont>(c);
        } else {
            return std::back_insert_iterator<Cont>(c);
        }
    }

    template<typename Cont>
    // clang-format off
        requires requires { typename Cont::value_type; } &&
        utf_code_unit<typename Cont::value_type>
    constexpr auto from_utf8_front_inserter(Cont & c)
    // clang-format on
    {
        if constexpr (sizeof(typename Cont::value_type) == 1) {
            return std::front_insert_iterator<Cont>(c);
        } else if constexpr (sizeof(typename Cont::value_type) == 2) {
            return utf_8_to_16_front_insert_iterator<Cont>(c);
        } else {
            return utf_8_to_32_front_insert_iterator<Cont>(c);
        }
    }

    template<typename Cont>
    // clang-format off
        requires requires { typename Cont::value_type; } &&
        utf_code_unit<typename Cont::value_type>
    constexpr auto from_utf16_front_inserter(Cont & c)
    // clang-format on
    {
        if constexpr (sizeof(typename Cont::value_type) == 1) {
            return utf_16_to_8_front_insert_iterator<Cont>(c);
        } else if constexpr (sizeof(typename Cont::value_type) == 2) {
            return std::front_insert_iterator<Cont>(c);
        } else {
            return utf_16_to_32_front_insert_iterator<Cont>(c);
        }
    }

    template<typename Cont>
    // clang-format off
        requires requires { typename Cont::value_type; } &&
        utf_code_unit<typename Cont::value_type>
    constexpr auto from_utf32_front_inserter(Cont & c)
    // clang-format on
    {
        if constexpr (sizeof(typename Cont::value_type) == 1) {
            return utf_32_to_8_front_insert_iterator<Cont>(c);
        } else if constexpr (sizeof(typename Cont::value_type) == 2) {
            return utf_32_to_16_front_insert_iterator<Cont>(c);
        } else {
            return std::front_insert_iterator<Cont>(c);
        }
    }

    namespace dtl {
        template<format Format>
        constexpr auto format_to_type()
        {
            if constexpr (Format == format::utf8) {
                return char8_t{};
            } else if constexpr (Format == format::utf16) {
                return char16_t{};
            } else {
                return char32_t{};
            }
        }
        template<
            typename I,
            bool SupportReverse = std::bidirectional_iterator<I>>
        struct first_and_curr
        {
            first_and_curr() = default;
            first_and_curr(I curr) : curr{curr} {}
            first_and_curr(const first_and_curr & other) = default;
            template<class I2>
            requires std::convertible_to<I2, I>
            first_and_curr(const first_and_curr<I2> & other) : curr{other.curr}
            {}

            I curr;
        };
        template<typename I>
        struct first_and_curr<I, true>
        {
            first_and_curr() = default;
            first_and_curr(I first, I curr) : first{first}, curr{curr} {}
            first_and_curr(const first_and_curr & other) = default;
            template<class I2>
            requires std::convertible_to<I2, I>
            first_and_curr(const first_and_curr<I2> & other) :
                first{other.first}, curr{other.curr}
            {}

            I first;
            I curr;
        };
    }

    template<
        format FromFormat,
        format ToFormat,
        utf_iter I,
        std::sentinel_for<I> S,
        transcoding_error_handler ErrorHandler>
    class utf_iterator
        : public stl_interfaces::iterator_interface<
              utf_iterator<FromFormat, ToFormat, I, S, ErrorHandler>,
              decltype(detail::bidirectional_at_most<I>()),
              decltype(dtl::format_to_type<ToFormat>()),
              decltype(dtl::format_to_type<ToFormat>())>
    {
        static_assert(
            FromFormat == format::utf8 || FromFormat == format::utf16 ||
            FromFormat == format::utf32);
        static_assert(
            ToFormat == format::utf8 || ToFormat == format::utf16 ||
            ToFormat == format::utf32);

    public:
        using value_type = decltype(dtl::format_to_type<ToFormat>());

        constexpr utf_iterator() = default;

        constexpr utf_iterator(
            I first, I it, S last) requires std::bidirectional_iterator<I>
            : first_and_curr_{first, it}, last_(last)
        {
            if (curr() != last_)
                read();
        }
        constexpr utf_iterator(I it, S last) requires(
            !std::bidirectional_iterator<I>) :
            first_and_curr_{it}, last_(last)
        {
            if (curr() != last_)
                read();
        }

        template<class I2, class S2>
        requires std::convertible_to<I2, I> && std::convertible_to<S2, S>
        constexpr utf_iterator(
            utf_iterator<FromFormat, ToFormat, I2, S2, ErrorHandler> const &
                other) :
            buf_(other.buf_),
            first_and_curr_(other.first_and_curr_),
            buf_index_(other.buf_index_),
            buf_last_(other.buf_last_),
            last_(other.last_)
        {
            if (curr() != last_)
                read();
        }

        constexpr I begin() const requires std::bidirectional_iterator<I> { return first(); }
        constexpr S end() const { return last_; }

        constexpr I base() const { return curr(); }

        constexpr value_type operator*() const
        {
            BOOST_ASSERT(buf_index_ < buf_last_);
            return buf_[buf_index_];
        }

        constexpr utf_iterator & operator++()
        {
            BOOST_ASSERT(buf_index_ != buf_last_ || curr() != last_);
            if (buf_index_ + 1 == buf_last_ && curr() != last_)
                read();
            else
                ++buf_index_;
            return *this;
        }

        constexpr utf_iterator &
        operator--() requires std::bidirectional_iterator<I>
        {
            BOOST_ASSERT(buf_index_ || curr() != first());
            if (!buf_index_ && curr() != first())
                read_reverse();
            else
                --buf_index_;
            return *this;
        }

        friend constexpr bool operator==(utf_iterator lhs, utf_iterator rhs)
        {
            if (lhs.curr() != rhs.curr())
                return false;

            if (lhs.buf_index_ == rhs.buf_index_ &&
                lhs.buf_last_ == rhs.buf_last_) {
                return true;
            }

            return lhs.buf_index_ == lhs.buf_last_ &&
                   rhs.buf_index_ == rhs.buf_last_;
        }

        friend constexpr bool
        operator==(utf_iterator lhs, S rhs) requires(!std::same_as<I, S>)
        {
            return lhs.curr() == rhs && lhs.buf_index_ == lhs.buf_last_;
        }

        // exposition only
        using base_type = stl_interfaces::iterator_interface<
            utf_iterator<FromFormat, ToFormat, I, S, ErrorHandler>,
            decltype(detail::bidirectional_at_most<I>()),
            value_type,
            value_type>;
        using base_type::operator++;
        using base_type::operator--;

    private:
        constexpr char32_t decode_code_point()
        {
            if constexpr (FromFormat == format::utf8) {
                char32_t cp = *curr();
                ++curr();
                if (cp < 0x80)
                    return cp;

                // clang-format off

                // It turns out that this naive implementation is faster than
                // the table implementation for the converting iterators.

            /*
                Unicode 3.9/D92
                Table 3-7. Well-Formed UTF-8 Byte Sequences

                Code Points        First Byte Second Byte Third Byte Fourth Byte
                ===========        ========== =========== ========== ===========
                U+0000..U+007F     00..7F
                U+0080..U+07FF     C2..DF     80..BF
                U+0800..U+0FFF     E0         A0..BF      80..BF
                U+1000..U+CFFF     E1..EC     80..BF      80..BF
                U+D000..U+D7FF     ED         80..9F      80..BF
                U+E000..U+FFFF     EE..EF     80..BF      80..BF
                U+10000..U+3FFFF   F0         90..BF      80..BF     80..BF
                U+40000..U+FFFFF   F1..F3     80..BF      80..BF     80..BF
                U+100000..U+10FFFF F4         80..8F      80..BF     80..BF
            */
                // clang-format on

                char8_t curr_c = cp;

                auto error = [&]() {
                    return ErrorHandler{}("Ill-formed UTF-8.");
                };

                // One-byte case handled above

                // Two-byte
                if (detail::in(0xc2, curr_c, 0xdf)) {
                    cp = curr_c & 0b00011111;
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0x80, curr_c, 0xbf))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                    // Three-byte
                } else if (curr_c == 0xe0) {
                    cp = curr_c & 0b00001111;
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0xa0, curr_c, 0xbf))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0x80, curr_c, 0xbf))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                } else if (detail::in(0xe1, curr_c, 0xec)) {
                    cp = curr_c & 0b00001111;
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0x80, curr_c, 0xbf))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0x80, curr_c, 0xbf))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                } else if (curr_c == 0xed) {
                    cp = curr_c & 0b00001111;
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0x80, curr_c, 0x9f))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0x80, curr_c, 0xbf))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                } else if (detail::in(0xee, curr_c, 0xef)) {
                    cp = curr_c & 0b00001111;
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0x80, curr_c, 0xbf))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0x80, curr_c, 0xbf))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                    // Four-byte
                } else if (curr_c == 0xf0) {
                    cp = curr_c & 0b00000111;
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0x90, curr_c, 0xbf))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0x80, curr_c, 0xbf))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0x80, curr_c, 0xbf))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                } else if (detail::in(0xf1, curr_c, 0xf3)) {
                    cp = curr_c & 0b00000111;
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0x80, curr_c, 0xbf))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0x80, curr_c, 0xbf))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0x80, curr_c, 0xbf))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                } else if (curr_c == 0xf4) {
                    cp = curr_c & 0b00000111;
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0x80, curr_c, 0x8f))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0x80, curr_c, 0xbf))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                    if (curr() == last_)
                        return error();
                    curr_c = *curr();
                    if (!detail::in(0x80, curr_c, 0xbf))
                        return error();
                    cp = (cp << 6) + (curr_c & 0b00111111);
                    ++curr();
                } else {
                    return error();
                }
                return cp;
            } else if constexpr (FromFormat == format::utf16) {
                char16_t hi = *curr();
                ++curr();
                if (!boost::text::surrogate(hi))
                    return hi;

                if (boost::text::low_surrogate(hi)) {
                    return ErrorHandler{}(
                        "Invalid UTF-16 sequence; lone trailing surrogate.");
                }

                // high surrogate
                if (curr() == last_) {
                    return ErrorHandler{}(
                        "Invalid UTF-16 sequence; lone leading surrogate.");
                }

                char16_t lo = *curr();
                ++curr();
                if (!boost::text::low_surrogate(lo)) {
                    return ErrorHandler{}(
                        "Invalid UTF-16 sequence; lone leading surrogate.");
                }

                return char32_t((hi - high_surrogate_base) << 10) +
                       (lo - low_surrogate_base);
            } else {
                char32_t retval = *curr();
                ++curr();
                return retval;
            }
        }

        constexpr char32_t
        decode_code_point_reverse() requires std::bidirectional_iterator<I>
        {
            if constexpr (FromFormat == format::utf8) {
                if (buf_index_ != buf_last_ || curr() != last_)
                    curr() = detail::decrement(first(), curr());

                curr() = detail::decrement(first(), curr());
                return decode_code_point();
            } else if constexpr (FromFormat == format::utf16) {
                if (buf_index_ != buf_last_ || curr() != last_) {
                    if (boost::text::low_surrogate(*--curr())) {
                        if (curr() != first() &&
                            boost::text::high_surrogate(*std::prev(curr()))) {
                            --curr();
                        }
                    }
                }

                auto prev = curr();

                char16_t lo = *--prev;
                if (!boost::text::surrogate(lo))
                    return lo;

                if (boost::text::high_surrogate(lo)) {
                    return ErrorHandler{}(
                        "Invalid UTF-16 sequence; lone leading surrogate.");
                }

                // low surrogate
                if (prev == first()) {
                    return ErrorHandler{}(
                        "Invalid UTF-16 sequence; lone trailing surrogate.");
                }

                char16_t hi = *--prev;
                if (!boost::text::high_surrogate(hi)) {
                    return ErrorHandler{}(
                        "Invalid UTF-16 sequence; lone trailing surrogate.");
                }

                return char32_t((hi - high_surrogate_base) << 10) +
                       (lo - low_surrogate_base);
            } else {
                if (buf_index_ != buf_last_ || curr() != last_)
                    --curr();

                return *std::prev(curr());
            }
        }

        template<class Out>
        constexpr Out encode_code_point(char32_t cp, Out out)
        {
            if constexpr (ToFormat == format::utf8) {
                if (cp < 0x80) {
                    *out++ = static_cast<char8_t>(cp);
                } else if (cp < 0x800) {
                    *out++ = static_cast<char8_t>(0xC0 + (cp >> 6));
                    *out++ = static_cast<char8_t>(0x80 + (cp & 0x3f));
                } else if (cp < 0x10000) {
                    *out++ = static_cast<char8_t>(0xe0 + (cp >> 12));
                    *out++ = static_cast<char8_t>(0x80 + ((cp >> 6) & 0x3f));
                    *out++ = static_cast<char8_t>(0x80 + (cp & 0x3f));
                } else {
                    *out++ = static_cast<char8_t>(0xf0 + (cp >> 18));
                    *out++ = static_cast<char8_t>(0x80 + ((cp >> 12) & 0x3f));
                    *out++ = static_cast<char8_t>(0x80 + ((cp >> 6) & 0x3f));
                    *out++ = static_cast<char8_t>(0x80 + (cp & 0x3f));
                }
            } else if constexpr (ToFormat == format::utf16) {
                if (cp < 0x10000) {
                    *out++ = static_cast<char16_t>(cp);
                } else {
                    *out++ =
                        static_cast<char16_t>(cp >> 10) + high_surrogate_base;
                    *out++ =
                        static_cast<char16_t>(cp & 0x3ff) + low_surrogate_base;
                }
            } else {
                *out++ = cp;
            }
            return out;
        }

        constexpr void read()
        {
            if constexpr (noexcept(ErrorHandler{}(""))) {
                char32_t cp = decode_code_point();
                auto it = encode_code_point(cp, buf_.begin());
                buf_index_ = 0;
                buf_last_ = it - buf_.begin();
            } else {
                auto curr = curr();
                auto buf = buf_;
                try {
                    char32_t cp = decode_code_point();
                    auto it = encode_code_point(cp, buf_.begin());
                    buf_index_ = 0;
                    buf_last_ = it - buf_.begin();
                } catch (...) {
                    buf_ = buf_;
                    curr() = curr;
                    throw;
                }
            }
        }

        constexpr void read_reverse()
        {
            if constexpr (noexcept(ErrorHandler{}(""))) {
                char32_t cp = decode_code_point_reverse();
                auto it = encode_code_point(cp, buf_.begin());
                buf_last_ = it - buf_.begin();
                buf_index_ = buf_last_ - 1;
            } else {
                auto curr = curr();
                auto buf = buf_;
                try {
                    char32_t cp = decode_code_point_reverse();
                    auto it = encode_code_point(cp, buf_.begin());
                    buf_last_ = it - buf_.begin();
                    buf_index_ = buf_last_ - 1;
                } catch (...) {
                    buf_ = buf_;
                    curr() = curr;
                    throw;
                }
            }
        }

        constexpr I first() const requires std::bidirectional_iterator<I>
        {
            return first_and_curr_.first;
        }
        constexpr I & curr() { return first_and_curr_.curr; }
        constexpr I curr() const { return first_and_curr_.curr; }

        std::array<value_type, 4 / static_cast<int>(ToFormat)> buf_;

        dtl::first_and_curr<I> first_and_curr_;

        uint8_t buf_index_ = 0;
        uint8_t buf_last_ = 0;

        [[no_unique_address]] S last_;

        template<
            format FromFormat2,
            format ToFormat2,
            utf_iter I2,
            std::sentinel_for<I2> S2,
            transcoding_error_handler ErrorHandler2>
        friend class utf_iterator;

        template<
            typename RepackedIterator,
            typename I2,
            typename S2,
            typename Then>
        friend struct boost::text::detail::bidi_repacker;
    };

}}}

#endif

#endif
