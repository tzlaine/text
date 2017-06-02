#ifndef BOOST_TEXT_TEXT_VIEW_HPP
#define BOOST_TEXT_TEXT_VIEW_HPP

#include <boost/text/utf8.hpp>

#include <boost/text/detail/utility.hpp>
#include <boost/text/detail/iterator.hpp>

#include <ostream>

#include <cassert>


namespace boost { namespace text {

    struct text;

    struct text_view
    {
        using iterator = char const *;
        using const_iterator = char const *;
        using reverse_iterator = detail::const_reverse_char_iterator;
        using const_reverse_iterator = detail::const_reverse_char_iterator;

        constexpr text_view () noexcept :
            data_ (nullptr),
            size_ (0)
        {}

        constexpr text_view (char const * c_str) :
            data_ (c_str),
            size_ (detail::strlen(c_str))
        {
            assert(detail::strlen(c_str) <= INT_MAX);
            if (!utf8::starts_encoded(begin(), end()))
                throw std::invalid_argument("The start of the given string is not valid UTF-8.");
            if (!utf8::ends_encoded(begin(), end()))
                throw std::invalid_argument("The end of the given string is not valid UTF-8.");
        }

        constexpr text_view (char const * c_str, int len) :
            data_ (c_str),
            size_ (len)
        {
            assert(0 <= len);
            if (!utf8::starts_encoded(begin(), end()))
                throw std::invalid_argument("The start of the given string is not valid UTF-8.");
            if (!utf8::ends_encoded(begin(), end()))
                throw std::invalid_argument("The end of the given string is not valid UTF-8.");
        }

        constexpr text_view (char const * c_str, utf8::unchecked_t) noexcept :
            data_ (c_str),
            size_ (detail::strlen(c_str))
        { assert(detail::strlen(c_str) <= INT_MAX); }

        constexpr text_view (char const * c_str, int len, utf8::unchecked_t) noexcept :
            data_ (c_str),
            size_ (len)
        { assert(0 <= len); }

        constexpr text_view (text const & t) noexcept;

        constexpr text_view (text_view const & rhs) noexcept :
            data_ (rhs.data_),
            size_ (rhs.size_)
        {}
        constexpr text_view & operator= (text_view const & rhs) noexcept
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

        constexpr char operator[] (int i) const noexcept
        {
            assert(i < size_);
            return data_[i];
        }

        constexpr text_view operator() (int lo, int hi) const
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

        constexpr text_view operator() (int lo) const
        {
            if (lo < 0)
                lo += size_;
            assert(0 <= lo && lo <= size_);
            return text_view(data_ + lo, size_ - lo);
        }

        constexpr int max_size () const noexcept
        { return INT_MAX; }

        // TODO: operator<=> () const
        constexpr int compare (text_view rhs) const noexcept
        { return detail::compare_impl(begin(), end(), rhs.begin(), rhs.end()); }

        friend constexpr bool operator== (text_view lhs, text_view rhs) noexcept
        { return lhs.compare(rhs) == 0; }

        friend constexpr bool operator!= (text_view lhs, text_view rhs) noexcept
        { return lhs.compare(rhs) != 0; }

        friend constexpr bool operator< (text_view lhs, text_view rhs) noexcept
        { return lhs.compare(rhs) < 0; }

        friend constexpr bool operator<= (text_view lhs, text_view rhs) noexcept
        { return lhs.compare(rhs) <= 0; }

        friend constexpr bool operator> (text_view lhs, text_view rhs) noexcept
        { return lhs.compare(rhs) > 0; }

        friend constexpr bool operator>= (text_view lhs, text_view rhs) noexcept
        { return lhs.compare(rhs) >= 0; }


        constexpr void swap (text_view & rhs) noexcept
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

        friend constexpr iterator begin (text_view v) noexcept
        { return v.begin(); }
        friend constexpr iterator end (text_view v) noexcept
        { return v.end(); }

        friend constexpr reverse_iterator rbegin (text_view v) noexcept
        { return v.rbegin(); }
        friend constexpr reverse_iterator rend (text_view v) noexcept
        { return v.rend(); }

        friend std::ostream & operator<< (std::ostream & os, text_view view)
        { return os.write(view.begin(), view.size()); }

    private:
        char const * data_;
        int size_;
    };

    namespace literals {

        inline constexpr text_view operator"" _tv (char const * str, std::size_t len) noexcept
        {
            assert(len < INT_MAX);
            return text_view(str, len);
        }

    }

    constexpr text_view encoding_checked (text_view view)
    {
        if (!utf8::encoded(view.begin(), view.end()))
            throw std::invalid_argument("Invalid UTF-8 encoding");
        return view;
    }

    struct repeated_text_view
    {
        using iterator = detail::const_repeated_chars_iterator;
        using const_iterator = detail::const_repeated_chars_iterator;

        constexpr repeated_text_view () noexcept : count_ (0) {}

        constexpr repeated_text_view (text_view view, std::ptrdiff_t count) noexcept :
            view_ (view),
            count_ (count)
        { assert(0 <= view.size()); }

        constexpr text_view view () const noexcept
        { return view_; }
        constexpr std::ptrdiff_t count () const noexcept
        { return count_; }

        constexpr bool empty () const noexcept
        { return view_.empty(); }

        constexpr std::ptrdiff_t size () const noexcept
        { return count_ * view_.size(); }

        constexpr const_iterator begin () const noexcept
        { return const_iterator(view_.begin(), view_.end(), count_); }
        constexpr const_iterator end () const noexcept
        { return const_iterator(view_.begin(), view_.end()); }

        friend constexpr iterator begin (repeated_text_view v) noexcept
        { return v.begin(); }
        friend constexpr iterator end (repeated_text_view v) noexcept
        { return v.end(); }

        friend std::ostream & operator<< (std::ostream & os, repeated_text_view rv)
        {
            for (std::ptrdiff_t i = 0; i < rv.count(); ++i) {
                os.write(rv.view().begin(), rv.view().size());
            }
            return os;
        }

    private:
        text_view view_;
        std::ptrdiff_t count_;
    };

    constexpr repeated_text_view repeat (text_view view, std::ptrdiff_t count)
    { return repeated_text_view(view, count); }

} }

#include <boost/text/text.hpp>

namespace boost { namespace text {

    inline constexpr text_view::text_view (text const & t) noexcept :
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

   5: text{,_view} guarantee they are UTF-8 encoded.

   6: text is strongly exception-safe, always 0-terminated.

   7: operator+(text & t, char c) does not exist, since a single char is no
   longer a single code point.

   8: First and last code point are checked in a text_view constructed from a
   UTF-8 C string.  This is to prevent slicing CPs.

   9: checked_encoding() is provided as a free function, since it does extra
   work (as opposed to the unchecked_t ctors which doe less that the vanilla
   ones).  This also reduces compile times.

   10: Unicode 9 is used.  No other versions are supported.

   11: text API leaves explicit null terminators out of inserts/replaces from
   {repeated_,}text_view, but the lower-level, less-safe Iter interface can be
   used to accomplish this (or a text_view("\0") hack).

   12: Negative indexing should be allowed on all random access ranges.  The
   disparity with builtin arrays shouldn't matter, as builtin arrays are
   incongruous for many other reasons.

*/

// Not rationale:

// TODO: Do text_view == using address + size to short-circuit string
// compares.

// TODO: Document how much more expensive post-incr/post-decr are for UTF-8
// iterators.

#endif
