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
            >::value,
            "from_utf32_iterator requires its Iter parameter to produce a 4-byte value_type."
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

        constexpr from_utf32_iterator & operator++ () noexcept
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
        constexpr from_utf32_iterator operator++ (int) noexcept
        {
            from_utf32_iterator retval = *this;
            ++*this;
            return retval;
        }

        constexpr from_utf32_iterator & operator-- () noexcept
        {
            if (index_ == 0 || buf_empty()) {
                --it_;
                index_ = read_into_buf();
            } else {
                --index_;
            }
            return *this;
        }
        constexpr from_utf32_iterator operator-- (int) noexcept
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
        bool buf_empty ()
        { return index_ == 4; }

        bool at_buf_end ()
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

} } }

#endif
