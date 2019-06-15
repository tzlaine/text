#ifndef BOOST_TEXT_UTF16_HPP
#define BOOST_TEXT_UTF16_HPP

#include <boost/text/utf8.hpp>


namespace boost { namespace text { namespace utf16 {

    namespace detail {
        template<typename T, typename U>
        struct enable_utf16_cp
            : std::enable_if<std::is_integral<T>::value && sizeof(T) == 2u, U>
        {
        };
        template<typename T, typename U = T>
        using enable_utf16_cp_t = typename enable_utf16_cp<T, U>::type;
    }

    /** Given the first (and possibly only) code unit of a UTF-16 code point,
        returns the number of code units occupied by that code point (in the
        range [1, 2]).  Returns a value < 0 if first is not a valid initial
        UTF-16 code unit.

        This function is constexpr in C++14 and later. */
    template<typename T>
    BOOST_TEXT_CXX14_CONSTEXPR detail::enable_utf16_cp_t<T, int>
    code_point_units(T first) noexcept
    {
        if (utf8::low_surrogate(first))
            return -1;
        if (utf8::high_surrogate(first))
            return 2;
        return 1;
    }

    /** Returns the first code unit in [first, last) that is not properly
        UTF-16 encoded, or last if no such code unit is found.

        This function is constexpr in C++14 and later. */
    template<typename T>
    BOOST_TEXT_CXX14_CONSTEXPR detail::enable_utf16_cp_t<T, T const *>
    find_invalid_encoding(T const * first, T const * last) noexcept
    {
        while (first != last) {
            int const cp_units = code_point_units(*first);
            if (cp_units == -1 || last - first < cp_units)
                return first;

            if (cp_units == 2 && !utf8::low_surrogate(*(first + 1)))
                return first;

            first += cp_units;
        }

        return last;
    }

    /** Returns true if [first, last) is properly UTF-16 encoded, or false
        otherwise.

        This function is constexpr in C++14 and later. */
    template<typename T>
    BOOST_TEXT_CXX14_CONSTEXPR detail::enable_utf16_cp_t<T, bool>
    encoded(T const * first, T const * last) noexcept
    {
        return find_invalid_encoding(first, last) == last;
    }

    /** Returns true if [first, last) is empty or the initial code units in
        [first, last) form a valid Unicode code point, or false otherwise.

        This function is constexpr in C++14 and later. */
    template<typename T>
    BOOST_TEXT_CXX14_CONSTEXPR detail::enable_utf16_cp_t<T, bool>
    starts_encoded(T const * first, T const * last) noexcept
    {
        if (first == last)
            return true;

        int const cp_units = code_point_units(*first);
        if (cp_units == -1 || last - first < cp_units)
            return false;

        return cp_units == 1 || utf8::low_surrogate(*(first + 1));
    }

    /** Returns true if [first, last) is empty or the final code units in
        [first, last) form a valid Unicode code point, or false otherwise.

        This function is constexpr in C++14 and later. */
    template<typename T>
    BOOST_TEXT_CXX14_CONSTEXPR detail::enable_utf16_cp_t<T, bool>
    ends_encoded(T const * first, T const * last) noexcept
    {
        if (first == last)
            return true;

        auto it = last;
        if (utf8::low_surrogate(*--it))
            --it;

        return starts_encoded(it, last);
    }

    /** Returns true if [first, last) is empty or the initial code units in
        [first, last) form a valid Unicode code point, or false otherwise. */
    template<typename Iter>
    detail::enable_utf16_cp_t<typename Iter::value, bool>
    starts_encoded(Iter first, Iter last) noexcept
    {
        if (first == last)
            return true;

        int const cp_units = code_point_units(*first);
        if (cp_units == -1 || last - first < cp_units)
            return false;

        return cp_units == 1 || utf8::low_surrogate(*(first + 1));
    }

    /** Returns true if [first, last) is empty or the final code units in
        [first, last) form a valid Unicode code point, or false otherwise. */
    template<typename Iter>
    detail::enable_utf16_cp_t<typename Iter::value, bool>
    ends_encoded(Iter first, Iter last) noexcept
    {
        if (first == last)
            return true;

        auto it = last;
        if (utf8::low_surrogate(*--it))
            --it;

        return starts_encoded(it, last);
    }


