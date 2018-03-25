#ifndef BOOST_TEXT_UTF8_HPP
#define BOOST_TEXT_UTF8_HPP

#include <boost/text/config.hpp>

#include <iterator>
#include <type_traits>
#include <stdexcept>

#include <cassert>


namespace boost { namespace text { namespace utf8 {

    namespace detail {

        constexpr bool
        in(unsigned char lo, unsigned char c, unsigned char hi) noexcept
        {
            return lo <= c && c <= hi;
        }

        /** A tag type used to instantiate some of the converting iterators. The
            use of this type indicates that the iterator should throw upon
            detecting an invalid Unicode encoding. */
        struct throw_on_encoding_error
        {};
    }

    /** A tag type passed to some constructors to indicate that no UTF-8
        encoding checks should be performed. */
    struct unchecked_t
    {};

    namespace {

        /** A value of unchecked_t that you can use when needed. */
        constexpr unchecked_t unchecked{};
    }

    /** The replacement character used to mark invalid portions of a Unicode
        sequence when converting between two encodings.

        \see Unicode 9, 3.2/C10 */
    constexpr uint32_t replacement_character() noexcept { return 0xfffd; }

    /** Given the first (and possibly only) code unit of a UTF-8 code point,
        returns the number of bytes occupied by that code point (in the range
        [1, 4]).  Returns a value < 0 if first is not a valid initial UTF-8
        code unit.

        This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR int
    code_point_bytes(unsigned char first) noexcept
    {
        using detail::in;
        if (first <= 0x7f)
            return 1;
        if (in(0xc2, first, 0xdf))
            return 2;
        if (in(0xe0, first, 0xef))
            return 3;
        if (in(0xf0, first, 0xf4))
            return 4;
        return -1;
    }

    /** Returns true if c is a UTF-8 continuation code unit, and false
        otherwise.  If optional parameters lo and hi are given, the code unit
        must also lie in the range [lo, hi]. */
    constexpr bool continuation(
        unsigned char c,
        unsigned char lo = 0x80,
        unsigned char hi = 0xbf) noexcept
    {
        return detail::in(lo, c, hi);
    }

    namespace detail {

        // Follow Table 3-7 in Unicode 9, 3.9/D92
        template<typename Iter>
        BOOST_TEXT_CXX14_CONSTEXPR Iter end_of_invalid_utf8(Iter it) noexcept
        {
            assert(!continuation(*it));

            using detail::in;

            if (in(0, *it, 0x7f))
                return nullptr;

            if (in(0xc2, *it, 0xdf)) {
                if (!continuation(*(it + 1)))
                    return it + 1;
                return nullptr;
            }

            if (in(0xe0, *it, 0xe0)) {
                if (!continuation(*(it + 1), 0xa0, 0xbf))
                    return it + 1;
                if (!continuation(*(it + 2)))
                    return it + 2;
                return nullptr;
            }
            if (in(0xe1, *it, 0xec)) {
                if (!continuation(*(it + 1)))
                    return it + 1;
                if (!continuation(*(it + 2)))
                    return it + 2;
                return nullptr;
            }
            if (in(0xed, *it, 0xed)) {
                if (!continuation(*(it + 1), 0x80, 0x9f))
                    return it + 1;
                if (!continuation(*(it + 2)))
                    return it + 2;
                return nullptr;
            }
            if (in(0xee, *it, 0xef)) {
                if (!continuation(*(it + 1)))
                    return it + 1;
                if (!continuation(*(it + 2)))
                    return it + 2;
                return nullptr;
            }

            if (in(0xf0, *it, 0xf0)) {
                if (!continuation(*(it + 1), 0x90, 0xbf))
                    return it + 1;
                if (!continuation(*(it + 2)))
                    return it + 2;
                if (!continuation(*(it + 3)))
                    return it + 3;
                return nullptr;
            }
            if (in(0xf1, *it, 0xf3)) {
                if (!continuation(*(it + 1)))
                    return it + 1;
                if (!continuation(*(it + 2)))
                    return it + 2;
                if (!continuation(*(it + 3)))
                    return it + 3;
                return nullptr;
            }
            if (in(0xf4, *it, 0xf4)) {
                if (!continuation(*(it + 1), 0x80, 0x8f))
                    return it + 1;
                if (!continuation(*(it + 2)))
                    return it + 2;
                if (!continuation(*(it + 3)))
                    return it + 3;
                return nullptr;
            }

            return it;
        }

        template<typename Iter>
        BOOST_TEXT_CXX14_CONSTEXPR Iter decrement(Iter it) noexcept
        {
            Iter retval = it;

            int backup = 0;
            while (backup < 4 && continuation(*--retval)) {
                ++backup;
            }
            backup = it - retval;

            if (continuation(*retval))
                return it - 1;

            Iter first_invalid = end_of_invalid_utf8(retval);
            if (first_invalid == retval)
                ++first_invalid;
            while (first_invalid && (first_invalid - retval) < backup) {
                backup -= first_invalid - retval;
                retval = first_invalid;
                first_invalid = end_of_invalid_utf8(retval);
                if (first_invalid == retval)
                    ++first_invalid;
            }

            if (1 < backup) {
                int const cp_bytes = code_point_bytes(*retval);
                if (cp_bytes < backup)
                    retval = it - 1;
            }

            return retval;
        }

