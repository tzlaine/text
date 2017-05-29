#ifndef BOOST_TEXT_UTF8_HPP
#define BOOST_TEXT_UTF8_HPP

#include <iterator>
#include <type_traits>
#include <stdexcept>

#include <cassert>


// TODO: Improve the error messages in exceptions.
// TODO: Stick with std::logic_error?  Maybe std::out_of_bounds instead?

namespace boost { namespace text { namespace utf8 {

    struct unchecked_t {};
    struct throw_on_encoding_error {};

    namespace {
        constexpr unchecked_t unchecked;
    }

    // Unicode 9, 3.2/C10
    constexpr uint32_t replacement_character () noexcept
    { return 0xfffd; }

    // TODO: Update based on table 3-7.  In fact, audit the rest of the code
    // for these kinds of changes.
    constexpr int code_point_bytes (char first) noexcept
    {
        if ((first & 0b10000000) == 0)
            return 1;
        if ((first & 0b11100000) == 0b11000000)
            return 2;
        if ((first & 0b11110000) == 0b11100000)
            return 3;
        if ((first & 0b11111000) == 0b11110000)
            return 4;
        return -1;
    }

    constexpr bool continuation (char c_) noexcept
    {
        unsigned const char c = c_;
        return 0x80 <= c && c <= 0xbfu;
    }

    constexpr bool encoded (char const * first, char const * last) noexcept
    {
        while (first != last) {
            int const cp_bytes = code_point_bytes(*first);
            if (cp_bytes == -1)
                return false;

            ++first;

            for (int i = 1; i < cp_bytes; ++i, ++first) {
                if (first == last || !continuation(*first))
                    return false;
            }
        }

        return true;
    }

    constexpr bool starts_encoded (char const * first, char const * last) noexcept
    {
        if (first == last)
            return true;

        int const cp_bytes = code_point_bytes(*first);
        if (cp_bytes == -1)
            return false;

        ++first;

        for (int i = 1; i < cp_bytes; ++i, ++first) {
            if (first == last || !continuation(*first))
                return false;
        }

        return true;
    }

    constexpr bool ends_encoded (char const * first, char const * last) noexcept
    {
        if (first == last)
            return true;

        int n = 1;
        while (first != --last && continuation(*last)) {
            ++n;
        }

        return code_point_bytes(*last) == n;
    }

    constexpr bool surrogate (uint32_t c) noexcept
    {
        uint32_t const high_surrogate_min = 0xd800;
        uint32_t const low_surrogate_max = 0xdfff;
        return high_surrogate_min <= c && c <= low_surrogate_max;
    }

    // Unicode 9, 3.4/D14
    constexpr bool reserved_noncharacter (uint32_t c) noexcept
    {
        bool const byte01_reserved =
            (c & 0xffff) == 0xffff || (c & 0xffff) == 0xfffe;
        bool const byte2_at_most_0x10 =
            ((c & 0xff0000u) >> 16) <= 0x10;
        return
            (byte01_reserved && byte2_at_most_0x10) ||
            (0xfdd0 <= c && c <= 0xfdef);
    }

    // Unicode 9, 3.9/D90
    constexpr bool valid_code_point (uint32_t c) noexcept
    { return c <= 0x10ffff && !surrogate(c) && !reserved_noncharacter(c); }



    // from_utf32_iterator

    template <typename Iter, typename Throw = void>
    struct from_utf32_iterator_t
    {
        using value_type = char;
        using difference_type = int;
        using pointer = char *;
        using reference = char;
        using iterator_category = std::bidirectional_iterator_tag;

        static bool const throw_on_error = std::is_same<Throw, throw_on_encoding_error>::value;

        static_assert(
            std::is_same<
                typename std::iterator_traits<Iter>::iterator_category,
                std::bidirectional_iterator_tag
            >::value ||
            std::is_same<
                typename std::iterator_traits<Iter>::iterator_category,
                std::random_access_iterator_tag
            >::value,
            "from_utf32_iterator requires its Iter parameter to be at least bidirectional."
        );
        static_assert(
            sizeof(typename std::iterator_traits<Iter>::value_type) == 4,
            "from_utf32_iterator requires its Iter parameter to produce a 4-byte value_type."
        );

