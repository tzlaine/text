#ifndef BOOST_TEXT_UTF8_HPP
#define BOOST_TEXT_UTF8_HPP

#include <iterator>
#include <type_traits>
#include <stdexcept>


namespace boost { namespace text { namespace utf8 {

    struct unchecked_t {};

    namespace {

        constexpr unchecked_t unchecked;

    }

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

    constexpr bool continuation (char first) noexcept
    { return (first & 0xc0) == 0x80; }

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

    template <typename Iter>
    struct from_utf32_iterator
    {
        using value_type = char;
        using difference_type = int;
        using pointer = char *;
        using reference = char;
        using iterator_category = std::bidirectional_iterator_tag;

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

        constexpr from_utf32_iterator () noexcept :
            it_ (),
            index_ (4),
            buf_ ()
        {}
        explicit constexpr from_utf32_iterator (Iter it) noexcept :
            it_ (it),
            index_ (4),
            buf_ ()
        {}

        constexpr reference operator* () const
        {
            if (buf_empty())
                read_into_buf();
            return buf_[index_];
        }

        constexpr from_utf32_iterator & operator++ ()
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
        constexpr from_utf32_iterator operator++ (int)
        {
            from_utf32_iterator retval = *this;
            ++*this;
            return retval;
        }

        constexpr from_utf32_iterator & operator-- ()
        {
            if (index_ == 0 || buf_empty()) {
                --it_;
                index_ = read_into_buf();
            } else {
                --index_;
            }
            return *this;
        }
        constexpr from_utf32_iterator operator-- (int)
        {
            from_utf32_iterator retval = *this;
            --*this;
            return retval;
        }

        // TODO: operator<=> () const
        friend constexpr bool operator== (from_utf32_iterator<Iter> lhs, from_utf32_iterator<Iter> rhs) noexcept
        {
            if (lhs.it_ != rhs.it_)
                return false;
            return
                lhs.index_ == rhs.index_ ||
                ((lhs.index_ == 0 || lhs.index_ == 4) && (rhs.index_ == 0 || rhs.index_ == 4));
        }
        friend constexpr bool operator!= (from_utf32_iterator<Iter> lhs, from_utf32_iterator<Iter> rhs) noexcept
        { return !(lhs.it_ == rhs.it_); }

    private:
        bool buf_empty () const
        { return index_ == 4; }

        bool at_buf_end () const
        { return buf_[index_] == '\0'; }