        template<typename Iter>
        BOOST_TEXT_CXX14_CONSTEXPR Iter decrement(Iter first, Iter it) noexcept
        {
            Iter retval = it;

            int backup = 0;
            while (backup < 4 && it != first && continuation(*--retval)) {
                ++backup;
            }
            backup = it - retval;

            if (continuation(*retval)) {
                if (it != first)
                    --it;
                return it;
            }

            Iter first_invalid = end_of_invalid_utf8(retval);
            if (first_invalid == retval)
                ++first_invalid;
            while (first_invalid && (first_invalid - retval) < backup) {
                backup -= first_invalid - retval;
                retval = first_invalid;
                first_invalid = end_of_invalid_utf8(retval);
                if (first_invalid == retval)
                    ++first_invalid;
            }

            if (1 < backup) {
                int const cp_bytes = code_point_bytes(*retval);
                if (cp_bytes < backup) {
                    if (it != first)
                        --it;
                    retval = it;
                }
            }

            return retval;
        }
    }

    /** Returns the first code unit in [first, last) that is not properly
        UTF-8 encoded, or last if no such code unit is found.

        This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR char const *
    find_invalid_encoding(char const * first, char const * last) noexcept
    {
        while (first != last) {
            int const cp_bytes = code_point_bytes(*first);
            if (cp_bytes == -1 || last - first < cp_bytes)
                return first;

            if (detail::end_of_invalid_utf8(first))
                return first;

            first += cp_bytes;
        }

        return last;
    }

    /** Returns true if [first, last) is properly UTF-8 encoded, or false
        otherwise.

        This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    encoded(char const * first, char const * last) noexcept
    {
        return find_invalid_encoding(first, last) == last;
    }

    /** Returns true if [first, last) is empty or the initial code units in
        [first, last) form a valid Unicode code point, or false otherwise.

        This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    starts_encoded(char const * first, char const * last) noexcept
    {
        if (first == last)
            return true;

        int const cp_bytes = code_point_bytes(*first);
        if (cp_bytes == -1 || last - first < cp_bytes)
            return false;

        return detail::end_of_invalid_utf8(first) == nullptr;
    }

    /** Returns true if [first, last) is empty or the final code units in
        [first, last) form a valid Unicode code point, or false otherwise.

        This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    ends_encoded(char const * first, char const * last) noexcept
    {
        if (first == last)
            return true;

        auto it = last;
        while (first != --it && continuation(*it))
            ;

        return starts_encoded(it, last);
    }

    /** Returns true if [first, last) is empty or the initial code units in
        [first, last) form a valid Unicode code point, or false otherwise. */
    template<typename Iter>
    bool starts_encoded(Iter first, Iter last) noexcept
    {
        if (first == last)
            return true;

        int const cp_bytes = code_point_bytes(*first);
        if (cp_bytes == -1 || last - first < cp_bytes)
            return false;

        char buf[5] = {0};
        for (int i = 0; i < (int)sizeof(buf); ++i) {
            buf[i] = *first;
            if (++first == last)
                break;
        }

        return detail::end_of_invalid_utf8(buf) == nullptr;
    }

    /** Returns true if [first, last) is empty or the final code units in
        [first, last) form a valid Unicode code point, or false otherwise. */
    template<typename Iter>
    bool ends_encoded(Iter first, Iter last) noexcept
    {
        if (first == last)
            return true;

        auto it = last;
        while (first != --it && continuation(*it))
            ;

        return starts_encoded(it, last);
    }

    /** Returns true if c is a Unicode surrogate, or false otherwise.

        This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool surrogate(uint32_t c) noexcept
    {
        uint32_t const high_surrogate_min = 0xd800;
        uint32_t const low_surrogate_max = 0xdfff;
        return high_surrogate_min <= c && c <= low_surrogate_max;
    }

    /** Returns true if c is a Unicode reserved noncharacter, or false
        otherwise.

        This function is constexpr in C++14 and later.

        \see Unicode 9, 3.4/D14 */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    reserved_noncharacter(uint32_t c) noexcept
    {
        bool const byte01_reserved =
            (c & 0xffff) == 0xffff || (c & 0xffff) == 0xfffe;
        bool const byte2_at_most_0x10 = ((c & 0xff0000u) >> 16) <= 0x10;
        return (byte01_reserved && byte2_at_most_0x10) ||
               (0xfdd0 <= c && c <= 0xfdef);
    }

    /** Returns true if c is a valid Unicode code point, or false otherwise.

        This function is constexpr in C++14 and later.

        \see Unicode 9, 3.9/D90 */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool valid_code_point(uint32_t c) noexcept
    {
        return c <= 0x10ffff && !surrogate(c) && !reserved_noncharacter(c);
    }