        constexpr from_utf32_iterator_t () noexcept :
            it_ (),
            index_ (4),
            buf_ ()
        {}
        explicit constexpr from_utf32_iterator_t (Iter it) noexcept :
            it_ (it),
            index_ (4),
            buf_ ()
        {}

        constexpr reference operator* () const noexcept(!throw_on_error)
        {
            if (buf_empty())
                read_into_buf();
            return buf_[index_];
        }

        constexpr from_utf32_iterator_t & operator++ () noexcept(!throw_on_error)
        {
            if (buf_empty())
                read_into_buf();
            ++index_;
            if (at_buf_end()) {
                ++it_;
                index_ = 4;
            }
            return *this;
        }
        constexpr from_utf32_iterator_t operator++ (int) noexcept(!throw_on_error)
        {
            from_utf32_iterator_t retval = *this;
            ++*this;
            return retval;
        }

        constexpr from_utf32_iterator_t & operator-- () noexcept(!throw_on_error)
        {
            if (index_ == 0 || buf_empty()) {
                --it_;
                index_ = read_into_buf();
            } else {
                --index_;
            }
            return *this;
        }
        constexpr from_utf32_iterator_t operator-- (int) noexcept(!throw_on_error)
        {
            from_utf32_iterator_t retval = *this;
            --*this;
            return retval;
        }

        // TODO: operator<=> () const
        friend constexpr bool operator== (from_utf32_iterator_t<Iter> lhs, from_utf32_iterator_t<Iter> rhs) noexcept
        {
            if (lhs.it_ != rhs.it_)
                return false;
            return
                lhs.index_ == rhs.index_ ||
                ((lhs.index_ == 0 || lhs.index_ == 4) && (rhs.index_ == 0 || rhs.index_ == 4));
        }
        friend constexpr bool operator!= (from_utf32_iterator_t<Iter> lhs, from_utf32_iterator_t<Iter> rhs) noexcept
        { return !(lhs.it_ == rhs.it_); }

    private:
        constexpr bool buf_empty () const noexcept
        { return index_ == 4; }

        constexpr bool at_buf_end () const noexcept
        { return buf_[index_] == '\0'; }

        constexpr int read_into_buf () const noexcept(!throw_on_error)
        {
            uint32_t c = static_cast<uint32_t>(*it_);
            if (!valid_code_point(c)) {
                if (throw_on_error)
                    throw std::logic_error("Invalid UTF-32 code point.");
                c = replacement_character();
            }
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
    };

    template <typename Iter>
    using from_utf32_iterator = from_utf32_iterator_t<Iter>;

    template <typename Iter>
    using from_utf32_iterator_throwing = from_utf32_iterator_t<Iter, throw_on_encoding_error>;



    // to_utf32_iterator

    template <typename Throw = void>
    struct to_utf32_iterator_t
    {
        using value_type = uint32_t;
        using difference_type = int;
        using pointer = uint32_t *;
        using reference = uint32_t;
        using iterator_category = std::bidirectional_iterator_tag;

        static bool const throw_on_error = std::is_same<Throw, throw_on_encoding_error>::value;

        constexpr to_utf32_iterator_t () noexcept :
            it_ (),
            next_ ()
        {}
        explicit constexpr to_utf32_iterator_t (char const * it) noexcept :
            it_ (it),
            next_ (it)
        {}

        constexpr reference operator* () const noexcept(!throw_on_error)
        { return get_value(); }

        constexpr to_utf32_iterator_t & operator++ () noexcept(!throw_on_error)
        {
            if (it_ != next_) {
                it_ = next_;
            } else {
                get_value();
                it_ = next_;
            }
            return *this;
        }
        constexpr to_utf32_iterator_t operator++ (int) noexcept(!throw_on_error)
        {
            to_utf32_iterator_t retval = *this;
            ++*this;
            return retval;
        }

