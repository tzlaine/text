#ifndef BOOST_TEXT_TEXT_VIEW_HPP
#define BOOST_TEXT_TEXT_VIEW_HPP

#include <boost/text/utf8.hpp>

#include <boost/text/detail/algorithm.hpp>
#include <boost/text/detail/iterator.hpp>
#include <boost/text/detail/utility.hpp>

#include <cassert>


namespace boost { namespace text {

    struct text;

    struct text_view
    {
        using iterator = char const *;
        using const_iterator = char const *;
        using reverse_iterator = detail::const_reverse_char_iterator;
        using const_reverse_iterator = detail::const_reverse_char_iterator;

        /** Default ctor.

            This function is constexpr.

            \post data() == nullptr && size() == 0 */
        constexpr text_view () noexcept :
            data_ (nullptr),
            size_ (0)
        {}

        /** Constructs a text_view from a null-terminated C string.  The UTF-8
            encoding is checked only at the beginning and end of the string,
            to prevent slicing of code points.  To fully check the encoding,
            use checked_encoding().

            This function is constexpr in C++14 and later.

            \pre strlen(c_str) <= max_size()
            \throw std::invalid_argument if the ends of the string are not valid UTF-8.
            \post data() == c_str && size() == strlen(c_str) */
        BOOST_TEXT_CXX14_CONSTEXPR text_view (char const * c_str) :
            data_ (c_str),
            size_ (detail::strlen(c_str))
        {
            assert(detail::strlen(c_str) <= max_size());
            if (!utf8::starts_encoded(begin(), end()))
                throw std::invalid_argument("The start of the given string is not valid UTF-8.");
            if (!utf8::ends_encoded(begin(), end()))
                throw std::invalid_argument("The end of the given string is not valid UTF-8.");
        }

        /** Constructs a text_view from an array of char.  The UTF-8 encoding
            is checked only at the beginning and end of the string, to prevent
            slicing of code points.  To fully check the encoding, use
            checked_encoding().

            This function is constexpr in C++14 and later.

            \throw std::invalid_argument if the ends of the string are not valid UTF-8.
            \pre len >= 0
            \post data() == c_str && size() == len */
        BOOST_TEXT_CXX14_CONSTEXPR text_view (char const * c_str, int len) :
            data_ (c_str),
            size_ (len)
        {
            assert(0 <= len);
            if (!utf8::starts_encoded(begin(), end()))
                throw std::invalid_argument("The start of the given string is not valid UTF-8.");
            if (!utf8::ends_encoded(begin(), end()))
                throw std::invalid_argument("The end of the given string is not valid UTF-8.");
        }

        /** Constructs a text_view from a null-terminated C string, without
            any check of UTF-8 encoding.

            This function is constexpr in C++14 and later.

            \pre strlen(c_str) <= max_size()
            \post data() == c_str && size() == strlen(c_str) */
        BOOST_TEXT_CXX14_CONSTEXPR
        text_view (char const * c_str, utf8::unchecked_t) noexcept :
            data_ (c_str),
            size_ (detail::strlen(c_str))
        { assert(detail::strlen(c_str) <= max_size()); }

        /** Constructs a text_view from an array of char, without any check of
            UTF-8 encoding.

            This function is constexpr in C++14 and later.

            \pre len >= 0
            \post data() == c_str && size() == len */
        BOOST_TEXT_CXX14_CONSTEXPR
        text_view (char const * c_str, int len, utf8::unchecked_t) noexcept :
            data_ (c_str),
            size_ (len)
        { assert(0 <= len); }

        /** Constructs a text_view from a text.

            \post data() == t.begin() && size() == t.size() */
        text_view (text const & t) noexcept;

        /** Constructs a text_view from a range of char.

            This function only participates in overload resolution if
            CharRange models the Char_range concept.

            \throw std::invalid_argument if the ends of the range are not
            valid UTF-8. */
        template <typename CharRange>
        explicit text_view (
            CharRange const & r,
            detail::rng_alg_ret_t<int *, CharRange> = 0
        )
        { *this = text_view(&*r.begin(), r.end() - r.begin()); }

        constexpr text_view (text_view const & rhs) noexcept :
            data_ (rhs.data_),
            size_ (rhs.size_)
        {}

        /** Assignment.

            This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR
        text_view & operator= (text_view const & rhs) noexcept
        {
            data_ = rhs.data_;
            size_ = rhs.size_;
            return *this;
        }

        constexpr const_iterator begin () const noexcept { return data_; }
        constexpr const_iterator end () const noexcept { return data_ + size_; }

        constexpr const_reverse_iterator rbegin () const noexcept { return reverse_iterator(end()); }
        constexpr const_reverse_iterator rend () const noexcept { return reverse_iterator(begin()); }

        constexpr bool empty () const noexcept
        { return size_ == 0; }

        constexpr int size () const noexcept
        { return size_; }

        /** Returns the i-th char of *this (not a reference).

            This function is constexpr in C++14 and later.

            \pre i < size() */
        BOOST_TEXT_CXX14_CONSTEXPR char operator[] (int i) const noexcept
        {
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
        BOOST_TEXT_CXX14_CONSTEXPR text_view operator() (int lo, int hi) const
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
        BOOST_TEXT_CXX14_CONSTEXPR text_view operator() (int cut) const
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

        /** Returns the maximum size a text_view can have. */
        constexpr int max_size () const noexcept
        { return INT_MAX; }

        /** Lexicographical compare.  Returns a value < 0 when *this is
            lexicographically less than rhs, 0 if *this == rhs, and a value >
            0 if *this is lexicographically greater than rhs.

            This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR int compare (text_view rhs) const noexcept
        { return detail::compare_impl(begin(), end(), rhs.begin(), rhs.end()); }

        /** Swaps *this with rhs.

            This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR void swap (text_view & rhs) noexcept
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

        /** Stream inserter; performs formatted output. */
        friend std::ostream & operator<< (std::ostream & os, text_view tv)
        {
            if (os.good()) {
                detail::pad_width_before(os, tv.size());
                if (os.good())
                    os.write(tv.begin(), tv.size());
                if (os.good())
                    detail::pad_width_after(os, tv.size());
            }
            return os;
        }

    private:
        char const * data_;
        int size_;
    };

