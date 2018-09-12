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
        using value_type = char;
        using size_type = std::ptrdiff_t;
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
        repeated_string_view(string_view sv, size_type count) noexcept :
            view_(sv),
            count_(count)
        {
            BOOST_ASSERT(0 <= sv.size());
            BOOST_ASSERT(0 <= count);
        }

        /** Constructs a repeated_string_view from a range of char.

            This function only participates in overload resolution if
            ContigCharRange models the ContigCharRange concept. */
        template<typename ContigCharRange>
        explicit repeated_string_view(
            ContigCharRange const & r,
            size_type count,
            detail::contig_rng_alg_ret_t<int *, ContigCharRange> = 0) :
            view_(string_view(r)),
            count_(count)
        {
            BOOST_ASSERT(0 <= view_.size());
            BOOST_ASSERT(0 <= count);
        }

        /** Constructs a repeated_string_view from a range of graphemes over
            an underlying range of char.

            This function only participates in overload resolution if
            ContigGraphemeRange models the ContigGraphemeRange concept. */
        template<typename ContigGraphemeRange>
        explicit repeated_string_view(
            ContigGraphemeRange const & r,
            size_type count,
            detail::contig_graph_rng_alg_ret_t<int *, ContigGraphemeRange> =
                0) :
            view_(string_view(r)),
            count_(count)
        {
            BOOST_ASSERT(0 <= view_.size());
            BOOST_ASSERT(0 <= count);
        }

        constexpr const_iterator begin() const noexcept
        {
            return const_iterator(view_.begin(), view_.size(), 0);
        }
        constexpr const_iterator end() const noexcept
        {
            return const_iterator(view_.begin(), view_.size(), size());
        }

        constexpr const_iterator cbegin() const noexcept { return begin(); }
        constexpr const_iterator cend() const noexcept { return end(); }

        constexpr const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator(end());
        }
        constexpr const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator(begin());
        }

        constexpr const_reverse_iterator crbegin() const noexcept
        {
            return rbegin();
        }
        constexpr const_reverse_iterator crend() const noexcept
        {
            return rend();
        }

        /** Returns the repeated view. */
        constexpr string_view view() const noexcept { return view_; }

        /** Returns the number of times the view is repeated. */
        constexpr size_type count() const noexcept { return count_; }

        /** Returns the char (not a reference) of *this at index i, or the
            char at index -i when i < 0.

            \pre 0 <= i && i <= size() || 0 <= -i && -i <= size()  */
        BOOST_TEXT_CXX14_CONSTEXPR char operator[](size_type i) const noexcept
        {
            if (i < 0)
                i += size();
            BOOST_ASSERT(0 <= i && i < size());
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
        unencoded_rope_view operator()(size_type lo, size_type hi) const;

        /** Returns a substring of *this, taken from the first cut chars when
            cut => 0, or the last -cut chars when cut < 0.

            This function is constexpr in C++14 and later.

            \pre 0 <= cut && cut <= size() || 0 <= -cut && -cut <= size() */
        unencoded_rope_view operator()(size_type cut) const;

        constexpr bool empty() const noexcept { return view_.empty(); }

        constexpr size_type size() const noexcept
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
                size_type tmp = count_;
                count_ = rhs.count_;
                rhs.count_ = tmp;
            }
        }

        /** Stream inserter; performs unformatted output. */
        friend std::ostream &
        operator<<(std::ostream & os, repeated_string_view rsv)
        {
            for (size_type i = 0; i < rsv.count(); ++i) {
                os.write(rsv.view().begin(), rsv.view().size());
            }
            return os;
        }

    private:
        string_view view_;
        size_type count_;
    };

    /** Creates a repeated_string_view from a string_view and a count.

        This function is constexpr in C++14 and later.

        \post count >= 0 */
    inline BOOST_TEXT_CXX14_CONSTEXPR repeated_string_view
    repeat(string_view sv, std::ptrdiff_t count)
    {
        BOOST_ASSERT(0 <= count);
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

    inline constexpr repeated_string_view::iterator
    cbegin(repeated_string_view rsv) noexcept
    {
        return rsv.cbegin();
    }
    inline constexpr repeated_string_view::iterator
    cend(repeated_string_view rsv) noexcept
    {
        return rsv.cend();
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

    inline constexpr repeated_string_view::reverse_iterator
    crbegin(repeated_string_view rsv) noexcept
    {
        return rsv.crbegin();
    }
    inline constexpr repeated_string_view::reverse_iterator
    crend(repeated_string_view rsv) noexcept
    {
        return rsv.crend();
    }

    inline int operator+(
        repeated_string_view lhs, repeated_string_view rhs) noexcept = delete;

    inline int
    operator+(string_view lhs, repeated_string_view rhs) noexcept = delete;

    inline int
    operator+(repeated_string_view lhs, string_view rhs) noexcept = delete;

}}

#include <boost/text/unencoded_rope_view.hpp>

#ifndef BOOST_TEXT_DOXYGEN

namespace std {
    template<>
    struct hash<boost::text::repeated_string_view>
    {
        using argument_type = boost::text::repeated_string_view;
        using result_type = std::size_t;
        result_type operator()(argument_type const & rsv) const noexcept
        {
            return boost::text::detail::hash_char_range(rsv);
        }
    };
}

#endif

#endif
