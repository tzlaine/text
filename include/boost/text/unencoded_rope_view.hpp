// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_UNENCODED_ROPE_VIEW_HPP
#define BOOST_TEXT_UNENCODED_ROPE_VIEW_HPP

#include <boost/text/detail/iterator.hpp>
#include <boost/text/detail/rope.hpp>


namespace boost { namespace text { inline namespace v1 {

    struct unencoded_rope;

    namespace detail {
        struct const_rope_view_iterator;
        using const_reverse_rope_view_iterator =
            stl_interfaces::reverse_iterator<const_rope_view_iterator>;
    }

    /** A reference to a substring of an unencoded_rope, string, or
        string_view. */
    struct unencoded_rope_view
    {
        using value_type = char;
        using size_type = std::size_t;
        using iterator = detail::const_rope_view_iterator;
        using const_iterator = detail::const_rope_view_iterator;
        using reverse_iterator = detail::const_reverse_rope_view_iterator;
        using const_reverse_iterator = detail::const_reverse_rope_view_iterator;

        /** Default ctor.

            \post size() == 0 && begin() == end() */
        unencoded_rope_view() noexcept : ref_(rope_ref()), which_(which::r) {}

        /** Constructs an unencoded_rope_view covering the entire given
            unencoded_rope. */
        unencoded_rope_view(unencoded_rope const & r) noexcept;

        unencoded_rope_view(unencoded_rope_view const & r) noexcept;

        /** Forbid construction from a temporary unencoded_rope. */
        unencoded_rope_view(unencoded_rope && r) noexcept = delete;

        /** Constructs a substring of r, taken from the range of chars at
            offsets [lo, hi).  If either of lo or hi is a negative value x, x
            is taken to be an offset from the end, and so x + size() is used
            instead.

            These preconditions apply to the values used after size() is added
            to any negative arguments.

            \pre 0 <= lo && lo <= r.size()
            \pre 0 <= hi && lhi <= r.size()
            \pre lo <= hi
            \post size() == r.size() && begin() == r.begin() + lo && end() ==
            r.begin() + hi */
        unencoded_rope_view(
            unencoded_rope const & r, size_type lo, size_type hi);

        /** Constructs an unencoded_rope_view covering the entire given
            string. */
        unencoded_rope_view(std::string const & s) noexcept;

        /** Forbid construction from a temporary string. */
        unencoded_rope_view(std::string && r) noexcept = delete;

        /** Constructs a substring of s, taken from the range of chars at
            offsets [lo, hi).  If either of lo or hi is a negative value x, x
            is taken to be an offset from the end, and so x + size() is used
            instead.

            These preconditions apply to the values used after size() is added
            to any negative arguments.

            \pre 0 <= lo && lo <= s.size()
            \pre 0 <= hi && lhi <= s.size()
            \pre lo <= hi
            \post size() == s.size() && begin() == s.begin() + lo && end() ==
            s.begin() + hi */
        unencoded_rope_view(std::string const & s, size_type lo, size_type hi);

        /** Constructs an unencoded_rope_view from a null-terminated C string.

            \pre strlen(c_str) <= max_size() */
        unencoded_rope_view(char const * c_str) noexcept :
            ref_(string_view(c_str)),
            which_(which::tv)
        {}

        /** Constructs an unencoded_rope_view covering the entire given
            string_view. */
        unencoded_rope_view(string_view sv) noexcept :
            ref_(sv),
            which_(which::tv)
        {}

#ifdef BOOST_TEXT_DOXYGEN

        /** Constructs a unencoded_rope_view from a range of char.

            This function only participates in overload resolution if
            `ContigCharRange` models the ContigCharRange concept. */
        template<typename ContigCharRange>
        explicit unencoded_rope_view(ContigCharRange const & r);

        /** Constructs a unencoded_rope_view from a range of graphemes over an
            underlying range of char.

            This function only participates in overload resolution if
            `ContigGraphemeRange` models the ContigGraphemeRange concept. */
        template<typename ContigGraphemeRange>
        explicit unencoded_rope_view(ContigGraphemeRange const & r);

#else