        constexpr to_utf32_iterator_t & operator-- () noexcept(!throw_on_error)
        {
            // TODO: Need something more robust, that catches errors at each
            // char as we go backward.
            char const * const initial_it = it_;
            int n = 1;
            while (continuation(*--it_)) {
                ++n;
            }
            // Use get_value() to check for errors.
            if (get_value() != replacement_character())
                next_ = initial_it;
            else
                next_ = it_;
            return *this;
        }
        constexpr to_utf32_iterator_t operator-- (int) noexcept(!throw_on_error)
        {
            to_utf32_iterator_t retval = *this;
            --*this;
            return retval;
        }

        // TODO: operator<=> () const
        friend constexpr bool operator== (to_utf32_iterator_t lhs, to_utf32_iterator_t rhs) noexcept
        { return lhs.it_ == rhs.it_; }
        friend constexpr bool operator!= (to_utf32_iterator_t lhs, to_utf32_iterator_t rhs) noexcept
        { return lhs.it_ != rhs.it_; }

    private:
        constexpr bool check_continuation (
            unsigned char c,
            unsigned char lo = 0x80,
            unsigned char hi = 0xbf
        ) const noexcept(!throw_on_error) {
            if (lo <= c && c <= hi) {
                return true;
            } else {
                if (throw_on_error)
                    throw std::logic_error("Invalid UTF-8 sequence.");
                return false;
            }
        }

        constexpr reference get_value () const noexcept(!throw_on_error)
        {
            uint32_t retval = 0;

            // TODO: Update the logic of starts_encoded() and ends_encoded too!
            /*
                Unicode 9, 3.9/D92
                Table 3-7. Well-Formed UTF-8 Byte Sequences

                Code Points          First Byte  Second Byte  Third Byte  Fourth Byte
                ===========          ==========  ===========  ==========  ===========
                U+0000..U+007F       00..7F                               
                U+0080..U+07FF       C2..DF      80..BF                   
                U+0800..U+0FFF       E0          A0..BF       80..BF      
                U+1000..U+CFFF       E1..EC      80..BF       80..BF      
                U+D000..U+D7FF       ED          80..9F       80..BF      
                U+E000..U+FFFF       EE..EF      80..BF       80..BF      
                U+10000..U+3FFFF     F0          90..BF       80..BF      80..BF
                U+40000..U+FFFFF     F1..F3      80..BF       80..BF      80..BF
                U+100000..U+10FFFF   F4          80..8F       80..BF      80..BF
            */

            int chars = 0;

            next_ = it_;
            unsigned char curr_c = *next_++;

            // One-byte
            if (curr_c <= 0x7f) {
                retval = curr_c;
                chars = 1;
            // Two-byte
            } else if (0xc2 <= curr_c && curr_c <= 0xdf) {
                retval = curr_c & 0b00011111;
                curr_c = *next_++;
                if (!check_continuation(curr_c))
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                chars = 2;
            // Three-byte
            } else if (curr_c == 0xe0) {
                retval = curr_c & 0b00001111;
                curr_c = *next_++;
                if (!check_continuation(curr_c, 0xa0, 0xbf))
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                curr_c = *next_++;
                if (!check_continuation(curr_c))
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                chars = 3;
            } else if (0xe1 <= curr_c && curr_c <= 0xec) {
                retval = curr_c & 0b00001111;
                curr_c = *next_++;
                if (!check_continuation(curr_c))
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                curr_c = *next_++;
                if (!check_continuation(curr_c))
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                chars = 3;
            } else if (curr_c == 0xed) {
                retval = curr_c & 0b00001111;
                curr_c = *next_++;
                if (!check_continuation(curr_c, 0x80, 0x9f))
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                curr_c = *next_++;
                if (!check_continuation(curr_c))
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                chars = 3;
            } else if (curr_c == 0xed || curr_c == 0xef) {
                retval = curr_c & 0b00001111;
                curr_c = *next_++;
                if (!check_continuation(curr_c))
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                curr_c = *next_++;
                if (!check_continuation(curr_c))
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                chars = 3;
            // Four-byte
            } else if (curr_c == 0xf0) {
                retval = curr_c & 0b00000111;
                curr_c = *next_++;
                if (!check_continuation(curr_c, 0x90, 0xbf))
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                curr_c = *next_++;
                if (!check_continuation(curr_c))
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                curr_c = *next_++;
                if (!check_continuation(curr_c))
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                chars = 4;
            } else if (0xf1 <= curr_c && curr_c <= 0xf3) {
                retval = curr_c & 0b00000111;
                curr_c = *next_++;
                if (!check_continuation(curr_c))
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                curr_c = *next_++;
                if (!check_continuation(curr_c))
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                curr_c = *next_++;
                if (!check_continuation(curr_c))
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                chars = 4;
            } else if (curr_c == 0xf4) {
                retval = curr_c & 0b00000111;
                curr_c = *next_++;
                if (curr_c <= 0x80 || 0x8f <= curr_c)
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                curr_c = *next_++;
                if (!check_continuation(curr_c))
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                curr_c = *next_++;
                if (!check_continuation(curr_c))
                    return replacement_character();
                retval = (retval << 6) + (curr_c & 0b00111111);
                chars = 4;
            } else {
                if (throw_on_error)
                    throw std::logic_error("Invalid UTF-8 sequence.");
                return replacement_character();
            }

            if (!valid_code_point(retval)) {
                if (throw_on_error)
                    throw std::logic_error("UTF-8 sequence results in invalid UTF-32 code point.");
                retval = replacement_character();
            }

            return retval;
        }