    /** A UTF-32 to UTF-16 converting iterator.  Set the ErrorHandler template
        parameter to control error handling.  The default ErrorHandler is
        use_replacement_character, which simply produces a replacement
        character.

        Iter must be a bidirectional iterator with a 4-byte integral
        value_type. */
    template<
        typename Iter,
        typename Sentinel = Iter,
        typename ErrorHandler = utf8::use_replacement_character>
    struct from_utf32_iterator
    {
        using value_type = char;
        using difference_type = int;
        using pointer = uint16_t *;
        using reference = uint16_t;
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
            index_(2),
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

        Iter first_;
        Iter it_;
        Sentinel last_;
        int index_;
        std::array<uint16_t, 4> buf_;

        static uint16_t const high_surrogate_base = 0xd7c0;
        static uint16_t const low_surrogate_base = 0xdc00;

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


    /** A UTF-16 to UTF-32 converting iterator.  Set the ErrorHandler template
        parameter to control error handling.  The default ErrorHandler is
        use_replacement_character, which simply produces a replacement
        character.

        Iter must be a bidirectional iterator with a 2-byte char
        value_type. */
    template<
        typename Iter,
        typename Sentinel = Iter,
        typename ErrorHandler = utf8::use_replacement_character>
    struct to_utf32_iterator
    {
        using value_type = uint32_t;
        using difference_type = int;
        using pointer = uint32_t *;
        using reference = uint32_t;
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
                return utf8::replacement_character();
            return get_value(*it_).value_;
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
            if (utf8::low_surrogate(*--it_)) {
                if (it_ != first_ && utf8::high_surrogate(*std::prev(it_)))
                    --it_;
            }
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

#ifndef BOOST_TEXT_DOXYGEN
    private:
        struct get_value_result
        {
            reference value_;
            Iter it_;
        };

        BOOST_TEXT_CXX14_CONSTEXPR bool at_end(Iter it) const
            noexcept(!throw_on_error)
        {
            if (it == last_) {
                ErrorHandler{}(
                    "Invalid UTF-16 sequence; expected another code unit "
                    "before the end of string.");
                return true;
            } else {
                return false;
            }
        }

        BOOST_TEXT_CXX14_CONSTEXPR get_value_result
        get_value(uint16_t curr) const noexcept(!throw_on_error)
        {
            uint32_t value = 0;
            Iter next = it_;

            if (utf8::high_surrogate(curr)) {
                value = (curr - high_surrogate_base) << 10;
                ++next;
                if (at_end(next)) {
                    return get_value_result{utf8::replacement_character(),
                                            next};
                }
                curr = *next;
                if (!utf8::low_surrogate(curr)) {
                    return get_value_result{utf8::replacement_character(),
                                            next};
                }
                value += curr - low_surrogate_base;
                ++next;
            } else if (utf8::low_surrogate(curr)) {
                value = ErrorHandler{}("Invalid initial UTF-16 code unit.");
                return get_value_result{utf8::replacement_character(), next};
            } else {
                value = curr;
                ++next;
            }

            if (!utf8::valid_code_point(value)) {
                value = ErrorHandler{}(
                    "UTF-16 sequence results in invalid UTF-32 code point.");
            }

            return get_value_result{value, next};
        }

        BOOST_TEXT_CXX14_CONSTEXPR Iter increment() const
            noexcept(!throw_on_error)
        {
            if (at_end(it_))
                return it_;
            return get_value(*it_).it_;
        }

        Iter first_;
        Iter it_;
        Sentinel last_;

        static uint16_t const high_surrogate_base = 0xd7c0;
        static uint16_t const low_surrogate_base = 0xdc00;

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


    /** An insert-iterator analogous to std::insert_iterator, that also
        converts UTF-32 to UTF-16.*/
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
            it_ = c_->insert(
                it_, static_cast<uint16_t>(cp >> 10) + high_surrogate_base);
            ++it_;
            it_ = c_->insert(
                it_, static_cast<uint16_t>(cp & 0x3ff) + low_surrogate_base);
            ++it_;
            return *this;
        }

        from_utf32_insert_iterator & operator*() noexcept { return *this; }
        from_utf32_insert_iterator & operator++() noexcept { return *this; }
        from_utf32_insert_iterator operator++(int)noexcept { return *this; }

    private:
        static uint16_t const high_surrogate_base = 0xd7c0;
        static uint16_t const low_surrogate_base = 0xdc00;

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
        converts UTF-32 to UTF-16.*/
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
            c_->push_back(
                static_cast<uint16_t>(cp >> 10) + high_surrogate_base);
            c_->push_back(
                static_cast<uint16_t>(cp & 0x3ff) + low_surrogate_base);
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
        static uint16_t const high_surrogate_base = 0xd7c0;
        static uint16_t const low_surrogate_base = 0xdc00;

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