        template<typename ContigCharRange>
        explicit unencoded_rope_view(
            ContigCharRange const & r,
            detail::contig_rng_alg_ret_t<int *, ContigCharRange> = 0);

        template<typename ContigGraphemeRange>
        explicit unencoded_rope_view(
            ContigGraphemeRange const & r,
            detail::contig_graph_rng_alg_ret_t<int *, ContigGraphemeRange> = 0);

#endif

        const_iterator begin() const noexcept;
        const_iterator end() const noexcept;

        const_iterator cbegin() const noexcept;
        const_iterator cend() const noexcept;

        const_reverse_iterator rbegin() const noexcept;
        const_reverse_iterator rend() const noexcept;

        const_reverse_iterator crbegin() const noexcept;
        const_reverse_iterator crend() const noexcept;

        bool empty() const noexcept;
        size_type size() const noexcept;

        /** Returns the char (not a reference) of *this at index i, or the
            char at index -i when i < 0.

            \pre 0 <= i && i <= size() || 0 <= -i && -i <= size()  */
        char operator[](size_type i) const noexcept;

        /** Returns a substring of *this, taken from the range of chars at
            offsets [lo, hi).  If either of lo or hi is a negative value x, x
            is taken to be an offset from the end, and so x + size() is used
            instead.

            These preconditions apply to the values used after size() is added
            to any negative arguments.

            \pre 0 <= lo && lo <= size()
            \pre 0 <= hi && lhi <= size()
            \pre lo <= hi */
        unencoded_rope_view operator()(std::ptrdiff_t lo, std::ptrdiff_t hi) const;

        /** Returns a substring of *this, taken from the first cut chars when
            cut => 0, or the last -cut chars when cut < 0.

            \pre 0 <= cut && cut <= size() || 0 <= -cut && -cut <= size() */
        unencoded_rope_view operator()(std::ptrdiff_t cut) const
        {
            size_type lo = 0;
            size_type hi = cut;
            if (cut < 0) {
                lo = cut + size();
                hi = size();
            }
            BOOST_ASSERT(0 <= lo && lo <= size());
            BOOST_ASSERT(0 <= hi && hi <= size());
            return operator()(lo, hi);
        }

        /** Returns the maximum size a string_view can have. */
        size_type max_size() const noexcept { return PTRDIFF_MAX; }

        /** Visits each segment s of the underlying unencoded_rope and calls
            f(s).  Each segment is a value whose type models a CharIter
            iterator-range.  Depending of the operation performed on each
            segment, this may be more efficient than iterating over [begin(),
            end()).

            \pre Fn is an Invocable accepting a single argument whose begin
            and end model CharIter. */
        template<typename Fn>
        void foreach_segment(Fn && f) const;

        /** Lexicographical compare.  Returns a value < 0 when *this is
            lexicographically less than rhs, 0 if *this == rhs, and a value >
            0 if *this is lexicographically greater than rhs. */
        int compare(unencoded_rope_view rhs) const noexcept;

        unencoded_rope_view & operator=(unencoded_rope_view const & r) noexcept;

        /** Assignment from an unencoded_rope. */
        unencoded_rope_view & operator=(unencoded_rope const & r) noexcept
        {
            return *this = unencoded_rope_view(r);
        }

        /** Assignment from a string. */
        unencoded_rope_view & operator=(std::string const & s) noexcept
        {
            return *this = unencoded_rope_view(s);
        }

        /** Forbid assignment from an unencoded_rope. */
        unencoded_rope_view & operator=(unencoded_rope && r) noexcept = delete;

        /** Forbid assignment from a string. */
        unencoded_rope_view & operator=(std::string && s) noexcept = delete;

        /** Assignment from a null-terminated C string.

            \pre strlen(c_str) <= max_size() */
        unencoded_rope_view & operator=(char const * c_str) noexcept
        {
            return *this = unencoded_rope_view(c_str);
        }

