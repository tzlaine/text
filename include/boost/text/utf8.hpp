#ifndef BOOST_TEXT_UTF8_HPP
#define BOOST_TEXT_UTF8_HPP

#include <boost/assert.hpp>
#include <boost/text/config.hpp>

#include <boost/throw_exception.hpp>

#include <array>
#include <iterator>
#include <type_traits>
#include <stdexcept>


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

        inline BOOST_TEXT_CXX14_CONSTEXPR int
        read_into_buf(uint32_t cp, char buf[5])
        {
            if (cp < 0x80) {
                buf[0] = static_cast<char>(cp);
                buf[1] = '\0';
                return 0;
            } else if (cp < 0x800) {
                buf[0] = static_cast<char>(0xC0 + (cp >> 6));
                buf[1] = static_cast<char>(0x80 + (cp & 0x3f));
                buf[2] = '\0';
                return 1;
            } else if (cp < 0x10000) {
                buf[0] = static_cast<char>(0xe0 + (cp >> 12));
                buf[1] = static_cast<char>(0x80 + ((cp >> 6) & 0x3f));
                buf[2] = static_cast<char>(0x80 + (cp & 0x3f));
                buf[3] = '\0';
                return 2;
            } else {
                buf[0] = static_cast<char>(0xf0 + (cp >> 18));
                buf[1] = static_cast<char>(0x80 + ((cp >> 12) & 0x3f));
                buf[2] = static_cast<char>(0x80 + ((cp >> 6) & 0x3f));
                buf[3] = static_cast<char>(0x80 + (cp & 0x3f));
                buf[4] = '\0';
                return 3;
            }
        }
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

        // optional is not constexpr friendly.
        template<typename Iter>
        struct optional_iter
        {
            constexpr optional_iter() : it_(), valid_(false) {}
            constexpr optional_iter(Iter it) : it_(it), valid_(true) {}

            BOOST_TEXT_CXX14_CONSTEXPR operator bool() const noexcept
            {
                return valid_;
            }
            BOOST_TEXT_CXX14_CONSTEXPR Iter operator*() const noexcept
            {
                BOOST_ASSERT(valid_);
                return it_;
            }
            Iter & operator*() noexcept
            {
                BOOST_ASSERT(valid_);
                return it_;
            }

            friend BOOST_TEXT_CXX14_CONSTEXPR bool
            operator==(optional_iter lhs, optional_iter rhs) noexcept
            {
                return lhs.valid_ == rhs.valid_ &&
                       (!lhs.valid_ || lhs.it_ == rhs.it_);
            }
            friend BOOST_TEXT_CXX14_CONSTEXPR bool
            operator!=(optional_iter lhs, optional_iter rhs) noexcept
            {
                return !(lhs == rhs);
            }

        private:
            Iter it_;
            bool valid_;
        };

        // Follow Table 3-7 in Unicode 9, 3.9/D92
        template<typename Iter>
        BOOST_TEXT_CXX14_CONSTEXPR optional_iter<Iter>
        end_of_invalid_utf8(Iter it) noexcept
        {
            BOOST_ASSERT(!continuation(*it));

            using detail::in;

            if (in(0, *it, 0x7f))
                return optional_iter<Iter>{};

            if (in(0xc2, *it, 0xdf)) {
                if (!continuation(*(it + 1)))
                    return it + 1;
                return optional_iter<Iter>{};
            }

            if (in(0xe0, *it, 0xe0)) {
                if (!continuation(*(it + 1), 0xa0, 0xbf))
                    return it + 1;
                if (!continuation(*(it + 2)))
                    return it + 2;
                return optional_iter<Iter>{};
            }
            if (in(0xe1, *it, 0xec)) {
                if (!continuation(*(it + 1)))
                    return it + 1;
                if (!continuation(*(it + 2)))
                    return it + 2;
                return optional_iter<Iter>{};
            }
            if (in(0xed, *it, 0xed)) {
                if (!continuation(*(it + 1), 0x80, 0x9f))
                    return it + 1;
                if (!continuation(*(it + 2)))
                    return it + 2;
                return optional_iter<Iter>{};
            }
            if (in(0xee, *it, 0xef)) {
                if (!continuation(*(it + 1)))
                    return it + 1;
                if (!continuation(*(it + 2)))
                    return it + 2;
                return optional_iter<Iter>{};
            }

            if (in(0xf0, *it, 0xf0)) {
                if (!continuation(*(it + 1), 0x90, 0xbf))
                    return it + 1;
                if (!continuation(*(it + 2)))
                    return it + 2;
                if (!continuation(*(it + 3)))
                    return it + 3;
                return optional_iter<Iter>{};
            }
            if (in(0xf1, *it, 0xf3)) {
                if (!continuation(*(it + 1)))
                    return it + 1;
                if (!continuation(*(it + 2)))
                    return it + 2;
                if (!continuation(*(it + 3)))
                    return it + 3;
                return optional_iter<Iter>{};
            }
            if (in(0xf4, *it, 0xf4)) {
                if (!continuation(*(it + 1), 0x80, 0x8f))
                    return it + 1;
                if (!continuation(*(it + 2)))
                    return it + 2;
                if (!continuation(*(it + 3)))
                    return it + 3;
                return optional_iter<Iter>{};
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

        return !detail::end_of_invalid_utf8(first);
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

        return !detail::end_of_invalid_utf8(buf);
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

    /** Returns true if c is a Unicode surrogate, or false otherwise. */
    inline constexpr bool surrogate(uint32_t c) noexcept
    {
        uint32_t const high_surrogate_min = 0xd800;
        uint32_t const low_surrogate_max = 0xdfff;
        return high_surrogate_min <= c && c <= low_surrogate_max;
    }

    /** Returns true if c is a Unicode high surrogate, or false otherwise. */
    inline constexpr bool high_surrogate(uint32_t c) noexcept
    {
        uint32_t const high_surrogate_min = 0xd800;
        uint32_t const high_surrogate_max = 0xdbff;
        return high_surrogate_min <= c && c <= high_surrogate_max;
    }

    /** Returns true if c is a Unicode low surrogate, or false otherwise. */
    inline constexpr bool low_surrogate(uint32_t c) noexcept
    {
        uint32_t const low_surrogate_min = 0xdc00;
        uint32_t const low_surrogate_max = 0xdfff;
        return low_surrogate_min <= c && c <= low_surrogate_max;
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
            boost::throw_exception(std::logic_error(msg));
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
        return !*p;
    }
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator!=(char const * p, null_sentinel)
    {
        return !!*p;
    }

    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator==(null_sentinel, char const * p)
    {
        return !*p;
    }
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator!=(null_sentinel, char const * p)
    {
        return !!*p;
    }


    /** A UTF-8 to UTF-16 converting iterator.  Set the ErrorHandler template
        parameter to control error handling.  The default ErrorHandler is
        use_replacement_character, which simply produces a replacement
        character.

        Iter must be a bidirectional iterator with a 1-byte char
        value_type. */
    template<
        typename Iter,
        typename Sentinel = Iter,
        typename ErrorHandler = use_replacement_character>
    struct to_utf16_iterator;


    /** A UTF-32 to UTF-8 converting iterator.  Set the ErrorHandler template
        parameter to control error handling.  The default ErrorHandler is
        use_replacement_character, which simply produces a replacement
        character.

        Iter must be a bidirectional iterator with a 4-byte integral
        value_type. */
    template<
        typename Iter,
        typename Sentinel = Iter,
        typename ErrorHandler = use_replacement_character>
    struct from_utf32_iterator
    {
        using value_type = char;
        using difference_type = int;
        using pointer = char *;
        using reference = char;
        using iterator_category = std::bidirectional_iterator_tag;

        static bool const throw_on_error =
            !noexcept(std::declval<ErrorHandler>()(0));

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
            first_(),
            it_(),
            last_(),
            index_(4),
            buf_()
        {}
        explicit BOOST_TEXT_CXX14_CONSTEXPR
        from_utf32_iterator(Iter first, Iter it, Sentinel last) noexcept :
            first_(first),
            it_(it),
            last_(last),
            index_(0),
            buf_()
        {
            if (it_ != last_)
                read_into_buf();
        }
        template<typename Iter2, typename Sentinel2>
        constexpr from_utf32_iterator(
            from_utf32_iterator<Iter2, Sentinel2, ErrorHandler> const &
                other) noexcept :
            first_(other.first_),
            it_(other.it_),
            last_(other.last_),
            index_(other.index_),
            buf_(other.buf_)
        {}

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR reference operator*() const
            noexcept(!throw_on_error)
        {
            return buf_[index_];
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR Iter base() const noexcept { return it_; }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR from_utf32_iterator &
        operator++() noexcept(!throw_on_error)
        {
            ++index_;
            if (at_buf_end()) {
                ++it_;
                index_ = 0;
                if (it_ != last_)
                    read_into_buf();
            }
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
            if (0 < index_) {
                --index_;
            } else {
                --it_;
                index_ = read_into_buf();
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

        template<
            typename Iter1,
            typename ErrorHandler1,
            typename Iter2,
            typename ErrorHandler2>
        friend BOOST_TEXT_CXX14_CONSTEXPR auto operator==(
            from_utf32_iterator<Iter1, ErrorHandler1> lhs,
            from_utf32_iterator<Iter2, ErrorHandler2> rhs) noexcept
            -> decltype(lhs.it_ == rhs.it_ && lhs.index_ == rhs.index_);

#ifndef BOOST_TEXT_DOXYGEN
    private:
        constexpr bool buf_empty() const noexcept { return index_ == 4; }

        constexpr bool at_buf_end() const noexcept
        {
            return buf_[index_] == '\0';
        }

        BOOST_TEXT_CXX14_CONSTEXPR int read_into_buf() noexcept(!throw_on_error)
        {
            uint32_t cp = static_cast<uint32_t>(*it_);
            if (!valid_code_point(cp))
                cp = ErrorHandler{}("Invalid UTF-32 code point.");
            index_ = 0;
            return detail::read_into_buf(cp, buf_.data());
        }

        Iter first_;
        Iter it_;
        Sentinel last_;
        int index_;
        std::array<char, 5> buf_;

        template<typename Iter2, typename Sentinel2, typename ErrorHandler2>
        friend struct from_utf32_iterator;
#endif
    };

    /** Returns a from_utf32_iterator<Iter> constructed from an Iter. */
    template<typename Iter, typename Sentinel>
    from_utf32_iterator<Iter, Sentinel>
    make_from_utf32_iterator(Iter first, Iter it, Sentinel last) noexcept
    {
        return from_utf32_iterator<Iter, Sentinel>(first, it, last);
    }

    /** This function is constexpr in C++14 and later. */
    template<
        typename Iter1,
        typename ErrorHandler1,
        typename Iter2,
        typename ErrorHandler2>
    BOOST_TEXT_CXX14_CONSTEXPR auto operator==(
        from_utf32_iterator<Iter1, ErrorHandler1> lhs,
        from_utf32_iterator<Iter2, ErrorHandler2> rhs) noexcept
        -> decltype(lhs.it_ == rhs.it_ && lhs.index_ == rhs.index_)
    {
        return lhs.it_ == rhs.it_ && lhs.index_ == rhs.index_;
    }

    /** This function is constexpr in C++14 and later. */
    template<
        typename Iter1,
        typename ErrorHandler1,
        typename Iter2,
        typename ErrorHandler2>
    BOOST_TEXT_CXX14_CONSTEXPR auto operator!=(
        from_utf32_iterator<Iter1, ErrorHandler1> lhs,
        from_utf32_iterator<Iter2, ErrorHandler2> rhs) noexcept
        -> decltype(!(lhs == rhs))
    {
        return !(lhs == rhs);
    }


    /** A UTF-8 to UTF-32 converting iterator.  Set the ErrorHandler template
        parameter to control error handling.  The default ErrorHandler is
        use_replacement_character, which simply produces a replacement
        character.

        Iter must be a bidirectional iterator with a 1-byte char
        value_type. */
    template<
        typename Iter,
        typename Sentinel = Iter,
        typename ErrorHandler = use_replacement_character>
    struct to_utf32_iterator
    {
        using value_type = uint32_t;
        using difference_type = int;
        using pointer = uint32_t *;
        using reference = uint32_t;
        using iterator_category = std::bidirectional_iterator_tag;

        static bool const throw_on_error =
            !noexcept(std::declval<ErrorHandler>()(0));

        constexpr to_utf32_iterator() noexcept : first_(), it_(), last_() {}
        explicit constexpr to_utf32_iterator(
            Iter first, Iter it, Sentinel last) noexcept :
            first_(first),
            it_(it),
            last_(last)
        {}
        template<typename Iter2, typename Sentinel2>
        constexpr to_utf32_iterator(
            to_utf32_iterator<Iter2, Sentinel2, ErrorHandler> const &
                other) noexcept :
            first_(other.first_),
            it_(other.it_),
            last_(other.last_)
        {}

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR reference operator*() const
            noexcept(!throw_on_error)
        {
            if (at_end(it_))
                return replacement_character();
            unsigned char curr_c = *it_;
            if (curr_c < 0x80)
                return curr_c;
            return get_value(curr_c).value_;
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
                    "code unit is missing.");
                return false;
            }
        }

        BOOST_TEXT_CXX14_CONSTEXPR bool at_end(Iter it) const
            noexcept(!throw_on_error)
        {
            if (it == last_) {
                ErrorHandler{}(
                    "Invalid UTF-8 sequence; expected another code unit "
                    "before the end of string.");
                return true;
            } else {
                return false;
            }
        }

        BOOST_TEXT_CXX14_CONSTEXPR get_value_result
        get_value(unsigned char curr_c) const noexcept(!throw_on_error)
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

            using detail::in;

            // One-byte case handled by caller

            // Two-byte
            if (in(0xc2, curr_c, 0xdf)) {
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
                value = ErrorHandler{}("Invalid initial UTF-8 code unit.");
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
            if (at_end(it_))
                return it_;
            unsigned char curr_c = *it_;
            if (curr_c < 0x80)
                return std::next(it_);
            return get_value(curr_c).it_;
        }

        Iter first_;
        Iter it_;
        Sentinel last_;

        template<typename Iter2, typename Sentinel2, typename ErrorHandler2>
        friend struct to_utf16_iterator;

        template<typename Iter2, typename Sentinel2, typename ErrorHandler2>
        friend struct to_utf32_iterator;

#endif
    };

    /** Returns a to_utf32_iterator<Iter> constructed from an Iter. */
    template<typename Iter, typename Sentinel>
    to_utf32_iterator<Iter, Sentinel>
    make_to_utf32_iterator(Iter first, Iter it, Sentinel last) noexcept
    {
        return to_utf32_iterator<Iter, Sentinel>(first, it, last);
    }

    /** This function is constexpr in C++14 and later. */
    template<
        typename Iter1,
        typename ErrorHandler1,
        typename Iter2,
        typename ErrorHandler2>
    BOOST_TEXT_CXX14_CONSTEXPR auto operator==(
        to_utf32_iterator<Iter1, ErrorHandler1> lhs,
        to_utf32_iterator<Iter2, ErrorHandler2> rhs) noexcept
        -> decltype(lhs.base() == rhs.base())
    {
        return lhs.base() == rhs.base();
    }

    /** This function is constexpr in C++14 and later. */
    template<
        typename Iter1,
        typename ErrorHandler1,
        typename Iter2,
        typename ErrorHandler2>
    BOOST_TEXT_CXX14_CONSTEXPR auto operator!=(
        to_utf32_iterator<Iter1, ErrorHandler1> lhs,
        to_utf32_iterator<Iter2, ErrorHandler2> rhs) noexcept
        -> decltype(!(lhs == rhs))
    {
        return !(lhs == rhs);
    }


    /** A UTF-16 to UTF-8 converting iterator.  Set the ErrorHandler template
        parameter to control error handling.  The default ErrorHandler is
        use_replacement_character, which simply produces a replacement
        character.

        Iter must be a bidirectional iterator with a 2-byte integral
        value_type. */
    template<
        typename Iter,
        typename Sentinel = Iter,
        typename ErrorHandler = use_replacement_character>
    struct from_utf16_iterator
    {
        using value_type = char;
        using difference_type = int;
        using pointer = char *;
        using reference = char;
        using iterator_category = std::bidirectional_iterator_tag;

        static bool const throw_on_error =
            !noexcept(std::declval<ErrorHandler>()(0));

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
            first_(),
            it_(),
            last_(),
            index_(4),
            buf_()
        {}
        explicit BOOST_TEXT_CXX14_CONSTEXPR
        from_utf16_iterator(Iter first, Iter it, Sentinel last) noexcept :
            first_(first),
            it_(it),
            last_(last),
            index_(0),
            buf_()
        {
            if (it_ != last_)
                read_into_buf();
        }
        template<typename Iter2, typename Sentinel2>
        constexpr from_utf16_iterator(
            from_utf16_iterator<Iter2, Sentinel2> const & other) noexcept :
            first_(other.first_),
            it_(other.it_),
            last_(other.last_),
            index_(other.index_),
            buf_(other.buf_)
        {}

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR reference operator*() const
            noexcept(!throw_on_error)
        {
            return buf_[index_];
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR Iter base() const noexcept { return it_; }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR from_utf16_iterator &
        operator++() noexcept(!throw_on_error)
        {
            ++index_;
            if (at_buf_end()) {
                increment();
                index_ = 0;
                if (it_ != last_)
                    read_into_buf();
            }
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
            if (0 < index_) {
                --index_;
            } else {
                decrement();
                index_ = read_into_buf();
            }
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

        template<
            typename Iter1,
            typename ErrorHandler1,
            typename Iter2,
            typename ErrorHandler2>
        friend BOOST_TEXT_CXX14_CONSTEXPR auto operator==(
            from_utf16_iterator<Iter1, ErrorHandler1> lhs,
            from_utf16_iterator<Iter2, ErrorHandler2> rhs) noexcept
            -> decltype(lhs.it_ == rhs.it_ && lhs.index_ == rhs.index_);

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator==(from_utf16_iterator lhs, null_sentinel rhs) noexcept
        {
            return !*lhs.it_;
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator==(null_sentinel lhs, from_utf16_iterator rhs) noexcept
        {
            return rhs == lhs;
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator!=(from_utf16_iterator lhs, null_sentinel rhs) noexcept
        {
            return !(lhs == rhs);
        }

        /** This function is constexpr in C++14 and later. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator!=(null_sentinel lhs, from_utf16_iterator rhs) noexcept
        {
            return !(rhs == lhs);
        }

#ifndef BOOST_TEXT_DOXYGEN
    private:
        BOOST_TEXT_CXX14_CONSTEXPR bool at_end() const noexcept(!throw_on_error)
        {
            if (it_ == last_) {
                ErrorHandler{}(
                    "Invalid UTF-16 sequence; expected another code unit "
                    "before the end of string.");
                return true;
            } else {
                return false;
            }
        }

        constexpr bool at_buf_end() const noexcept
        {
            return buf_[index_] == '\0';
        }

        BOOST_TEXT_CXX14_CONSTEXPR int read_into_buf() noexcept(!throw_on_error)
        {
            Iter next = it_;

            uint32_t first = static_cast<uint32_t>(*next);
            uint32_t second = 0;
            uint32_t cp = first;
            if (high_surrogate(first)) {
                if (at_end())
                    cp = replacement_character();
                else {
                    second = static_cast<uint32_t>(*++next);
                    if (!low_surrogate(second)) {
                        ErrorHandler{}(
                            "Invalid UTF-16 sequence; expected low surrogate "
                            "after high surrogate.");
                        cp = replacement_character();
                    } else {
                        cp = (first << 10) + second + surrogate_offset;
                    }
                }
            } else if (surrogate(first)) {
                ErrorHandler{}("Invalid initial UTF-16 code unit.");
                cp = replacement_character();
            }

            if (!valid_code_point(cp)) {
                cp = ErrorHandler{}(
                    "UTF-16 sequence results in invalid UTF-32 code "
                    "point.");
            }

            return detail::read_into_buf(cp, buf_.data());
        }

        BOOST_TEXT_CXX14_CONSTEXPR void increment() noexcept
        {
            if (high_surrogate(*it_)) {
                ++it_;
                if (it_ != last_ && low_surrogate(*it_))
                    ++it_;
            } else {
                ++it_;
            }
        }

        BOOST_TEXT_CXX14_CONSTEXPR void decrement() noexcept
        {
            if (low_surrogate(*--it_)) {
                if (it_ != first_)
                    --it_;
            }
        }

        Iter first_;
        Iter it_;
        Sentinel last_;
        int index_;
        std::array<char, 5> buf_;

        // Unicode 9, 3.8/D71-D74

        static uint32_t const high_surrogate_min = 0xd800;
        static uint32_t const high_surrogate_max = 0xdbff;

        static uint32_t const low_surrogate_min = 0xdc00;
        static uint32_t const low_surrogate_max = 0xdfff;

        static uint32_t const surrogate_offset =
            0x10000 - (high_surrogate_min << 10) - low_surrogate_min;

        template<typename Iter2, typename Sentinel2, typename ErrorHandler2>
        friend struct from_utf16_iterator;
#endif
    };

    /** Returns a from_utf16_iterator<Iter> constructed from an Iter. */
    template<typename Iter, typename Sentinel>
    from_utf16_iterator<Iter, Sentinel>
    make_from_utf16_iterator(Iter first, Iter it, Sentinel last) noexcept
    {
        return from_utf16_iterator<Iter, Sentinel>(first, it, last);
    }

    /** This function is constexpr in C++14 and later. */
    template<
        typename Iter1,
        typename ErrorHandler1,
        typename Iter2,
        typename ErrorHandler2>
    BOOST_TEXT_CXX14_CONSTEXPR auto operator==(
        from_utf16_iterator<Iter1, ErrorHandler1> lhs,
        from_utf16_iterator<Iter2, ErrorHandler2> rhs) noexcept
        -> decltype(lhs.it_ == rhs.it_ && lhs.index_ == rhs.index_)
    {
        return lhs.it_ == rhs.it_ && lhs.index_ == rhs.index_;
    }

    /** This function is constexpr in C++14 and later. */
    template<
        typename Iter1,
        typename ErrorHandler1,
        typename Iter2,
        typename ErrorHandler2>
    BOOST_TEXT_CXX14_CONSTEXPR auto operator!=(
        from_utf16_iterator<Iter1, ErrorHandler1> lhs,
        from_utf16_iterator<Iter2, ErrorHandler2> rhs) noexcept
        -> decltype(!(lhs == rhs))
    {
        return !(lhs == rhs);
    }


    template<typename Iter, typename Sentinel, typename ErrorHandler>
    struct to_utf16_iterator
    {
        using value_type = uint16_t;
        using difference_type = int;
        using pointer = uint16_t *;
        using reference = uint16_t;
        using iterator_category = std::bidirectional_iterator_tag;

        static bool const throw_on_error =
            !noexcept(std::declval<ErrorHandler>()(0));

        constexpr to_utf16_iterator() noexcept : it_(), index_(2), buf_() {}
        explicit BOOST_TEXT_CXX14_CONSTEXPR
        to_utf16_iterator(Iter first, Iter it, Sentinel last) noexcept :
            it_(first, it, last),
            index_(0),
            buf_()
        {
            if (it_.it_ != it_.last_)
                read_into_buf();
        }
        template<typename Iter2, typename Sentinel2>
        constexpr to_utf16_iterator(
            to_utf16_iterator<Iter2, Sentinel2, ErrorHandler> const &
                other) noexcept :
            it_(other.it_),
            index_(other.index_),
            buf_(other.buf_)
        {}

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR reference operator*() const
            noexcept(!throw_on_error)
        {
            return buf_[index_];
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR Iter base() const noexcept
        {
            return it_.base();
        }

        /** This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR to_utf16_iterator &
        operator++() noexcept(!throw_on_error)
        {
            ++index_;
            if (at_buf_end()) {
                ++it_;
                index_ = 0;
                if (it_.it_ != it_.last_)
                    read_into_buf();
            }
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
            if (0 < index_) {
                --index_;
            } else {
                --it_;
                index_ = read_into_buf();
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

        template<
            typename Iter1,
            typename ErrorHandler1,
            typename Iter2,
            typename ErrorHandler2>
        friend BOOST_TEXT_CXX14_CONSTEXPR auto operator==(
            to_utf16_iterator<Iter1, ErrorHandler1> lhs,
            to_utf16_iterator<Iter2, ErrorHandler2> rhs) noexcept
            -> decltype(lhs.it_ == rhs.it_ && lhs.index_ == rhs.index_);

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
        constexpr bool at_buf_end() const noexcept { return buf_[index_] == 0; }

        BOOST_TEXT_CXX14_CONSTEXPR int read_into_buf() noexcept(!throw_on_error)
        {
            uint32_t const value = *it_;

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

        to_utf32_iterator<Iter, Sentinel> it_;
        int index_;
        std::array<uint16_t, 4> buf_;

        static uint16_t const high_surrogate_base = 0xd7c0;
        static uint16_t const low_surrogate_base = 0xdc00;

        template<typename Iter2, typename Sentinel2, typename ErrorHandler2>
        friend struct to_utf16_iterator;
#endif
    };

    /** Returns a to_utf16_iterator<Iter> constructed from an Iter. */
    template<typename Iter, typename Sentinel>
    to_utf16_iterator<Iter, Sentinel>
    make_to_utf16_iterator(Iter first, Iter it, Sentinel last) noexcept
    {
        return to_utf16_iterator<Iter, Sentinel>(first, it, last);
    }

    /** This function is constexpr in C++14 and later. */
    template<
        typename Iter1,
        typename ErrorHandler1,
        typename Iter2,
        typename ErrorHandler2>
    BOOST_TEXT_CXX14_CONSTEXPR auto operator==(
        to_utf16_iterator<Iter1, ErrorHandler1> lhs,
        to_utf16_iterator<Iter2, ErrorHandler2> rhs) noexcept
        -> decltype(lhs.it_ == rhs.it_ && lhs.index_ == rhs.index_)
    {
        return lhs.it_ == rhs.it_ && lhs.index_ == rhs.index_;
    }

    /** This function is constexpr in C++14 and later. */
    template<
        typename Iter1,
        typename ErrorHandler1,
        typename Iter2,
        typename ErrorHandler2>
    BOOST_TEXT_CXX14_CONSTEXPR auto operator!=(
        to_utf16_iterator<Iter1, ErrorHandler1> lhs,
        to_utf16_iterator<Iter2, ErrorHandler2> rhs) noexcept
        -> decltype(!(lhs == rhs))
    {
        return !(lhs == rhs);
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
            Container & c, typename Container::iterator it) noexcept :
            c_(&c),
            it_(it)
        {}

        from_utf32_insert_iterator & operator=(uint32_t cp)
        {
            uint32_t cps[1] = {cp};
            char chars[4];
            auto const chars_end = std::copy(
                from_utf32_iterator<uint32_t const *>(cps, cps, cps + 1),
                from_utf32_iterator<uint32_t const *>(cps, cps + 1, cps + 1),
                chars);
            for (char * char_it = chars; char_it != chars_end; ++char_it) {
                it_ = c_->insert(it_, *char_it);
                ++it_;
            }
            return *this;
        }

        from_utf32_insert_iterator & operator*() noexcept { return *this; }
        from_utf32_insert_iterator & operator++() noexcept { return *this; }
        from_utf32_insert_iterator operator++(int)noexcept { return *this; }

    private:
        Container * c_;
        typename Container::iterator it_;
    };

    /** Returns a from_utf32_insert_iterator<Container> constructed from the
        given container and iterator. */
    template<typename Container>
    from_utf32_insert_iterator<Container>
    from_utf32_inserter(Container & c, typename Container::iterator it) noexcept
    {
        return from_utf32_insert_iterator<Container>(c, it);
    }

    /** An insert-iterator analogous to std::back_insert_iterator, that also
        converts UTF-32 to UTF-8.*/
    template<typename Container>
    struct from_utf32_back_insert_iterator
    {
        using value_type = void;
        using difference_type = void;
        using pointer = void;
        using reference = void;
        using iterator_category = std::output_iterator_tag;

        from_utf32_back_insert_iterator(Container & c) noexcept : c_(&c) {}

        from_utf32_back_insert_iterator & operator=(uint32_t cp)
        {
            uint32_t cps[1] = {cp};
            char chars[4];
            auto const chars_end = std::copy(
                from_utf32_iterator<uint32_t const *>(cps, cps, cps + 1),
                from_utf32_iterator<uint32_t const *>(cps, cps + 1, cps + 1),
                chars);
            for (char * it = chars; it < chars_end; ++it) {
                c_->push_back(*it);
            }
            return *this;
        }

        from_utf32_back_insert_iterator & operator*() noexcept { return *this; }
        from_utf32_back_insert_iterator & operator++() noexcept
        {
            return *this;
        }
        from_utf32_back_insert_iterator operator++(int)noexcept
        {
            return *this;
        }

    private:
        Container * c_;
    };

    /** Returns a from_utf32_insert_iterator<Container> constructed from the
        given container and iterator. */
    template<typename Container>
    from_utf32_back_insert_iterator<Container>
    from_utf32_back_inserter(Container & c) noexcept
    {
        return from_utf32_back_insert_iterator<Container>(c);
    }

}}}

#endif