    /** An error handler type that can be used with the converting iterators;
        provides the Unicode replacement character on errors. */
    struct use_replacement_character
    {
        constexpr uint32_t operator()(char const *) noexcept
        {
            return replacement_character();
        }
    };

    /** An error handler type that can be used with the converting iterators;
        throws std::logic_error on errors. */
    struct throw_logic_error
    {
        uint32_t operator()(char const * msg)
        {
            throw std::logic_error(msg);
            return 0;
        }
    };


    /** A sentinel type that compares equal to a to_utf32_iterator or
        to_utf16_iterator when the iterator points to a 0. */
    struct null_sentinel
    {};


    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator==(char const * p, null_sentinel)
    {
        return p == nullptr;
    }
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator!=(char const * p, null_sentinel)
    {
        return p != nullptr;
    }

    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator==(null_sentinel, char const * p)
    {
        return p == nullptr;
    }
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator!=(null_sentinel, char const * p)
    {
        return p != nullptr;
    }


    /** A UTF-32 to UTF-8 converting iterator.  Set the ErrorHandler template
        parameter to control error handling.  The default ErrorHandler is
        use_replacement_character, which simply produces a replacement
        character.

        Iter must be a bidirectional iterator with a 4-byte integral
        value_type. */
    template<typename Iter, typename ErrorHandler = use_replacement_character>
    struct from_utf32_iterator
    {
        using value_type = char;
        using difference_type = int;
        using pointer = char *;
        using reference = char;
        using iterator_category = std::bidirectional_iterator_tag;

        static bool const throw_on_error =
            noexcept(std::declval<ErrorHandler>()(0));

        static_assert(
            std::is_same<
                typename std::iterator_traits<Iter>::iterator_category,
                std::bidirectional_iterator_tag>::value ||
                std::is_same<
                    typename std::iterator_traits<Iter>::iterator_category,
                    std::random_access_iterator_tag>::value,
            "from_utf32_iterator requires its Iter parameter to be at least "
            "bidirectional.");
        static_assert(
            sizeof(typename std::iterator_traits<Iter>::value_type) == 4,
            "from_utf32_iterator requires its Iter parameter to produce a "
            "4-byte value_type.");