        /** Assignment from a string_view. */
        unencoded_rope_view & operator=(string_view sv) noexcept
        {
            return *this = unencoded_rope_view(sv);
        }

#ifdef BOOST_TEXT_DOXYGEN

        /** Assignment from a range of char.

            This function only participates in overload resolution if
            `ContigCharRange` models the ContigCharRange concept. */
        template<typename ContigCharRange>
        unencoded_rope_view & operator=(ContigCharRange const & r);

        /** Assignment from a range of graphemes over an underlying range of
            char.

            This function only participates in overload resolution if
            `ContigGraphemeRange` models the ContigGraphemeRange concept. */
        template<typename ContigGraphemeRange>
        unencoded_rope_view & operator=(ContigGraphemeRange const & r);

#else

        template<typename ContigCharRange>
        auto operator=(ContigCharRange const & r) -> detail::
            contig_rng_alg_ret_t<unencoded_rope_view &, ContigCharRange>
        {
            return *this = unencoded_rope_view(r);
        }

        template<typename ContigGraphemeRange>
        auto operator=(ContigGraphemeRange const & r)
            -> detail::contig_graph_rng_alg_ret_t<
                unencoded_rope_view &,
                ContigGraphemeRange>
        {
            return *this = unencoded_rope_view(r);
        }

#endif

#ifndef BOOST_TEXT_DOXYGEN

    private:
        enum class which { r, tv };

        struct rope_ref
        {
            rope_ref() : r_(nullptr), lo_(0), hi_(0) {}
            rope_ref(
                unencoded_rope const * r,
                std::ptrdiff_t lo,
                std::ptrdiff_t hi) :
                r_(r),
                lo_(lo),
                hi_(hi)
            {}

            unencoded_rope const * r_;
            size_type lo_;
            size_type hi_;
        };

        union ref
        {
            ref(rope_ref r) : r_(r) {}
            ref(string_view tv) : tv_(tv) {}

            rope_ref r_;
            string_view tv_;
        };

        unencoded_rope_view(
            unencoded_rope const * r, std::size_t lo, std::size_t hi) :
            ref_(rope_ref(r, lo, hi)), which_(which::r)
        {}

        ref ref_;
        which which_;

        friend struct unencoded_rope;
#endif
    };

    inline bool
    operator==(unencoded_rope_view lhs, unencoded_rope_view rhs) noexcept
    {
        return lhs.compare(rhs) == 0;
    }

    inline bool
    operator!=(unencoded_rope_view lhs, unencoded_rope_view rhs) noexcept
    {
        return lhs.compare(rhs) != 0;
    }

    inline bool
    operator<(unencoded_rope_view lhs, unencoded_rope_view rhs) noexcept
    {
        return lhs.compare(rhs) < 0;
    }

    inline bool
    operator<=(unencoded_rope_view lhs, unencoded_rope_view rhs) noexcept
    {
        return lhs.compare(rhs) <= 0;
    }

    inline bool
    operator>(unencoded_rope_view lhs, unencoded_rope_view rhs) noexcept
    {
        return lhs.compare(rhs) > 0;
    }

    inline bool
    operator>=(unencoded_rope_view lhs, unencoded_rope_view rhs) noexcept
    {
        return lhs.compare(rhs) >= 0;
    }

    inline int
    operator+(unencoded_rope_view lhs, unencoded_rope_view rhs) = delete;

}}}

#include <boost/text/unencoded_rope.hpp>

#ifndef BOOST_TEXT_DOXYGEN

namespace std {
    template<>
    struct hash<boost::text::v1::unencoded_rope_view>
    {
        using argument_type = boost::text::v1::unencoded_rope_view;
        using result_type = std::size_t;
        result_type operator()(argument_type const & urv) const noexcept
        {
            return boost::text::v1::detail::hash_char_range(urv);
        }
    };
}

#endif

#endif
