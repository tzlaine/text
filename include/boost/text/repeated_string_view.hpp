#ifndef BOOST_TEXT_REPEATED_STRING_VIEW_HPP
#define BOOST_TEXT_REPEATED_STRING_VIEW_HPP

#include <boost/text/string_view.hpp>


namespace boost { namespace text {

    struct unencoded_rope_view;

    /** A string_view, repeated count() times.  This is useful for representing
        a single char (e.g. for whitespace padding) or sequence of chars,
        repeated many times, without allocating storage. */
    struct repeated_string_view
    {
        using iterator = detail::const_repeated_chars_iterator;
        using const_iterator = detail::const_repeated_chars_iterator;
        using reverse_iterator = detail::const_reverse_repeated_chars_iterator;
        using const_reverse_iterator =
            detail::const_reverse_repeated_chars_iterator;

        /** Default ctor.

            This function is constexpr.

            \post view() == string_view() && count() == 0 */
        constexpr repeated_string_view() noexcept : count_(0) {}

        /** Constructs a repeated_string_view from a string_view and a count.

            This function is constexpr in C++14 and later.

            \post view() == sv && count() == count */
        BOOST_TEXT_CXX14_CONSTEXPR
        repeated_string_view(string_view sv, std::ptrdiff_t count) noexcept :
            view_(sv),
            count_(count)
        {
            assert(0 <= sv.size());
            assert(0 <= count);
        }

        /** Constructs a repeated_string_view from a range of char.

            This function only participates in overload resolution if
            ContigCharRange models the ContigCharRange concept. */
        template<typename ContigCharRange>
        explicit repeated_string_view(
            ContigCharRange const & r,
            std::ptrdiff_t count,
            detail::contig_rng_alg_ret_t<int *, ContigCharRange> = 0) :
            view_(string_view(r)),
            count_(count)
        {
            assert(0 <= view_.size());
            assert(0 <= count);
        }

        /** Constructs a repeated_string_view from a range of graphemes over
            an underlying range of char.

            This function only participates in overload resolution if
            ContigGraphemeRange models the ContigGraphemeRange concept. */
        template<typename ContigGraphemeRange>
        explicit repeated_string_view(
            ContigGraphemeRange const & r,
            std::ptrdiff_t count,
            detail::contig_graph_rng_alg_ret_t<int *, ContigGraphemeRange> =
                0) :
            view_(string_view(r)),
            count_(count)
        {
            assert(0 <= view_.size());
            assert(0 <= count);
        }

        constexpr const_iterator begin() const noexcept
        {
            return const_iterator(view_.begin(), view_.size(), 0);
        }
        constexpr const_iterator end() const noexcept
        {
            return const_iterator(view_.begin(), view_.size(), size());
        }

        constexpr const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator(end());
        }
        constexpr const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator(begin());
        }

        /** Returns the repeated view. */
        constexpr string_view view() const noexcept { return view_; }

        /** Returns the number of times the view is repeated. */
        constexpr std::ptrdiff_t count() const noexcept { return count_; }

        /** Returns the i-th char of *this (not a reference).

            This function is constexpr in C++14 and later.

            \pre i < size() */
        BOOST_TEXT_CXX14_CONSTEXPR char operator[](int i) const noexcept
        {
            assert(i < size());
            return begin()[i];
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
            \pre lo <= hi */
        unencoded_rope_view operator()(int lo, int hi) const;

        /** Returns a substring of *this, taken from the first cut chars when
            cut => 0, or the last -cut chars when cut < 0.

            This function is constexpr in C++14 and later.

            \pre 0 <= cut && cut <= size() || 0 <= -cut && -cut <= size() */
        unencoded_rope_view operator()(int cut) const;

        constexpr bool empty() const noexcept { return view_.empty(); }

        constexpr std::ptrdiff_t size() const noexcept
        {
            return count_ * view_.size();
        }


        /** Swaps *this with rhs.

            This function is constexpr in C++14 and later. */
        BOOST_TEXT_CXX14_CONSTEXPR void
        swap(repeated_string_view & rhs) noexcept
        {
            {
                string_view tmp = view_;
                view_ = rhs.view_;
                rhs.view_ = tmp;
            }
            {
                std::ptrdiff_t tmp = count_;
                count_ = rhs.count_;
                rhs.count_ = tmp;
            }
        }

        /** Stream inserter; performs unformatted output. */
        friend std::ostream &
        operator<<(std::ostream & os, repeated_string_view rsv)
        {
            for (std::ptrdiff_t i = 0; i < rsv.count(); ++i) {
                os.write(rsv.view().begin(), rsv.view().size());
            }
            return os;
        }

    private:
        string_view view_;
        std::ptrdiff_t count_;
    };

    /** Creates a repeated_string_view from a string_view and a count.

        This function is constexpr in C++14 and later.

        \post count >= 0 */
    inline BOOST_TEXT_CXX14_CONSTEXPR repeated_string_view
    repeat(string_view sv, std::ptrdiff_t count)
    {
        assert(0 <= count);
        return repeated_string_view(sv, count);
    }

    inline bool
    operator==(repeated_string_view lhs, repeated_string_view rhs) noexcept
    {
        return algorithm::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    inline bool
    operator!=(repeated_string_view lhs, repeated_string_view rhs) noexcept
    {
        return !(lhs == rhs);
    }

    inline constexpr repeated_string_view::iterator
    begin(repeated_string_view rsv) noexcept
    {
        return rsv.begin();
    }
    inline constexpr repeated_string_view::iterator
    end(repeated_string_view rsv) noexcept
    {
        return rsv.end();
    }

    inline constexpr repeated_string_view::reverse_iterator
    rbegin(repeated_string_view rsv) noexcept
    {
        return rsv.rbegin();
    }
    inline constexpr repeated_string_view::reverse_iterator
    rend(repeated_string_view rsv) noexcept
    {
        return rsv.rend();
    }

}}

#    include <boost/text/unencoded_rope_view.hpp>

#endif
