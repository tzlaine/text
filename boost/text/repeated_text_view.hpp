#ifndef BOOST_TEXT_REPEATED_TEXT_VIEW_HPP
#define BOOST_TEXT_REPEATED_TEXT_VIEW_HPP

#include <boost/text/text_view.hpp>


namespace boost { namespace text {

    /** A text_view, repeated count() times.  This is useful for representing
        a single char (e.g. for whitespace padding) or sequence of chars,
        repeated many times, without allocating storage. */
    struct repeated_text_view
    {
        using iterator = detail::const_repeated_chars_iterator;
        using const_iterator = detail::const_repeated_chars_iterator;
        using reverse_iterator = detail::const_reverse_repeated_chars_iterator;
        using const_reverse_iterator = detail::const_reverse_repeated_chars_iterator;

        /** Default ctor.

            This function is constexpr.

            \post view() == text_view() && count() == 0 */
        constexpr repeated_text_view () noexcept : count_ (0) {}

        /** Constructs a repeated_text_view from a text_view and a count.

            This function is constexpr in C++14 and later.

            \post view() == tv && count() == count */
        BOOST_TEXT_CXX14_CONSTEXPR
        repeated_text_view (text_view tv, std::ptrdiff_t count) noexcept :
            view_ (tv),
            count_ (count)
        {
            assert(0 <= tv.size());
            assert(0 <= count);
        }

        constexpr const_iterator begin () const noexcept
        { return const_iterator(view_.begin(), view_.size(), 0); }
        constexpr const_iterator end () const noexcept
        { return const_iterator(view_.begin(), view_.size(), size()); }

        constexpr const_reverse_iterator rbegin () const noexcept
        { return const_reverse_iterator(end()); }
        constexpr const_reverse_iterator rend () const noexcept
        { return const_reverse_iterator(begin()); }

        /** Returns the repeated view. */
        constexpr text_view view () const noexcept
        { return view_; }

        /** Returns the number of times the view is repeated. */
        constexpr std::ptrdiff_t count () const noexcept
        { return count_; }

        /** Returns the i-th char of *this (not a reference).

            This function is constexpr in C++14 and later.

            \pre i < size() */
        BOOST_TEXT_CXX14_CONSTEXPR char operator[] (int i) const noexcept
        {
            assert(i < size());
            return begin()[i];
        }

        constexpr bool empty () const noexcept
        { return view_.empty(); }

        constexpr std::ptrdiff_t size () const noexcept
        { return count_ * view_.size(); }


        /** Lexicographical compare.  Returns a value < 0 when *this is
            lexicographically less than rhs, 0 if *this == rhs, and a value >
            0 if *this is lexicographically greater than rhs.

            This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR
        int compare (repeated_text_view rhs) const noexcept
        {
            if (view_ == rhs.view_) {
                if (count_ < rhs.count_)
                    return -1;
                else if (count_ == rhs.count_)
                    return 0;
                else
                    return 1;
            } else {
                repeated_text_view shorter = view().size() < rhs.view().size() ? *this : rhs;
                repeated_text_view longer = view().size() < rhs.view().size() ? rhs : *this;
                if (shorter.view() == longer.view()(shorter.view().size())) {
                    // If one is a prefix of the other, the prefix might be
                    // repeated within the other an arbitrary number of times,
                    // so we need to do this the hard way...
                    const_iterator lhs_first = begin();
                    const_iterator const lhs_last = end();
                    const_iterator rhs_first = rhs.begin();
                    const_iterator const rhs_last = rhs.end();
                    while (lhs_first != lhs_last && rhs_first != rhs_last) {
                        if (*lhs_first < *rhs_first)
                            return -1;
                        else if (*lhs_first > *rhs_first)
                            return 1;
                        ++lhs_first;
                        ++rhs_first;
                    }
                    if (lhs_first == lhs_last) {
                        if (rhs_first == rhs_last)
                            return 0;
                        else
                            return -1;
                    } else {
                        return 1;
                    }
                } else {
                    return view_.compare(rhs.view_);
                }
            }
        }

        /** Swaps *this with rhs.

            This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR void swap (repeated_text_view & rhs) noexcept
        {
            {
                text_view tmp = view_;
                view_ = rhs.view_;
                rhs.view_ = tmp;
            }
            {
                std::ptrdiff_t tmp = count_;
                count_ = rhs.count_;
                rhs.count_ = tmp;
            }
        }

        /** Stream inserter; performs formatted output. */
        friend std::ostream & operator<< (std::ostream & os, repeated_text_view rtv)
        {
            if (os.good()) {
                detail::pad_width_before(os, rtv.size());
                for (std::ptrdiff_t i = 0; i < rtv.count(); ++i) {
                    if (!os.good())
                        break;
                    os.write(rtv.view().begin(), rtv.view().size());
                }
                if (os.good())
                    detail::pad_width_after(os, rtv.size());
            }
            return os;
        }

    private:
        text_view view_;
        std::ptrdiff_t count_;
    };

    /** Creates a repeated_text_view from a text_view and a count.

        This function is constexpr in C++14 and later.

        \post count >= 0 */
    inline BOOST_TEXT_CXX14_CONSTEXPR
    repeated_text_view repeat (text_view tv, std::ptrdiff_t count)
    {
        assert(0 <= count);
        return repeated_text_view(tv, count);
    }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR
    bool operator== (repeated_text_view lhs, repeated_text_view rhs) noexcept
    { return lhs.compare(rhs) == 0; }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR
    bool operator!= (repeated_text_view lhs, repeated_text_view rhs) noexcept
    { return lhs.compare(rhs) != 0; }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator< (repeated_text_view lhs, repeated_text_view rhs) noexcept
    { return lhs.compare(rhs) < 0; }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator<= (repeated_text_view lhs, repeated_text_view rhs) noexcept
    { return lhs.compare(rhs) <= 0; }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator> (repeated_text_view lhs, repeated_text_view rhs) noexcept
    { return lhs.compare(rhs) > 0; }

    /** This function is constexpr in C++14 and later. */
    inline BOOST_TEXT_CXX14_CONSTEXPR bool
    operator>= (repeated_text_view lhs, repeated_text_view rhs) noexcept
    { return lhs.compare(rhs) >= 0; }

    inline constexpr repeated_text_view::iterator begin (repeated_text_view rtv) noexcept
    { return rtv.begin(); }
    inline constexpr repeated_text_view::iterator end (repeated_text_view rtv) noexcept
    { return rtv.end(); }

    inline constexpr repeated_text_view::reverse_iterator rbegin (repeated_text_view rtv) noexcept
    { return rtv.rbegin(); }
    inline constexpr repeated_text_view::reverse_iterator rend (repeated_text_view rtv) noexcept
    { return rtv.rend(); }

} }

#endif
