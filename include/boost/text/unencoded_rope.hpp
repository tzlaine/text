// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_UNENCODED_ROPE_HPP
#define BOOST_TEXT_UNENCODED_ROPE_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/estimated_width.hpp>
#include <boost/text/segmented_vector.hpp>
#include <boost/text/transcode_view.hpp>
#include <boost/text/detail/iterator.hpp>
#include <boost/text/detail/rope.hpp>
#include <boost/text/detail/rope_iterator.hpp>

#include <boost/algorithm/cxx14/equal.hpp>

#ifdef BOOST_TEXT_TESTING
#include <iostream>
#else
#include <iosfwd>
#endif

#include <string>


namespace boost { namespace text {

    struct unencoded_rope_view;

    /** A mutable sequence of char with copy-on-write semantics.  An
        unencoded_rope is non-contiguous and is not null-terminated. */
    struct unencoded_rope : segmented_vector<char, std::string>
    {
        /** Default ctor.

            \post size() == 0 && begin() == end() */
        unencoded_rope() noexcept {}

        unencoded_rope(unencoded_rope const &) = default;
        unencoded_rope(unencoded_rope &&) noexcept = default;

        /** Constructs an unencoded_rope from a null-terminated string. */
        unencoded_rope(char const * c_str) { insert(begin(), c_str); }

        /** Constructs an unencoded_rope from an unencoded_rope_view. */
        explicit unencoded_rope(unencoded_rope_view rv);

        /** Move-constructs an unencoded_rope from a string. */
        explicit unencoded_rope(std::string && s)
        {
            insert(begin(), std::move(s));
        }

#ifdef BOOST_TEXT_DOXYGEN

        /** Constructs a unencoded_rope from a range of char.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept. */
        template<typename CharRange>
        explicit unencoded_rope(CharRange const & r);

        /** Constructs an unencoded_rope from a sequence of char.

            This function only participates in overload resolution if
            `CharIter` models the CharIter concept. */
        template<typename CharIter, typename Sentinel>
        unencoded_rope(CharIter first, Sentinel last);

        /** Constructs a unencoded_rope from a range of graphemes over an
            underlying range of char.

            This function only participates in overload resolution if
            `GraphemeRange` models the GraphemeRange concept. */
        template<typename GraphemeRange>
        explicit unencoded_rope(GraphemeRange const & r);

#else

        template<typename CharRange>
        explicit unencoded_rope(
            CharRange const & r, detail::rng_alg_ret_t<int *, CharRange> = 0)
        {
            insert(begin(), r);
        }

        template<typename CharIter, typename Sentinel>
        unencoded_rope(
            CharIter first,
            Sentinel last,
            detail::char_iter_ret_t<void *, CharIter> = 0)
        {
            insert(begin(), first, last);
        }

        template<typename GraphemeRange>
        explicit unencoded_rope(
            GraphemeRange const & r,
            detail::graph_rng_alg_ret_t<int *, GraphemeRange> = 0)
        {
            insert(
                begin(),
                std::begin(r).base().base(),
                std::end(r).base().base());
        }

#endif

        unencoded_rope & operator=(unencoded_rope const &) = default;
        unencoded_rope & operator=(unencoded_rope &&) noexcept = default;

        /** Assignment from an unencoded_rope_view. */
        unencoded_rope & operator=(unencoded_rope_view rv);

        /** Move-assignment from a string. */
        unencoded_rope & operator=(std::string && s)
        {
            unencoded_rope temp(std::move(s));
            swap(temp);
            return *this;
        }

        /** Assignment from a null-terminated string. */
        unencoded_rope & operator=(char const * c_str)
        {
            unencoded_rope temp(c_str);
            swap(temp);
            return *this;
        }

#ifdef BOOST_TEXT_DOXYGEN

        /** Assignment from a range of char.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept. */
        template<typename CharRange>
        unencoded_rope & operator=(CharRange const & r);