        constexpr int read_into_buf () const
        {
            uint32_t const c = static_cast<uint32_t>(*it_);
            if (0x10ffff < c)
                throw std::logic_error("Invalid UTF-32 code point.");
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

    struct to_utf32_iterator
    {
        using value_type = uint32_t;
        using difference_type = int;
        using pointer = uint32_t *;
        using reference = uint32_t;
        using iterator_category = std::bidirectional_iterator_tag;

        constexpr to_utf32_iterator () noexcept :
            it_ (),
            next_ (),
            value_ (no_value)
        {}
        explicit constexpr to_utf32_iterator (char const * it) noexcept :
            it_ (it),
            next_ (it),
            value_ (no_value)
        {}

        constexpr reference operator* () const
        {
            if (value_ == no_value)
                value_ = get_value();
            return value_;
        }

        constexpr to_utf32_iterator & operator++ ()
        {
            if (it_ != next_) {
                it_ = next_;
                value_ = no_value;
            } else {
                value_ = get_value();
                it_ = next_;
            }
            return *this;
        }
        constexpr to_utf32_iterator operator++ (int)
        {
            to_utf32_iterator retval = *this;
            ++*this;
            return retval;
        }

        constexpr to_utf32_iterator & operator-- ()
        {
            int n = 1;
            while (continuation(*--it_)) {
                ++n;
            }
            if (code_point_bytes(*it_) != n)
                throw std::logic_error("Invalid UTF-8 sequence.");
            next_ = it_;
            return *this;
        }
        constexpr to_utf32_iterator operator-- (int)
        {
            to_utf32_iterator retval = *this;
            --*this;
            return retval;
        }

        // TODO: operator<=> () const
        friend constexpr bool operator== (to_utf32_iterator lhs, to_utf32_iterator rhs) noexcept
        { return lhs.it_ == rhs.it_; }
        friend constexpr bool operator!= (to_utf32_iterator lhs, to_utf32_iterator rhs) noexcept
        { return lhs.it_ != rhs.it_; }

    private:
        constexpr reference get_value () const
        {
            uint32_t retval = 0;
            next_ = it_;
            if ((*next_ & 0b10000000) == 0) {
                retval += *next_++ & 0b01111111;
            } else if ((*next_ & 0b11100000) == 0b11000000) {
                retval += *next_++ & 0b00011111;
                retval += *next_++ & 0b00111111;
            } else if ((*next_ & 0b11110000) == 0b11100000) {
                retval += *next_++ & 0x7f;
                retval += *next_++ & 0b00001111;
                retval += *next_++ & 0b00111111;
            } else if ((*next_ & 0b11111000) == 0b11110000) {
                retval += *next_++ & 0x7f;
                retval += *next_++ & 0b00000111;
                retval += *next_++ & 0b00111111;
                retval += *next_++ & 0b00111111;
            } else {
                throw std::logic_error("Invalid UTF-8 sequence.");
            }
            if (0x10ffff < retval)
                throw std::logic_error("UTF-8 sequence results in invalid UTF-32 code point.");
            return retval;
        }

        char const * it_;
        mutable char const * next_;
        mutable uint32_t value_;

        static uint32_t const no_value = 0xffffffff;
    };

    template <typename Iter>
    struct from_utf16_iterator
    {
        using value_type = char;
        using difference_type = int;
        using pointer = char *;
        using reference = char;
        using iterator_category = std::bidirectional_iterator_tag;

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

        constexpr from_utf16_iterator () noexcept :
            it_ (),
            index_ (4),
            buf_ ()
        {}
        explicit constexpr from_utf16_iterator (Iter it) noexcept :
            it_ (it),
            index_ (4),
            buf_ ()
        {}

        constexpr reference operator* () const
        {
            if (buf_empty())
                incr_read_into_buf();
            return buf_[index_];
        }

        constexpr from_utf16_iterator & operator++ ()
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
        constexpr from_utf16_iterator operator++ (int)
        {
            from_utf16_iterator retval = *this;
            ++*this;
            return retval;
        }

        constexpr from_utf16_iterator & operator-- ()
        {
            if (index_ == 0 || buf_empty()) {
                --it_;
                index_ = decr_read_into_buf();
            } else {
                --index_;
            }
            return *this;
        }
        constexpr from_utf16_iterator operator-- (int)
        {
            from_utf16_iterator retval = *this;
            --*this;
            return retval;
        }

        // TODO: operator<=> () const
        friend constexpr bool operator== (from_utf16_iterator<Iter> lhs, from_utf16_iterator<Iter> rhs) noexcept
        {
            if (lhs.it_ != rhs.it_)
                return false;
            return
                lhs.index_ == rhs.index_ ||
                ((lhs.index_ == 0 || lhs.index_ == 4) && (rhs.index_ == 0 || rhs.index_ == 4));
        }
        friend constexpr bool operator!= (from_utf16_iterator<Iter> lhs, from_utf16_iterator<Iter> rhs) noexcept
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

        constexpr bool surrogate (uint32_t c) const noexcept
        { return high_surrogate(c) || low_surrogate(c); }

        constexpr int read_into_buf (uint32_t first, uint32_t second) const
        {
            uint32_t c = first;

            if (surrogate(first)) {
                if (!high_surrogate(first))
                    throw std::logic_error("Invalid UTF-16 sequence.");

                if (!low_surrogate(second))
                    c = (c << 10) + second + surrogate_offset;
                else
                    throw std::logic_error("Invalid UTF-16 sequence.");
            }

            if (0x10ffff < c)
                throw std::logic_error("Invalid UTF-32 code point.");

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

        constexpr void incr_read_into_buf () const
        {
            uint32_t const first = static_cast<uint32_t>(*it_);
            uint32_t second = 0;
            if (high_surrogate(first))
                second = static_cast<uint32_t>(*++const_cast<Iter &>(it_));
            read_into_buf(first, second);
        }

        constexpr void decr_read_into_buf () const
        {
            uint32_t first = static_cast<uint32_t>(*it_);
            uint32_t second = 0;
            if (low_surrogate(first)) {
                second = first;
                first = static_cast<uint32_t>(*--const_cast<Iter &>(it_));
            }
            read_into_buf(first, second);
        }

        Iter it_;
        mutable int index_;
        mutable char buf_[5];

        static uint32_t const high_surrogate_min = 0xd800;
        static uint32_t const high_surrogate_max = 0xdbff;

        static uint32_t const low_surrogate_min = 0xdc00;
        static uint32_t const low_surrogate_max = 0xdfff;

        static uint32_t const surrogate_offset =
            0x10000 - (high_surrogate_min << 10) - low_surrogate_min;
    };

    struct to_utf16_iterator
    {
        using value_type = uint16_t;
        using difference_type = int;
        using pointer = uint16_t *;
        using reference = uint16_t;
        using iterator_category = std::bidirectional_iterator_tag;

        constexpr to_utf16_iterator () noexcept :
            it_ (),
            next_ (),
            index_ (2),
            buf_ ()
        {}
        explicit constexpr to_utf16_iterator (char const * it) noexcept :
            it_ (it),
            next_ (it),
            index_ (2),
            buf_ ()
        {}

        constexpr reference operator* () const
        {
            if (buf_empty())
                read_into_buf();
            return buf_[index_];
        }

        constexpr to_utf16_iterator & operator++ ()
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
        constexpr to_utf16_iterator operator++ (int)
        {
            to_utf16_iterator retval = *this;
            ++*this;
            return retval;
        }

        constexpr to_utf16_iterator & operator-- ()
        {
            if (index_ == 0 || buf_empty()) {
                int n = 1;
                while (continuation(*--it_)) {
                    ++n;
                }
                if (code_point_bytes(*it_) != n)
                    throw std::logic_error("Invalid UTF-8 sequence.");
                next_ = it_;
            } else {
                --index_;
            }
            return *this;
        }
        constexpr to_utf16_iterator operator-- (int)
        {
            to_utf16_iterator retval = *this;
            --*this;
            return retval;
        }

        // TODO: operator<=> () const
        friend constexpr bool operator== (to_utf16_iterator lhs, to_utf16_iterator rhs) noexcept
        {
            if (lhs.it_ != rhs.it_)
                return false;
            return lhs.index_ == rhs.index_ || (lhs.index_ != 1 && rhs.index_ != 1);
        }
        friend constexpr bool operator!= (to_utf16_iterator lhs, to_utf16_iterator rhs) noexcept
        { return !(lhs.it_ == rhs.it_); }

    private:
        bool buf_empty () const
        { return index_ == 2; }

        bool at_buf_end () const
        { return buf_[index_] == 0; }

        constexpr void read_into_buf () const
        {
            uint32_t value = 0;

            next_ = it_;
            if ((*next_ & 0b10000000) == 0) {
                value += *next_++ & 0b01111111;
            } else if ((*next_ & 0b11100000) == 0b11000000) {
                value += *next_++ & 0b00011111;
                value += *next_++ & 0b00111111;
            } else if ((*next_ & 0b11110000) == 0b11100000) {
                value += *next_++ & 0x7f;
                value += *next_++ & 0b00001111;
                value += *next_++ & 0b00111111;
            } else if ((*next_ & 0b11111000) == 0b11110000) {
                value += *next_++ & 0x7f;
                value += *next_++ & 0b00000111;
                value += *next_++ & 0b00111111;
                value += *next_++ & 0b00111111;
            } else {
                throw std::logic_error("Invalid UTF-8 sequence.");
            }

            if (0x10ffff < value)
                throw std::logic_error("UTF-8 sequence results in invalid UTF-32 code point.");

            if (value < 0x10000) {
                if ((value & 0xFFFFF800) == 0xd800) // surrogate check
                    throw std::logic_error("UTF-8 sequence results in invalid UTF-32 code point.");
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

} } }

#endif