        char const * it_;
        mutable char const * next_;
    };

    using to_utf32_iterator = to_utf32_iterator_t<>;

    template <typename Iter>
    using to_utf32_iterator_throwing = to_utf32_iterator_t<throw_on_encoding_error>;



    // from_utf16_iterator

    template <typename Iter, typename Throw = void>
    struct from_utf16_iterator_t
    {
        using value_type = char;
        using difference_type = int;
        using pointer = char *;
        using reference = char;
        using iterator_category = std::bidirectional_iterator_tag;

        static bool const throw_on_error = std::is_same<Throw, throw_on_encoding_error>::value;

        static_assert(
            std::is_same<
                typename std::iterator_traits<Iter>::iterator_category,
                std::bidirectional_iterator_tag
            >::value ||
            std::is_same<
                typename std::iterator_traits<Iter>::iterator_category,
                std::random_access_iterator_tag
            >::value,
            "from_utf16_iterator requires its Iter parameter to be at least bidirectional."
        );
        static_assert(
            sizeof(typename std::iterator_traits<Iter>::value_type) == 2,
            "from_utf16_iterator requires its Iter parameter to produce a 2-byte value_type."
        );

        constexpr from_utf16_iterator_t () noexcept :
            it_ (),
            index_ (4),
            buf_ ()
        {}
        explicit constexpr from_utf16_iterator_t (Iter it) noexcept :
            it_ (it),
            index_ (4),
            buf_ ()
        {}

        constexpr reference operator* () const noexcept(!throw_on_error)
        {
            if (buf_empty())
                incr_read_into_buf();
            return buf_[index_];
        }

        constexpr from_utf16_iterator_t & operator++ () noexcept(!throw_on_error)
        {
            if (buf_empty())
                incr_read_into_buf();
            ++index_;
            if (at_buf_end()) {
                ++it_;
                incr_read_into_buf();
            }
            return *this;
        }
        constexpr from_utf16_iterator_t operator++ (int) noexcept(!throw_on_error)
        {
            from_utf16_iterator_t retval = *this;
            ++*this;
            return retval;
        }

        constexpr from_utf16_iterator_t & operator-- () noexcept(!throw_on_error)
        {
            if (index_ == 0 || buf_empty()) {
                --it_;
                index_ = decr_read_into_buf();
            } else {
                --index_;
            }
            return *this;
        }
        constexpr from_utf16_iterator_t operator-- (int) noexcept(!throw_on_error)
        {
            from_utf16_iterator_t retval = *this;
            --*this;
            return retval;
        }

        // TODO: operator<=> () const
        friend constexpr bool operator== (from_utf16_iterator_t<Iter> lhs, from_utf16_iterator_t<Iter> rhs) noexcept
        {
            if (lhs.it_ != rhs.it_)
                return false;
            return
                lhs.index_ == rhs.index_ ||
                ((lhs.index_ == 0 || lhs.index_ == 4) && (rhs.index_ == 0 || rhs.index_ == 4));
        }
        friend constexpr bool operator!= (from_utf16_iterator_t<Iter> lhs, from_utf16_iterator_t<Iter> rhs) noexcept
        { return !(lhs.it_ == rhs.it_); }

