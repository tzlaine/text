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
    struct unencoded_rope
        : boost::stl_interfaces::sequence_container_interface<
              unencoded_rope,
              boost::stl_interfaces::element_layout::discontiguous>
    {
        using value_type = char;
        using pointer = char *;
        using const_pointer = char const *;
        using reference = value_type const &;
        using const_reference = reference;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using iterator = detail::const_vector_iterator<char, std::string>;
        using const_iterator = iterator;
        using reverse_iterator = stl_interfaces::reverse_iterator<iterator>;
        using const_reverse_iterator = reverse_iterator;

        /** Default ctor.

            \post size() == 0 && begin() == end() */
        unencoded_rope() noexcept {}

        unencoded_rope(unencoded_rope const &) = default;
        unencoded_rope(unencoded_rope &&) noexcept = default;

        /** Constructs an unencoded_rope from a null-terminated string. */
        unencoded_rope(char const * c_str)
        {
            seg_vec_.insert(begin(), std::string(c_str));
        }

        /** Constructs an unencoded_rope from an unencoded_rope_view. */
        explicit unencoded_rope(unencoded_rope_view rv);

        /** Move-constructs an unencoded_rope from a string. */
        explicit unencoded_rope(std::string && s)
        {
            seg_vec_.insert(begin(), std::move(s));
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

        const_iterator begin() noexcept { return seg_vec_.begin(); }
        const_iterator end() noexcept { return seg_vec_.end(); }

        size_type max_size() const noexcept { return seg_vec_.max_size(); }

        template<typename... Args>
        const_reference emplace_front(Args &&... args)
        {
            return *emplace(begin(), (Args &&) args...);
        }
        template<typename... Args>
        const_reference emplace_back(Args &&... args)
        {
            return *emplace(end(), (Args &&) args...);
        }
        template<typename... Args>
        const_iterator emplace(const_iterator at, Args &&... args)
        {
            char input[1] = {char{(Args &&) args...}};
            return insert(at, input, input + 1);
        }

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

        /** Erases the portion of *this delimited by rv.

            \pre rv.begin() <= rv.begin() && rv.end() <= end() */
        unencoded_rope & erase(const_iterator first, const_iterator last)
        {
            seg_vec_.erase(first, last);
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
            seg_vec_.replace(first, last, std::string(c_str));
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
            seg_vec_.replace(first, last, std::move(s));
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
            seg_vec_.replace(first, last, r.begin(), r.end());
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
            seg_vec_.replace(first1, last1, first2, last2);
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

#ifdef BOOST_TEXT_DOXYGEN

        /** Inserts `r` into `*this` at position `at`.

            This function only participates in overload resolution if
            `replace(at, at, std::forward<Range>(r))` is well-formed.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template<typename Range>
        const_iterator insert(const_iterator at, Range && r);

        /** Inserts `[first, last)` into `*this` at position `at`.

            This function only participates in overload resolution if
            `replace(at, at, first, last)` is well-formed.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template<typename Iter, typename Sentinel>
        const_iterator insert(const_iterator at, Iter first, Sentinel last);

        /** Appends `x` to `*this`.

            This function only participates in overload resolution if
            `insert(end(), std::forward<T>(x))` is well-formed. */
        template<typename T>
        unencoded_rope & operator+=(T && x);

#else

        template<typename R>
        auto insert(const_iterator at, R && r)
            -> decltype(replace(at, at, std::forward<R>(r)), const_iterator{})
        {
            auto const at_offset = at - begin();
            replace(at, at, std::forward<R>(r));
            return begin() + at_offset;
        }

        template<typename I, typename S>
        auto insert(const_iterator at, I first, S last)
            -> decltype(replace(at, at, first, last), const_iterator{})
        {
            auto const at_offset = at - begin();
            replace(at, at, first, last);
            return begin() + at_offset;
        }

        template<typename T>
        auto operator+=(T && x)
            -> decltype(insert(end(), std::forward<T>(x)), *this)
        {
            insert(end(), std::forward<T>(x));
            return *this;
        }

#endif

        void swap(unencoded_rope & other) { seg_vec_.swap(other.seg_vec_); }

        using base_type = boost::stl_interfaces::sequence_container_interface<
            unencoded_rope,
            boost::stl_interfaces::element_layout::discontiguous>;
        using base_type::begin;
        using base_type::end;
        using base_type::insert;
        using base_type::erase;

        /** Returns true if `*this` and `other` contain the same root node
            pointer.  This is useful when you want to check for equality
            between two `unencoded_rope`s that are likely to have originated
            from the same initial `segmented_vector`, and may have since been
            mutated. */
        bool equal_root(unencoded_rope other) const noexcept
        {
            return seg_vec_.equal_root(other.seg_vec_);
        }

        /** Stream inserter; performs formatted output. */
        friend std::ostream & operator<<(std::ostream & os, unencoded_rope r)
        {
            if (os.good()) {
                auto const size = boost::text::estimated_width_of_graphemes(
                    boost::text::as_utf32(r));
                detail::pad_width_before(os, size);
                if (os.good())
                    r.seg_vec_.foreach_segment(detail::segment_inserter{os});
                if (os.good())
                    detail::pad_width_after(os, size);
            }
            return os;
        }

#ifndef BOOST_TEXT_DOXYGEN

    private:
        template<typename R>
        unencoded_rope &
        replace_shim(unencoded_rope_view const & old_substr, R && r);

        template<typename I, typename S>
        unencoded_rope &
        replace_shim(unencoded_rope_view const & old_substr, I first, S last);

        segmented_vector<char, std::string> seg_vec_;

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

    inline unencoded_rope & unencoded_rope::erase(unencoded_rope_view rv)
    {
        seg_vec_.erase(rv.begin().as_rope_iter(), rv.end().as_rope_iter());
        return *this;
    }

    inline unencoded_rope & unencoded_rope::replace(
        const_iterator first, const_iterator last, unencoded_rope_view rv)
    {
        seg_vec_.replace(first, last, std::string(rv.begin(), rv.end()));
        return *this;
    }

    template<typename R>
    unencoded_rope &
    unencoded_rope::replace_shim(unencoded_rope_view const & old_substr, R && r)
    {
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
        return replace(
            old_substr.begin().as_rope_iter(),
            old_substr.end().as_rope_iter(),
            first,
            last);
    }

}}

#endif

namespace boost { namespace text {

#if defined(__cpp_lib_concepts)

    /** Creates a new `unencoded_rope` object that is the concatenation of `t`
        and some object `x` for which `ur.insert(ur.end(), x)` is
        well-formed. */
    template<typename T>
    unencoded_rope operator+(unencoded_rope ur, T const & x)
        // clang-format off
        requires requires { ur.insert(ur.end(), x); }
    // clang-format on
    {
        ur.insert(ur.end(), x);
        return ur;
    }

    /** Creates a new `unencoded_rope` object that is the concatenation of `x`
        and `t`, where `x` is an object for which `ur.insert(ur.begin(), x)`
        is well-formed. */
    template<typename T>
    auto operator+(T const & x, unencoded_rope ur)
        // clang-format off
        requires requires { ur.insert(ur.begin(), x); } &&
            (!std::is_same_v<T, unencoded_rope>)
    // clang-format on
    {
        ur.insert(ur.begin(), x);
        return ur;
    }

#else

    /** Creates a new `unencoded_rope` object that is the concatenation of `t`
        and some object `x` for which `ur.insert(ur.end(), x)` is
        well-formed. */
    template<typename T>
    auto operator+(unencoded_rope ur, T const & x)
        -> decltype(ur.insert(ur.end(), x), unencoded_rope{})
    {
        ur.insert(ur.end(), x);
        return ur;
    }

    /** Creates a new `unencoded_rope` object that is the concatenation of `x`
        and `t`, where `x` is an object for which `ur.insert(ur.begin(), x)`
        is well-formed. */
    template<typename T>
    auto operator+(T const & x, unencoded_rope ur) -> std::enable_if_t<
        !std::is_same<T, unencoded_rope>::value,
        decltype(ur.insert(ur.begin(), x), unencoded_rope{})>
    {
        ur.insert(ur.begin(), x);
        return ur;
    }

#endif

    inline unencoded_rope_view::unencoded_rope_view(
        unencoded_rope const & r) noexcept :
        ref_(rope_ref(&r.seg_vec_, 0, r.size())), which_(which::r)
    {}

    inline unencoded_rope_view::unencoded_rope_view(
        unencoded_rope const & r, size_type lo, size_type hi) :
        ref_(rope_ref(&r.seg_vec_, lo, hi)), which_(which::r)
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