    namespace literals {

        /** Creates a text_view from a char string literal.

            This function is constexpr in C++14 and later.

            \throw std::invalid_argument if the ends of the string are not
            valid UTF-8.  */
        inline BOOST_TEXT_CXX14_CONSTEXPR
        text_view operator"" _tv (char const * str, std::size_t len) noexcept
        {
            assert(len < INT_MAX);
            return text_view(str, len);
        }

    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR
    bool operator== (text_view lhs, text_view rhs) noexcept
    { return lhs.compare(rhs) == 0; }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR
    bool operator!= (text_view lhs, text_view rhs) noexcept
    { return lhs.compare(rhs) != 0; }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR
    bool operator< (text_view lhs, text_view rhs) noexcept
    { return lhs.compare(rhs) < 0; }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR
    bool operator<= (text_view lhs, text_view rhs) noexcept
    { return lhs.compare(rhs) <= 0; }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR
    bool operator> (text_view lhs, text_view rhs) noexcept
    { return lhs.compare(rhs) > 0; }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR
    bool operator>= (text_view lhs, text_view rhs) noexcept
    { return lhs.compare(rhs) >= 0; }

    inline constexpr text_view::iterator begin (text_view tv) noexcept
    { return tv.begin(); }
    inline constexpr text_view::iterator end (text_view tv) noexcept
    { return tv.end(); }

    inline constexpr text_view::reverse_iterator rbegin (text_view tv) noexcept
    { return tv.rbegin(); }
    inline constexpr text_view::reverse_iterator rend (text_view tv) noexcept
    { return tv.rend(); }

    /** Forwards tv when it is entirely UTF-8 encoded; throws otherwise.

        This function is constexpr in C++14 and later.

        \throw std::invalid_argument when tv is not UTF-8 encoded. */
    inline BOOST_TEXT_CXX14_CONSTEXPR text_view checked_encoding (text_view tv)
    {
        if (!utf8::encoded(tv.begin(), tv.end()))
            throw std::invalid_argument("Invalid UTF-8 encoding");
        return tv;
    }

} }

#include <boost/text/text.hpp>

namespace boost { namespace text {

    inline text_view::text_view (text const & t) noexcept :
        data_ (t.begin()),
        size_ (t.size())
    {}

} }

/* Rationale

   1: use of signed types >= sizeof(int) for sizes -- better code gen;
   negative indexing

   2: including null terminator in strings

   3: removal of data(), front(), back(), assign()

   4: text{,_view}::iterator is a char const *, and that's fine -- don't use
   the bare iterators unless you know what you're doing.

   5: text{,_view} guarantee they are UTF-8 encoded, as long as one only uses
   the safe interfaces.

   6: text is strongly exception-safe, always 0-terminated.

   7: operator+(text & t, char c) does not exist, since a single char is no
   longer a single code point.

   8: First and last code point are checked in a text_view constructed from a
   UTF-8 C string.  This is to prevent slicing CPs.

   9: checked_encoding() is provided as a free function, since it does extra
   work (as opposed to the unchecked_t ctors which doe less that the vanilla
   ones).  This also reduces compile times.

   10: Unicode 9 is used.  No other versions are supported.

   11: The direct use of iterators in an interface is a signal that the
   interface is unsafe.  All others are safe.  An unsafe iterator interface
   can be made safe again by using a converting iterator.

   12: text API leaves explicit null terminators out of inserts/replaces from
   {repeated_,}text_view, but the lower-level, less-safe Iter interface can be
   used to accomplish this (or a text_view("\0") hack).

   13: Negative indexing should be allowed on all random access ranges.  The
   disparity with builtin arrays shouldn't matter, as builtin arrays are
   incongruous for many other reasons.

   14: There is no CharRange for rope like there is for text.  text converts a
   CharRange into a text_view, but then always copies the bytes into its
   storage.  rope does not convert text_views into allocated storage, but does
   so with texts.  So the user should be able to choose what kind of segment
   gets inserted into the rope by constructing either a text or text_view
   explicitly first.  Just as importantly, consider: x + std::string("foo").
   Should this create a rope that refers to the contents of the temporary?
   Should it create a text that owns the contents?  The user should decide
   this, case-by-case.

   15: For an overloaded function foo(), providing an overload for each of
   {text,rope}_view will require you to also add an overload for char const *,
   so pick one or the otehr.

*/

// Not rationale:

// TODO: Do text_view == using address + size to short-circuit string
// compares.

// TODO: Document how much more expensive post-incr/post-decr are for UTF-8
// iterators.

#endif