    private:
        constexpr bool buf_empty () const noexcept
        { return index_ == 4; }

        constexpr bool at_buf_end () const noexcept
        { return buf_[index_] == '\0'; }

        constexpr bool high_surrogate (uint32_t c) const noexcept
        { return high_surrogate_min <= c && c <= high_surrogate_max; }

        constexpr bool low_surrogate (uint32_t c) const noexcept
        { return low_surrogate_min <= c && c <= low_surrogate_max; }

        constexpr int read_into_buf (uint32_t first, uint32_t second) const noexcept(!throw_on_error)
        {
            uint32_t c = first;

            if (high_surrogate(first)) {
                assert(low_surrogate(second));
                c = (c << 10) + second + surrogate_offset;
            }

            if (!valid_code_point(c)) {
                if (throw_on_error)
                    throw std::logic_error("Invalid UTF-32 code point.");
                c = replacement_character();
            }

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

        constexpr void incr_read_into_buf () const noexcept(!throw_on_error)
        {
            uint32_t first = static_cast<uint32_t>(*it_);
            uint32_t second = 0;
            if (high_surrogate(first)) {
                second = static_cast<uint32_t>(*++const_cast<Iter &>(it_));
            } else if (surrogate(first)) {
                if (throw_on_error)
                    throw std::logic_error("Invalid UTF-16 sequence.");
                first = replacement_character();
            }
            read_into_buf(first, second);
        }

        constexpr void decr_read_into_buf () const noexcept(!throw_on_error)
        {
            uint32_t first = static_cast<uint32_t>(*it_);
            uint32_t second = 0;
            if (low_surrogate(first)) {
                second = first;
                first = static_cast<uint32_t>(*--const_cast<Iter &>(it_));
            } else if (surrogate(first)) {
                if (throw_on_error)
                    throw std::logic_error("Invalid UTF-16 sequence.");
                first = replacement_character();
            }
            read_into_buf(first, second);
        }

        Iter it_;
        mutable int index_;
        mutable char buf_[5];

        // Unicode 9, 3.8/D71-D74

        static uint32_t const high_surrogate_min = 0xd800;
        static uint32_t const high_surrogate_max = 0xdbff;

        static uint32_t const low_surrogate_min = 0xdc00;
        static uint32_t const low_surrogate_max = 0xdfff;

        static uint32_t const surrogate_offset =
            0x10000 - (high_surrogate_min << 10) - low_surrogate_min;
    };

    template <typename Iter>
    using from_utf16_iterator = from_utf16_iterator_t<Iter>;

    template <typename Iter>
    using from_utf16_iterator_throwing = from_utf16_iterator_t<Iter, throw_on_encoding_error>;



    // to_utf16_iterator

    template <typename Throw = void>
    struct to_utf16_iterator_t
    {
        using value_type = uint16_t;
        using difference_type = int;
        using pointer = uint16_t *;
        using reference = uint16_t;
        using iterator_category = std::bidirectional_iterator_tag;

        static bool const throw_on_error = std::is_same<Throw, throw_on_encoding_error>::value;

        constexpr to_utf16_iterator_t () noexcept :
            it_ (),
            next_ (),
            index_ (2),
            buf_ ()
        {}
        explicit constexpr to_utf16_iterator_t (char const * it) noexcept :
            it_ (it),
            next_ (it),
            index_ (2),
            buf_ ()
        {}

        constexpr reference operator* () const noexcept(!throw_on_error)
        {
            if (buf_empty())
                read_into_buf();
            return buf_[index_];
        }

        constexpr to_utf16_iterator_t & operator++ () noexcept(!throw_on_error)
        {
            if (buf_empty())
                read_into_buf();
            ++index_;
            if (at_buf_end()) {
                it_ = next_;
                index_ = 2;
            }
            return *this;
        }
        constexpr to_utf16_iterator_t operator++ (int) noexcept(!throw_on_error)
        {
            to_utf16_iterator_t retval = *this;
            ++*this;
            return retval;
        }