        /** Assignment from a range of graphemes over an underlying range of
            char.

            This function only participates in overload resolution if
            `GraphemeRange` models the GraphemeRange concept. */
        template<typename GraphemeRange>
        unencoded_rope & operator=(GraphemeRange const & r);

#else

        template<typename CharRange>
        auto operator=(CharRange const & r)
            -> detail::rng_alg_ret_t<unencoded_rope &, CharRange>
        {
            unencoded_rope temp(r);
            swap(temp);
            return *this;
        }

        template<typename GraphemeRange>
        auto operator=(GraphemeRange const & r)
            -> detail::graph_rng_alg_ret_t<unencoded_rope &, GraphemeRange>
        {
            unencoded_rope temp(r);
            swap(temp);
            return *this;
        }

#endif

        /** Returns a substring of *this as an unencoded_rope_view, taken from
            the range of chars at offsets [lo, hi).  If either of lo or hi is a
            negative value x, x is taken to be an offset from the end, and so x
            + size() is used instead.

            These preconditions apply to the values used after size() is added
            to any negative arguments.

            \pre 0 <= lo && lo <= size()
            \pre 0 <= hi && lhi <= size()
            \pre lo <= hi */
        unencoded_rope_view operator()(std::ptrdiff_t lo, std::ptrdiff_t hi) const;

        /** Inserts the null-terminated string into *this starting at offset
            at. */
        unencoded_rope & insert(size_type at, char const * c_str)
        {
            insert(begin() + at, c_str);
            return *this;
        }

        /** Inserts the null-terminated string into *this starting at position
            at. */
        const_iterator insert(const_iterator at, char const * c_str)
        {
            return insert(at, std::string(c_str));
        }

        /** Inserts the sequence of char from rv into *this starting at offset
            at. */
        unencoded_rope & insert(size_type at, unencoded_rope_view rv);

