#ifndef BOOST_TEXT_STRING_VIEW_HPP
#define BOOST_TEXT_STRING_VIEW_HPP

#include <boost/text/config.hpp>

#include <boost/text/detail/algorithm.hpp>
#include <boost/text/detail/iterator.hpp>
#include <boost/text/detail/utility.hpp>

#include <cassert>


namespace boost { namespace text {

    struct string;

    /** A reference to a constant contiguous sequence of char.  The sequence
        is assumed to be UTF-8 encoded, though it is possible to construct a
        sequence which is not. */
    struct string_view
    {
        using iterator = char const *;
        using const_iterator = char const *;
        using reverse_iterator = detail::const_reverse_char_iterator;
        using const_reverse_iterator = detail::const_reverse_char_iterator;

        /** Default ctor.

            This function is constexpr.

            \post data() == nullptr && size() == 0 */
        constexpr string_view() noexcept : data_(nullptr), size_(0) {}

        /** Constructs a string_view from a null-terminated C string.  The UTF-8
            encoding is checked only at the beginning and end of the string,
            to prevent slicing of code points.  To fully check the encoding,
            use checked_encoding().

            This function is constexpr in C++14 and later.

            \pre strlen(c_str) <= max_size()
            \throw std::invalid_argument if the ends of the string are not valid
           UTF-8. \post data() == c_str && size() == strlen(c_str) */
        BOOST_TEXT_CXX14_CONSTEXPR string_view(char const * c_str) :
            data_(c_str),
            size_(detail::strlen(c_str))
        {
            assert(detail::strlen(c_str) <= max_size());
        }

        /** Constructs a string_view from an array of char.  The UTF-8 encoding
            is checked only at the beginning and end of the string, to prevent
            slicing of code points.  To fully check the encoding, use
            checked_encoding().

            This function is constexpr in C++14 and later.

            \throw std::invalid_argument if the ends of the string are not valid
           UTF-8. \pre len >= 0 \post data() == c_str && size() == len */
        BOOST_TEXT_CXX14_CONSTEXPR string_view(char const * c_str, int len) :
            data_(c_str),
            size_(len)
        {
            assert(0 <= len);
        }

        /** Constructs a string_view from an array of char.  The UTF-8 encoding
            is checked only at the beginning and end of the string, to prevent
            slicing of code points.  To fully check the encoding, use
            checked_encoding().

            This function is constexpr in C++14 and later.

            \throw std::invalid_argument if the ends of the string are not valid
           UTF-8. \pre len >= 0 \post data() == c_str && size() == len */
        template<int N>
        BOOST_TEXT_CXX14_CONSTEXPR string_view(char const (&c_str)[N]) :
            string_view(c_str, N)
        {}

        /** Constructs a string_view from a string.

            \post data() == t.begin() && size() == t.size() */
        string_view(string const & t) noexcept;

        /** Constructs a string_view from a range of char.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if the ends of the range are not
            valid UTF-8. */
        template<typename CharRange>
        explicit string_view(
            CharRange const & r, detail::rng_alg_ret_t<int *, CharRange> = 0)
        {
            using std::begin;
            using std::end;
            if (begin(r) == end(r)) {
                data_ = nullptr;
                size_ = 0;
            } else {
                *this = string_view(&*begin(r), end(r) - begin(r));
            }
        }

        constexpr string_view(string_view const & rhs) noexcept :
            data_(rhs.data_),
            size_(rhs.size_)
        {}

        /** Assignment.

            This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR
        string_view & operator=(string_view const & rhs) noexcept
        {
            data_ = rhs.data_;
            size_ = rhs.size_;
            return *this;
        }

        constexpr const_iterator begin() const noexcept { return data_; }
        constexpr const_iterator end() const noexcept { return data_ + size_; }

        constexpr const_reverse_iterator rbegin() const noexcept
        {
            return reverse_iterator(end());
        }
        constexpr const_reverse_iterator rend() const noexcept
        {
            return reverse_iterator(begin());
        }

        constexpr bool empty() const noexcept { return size_ == 0; }

        constexpr int size() const noexcept { return size_; }

        /** Returns the char (not a reference) of *this at index i, or the
            char at index -i when i < 0.

            This function is constexpr in C++14 and later.

            \pre 0 <= i && i <= size() || 0 <= -i && -i <= size()  */
        BOOST_TEXT_CXX14_CONSTEXPR char operator[](int i) const noexcept
        {
            if (i < 0)
                i += size_;
            assert(i < size_);
            return data_[i];
        }

