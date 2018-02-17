#ifndef BOOST_TEXT_TEXT_VIEW_HPP
#define BOOST_TEXT_TEXT_VIEW_HPP

#include <boost/text/grapheme_iterator.hpp>
#include <boost/text/utf8.hpp>

#include <iterator>
#include <cassert>


namespace boost { namespace text {

    struct text;

    /** TODO */
    struct text_view
    {
        using iterator =
            grapheme_iterator<utf8::to_utf32_iterator<char const *>>;
        using const_iterator = iterator;
        using reverse_iterator = std::reverse_iterator<const_iterator>;
        using const_reverse_iterator = reverse_iterator;

        /** Default ctor.

            This function is constexpr.

            \post data() == nullptr && size() == 0 */
        text_view() noexcept :
            first_(make_iter(nullptr, nullptr, nullptr)),
            last_(make_iter(nullptr, nullptr, nullptr))
        {}

        /** Constructs a text_view from a null-terminated C string.  The UTF-8
            encoding is checked only at the beginning and end of the string,
            to prevent slicing of code points.  To fully check the encoding,
            use checked_encoding().

            This function is constexpr in C++14 and later.

            \pre strlen(c_str) <= max_size()
            \throw std::invalid_argument if the ends of the string are not valid
           UTF-8. \post data() == c_str && size() == strlen(c_str) */
        text_view(char const * c_str)
        {
            auto const len = strlen(c_str);
            first_ = make_iter(c_str, c_str, c_str + len);
            last_ = make_iter(c_str, c_str + len, c_str + len);
        }

        /** Constructs a text_view from an array of char.  The UTF-8 encoding
            is checked only at the beginning and end of the string, to prevent
            slicing of code points.  To fully check the encoding, use
            checked_encoding().

            This function is constexpr in C++14 and later.

            \throw std::invalid_argument if the ends of the string are not valid
           UTF-8. \pre len >= 0 \post data() == c_str && size() == len */
        text_view(char const * c_str, int len) :
            first_(make_iter(c_str, c_str, c_str + len)),
            last_(make_iter(c_str, c_str + len, c_str + len))
        {
            assert(0 <= len);
        }

        /** Constructs a text_view from a text.

            \post data() == t.begin() && size() == t.size() */
        text_view(text const & t) noexcept;

        // TODO: This should be a possibly discontiguous range, and
        // string_view et al should use a contiguous char range only.
        /** Constructs a text_view from a range of char.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if the ends of the range are not
            valid UTF-8. */
        template<typename CharRange>
        explicit text_view(
            CharRange const & r, detail::rng_alg_ret_t<int *, CharRange> = 0)
        {
            using std::begin;
            using std::end;
            first_ = make_iter(&*begin(r));
            last_ = make_iter(&*end(r));
        }

        const_iterator begin() const noexcept { return first_; }
        const_iterator end() const noexcept { return last_; }

        const_reverse_iterator rbegin() const noexcept
        {
            return reverse_iterator(--end());
        }
        const_reverse_iterator rend() const noexcept
        {
            return reverse_iterator(--begin());
        }

        bool empty() const noexcept { return first_ == last_; }

        int size() const noexcept
        {
            return last_.base().base() - first_.base().base();
        }

#if 0 // TODO: Replace with iterator versions?
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
        text_view operator()(int lo, int hi) const
        {
            if (lo < 0)
                lo += size_;
            if (hi < 0)
                hi += size_;
            assert(0 <= lo && lo <= size_);
            assert(0 <= hi && hi <= size_);
            assert(lo <= hi);
            return text_view(data_ + lo, hi - lo);
        }

        /** Returns a substring of *this, taken from the first cut chars when
            cut => 0, or the last -cut chars when cut < 0.

            This function is constexpr in C++14 and later.

            \pre 0 <= cut && cut <= size() || 0 <= -cut && -cut <= size()
            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8. */
        text_view operator()(int cut) const
        {
            int lo = 0;
            int hi = cut;
            if (cut < 0) {
                lo = cut + size_;
                hi = size_;
            }
            assert(0 <= lo && lo <= size_);
            assert(0 <= hi && hi <= size_);
            return text_view(data_ + lo, hi - lo);
        }
#endif

        /** Returns the maximum size a text_view can have. */
        int max_size() const noexcept { return INT_MAX; }

#if 0
        /** Lexicographical compare.  Returns a value < 0 when *this is
            lexicographically less than rhs, 0 if *this == rhs, and a value >
            0 if *this is lexicographically greater than rhs.

            This function is constexpr in C++14 and later. */
        int compare(text_view rhs) const noexcept
        {
            return detail::compare_impl(begin(), end(), rhs.begin(), rhs.end());
        }
#endif

        /** Swaps *this with rhs.

            This function is constexpr in C++14 and later. */
        void swap(text_view & rhs) noexcept
        {
            std::swap(first_, rhs.first_);
            std::swap(last_, rhs.last_);
        }

        // TODO: Make formatted.
        /** Stream inserter; performs unformatted output. */
        friend std::ostream & operator<<(std::ostream & os, text_view tv)
        {
            return os.write(tv.begin().base().base(), tv.size());
        }

    private:
        static iterator
        make_iter(char const * first, char const * it, char const * last)
        {
            return iterator{utf8::to_utf32_iterator<char const *>{first},
                            utf8::to_utf32_iterator<char const *>{it},
                            utf8::to_utf32_iterator<char const *>{last}};
        }

        iterator first_;
        iterator last_;
    };

#if 0
    /** This function is constexpr in C++14 and later. */
    inline bool operator==(text_view lhs, text_view rhs) noexcept
    {
        return lhs.compare(rhs) == 0;
    }

    /** This function is constexpr in C++14 and later. */
    inline bool operator!=(text_view lhs, text_view rhs) noexcept
    {
        return lhs.compare(rhs) != 0;
    }

    /** This function is constexpr in C++14 and later. */
    inline bool operator<(text_view lhs, text_view rhs) noexcept
    {
        return lhs.compare(rhs) < 0;
    }

    /** This function is constexpr in C++14 and later. */
    inline bool operator<=(text_view lhs, text_view rhs) noexcept
    {
        return lhs.compare(rhs) <= 0;
    }

    /** This function is constexpr in C++14 and later. */
    inline bool operator>(text_view lhs, text_view rhs) noexcept
    {
        return lhs.compare(rhs) > 0;
    }

    /** This function is constexpr in C++14 and later. */
    inline bool operator>=(text_view lhs, text_view rhs) noexcept
    {
        return lhs.compare(rhs) >= 0;
    }
#endif

    inline text_view::iterator begin(text_view tv) noexcept
    {
        return tv.begin();
    }
    inline text_view::iterator end(text_view tv) noexcept { return tv.end(); }

    inline text_view::reverse_iterator rbegin(text_view tv) noexcept
    {
        return tv.rbegin();
    }
    inline text_view::reverse_iterator rend(text_view tv) noexcept
    {
        return tv.rend();
    }

}}

#include <boost/text/text.hpp>

namespace boost { namespace text {

    inline text_view::text_view(text const & t) noexcept :
        first_(t.begin()),
        last_(t.end())
    {}

}}

#endif