        /** Inserts the sequence of char from rv into *this starting at
            position at. */
        const_iterator insert(const_iterator at, unencoded_rope_view rv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Inserts the char sequence r into *this starting at offset at.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept. */
        template<typename CharRange>
        auto insert(size_type at, CharRange const & r);

        /** Inserts the char sequence r into *this starting at position at.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept. */
        template<typename CharRange>
        auto insert(const_iterator at, CharRange const & r);

        /** Inserts the char sequence [first, last) into *this starting at
            offset at.

            This function only participates in overload resolution if
            `CharIter` models the CharIter concept. */
        template<typename CharIter, typename Sentinel>
        unencoded_rope & insert(size_type at, CharIter first, Sentinel last);

#else

        template<typename CharRange>
        auto insert(size_type at, CharRange const & r)
            -> detail::rng_alg_ret_t<unencoded_rope &, CharRange, string_view>
        {
            insert(begin() + at, std::begin(r), std::end(r));
            return *this;
        }

        template<typename CharRange>
        auto insert(const_iterator at, CharRange const & r)
            -> detail::rng_alg_ret_t<const_iterator, CharRange, string_view>
        {
            return insert(at, std::begin(r), std::end(r));
        }

        template<typename CharIter, typename Sentinel>
        auto insert(size_type at, CharIter first, Sentinel last)
            -> detail::char_iter_ret_t<unencoded_rope &, CharIter>
        {
            insert(begin() + at, first, last);
            return *this;
        }

#endif

        /** Erases the portion of *this delimited by rv.

            \pre rv.begin() <= rv.begin() && rv.end() <= end() */
        unencoded_rope & erase(const_iterator first, const_iterator last)
        {
            segmented_vector::erase(first, last);
            return *this;
        }

        /** Erases the portion of *this delimited by rv.

            \pre rv.begin() <= rv.begin() && rv.end() <= end() */
        unencoded_rope & erase(unencoded_rope_view rv);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from c_str.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        unencoded_rope &
        replace(const_iterator first, const_iterator last, char const * c_str)
        {
            segmented_vector::replace(first, last, std::string(c_str));
            return *this;
        }

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from rv.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        unencoded_rope & replace(
            const_iterator first, const_iterator last, unencoded_rope_view rv);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from t by moving the contents of t.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        unencoded_rope &
        replace(const_iterator first, const_iterator last, std::string && s)
        {
            segmented_vector::replace(first, last, std::move(s));
            return *this;
        }

#ifdef BOOST_TEXT_DOXYGEN

        /** Replaces the portion of *this delimited by old_substr with the
            char sequence r.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template<typename CharRange>
        auto replace(const_iterator first, const_iterator last, CharRange const & r);

        /** Replaces the portion of *this delimited by old_substr with the
            char sequence [first, last).

            This function only participates in overload resolution if
            `CharIter` models the CharIter concept.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template<typename CharIter, typename Sentinel>
        unencoded_rope &
        replace(const_iterator first, const_iterator last, CharIter first, Sentinel last);

        /** Replaces the portion of `*this` delimited by `old_substr` with
            `r`.

            This function only participates in overload resolution if
            `replace(old_substr.begin().as_rope_iter(),
            old_substr.end().as_rope_iter(), std::forward<Range>(r))` is
            well-formed.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template<typename Range>
        unencoded_rope &
        replace(unencoded_rope_view old_substr, Range && r);

        /** Replaces the portion of `*this` delimited by `old_substr` with
            `[first, last)`.

            This function only participates in overload resolution if
            `replace(old_substr.begin().as_rope_iter(),
            old_substr.end().as_rope_iter(), first, last)` is well-formed.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template<typename Iter, typename Sentinel>
        unencoded_rope &
        replace(unencoded_rope_view old_substr, Iter first, Sentinel last);

#else

        template<typename CharRange>
        auto replace(const_iterator first, const_iterator last, CharRange const & r)
            -> detail::rng_alg_ret_t<unencoded_rope &, CharRange, string_view>
        {
            segmented_vector::replace(first, last, r.begin(), r.end());
            return *this;
        }

        template<typename CharIter, typename Sentinel>
        auto replace(
            const_iterator first1,
            const_iterator last1,
            CharIter first2,
            Sentinel last2)
            -> detail::char_iter_ret_t<unencoded_rope &, CharIter>
        {
            segmented_vector::replace(first1, last1, first2, last2);
            return *this;
        }

        template<typename R>
        auto replace(unencoded_rope_view const & old_substr, R && r)
            -> decltype(replace(const_iterator{}, const_iterator{}, r))
        {
            return replace_shim<R>(old_substr, (R &&) r);
        }

        template<typename I, typename S>
        auto replace(unencoded_rope_view const & old_substr, I first, S last)
            -> decltype(
                replace(const_iterator{}, const_iterator{}, first, last))
        {
            return replace_shim<I, S>(old_substr, first, last);
        }

#endif

        /** Appends c_str to *this. */
        unencoded_rope & operator+=(char const * c_str);

        /** Appends rv to *this. */
        unencoded_rope & operator+=(unencoded_rope_view rv);

        /** Appends t to *this, by moving its contents into *this. */
        unencoded_rope & operator+=(std::string && s)
        {
            insert(end(), std::move(s));
            return *this;
        }


#ifdef BOOST_TEXT_DOXYGEN

        /** Append r to *this.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept. */
        template<typename CharRange>
        unencoded_rope & operator+=(CharRange const & r);

#else

        template<typename CharRange>
        auto operator+=(CharRange const & r)
            -> detail::rng_alg_ret_t<unencoded_rope &, CharRange, std::string>
        {
            insert(end(), std::begin(r), std::end(r));
            return *this;
        }

#endif

        /** Stream inserter; performs formatted output. */
        friend std::ostream & operator<<(std::ostream & os, unencoded_rope r)
        {
            if (os.good()) {
                auto const size = boost::text::estimated_width_of_graphemes(
                    boost::text::as_utf32(r));
                detail::pad_width_before(os, size);
                if (os.good())
                    r.foreach_segment(detail::segment_inserter{os});
                if (os.good())
                    detail::pad_width_after(os, size);
            }
            return os;
        }

        using base_type = segmented_vector<char, std::string>;
        using base_type::insert;
        using base_type::erase;
        using base_type::replace;

#ifndef BOOST_TEXT_DOXYGEN

    private:
        enum allocation_note_t { would_allocate, would_not_allocate };

        template<typename R>
        unencoded_rope &
        replace_shim(unencoded_rope_view const & old_substr, R && r);

        template<typename I, typename S>
        unencoded_rope &
        replace_shim(unencoded_rope_view const & old_substr, I first, S last);

        bool self_reference(unencoded_rope_view rv) const;

        friend struct unencoded_rope_view;

#endif
    };

}}

#include <boost/text/unencoded_rope_view.hpp>

#ifndef BOOST_TEXT_DOXYGEN

namespace boost { namespace text {