        /** Returns a substring of *this, taken from the range of chars at
            offsets [lo, hi).  If either of lo or hi is a negative value x, x
            is taken to be an offset from the end, and so x + size() is used
            instead.

            This function is constexpr in C++14 and later.

            These preconditions apply to the values used after size() is added
            to any negative arguments.

            \pre 0 <= lo && lo <= size()
            \pre 0 <= hi && lhi <= size()
            \pre lo <= hi
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
        BOOST_TEXT_CXX14_CONSTEXPR string_view operator()(int lo, int hi) const
        {
            if (lo < 0)
                lo += size_;
            if (hi < 0)
                hi += size_;
            assert(0 <= lo && lo <= size_);
            assert(0 <= hi && hi <= size_);
            assert(lo <= hi);
            return string_view(data_ + lo, hi - lo);
        }

        /** Returns a substring of *this, taken from the first cut chars when
            cut => 0, or the last -cut chars when cut < 0.

            This function is constexpr in C++14 and later.

            \pre 0 <= cut && cut <= size() || 0 <= -cut && -cut <= size()
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
        BOOST_TEXT_CXX14_CONSTEXPR string_view operator()(int cut) const
        {
            int lo = 0;
            int hi = cut;
            if (cut < 0) {
                lo = cut + size_;
                hi = size_;
            }
            assert(0 <= lo && lo <= size_);
            assert(0 <= hi && hi <= size_);
            return string_view(data_ + lo, hi - lo);
        }

        /** Returns the maximum size a string_view can have. */
        constexpr int max_size() const noexcept { return INT_MAX; }

        /** Lexicographical compare.  Returns a value < 0 when *this is
            lexicographically less than rhs, 0 if *this == rhs, and a value >
            0 if *this is lexicographically greater than rhs.

            This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR int compare(string_view rhs) const noexcept
        {
            return detail::compare_impl(begin(), end(), rhs.begin(), rhs.end());
        }

        /** Swaps *this with rhs.

            This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR void swap(string_view & rhs) noexcept
        {
            {
                char const * tmp = data_;
                data_ = rhs.data_;
                rhs.data_ = tmp;
            }
            {
                int tmp = size_;
                size_ = rhs.size_;
                rhs.size_ = tmp;
            }
        }

        /** Stream inserter; performs unformatted output. */
        friend std::ostream & operator<<(std::ostream & os, string_view tv)
        {
            return os.write(tv.begin(), tv.size());
        }

    private:
        char const * data_;
        int size_;
    };

    namespace literals {

        /** Creates a string_view from a char string literal.

            This function is constexpr in C++14 and later.

            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8.  */
        inline BOOST_TEXT_CXX14_CONSTEXPR string_view
        operator"" _sv(char const * str, std::size_t len) noexcept
        {
            assert(len < INT_MAX);
            return string_view(str, len);
        }
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator==(string_view lhs, string_view rhs) noexcept
    {
        return lhs.compare(rhs) == 0;
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator==(char const * lhs, string_view rhs) noexcept
    {
        return string_view(lhs) == rhs;
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator==(string_view lhs, char const * rhs) noexcept
    {
        return lhs == string_view(rhs);
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator!=(string_view lhs, string_view rhs) noexcept
    {
        return lhs.compare(rhs) != 0;
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator!=(char const * lhs, string_view rhs) noexcept
    {
        return string_view(lhs) != rhs;
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator!=(string_view lhs, char const * rhs) noexcept
    {
        return lhs != string_view(rhs);
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator<(string_view lhs, string_view rhs) noexcept
    {
        return lhs.compare(rhs) < 0;
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator<(char const * lhs, string_view rhs) noexcept
    {
        return string_view(lhs) < rhs;
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator<(string_view lhs, char const * rhs) noexcept
    {
        return lhs < string_view(rhs);
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator<=(string_view lhs, string_view rhs) noexcept
    {
        return lhs.compare(rhs) <= 0;
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator<=(char const * lhs, string_view rhs) noexcept
    {
        return string_view(lhs) <= rhs;
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator<=(string_view lhs, char const * rhs) noexcept
    {
        return lhs <= string_view(rhs);
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator>(string_view lhs, string_view rhs) noexcept
    {
        return lhs.compare(rhs) > 0;
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator>(char const * lhs, string_view rhs) noexcept
    {
        return string_view(lhs) > rhs;
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator>(string_view lhs, char const * rhs) noexcept
    {
        return lhs > string_view(rhs);
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator>=(string_view lhs, string_view rhs) noexcept
    {
        return lhs.compare(rhs) >= 0;
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator>=(char const * lhs, string_view rhs) noexcept
    {
        return string_view(lhs) >= rhs;
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator>=(string_view lhs, char const * rhs) noexcept
    {
        return lhs >= string_view(rhs);
    }

    inline constexpr string_view::iterator begin(string_view tv) noexcept
    {
        return tv.begin();
    }
    inline constexpr string_view::iterator end(string_view tv) noexcept
    {
        return tv.end();
    }

    inline constexpr string_view::reverse_iterator
    rbegin(string_view tv) noexcept
    {
        return tv.rbegin();
    }
    inline constexpr string_view::reverse_iterator rend(string_view tv) noexcept
    {
        return tv.rend();
    }

}}

#include <boost/text/string.hpp>

namespace boost { namespace text {

    inline string_view::string_view(string const & t) noexcept :
        data_(t.begin()),
        size_(t.size())
    {}

}}

#endif