        constexpr to_utf16_iterator_t & operator-- () noexcept(!throw_on_error)
        {
            if (index_ == 0 || buf_empty()) {
                int n = 1;
                while (continuation(*--it_)) {
                    ++n;
                }
                if (code_point_bytes(*it_) != n) {
                    if (throw_on_error)
                        throw std::logic_error("Invalid UTF-8 sequence.");
                }
                next_ = it_;
            } else {
                --index_;
            }
            return *this;
        }
        constexpr to_utf16_iterator_t operator-- (int) noexcept(!throw_on_error)
        {
            to_utf16_iterator_t retval = *this;
            --*this;
            return retval;
        }

        // TODO: operator<=> () const
        friend constexpr bool operator== (to_utf16_iterator_t lhs, to_utf16_iterator_t rhs) noexcept
        {
            if (lhs.it_ != rhs.it_)
                return false;
            return lhs.index_ == rhs.index_ || (lhs.index_ != 1 && rhs.index_ != 1);
        }
        friend constexpr bool operator!= (to_utf16_iterator_t lhs, to_utf16_iterator_t rhs) noexcept
        { return !(lhs.it_ == rhs.it_); }

    private:
        constexpr bool buf_empty () const noexcept
        { return index_ == 2; }

        constexpr bool at_buf_end () const noexcept
        { return buf_[index_] == 0; }

        constexpr void pack_replacement_character () const noexcept
        {
            buf_[0] = replacement_character();
            buf_[1] = 0;
        }

        constexpr bool check_continuation (char c) const noexcept(!throw_on_error)
        {
            if (continuation(c)) {
                return true;
            } else {
                if (throw_on_error)
                    throw std::logic_error("Invalid UTF-8 sequence.");
                pack_replacement_character();
                return false;
            }
        }

        constexpr void read_into_buf () const noexcept(!throw_on_error)
        {
            uint32_t value = 0;

            // TODO: Update logic to match Table 3-7!

            next_ = it_;
            int chars = 0;
            if ((*next_ & 0b10000000) == 0) {
                chars = 1;
                value += *next_++ & 0b01111111;
            } else if ((*next_ & 0b11100000) == 0b11000000) {
                chars = 2;
                value += *next_++ & 0b00011111;
                if (!check_continuation(*next_))
                    return;
                value += *next_++ & 0b00111111;
            } else if ((*next_ & 0b11110000) == 0b11100000) {
                chars = 3;
                value += *next_++ & 0x7f;
                if (!check_continuation(*next_))
                    return;
                value += *next_++ & 0b00001111;
                if (!check_continuation(*next_))
                    return;
                value += *next_++ & 0b00111111;
            } else if ((*next_ & 0b11111000) == 0b11110000) {
                chars = 4;
                value += *next_++ & 0x7f;
                if (!check_continuation(*next_))
                    return;
                value += *next_++ & 0b00000111;
                if (!check_continuation(*next_))
                    return;
                value += *next_++ & 0b00111111;
                if (!check_continuation(*next_))
                    return;
                value += *next_++ & 0b00111111;
            } else {
                if (throw_on_error)
                    throw std::logic_error("Invalid UTF-8 sequence.");
                pack_replacement_character();
                return;
            }

            if (!valid_code_point(value)) {
                if (throw_on_error)
                    throw std::logic_error("UTF-8 sequence results in invalid UTF-32 code point.");
                pack_replacement_character();
                return;
            }

            if (value < 0x10000) {
                buf_[0] = static_cast<uint16_t>(value);
                buf_[1] = 0;
            } else {
                buf_[0] = static_cast<uint16_t>(value >> 10) + high_surrogate_base;
                buf_[1] = static_cast<uint16_t>(value & 0x3ff) + low_surrogate_base;
                buf_[2] = 0;
            }
        }

        char const * it_;
        mutable char const * next_;
        mutable int index_;
        mutable uint16_t buf_[3];

        static uint16_t const high_surrogate_base = 0xd7c0;
        static uint16_t const low_surrogate_base = 0xdc00;
    };

    using to_utf16_iterator = to_utf16_iterator_t<>;

    template <typename Iter>
    using to_utf16_iterator_throwing = to_utf16_iterator_t<throw_on_encoding_error>;

} } }

#endif