    inline unencoded_rope::unencoded_rope(unencoded_rope_view rv)
    {
        insert(begin(), rv);
    }

    inline unencoded_rope & unencoded_rope::operator=(unencoded_rope_view rv)
    {
        unencoded_rope temp(rv);
        swap(temp);
        return *this;
    }

    inline unencoded_rope &
    unencoded_rope::insert(size_type at, unencoded_rope_view rv)
    {
        insert(begin() + at, rv);
        return *this;
    }

    inline unencoded_rope::const_iterator
    unencoded_rope::insert(const_iterator at, unencoded_rope_view rv)
    {
        return insert(at, std::string(rv.begin(), rv.end()));
    }

    inline unencoded_rope & unencoded_rope::erase(unencoded_rope_view rv)
    {
        BOOST_ASSERT(self_reference(rv));
        segmented_vector::erase(
            rv.begin().as_rope_iter(), rv.end().as_rope_iter());
        return *this;
    }

    inline unencoded_rope & unencoded_rope::replace(
        const_iterator first, const_iterator last, unencoded_rope_view rv)
    {
        segmented_vector::replace(
            first, last, std::string(rv.begin(), rv.end()));
        return *this;
    }

    template<typename R>
    unencoded_rope &
    unencoded_rope::replace_shim(unencoded_rope_view const & old_substr, R && r)
    {
        BOOST_ASSERT(self_reference(old_substr));
        replace(
            old_substr.begin().as_rope_iter(),
            old_substr.end().as_rope_iter(),
            (R &&) r);
        return *this;
    }

    template<typename I, typename S>
    unencoded_rope & unencoded_rope::replace_shim(
        unencoded_rope_view const & old_substr, I first, S last)
    {
        BOOST_ASSERT(self_reference(old_substr));
        return replace(
            old_substr.begin().as_rope_iter(),
            old_substr.end().as_rope_iter(),
            first,
            last);
    }

    inline unencoded_rope & unencoded_rope::operator+=(char const * c_str)
    {
        insert(end(), unencoded_rope_view(c_str));
        return *this;
    }

    inline unencoded_rope & unencoded_rope::operator+=(unencoded_rope_view rv)
    {
        insert(end(), std::string(rv.begin(), rv.end()));
        return *this;
    }

    inline unencoded_rope_view unencoded_rope::
    operator()(std::ptrdiff_t lo, std::ptrdiff_t hi) const
    {
        if (lo < 0)
            lo += size();
        if (hi < 0)
            hi += size();
        BOOST_ASSERT(0 <= lo && lo <= (std::ptrdiff_t)size());
        BOOST_ASSERT(0 <= hi && hi <= (std::ptrdiff_t)size());
        BOOST_ASSERT(lo <= hi);
        return unencoded_rope_view(*this, lo, hi);
    }


    inline bool unencoded_rope::self_reference(unencoded_rope_view rv) const
    {
        return rv.which_ == unencoded_rope_view::which::r &&
               rv.ref_.r_.r_ == this;
    }

}}

#endif

namespace boost { namespace text {

    /** Creates a new unencoded_rope object that is the concatenation of r
        and c_str. */
    inline unencoded_rope operator+(unencoded_rope r, char const * c_str)
    {
        return r += string_view(c_str);
    }