        constexpr from_utf32_iterator() noexcept :
            it_(),
            index_(4),
            buf_(),
            partial_decrement_(false)
        {}
        explicit constexpr from_utf32_iterator(Iter it) noexcept :
            it_(it),
            index_(4),
            buf_(),
            partial_decrement_(false)
        {}

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR reference operator*() const
            noexcept(!throw_on_error)
        {
            if (buf_empty()) {
                index_ = read_into_buf();
                if (!partial_decrement_)
                    index_ = 0;
            }
            partial_decrement_ = false;
            return buf_[index_];
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR Iter base() const noexcept { return it_; }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR from_utf32_iterator &
        operator++() noexcept(!throw_on_error)
        {
            if (partial_decrement_) {
                ++it_;
            } else {
                if (buf_empty()) {
                    read_into_buf();
                    index_ = 0;
                }
                ++index_;
                if (at_buf_end()) {
                    ++it_;
                    index_ = 4;
                }
            }
            partial_decrement_ = false;
            return *this;
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR from_utf32_iterator
        operator++(int)noexcept(!throw_on_error)
        {
            from_utf32_iterator retval = *this;
            ++*this;
            return retval;
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR from_utf32_iterator &
        operator--() noexcept(!throw_on_error)
        {
            if (index_ == 0 || buf_empty()) {
                --it_;
                index_ = 4;
                partial_decrement_ = true;
            } else {
                --index_;
            }
            return *this;
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR from_utf32_iterator
        operator--(int)noexcept(!throw_on_error)
        {
            from_utf32_iterator retval = *this;
            --*this;
            return retval;
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool operator==(
            from_utf32_iterator<Iter> lhs,
            from_utf32_iterator<Iter> rhs) noexcept
        {
            if (lhs.it_ != rhs.it_)
                return false;
            return lhs.index_ == rhs.index_ ||
                   ((lhs.index_ == 0 || lhs.index_ == 4) &&
                    (rhs.index_ == 0 || rhs.index_ == 4));
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool operator!=(
            from_utf32_iterator<Iter> lhs,
            from_utf32_iterator<Iter> rhs) noexcept
        {
            return !(lhs.it_ == rhs.it_);
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        constexpr bool buf_empty() const noexcept { return index_ == 4; }

        constexpr bool at_buf_end() const noexcept
        {
            return buf_[index_] == '\0';
        }

        BOOST_TEXT_CXX14_CONSTEXPR int read_into_buf() const
            noexcept(!throw_on_error)
        {
            uint32_t c = static_cast<uint32_t>(*it_);
            if (!valid_code_point(c))
                c = ErrorHandler{}("Invalid UTF-32 code point.");
            index_ = 0;
            if (c < 0x80) {
                buf_[0] = static_cast<char>(c);
                buf_[1] = '\0';
                return 0;
            } else if (c < 0x800) {
                buf_[0] = static_cast<char>(0xC0 + (c >> 6));
                buf_[1] = static_cast<char>(0x80 + (c & 0x3f));
                buf_[2] = '\0';
                return 1;
            } else if (c < 0x10000) {
                buf_[0] = static_cast<char>(0xe0 + (c >> 12));
                buf_[1] = static_cast<char>(0x80 + ((c >> 6) & 0x3f));
                buf_[2] = static_cast<char>(0x80 + (c & 0x3f));
                buf_[3] = '\0';
                return 2;
            } else {
                buf_[0] = static_cast<char>(0xf0 + (c >> 18));
                buf_[1] = static_cast<char>(0x80 + ((c >> 12) & 0x3f));
                buf_[2] = static_cast<char>(0x80 + ((c >> 6) & 0x3f));
                buf_[3] = static_cast<char>(0x80 + (c & 0x3f));
                buf_[4] = '\0';
                return 3;
            }
        }

        Iter it_;
        mutable int index_;
        mutable char buf_[5];
        mutable bool partial_decrement_;
#endif
    };

    /** Returns a from_utf32_iterator<Iter> constructed from an Iter. */
    template<typename Iter>
    from_utf32_iterator<Iter> make_from_utf32_iterator(Iter it)
    {
        return from_utf32_iterator<Iter>(it);
    }


    /** A UTF-8 to UTF-32 converting iterator.  Set the ErrorHandler template
        parameter to control error handling.  The default ErrorHandler is
        use_replacement_character, which simply produces a replacement
        character.

        Iter must be a bidirectional iterator with a 1-byte char
        value_type. */
    template<
        typename Iter,
        typename Sentinel,
        typename ErrorHandler = use_replacement_character>
    struct to_utf32_iterator
    {
        using value_type = uint32_t;
        using difference_type = int;
        using pointer = uint32_t *;
        using reference = uint32_t;
        using iterator_category = std::bidirectional_iterator_tag;

        static bool const throw_on_error =
            noexcept(std::declval<ErrorHandler>()(0));

        constexpr to_utf32_iterator() noexcept : first_(), it_(), last_() {}
        explicit constexpr to_utf32_iterator(
            Iter first, Iter it, Sentinel last) noexcept :
            first_(first),
            it_(it),
            last_(last)
        {}

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR reference operator*() const
            noexcept(!throw_on_error)
        {
            return get_value().value_;
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR Iter base() const noexcept { return it_; }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR to_utf32_iterator &
        operator++() noexcept(!throw_on_error)
        {
            it_ = increment();
            return *this;
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR to_utf32_iterator
        operator++(int)noexcept(!throw_on_error)
        {
            to_utf32_iterator retval = *this;
            ++*this;
            return retval;
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR to_utf32_iterator &
        operator--() noexcept(!throw_on_error)
        {
            it_ = detail::decrement(first_, it_);
            return *this;
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR to_utf32_iterator
        operator--(int)noexcept(!throw_on_error)
        {
            to_utf32_iterator retval = *this;
            --*this;
            return retval;
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator==(to_utf32_iterator lhs, to_utf32_iterator rhs) noexcept
        {
            return lhs.it_ == rhs.it_;
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator!=(to_utf32_iterator lhs, to_utf32_iterator rhs) noexcept
        {
            return lhs.it_ != rhs.it_;
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator==(to_utf32_iterator lhs, null_sentinel rhs) noexcept
        {
            return !*lhs.it_;
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator==(null_sentinel lhs, to_utf32_iterator rhs) noexcept
        {
            return rhs == lhs;
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator!=(to_utf32_iterator lhs, null_sentinel rhs) noexcept
        {
            return !(lhs == rhs);
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator!=(null_sentinel lhs, to_utf32_iterator rhs) noexcept
        {
            return !(rhs == lhs);
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        struct get_value_result
        {
            reference value_;
            Iter it_;
        };

        BOOST_TEXT_CXX14_CONSTEXPR bool check_continuation(
            unsigned char c,
            unsigned char lo = 0x80,
            unsigned char hi = 0xbf) const noexcept(!throw_on_error)
        {
            if (continuation(c, lo, hi)) {
                return true;
            } else {
                ErrorHandler{}(
                    "Invalid UTF-8 sequence; an expected continuation "
                    "character is missing.");
                return false;
            }
        }

        BOOST_TEXT_CXX14_CONSTEXPR bool at_end(Iter it) const
            noexcept(!throw_on_error)
        {
            if (it == last_) {
                ErrorHandler{}(
                    "Invalid UTF-8 sequence; another code point before the end "
                    "of string.");
                return true;
            } else {
                return false;
            }
        }

        BOOST_TEXT_CXX14_CONSTEXPR get_value_result get_value() const
            noexcept(!throw_on_error)
        {
            /*
                Unicode 9, 3.9/D92
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

            uint32_t value = 0;
            Iter next = it_;

            if (at_end(next))
                return get_value_result{replacement_character(), next};

            unsigned char curr_c = *next;

            using detail::in;


            // One-byte
            if (curr_c <= 0x7f) {
                value = curr_c;
                ++next;
                // Two-byte
            } else if (in(0xc2, curr_c, 0xdf)) {
                value = curr_c & 0b00011111;
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
                // Three-byte
            } else if (curr_c == 0xe0) {
                value = curr_c & 0b00001111;
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c, 0xa0, 0xbf))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
            } else if (in(0xe1, curr_c, 0xec)) {
                value = curr_c & 0b00001111;
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
            } else if (curr_c == 0xed) {
                value = curr_c & 0b00001111;
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c, 0x80, 0x9f))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
            } else if (in(0xed, curr_c, 0xef)) {
                value = curr_c & 0b00001111;
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
                // Four-byte
            } else if (curr_c == 0xf0) {
                value = curr_c & 0b00000111;
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c, 0x90, 0xbf))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
            } else if (in(0xf1, curr_c, 0xf3)) {
                value = curr_c & 0b00000111;
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
            } else if (curr_c == 0xf4) {
                value = curr_c & 0b00000111;
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c, 0x80, 0x8f))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
                if (at_end(next))
                    return get_value_result{replacement_character(), next};
                curr_c = *next;
                if (!check_continuation(curr_c))
                    return get_value_result{replacement_character(), next};
                value = (value << 6) + (curr_c & 0b00111111);
                ++next;
            } else {
                value = ErrorHandler{}("Invalid initial UTF-8 character.");
                ++next;
            }

            if (!valid_code_point(value)) {
                value = ErrorHandler{}(
                    "UTF-8 sequence results in invalid UTF-32 code point.");
            }

            return get_value_result{value, next};
        }

        BOOST_TEXT_CXX14_CONSTEXPR Iter increment() const
            noexcept(!throw_on_error)
        {
            return get_value().it_;
        }

        Iter first_;
        Iter it_;
        Sentinel last_;
#endif
    };

    /** Returns a to_utf32_iterator<Iter> constructed from an Iter. */
    template<typename Iter, typename Sentinel>
    to_utf32_iterator<Iter, Sentinel>
    make_to_utf32_iterator(Iter first, Iter it, Sentinel last)
    {
        return to_utf32_iterator<Iter, Sentinel>(it);
    }


    /** A UTF-16 to UTF-8 converting iterator.  Set the ErrorHandler template
        parameter to control error handling.  The default ErrorHandler is
        use_replacement_character, which simply produces a replacement
        character.

        Iter must be a bidirectional iterator with a 2-byte integral
        value_type. */
    template<typename Iter, typename ErrorHandler = use_replacement_character>
    struct from_utf16_iterator
    {
        using value_type = char;
        using difference_type = int;
        using pointer = char *;
        using reference = char;
        using iterator_category = std::bidirectional_iterator_tag;

        static bool const throw_on_error =
            noexcept(std::declval<ErrorHandler>()(0));

        static_assert(
            std::is_same<
                typename std::iterator_traits<Iter>::iterator_category,
                std::bidirectional_iterator_tag>::value ||
                std::is_same<
                    typename std::iterator_traits<Iter>::iterator_category,
                    std::random_access_iterator_tag>::value,
            "from_utf16_iterator requires its Iter parameter to be at least "
            "bidirectional.");
        static_assert(
            sizeof(typename std::iterator_traits<Iter>::value_type) == 2,
            "from_utf16_iterator requires its Iter parameter to produce a "
            "2-byte value_type.");

        constexpr from_utf16_iterator() noexcept :
            it_(),
            next_(),
            index_(4),
            buf_(),
            partial_decrement_(false)
        {}
        explicit constexpr from_utf16_iterator(Iter it) noexcept :
            it_(it),
            next_(it),
            index_(4),
            buf_(),
            partial_decrement_(false)
        {}

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR reference operator*() const
            noexcept(!throw_on_error)
        {
            if (buf_empty()) {
                if (partial_decrement_)
                    index_ = decr_read_into_buf();
                else
                    index_ = incr_read_into_buf();
            }
            partial_decrement_ = false;
            return buf_[index_];
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR Iter base() const noexcept { return it_; }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR from_utf16_iterator &
        operator++() noexcept(!throw_on_error)
        {
            if (partial_decrement_) {
                ++it_;
            } else {
                if (buf_empty())
                    index_ = incr_read_into_buf();
                ++index_;
                if (at_buf_end()) {
                    if (it_ == next_)
                        incr_read_into_buf();
                    it_ = next_;
                    index_ = 4;
                }
            }
            partial_decrement_ = false;
            return *this;
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR from_utf16_iterator
        operator++(int)noexcept(!throw_on_error)
        {
            from_utf16_iterator retval = *this;
            ++*this;
            return retval;
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR from_utf16_iterator &
        operator--() noexcept(!throw_on_error)
        {
            if (partial_decrement_)
                index_ = decr_read_into_buf();
            if (index_ == 0 || buf_empty()) {
                --it_;
                index_ = 4;
                partial_decrement_ = true;
            } else {
                --index_;
            }
            next_ = it_;
            return *this;
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR from_utf16_iterator
        operator--(int)noexcept(!throw_on_error)
        {
            from_utf16_iterator retval = *this;
            --*this;
            return retval;
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool operator==(
            from_utf16_iterator<Iter> lhs,
            from_utf16_iterator<Iter> rhs) noexcept
        {
            if (lhs.it_ != rhs.it_)
                return false;
            return lhs.index_ == rhs.index_ ||
                   ((lhs.index_ == 0 || lhs.index_ == 4) &&
                    (rhs.index_ == 0 || rhs.index_ == 4));
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool operator!=(
            from_utf16_iterator<Iter> lhs,
            from_utf16_iterator<Iter> rhs) noexcept
        {
            return !(lhs.it_ == rhs.it_);
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        constexpr bool buf_empty() const noexcept { return index_ == 4; }

        constexpr bool at_buf_end() const noexcept
        {
            return buf_[index_] == '\0';
        }

        constexpr bool high_surrogate(uint32_t c) const noexcept
        {
            return high_surrogate_min <= c && c <= high_surrogate_max;
        }

        constexpr bool low_surrogate(uint32_t c) const noexcept
        {
            return low_surrogate_min <= c && c <= low_surrogate_max;
        }

        BOOST_TEXT_CXX14_CONSTEXPR int
        read_into_buf(uint32_t first, uint32_t second) const
            noexcept(!throw_on_error)
        {
            uint32_t c = first;

            if (high_surrogate(first)) {
                assert(low_surrogate(second));
                c = (c << 10) + second + surrogate_offset;
            }

            if (!valid_code_point(c)) {
                c = ErrorHandler{}(
                    "UTF-16 sequence results in invalid UTF-32 code "
                    "point.");
            }

            if (c < 0x80) {
                buf_[0] = static_cast<char>(c);
                buf_[1] = '\0';
                return 0;
            } else if (c < 0x800) {
                buf_[0] = static_cast<char>(0xC0 + (c >> 6));
                buf_[1] = static_cast<char>(0x80 + (c & 0x3f));
                buf_[2] = '\0';
                return 1;
            } else if (c < 0x10000) {
                buf_[0] = static_cast<char>(0xe0 + (c >> 12));
                buf_[1] = static_cast<char>(0x80 + ((c >> 6) & 0x3f));
                buf_[2] = static_cast<char>(0x80 + (c & 0x3f));
                buf_[3] = '\0';
                return 2;
            } else {
                buf_[0] = static_cast<char>(0xf0 + (c >> 18));
                buf_[1] = static_cast<char>(0x80 + ((c >> 12) & 0x3f));
                buf_[2] = static_cast<char>(0x80 + ((c >> 6) & 0x3f));
                buf_[3] = static_cast<char>(0x80 + (c & 0x3f));
                buf_[4] = '\0';
                return 3;
            }
        }

        BOOST_TEXT_CXX14_CONSTEXPR int incr_read_into_buf() const
            noexcept(!throw_on_error)
        {
            uint32_t first = static_cast<uint32_t>(*it_);
            uint32_t second = 0;
            next_ = it_;
            ++next_;
            if (high_surrogate(first)) {
                second = static_cast<uint32_t>(*next_);
                ++next_;
            } else if (surrogate(first)) {
                first = ErrorHandler{}("Invalid initial UTF-16 character.");
            }
            read_into_buf(first, second);
            return 0;
        }

        BOOST_TEXT_CXX14_CONSTEXPR int decr_read_into_buf() const
            noexcept(!throw_on_error)
        {
            uint32_t first = static_cast<uint32_t>(*it_);
            uint32_t second = 0;
            if (low_surrogate(first)) {
                second = first;
                first = static_cast<uint32_t>(*--const_cast<Iter &>(it_));
            } else if (surrogate(first)) {
                first = ErrorHandler{}("Invalid folliwng UTF-16 sequence.");
            }
            return read_into_buf(first, second);
        }

        Iter it_;
        mutable Iter next_;
        mutable int index_;
        mutable char buf_[5];
        mutable bool partial_decrement_;

        // Unicode 9, 3.8/D71-D74

        static uint32_t const high_surrogate_min = 0xd800;
        static uint32_t const high_surrogate_max = 0xdbff;

        static uint32_t const low_surrogate_min = 0xdc00;
        static uint32_t const low_surrogate_max = 0xdfff;

        static uint32_t const surrogate_offset =
            0x10000 - (high_surrogate_min << 10) - low_surrogate_min;
#endif
    };

    /** Returns a from_utf16_iterator<Iter> constructed from an Iter. */
    template<typename Iter>
    from_utf16_iterator<Iter> make_from_utf16_iterator(Iter it)
    {
        return from_utf16_iterator<Iter>(it);
    }


    /** A UTF-8 to UTF-16 converting iterator.  Set the ErrorHandler template
        parameter to control error handling.  The default ErrorHandler is
        use_replacement_character, which simply produces a replacement
        character.

        Iter must be a bidirectional iterator with a 1-byte char
        value_type. */
    template<typename Iter, typename ErrorHandler = use_replacement_character>
    struct to_utf16_iterator
    {
        using value_type = uint16_t;
        using difference_type = int;
        using pointer = uint16_t *;
        using reference = uint16_t;
        using iterator_category = std::bidirectional_iterator_tag;

        static bool const throw_on_error =
            noexcept(std::declval<ErrorHandler>()(0));

        constexpr to_utf16_iterator() noexcept :
            it_(),
            next_(),
            index_(2),
            buf_(),
            partial_decrement_(false)
        {}
        explicit constexpr to_utf16_iterator(Iter it) noexcept :
            it_(it),
            next_(it),
            index_(2),
            buf_(),
            partial_decrement_(false)
        {}

        // TODO: This needs to change; this is a thread-unsafe const member!
        // Same for the other iterators....
        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR reference operator*() const
            noexcept(!throw_on_error)
        {
            if (buf_empty()) {
                if (partial_decrement_)
                    const_cast<Iter &>(it_) = detail::decrement(it_ + 1);
                index_ = read_into_buf();
                if (!partial_decrement_)
                    index_ = 0;
            }
            partial_decrement_ = false;
            return buf_[index_];
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR Iter base() const noexcept { return it_; }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR to_utf16_iterator &
        operator++() noexcept(!throw_on_error)
        {
            if (partial_decrement_) {
                ++it_;
            } else {
                if (buf_empty()) {
                    read_into_buf();
                    index_ = 0;
                }
                ++index_;
                if (at_buf_end()) {
                    it_ = next_;
                    index_ = 2;
                }
            }
            partial_decrement_ = false;
            return *this;
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR to_utf16_iterator
        operator++(int)noexcept(!throw_on_error)
        {
            to_utf16_iterator retval = *this;
            ++*this;
            return retval;
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR to_utf16_iterator &
        operator--() noexcept(!throw_on_error)
        {
            if (partial_decrement_)
                it_ = detail::decrement(it_ + 1);
            if (index_ == 0 || buf_empty()) {
                --it_;
                index_ = 2;
                partial_decrement_ = true;
            } else {
                --index_;
            }
            return *this;
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR to_utf16_iterator
        operator--(int)noexcept(!throw_on_error)
        {
            to_utf16_iterator retval = *this;
            --*this;
            return retval;
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator==(to_utf16_iterator lhs, to_utf16_iterator rhs) noexcept
        {
            if (lhs.it_ != rhs.it_)
                return false;
            return lhs.index_ == rhs.index_ ||
                   (lhs.index_ != 1 && rhs.index_ != 1);
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator!=(to_utf16_iterator lhs, to_utf16_iterator rhs) noexcept
        {
            return !(lhs.it_ == rhs.it_);
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator==(to_utf16_iterator lhs, null_sentinel rhs) noexcept
        {
            return !*lhs.it_;
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator==(null_sentinel lhs, to_utf16_iterator rhs) noexcept
        {
            return rhs == lhs;
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator!=(to_utf16_iterator lhs, null_sentinel rhs) noexcept
        {
            return !(lhs == rhs);
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator!=(null_sentinel lhs, to_utf16_iterator rhs) noexcept
        {
            return !(rhs == lhs);
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        constexpr bool buf_empty() const noexcept { return index_ == 2; }

        constexpr bool at_buf_end() const noexcept { return buf_[index_] == 0; }

        BOOST_TEXT_CXX14_CONSTEXPR bool check_continuation(
            unsigned char c,
            unsigned char lo = 0x80,
            unsigned char hi = 0xbf) const noexcept(!throw_on_error)
        {
            if (continuation(c, lo, hi)) {
                return true;
            } else {
                buf_[0] = ErrorHandler{}(
                    "Invalid UTF-8 sequence; an expected continuation "
                    "character is missing.");
                buf_[1] = 0;
                return false;
            }
        }

        BOOST_TEXT_CXX14_CONSTEXPR int read_into_buf() const
            noexcept(!throw_on_error)
        {
            uint32_t value = 0;

            next_ = it_;
            unsigned char curr_c = *next_;

            using detail::in;

            // One-byte
            if (curr_c <= 0x7f) {
                value = curr_c;
                ++next_;
                // Two-byte
            } else if (in(0xc2, curr_c, 0xdf)) {
                value = curr_c & 0b00011111;
                curr_c = *++next_;
                if (!check_continuation(curr_c))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                ++next_;
                // Three-byte
            } else if (curr_c == 0xe0) {
                value = curr_c & 0b00001111;
                curr_c = *++next_;
                if (!check_continuation(curr_c, 0xa0, 0xbf))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                curr_c = *++next_;
                if (!check_continuation(curr_c))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                ++next_;
            } else if (in(0xe1, curr_c, 0xec)) {
                value = curr_c & 0b00001111;
                curr_c = *++next_;
                if (!check_continuation(curr_c))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                curr_c = *++next_;
                if (!check_continuation(curr_c))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                ++next_;
            } else if (curr_c == 0xed) {
                value = curr_c & 0b00001111;
                curr_c = *++next_;
                if (!check_continuation(curr_c, 0x80, 0x9f))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                curr_c = *++next_;
                if (!check_continuation(curr_c))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                ++next_;
            } else if (in(0xed, curr_c, 0xef)) {
                value = curr_c & 0b00001111;
                curr_c = *++next_;
                if (!check_continuation(curr_c))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                curr_c = *++next_;
                if (!check_continuation(curr_c))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                ++next_;
                // Four-byte
            } else if (curr_c == 0xf0) {
                value = curr_c & 0b00000111;
                curr_c = *++next_;
                if (!check_continuation(curr_c, 0x90, 0xbf))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                curr_c = *++next_;
                if (!check_continuation(curr_c))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                curr_c = *++next_;
                if (!check_continuation(curr_c))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                ++next_;
            } else if (in(0xf1, curr_c, 0xf3)) {
                value = curr_c & 0b00000111;
                curr_c = *++next_;
                if (!check_continuation(curr_c))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                curr_c = *++next_;
                if (!check_continuation(curr_c))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                curr_c = *++next_;
                if (!check_continuation(curr_c))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                ++next_;
            } else if (curr_c == 0xf4) {
                value = curr_c & 0b00000111;
                curr_c = *++next_;
                if (!check_continuation(curr_c, 0x80, 0x8f))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                curr_c = *++next_;
                if (!check_continuation(curr_c))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                curr_c = *++next_;
                if (!check_continuation(curr_c))
                    return 0;
                value = (value << 6) + (curr_c & 0b00111111);
                ++next_;
            } else {
                buf_[0] = ErrorHandler{}("Invalid initial UTF-8 character.");
                buf_[1] = 0;
                ++next_;
                return 0;
            }

            if (!valid_code_point(value)) {
                buf_[0] = ErrorHandler{}(
                    "UTF-8 sequence results in invalid UTF-32 code point.");
                buf_[1] = 0;
                return 0;
            }

            index_ = 0;

            if (value < 0x10000) {
                buf_[0] = static_cast<uint16_t>(value);
                buf_[1] = 0;
                return 0;
            } else {
                buf_[0] =
                    static_cast<uint16_t>(value >> 10) + high_surrogate_base;
                buf_[1] =
                    static_cast<uint16_t>(value & 0x3ff) + low_surrogate_base;
                buf_[2] = 0;
                return 1;
            }
        }

        Iter it_;
        mutable Iter next_;
        mutable int index_;
        mutable uint16_t buf_[3];
        mutable bool partial_decrement_;

        static uint16_t const high_surrogate_base = 0xd7c0;
        static uint16_t const low_surrogate_base = 0xdc00;
#endif
    };

    /** Returns a to_utf16_iterator<Iter> constructed from an Iter. */
    template<typename Iter>
    to_utf16_iterator<Iter> make_to_utf16_iterator(Iter it)
    {
        return to_utf16_iterator<Iter>(it);
    }

    /** An insert-iterator analogous to std::insert_iterator, that also
        converts UTF-32 to UTF-8.*/
    template<typename Container>
    struct from_utf32_insert_iterator
    {
        using value_type = void;
        using difference_type = void;
        using pointer = void;
        using reference = void;
        using iterator_category = std::output_iterator_tag;

        from_utf32_insert_iterator(
            Container & c, typename Container::iterator it) :
            c_(&c),
            it_(it)
        {}

        from_utf32_insert_iterator & operator=(uint32_t cp)
        {
            uint32_t cps[1] = {cp};
            char chars[4];
            auto const chars_end = std::copy(
                from_utf32_iterator<uint32_t const *>(cps),
                from_utf32_iterator<uint32_t const *>(cps + 1),
                chars);
            it_ = c_->insert(it_, chars, chars_end);
            return *this;
        }

        from_utf32_insert_iterator & operator*() { return *this; }
        from_utf32_insert_iterator & operator++() { return *this; }
        from_utf32_insert_iterator operator++(int) { return *this; }

    private:
        Container * c_;
        typename Container::iterator it_;
    };

    /** Returns a from_utf32_insert_iterator<Container> constructed from the
        given container and iterator. */
    template<typename Container>
    from_utf32_insert_iterator<Container>
    from_utf32_inserter(Container & c, typename Container::iterator it)
    {
        return from_utf32_insert_iterator<Container>(c, it);
    }

}}}

#endif