    /** Creates a new unencoded_rope object that is the concatenation of c_str
        and r. */
    inline unencoded_rope operator+(char const * c_str, unencoded_rope r)
    {
        return r.insert(0, c_str);
    }

    /** Creates a new unencoded_rope object that is the concatenation of r
        and r2. */
    inline unencoded_rope operator+(unencoded_rope r, unencoded_rope r2)
    {
        return r += r2;
    }

    /** Creates a new unencoded_rope object that is the concatenation of r
       and rv. */
    inline unencoded_rope operator+(unencoded_rope r, unencoded_rope_view rv)
    {
        return r += rv;
    }

    /** Creates a new unencoded_rope object that is the concatenation of rv
       and r. */
    inline unencoded_rope operator+(unencoded_rope_view rv, unencoded_rope r)
    {
        return r.insert(0, rv);
    }

    /** Creates a new unencoded_rope object that is the concatenation of r
        and t, by moving the contents of t into the result. */
    inline unencoded_rope operator+(unencoded_rope r, std::string && s)
    {
        return r += std::move(s);
    }

    /** Creates a new unencoded_rope object that is the concatenation of t
        and r, by moving the contents of t into the result. */
    inline unencoded_rope operator+(std::string && s, unencoded_rope r)
    {
        return r.insert(0, std::move(s));
    }

#ifdef BOOST_TEXT_DOXYGEN

    /** Creates a new unencoded_rope object that is the concatenation of ur
        and r.

        This function only participates in overload resolution if
        `CharRange` models the CharRange concept. */
    template<typename CharRange>
    unencoded_rope & operator+(unencoded_rope ur, CharRange const & r);

    /** Creates a new unencoded_rope object that is the concatenation of r
        and ur.

        This function only participates in overload resolution if
        `CharRange` models the CharRange concept. */
    template<typename CharRange>
    unencoded_rope & operator+(CharRange const & r, unencoded_rope const & ur);

#else

    template<typename CharRange>
    auto operator+(unencoded_rope ur, CharRange const & r)
        -> detail::rng_alg_ret_t<unencoded_rope, CharRange, std::string>
    {
        return ur += r;
    }

    template<typename CharRange>
    auto operator+(CharRange const & r, unencoded_rope ur)
        -> detail::rng_alg_ret_t<unencoded_rope, CharRange, std::string>
    {
        return ur.insert(0, std::begin(r), std::end(r));
    }

#endif


    inline unencoded_rope_view::unencoded_rope_view(
        unencoded_rope const & r) noexcept :
        ref_(rope_ref(&r, 0, r.size())),
        which_(which::r)
    {}

    inline unencoded_rope_view::unencoded_rope_view(
        unencoded_rope const & r, size_type lo, size_type hi) :
        ref_(rope_ref(&r, lo, hi)),
        which_(which::r)
    {}

    namespace detail {

#ifdef BOOST_TEXT_TESTING
        template<typename T, typename Segment>
        inline void dump_tree(
            std::ostream & os,
            node_ptr<T, Segment> const & root,
            int key,
            int indent)
        {
            os << std::string(indent * 4, ' ')
               << (root->leaf_ ? "LEAF" : "INTR") << " @0x" << std::hex
               << root.get();
            if (key != -1)
                os << " < " << std::dec << key;
            os << " (" << root->refs_ << " refs)\n";
            if (!root->leaf_) {
                int i = 0;
                for (auto const & child : children(root)) {
                    dump_tree(os, child, keys(root)[i++], indent + 1);
                }
            }
        }
#endif
    }
}}

#ifndef BOOST_TEXT_DOXYGEN

namespace std {
    template<>
    struct hash<boost::text::unencoded_rope>
    {
        using argument_type = boost::text::unencoded_rope;
        using result_type = std::size_t;
        result_type operator()(argument_type const & ur) const noexcept
        {
            return boost::text::detail::hash_char_range(ur);
        }
    };
}

#endif

#endif
