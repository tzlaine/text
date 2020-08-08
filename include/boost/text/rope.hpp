// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_ROPE_HPP
#define BOOST_TEXT_ROPE_HPP

#include <boost/text/estimated_width.hpp>
#include <boost/text/grapheme.hpp>
#include <boost/text/grapheme_iterator.hpp>
#include <boost/text/normalize_algorithm.hpp>
#include <boost/text/unencoded_rope.hpp>
#include <boost/text/text_fwd.hpp>
#include <boost/text/transcode_iterator.hpp>

#include <iterator>


#ifndef BOOST_TEXT_DOXYGEN

#ifdef BOOST_TEXT_TESTING
#define BOOST_TEXT_CHECK_ROPE_NORMALIZATION()                                  \
    do {                                                                       \
        std::string str(rope_.begin(), rope_.end());                           \
        normalize<nf::fcc>(str);                                               \
        BOOST_ASSERT(rope_ == str);                                            \
    } while (false)
#else
#define BOOST_TEXT_CHECK_ROPE_NORMALIZATION()
#endif

#endif

namespace boost { namespace text {

    template<typename Iter>
    struct replace_result;
    struct rope_view;

    /** A mutable sequence of graphemes with copy-on-write semantics.  A rope
        is non-contiguous and is not null-terminated.  The underlying storage
        is an unencoded_rope that is UTF-8-encoded and FCC-normalized. */
    struct rope
    {
        using value_type = grapheme;
        using size_type = std::size_t;
        using iterator = grapheme_iterator<
            utf_8_to_32_iterator<unencoded_rope::const_iterator>>;
        using const_iterator = iterator;
        using reverse_iterator = stl_interfaces::reverse_iterator<iterator>;
        using const_reverse_iterator = reverse_iterator;

        /** Default ctor. */
        rope() {}

        /** Constructs a text from a pair of iterators. */
        rope(const_iterator first, const_iterator last) :
            rope(first.base().base(), last.base().base())
        {}

        /** Constructs a rope from a null-terminated string. */
        rope(char const * c_str);

#if defined(__cpp_char8_t)
        /** Constructs a rope from a null-terminated string. */
        rope(char8_t const * c_str);
#endif

        /** Constructs a rope from a rope_view. */
        explicit rope(rope_view rv);

        /** Constructs a rope from a text. */
        explicit rope(text t);

#ifdef BOOST_TEXT_DOXYGEN

        /** Constructs a rope from a range of char.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept. */
        template<typename CharRange>
        explicit rope(CharRange const & r);

        /** Constructs a rope from a sequence of char.

            This function only participates in overload resolution if
            `CharIter` models the CharIter concept. */
        template<typename CharIter, typename Sentinel>
        rope(CharIter first, Sentinel last);

#else

        template<typename CharRange>
        explicit rope(
            CharRange const & r, detail::rng_alg_ret_t<int *, CharRange> = 0) :
            rope_(std::begin(r), std::end(r))
        {}

        template<typename CharIter, typename Sentinel>
        rope(
            CharIter first,
            Sentinel last,
            detail::char_iter_ret_t<void *, CharIter> = 0);

#endif

        /** Assignment from a null-terminated string. */
        rope & operator=(char const * c_str)
        {
            rope temp(c_str);
            swap(temp);
            return *this;
        }

#if defined(__cpp_char8_t)
        /** Assignment from a null-terminated string. */
        rope & operator=(char8_t const * c_str)
        {
            return *this = (char const *)c_str;
        }
#endif

        /** Assignment from a rope_view. */
        rope & operator=(rope_view rv);

        /** Assignment from a string_view. */
        rope & operator=(string_view sv)
        {
            rope temp(sv);
            swap(temp);
            return *this;
        }

        /** Move-assignment from a text. */
        rope & operator=(text t);

        const_iterator begin() const noexcept
        {
            return make_iter(rope_.begin(), rope_.begin(), rope_.end());
        }
        const_iterator end() const noexcept
        {
            return make_iter(rope_.begin(), rope_.end(), rope_.end());
        }

        const_iterator cbegin() const noexcept { return begin(); }
        const_iterator cend() const noexcept { return end(); }

        const_reverse_iterator rbegin() const noexcept
        {
            return reverse_iterator(end());
        }
        const_reverse_iterator rend() const noexcept
        {
            return reverse_iterator(begin());
        }

        const_reverse_iterator crbegin() const noexcept { return rbegin(); }
        const_reverse_iterator crend() const noexcept { return rend(); }

        /** Returns true if begin() == end(), false otherwise. */
        bool empty() const noexcept { return rope_.empty(); }

        /** Returns the number of bytes controlled by *this, not including the
            null terminator. */
        size_type storage_bytes() const noexcept { return rope_.size(); }

        /** Returns the number of graphemes in *this.  This operation is
            O(n). */
        size_type distance() const noexcept
        {
            return std::distance(begin(), end());
        }

        /** Returns the maximum size in bytes a rope can have. */
        size_type max_bytes() const noexcept { return PTRDIFF_MAX; }

        /** Returns true if *this and rhs contain the same root node pointer.
            This is useful when you want to check for equality between two
            unencoded_ropes that are likely to have originated from the same
            initial unencoded_rope, and may have since been mutated. */
        bool equal_root(rope rhs) const noexcept
        {
            return rope_.equal_root(rhs.rope_);
        }

        /** Clear. */
        void clear() noexcept { rope_.clear(); }

        /** Inserts the sequence of char from c_str into *this starting at
            position at. */
        replace_result<const_iterator>
        insert(const_iterator at, char const * c_str)
        {
            return insert_impl(
                at, c_str, c_str + strlen(c_str), insertion_not_normalized);
        }

#if defined(__cpp_char8_t)
        /** Inserts the sequence of char from c_str into *this starting at
            position at. */
        replace_result<const_iterator>
        insert(const_iterator at, char8_t const * c_str)
        {
            return insert_impl(
                at,
                c_str,
                c_str + strlen((char const *)c_str),
                insertion_not_normalized);
        }
#endif

        /** Inserts the sequence of char from rv into *this starting at position
            at. */
        replace_result<const_iterator> insert(const_iterator at, rope_view rv);

        /** Inserts the sequence of char from sv into *this starting at position
            at. */
        replace_result<const_iterator>
        insert(const_iterator at, string_view sv)
        {
            return insert_impl(
                at, sv.begin(), sv.end(), insertion_not_normalized);
        }

#ifdef BOOST_TEXT_DOXYGEN

        /** Inserts the char range r into *this starting at position at.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept. */
        template<typename CharRange>
        replace_result<const_iterator>
        insert(const_iterator at, CharRange const & r);

        /** Inserts the char sequence [first, last) into *this starting at
            position at.

            This function only participates in overload resolution if
            `CharIter` models the CharIter concept. */
        template<typename CharIter>
        replace_result<const_iterator>
        insert(const_iterator at, CharIter first, CharIter last);

#else

        template<typename CharRange>
        auto insert(const_iterator at, CharRange const & r)
            -> detail::rng_alg_ret_t<replace_result<const_iterator>, CharRange>
        {
            return insert_impl(
                at, r.begin(), r.end(), insertion_not_normalized);
        }

        template<typename CharIter>
        auto insert(const_iterator at, CharIter first, CharIter last)
            -> detail::
                char_iter_ret_t<replace_result<const_iterator>, CharIter>
        {
            return insert_impl(at, first, last, insertion_not_normalized);
        }

#endif

        /** Inserts the sequence [first, last) into *this starting at position
            at. */
        replace_result<const_iterator>
        insert(const_iterator at, const_iterator first, const_iterator last)
        {
            return insert(at, first.base().base(), last.base().base());
        }

        /** Inserts the grapheme g into *this at position at. */
        replace_result<const_iterator>
        insert(const_iterator at, grapheme const & g);

        /** Inserts the grapheme g into *this at position at. */
        template<typename CPIter>
        replace_result<const_iterator>
        insert(const_iterator at, grapheme_ref<CPIter> g);

        /** Erases the portion of *this delimited by rv.

            \pre !std::less(rv.begin().base().base(), begin().base().base()) &&
            !std::less(end().base().base(), rv.end().base().base()) */
        replace_result<const_iterator> erase(rope_view rv);

        /** Erases the portion of *this delimited by [first, last).

            \pre first <= last */
        replace_result<const_iterator>
        erase(const_iterator first, const_iterator last)
        {
            auto const rope_first = first.base().base();
            auto const rope_last = last.base().base();
            auto const retval =
                detail::erase_impl<true, nf::fcc>(rope_, rope_first, rope_last);
            return mutation_result(retval);
        }

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        replace_result<const_iterator>
        replace(rope_view old_substr, rope_view new_substr);

        /** Replaves the  portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        replace_result<const_iterator>
        replace(rope_view old_substr, string_view new_substr);

#ifdef BOOST_TEXT_DOXYGEN

        /** Replaces the portion of *this delimited by old_substr with the
            char range r.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        template<typename CharRange>
        replace_result<const_iterator>
        replace(rope_view old_substr, CharRange const & r);

        /** Replaces the portion of *this delimited by old_substr with the
            char sequence [first, last).

            This function only participates in overload resolution if
            `CharIter` models the CharIter concept.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        template<typename CharIter>
        replace_result<const_iterator>
        replace(rope_view old_substr, CharIter first, CharIter last);

#else

        template<typename CharRange>
        auto replace(rope_view old_substr, CharRange const & r)
            -> detail::rng_alg_ret_t<replace_result<const_iterator>, CharRange>;

        template<typename CharIter>
        auto replace(rope_view old_substr, CharIter first, CharIter last)
            -> detail::
                char_iter_ret_t<replace_result<const_iterator>, CharIter>;

#endif

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from c_str.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        replace_result<const_iterator>
        replace(rope_view old_substr, char const * c_str);

#if defined(__cpp_char8_t)
        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from c_str.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        replace_result<const_iterator>
        replace(rope_view old_substr, char8_t const * c_str);
#endif

        /** Replaces the portion of *this delimited by old_substr with the
            sequence [first, last).

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        replace_result<const_iterator> replace(
            rope_view old_substr, const_iterator first, const_iterator last);

        /** Swaps *this with rhs. */
        void swap(rope & rhs) noexcept { rope_.swap(rhs.rope_); }

        /** Removes and returns the underlying unencoded_rope from *this. */
        unencoded_rope extract() && noexcept { return std::move(rope_); }

        /** Replaces the underlying unencoded_rope in *this.

            \pre ur is normalized FCC. */
        void replace(unencoded_rope && ur) noexcept { rope_ = std::move(ur); }

        /** TODO */
        template<typename T>
        auto operator+=(T && x) -> decltype(*this = std::forward<T>(x))
        {
            insert(end(), std::forward<T>(x));
            return *this;
        }

        /** Stream inserter; performs formatted output, in UTF-8 encoding. */
        friend std::ostream & operator<<(std::ostream & os, rope const & r)
        {
            if (os.good()) {
                auto const size = boost::text::estimated_width_of_graphemes(
                    r.begin().base(), r.end().base());
                detail::pad_width_before(os, size);
                if (os.good())
                    os << boost::text::as_utf8(r.rope_);
                if (os.good())
                    detail::pad_width_after(os, size);
            }
            return os;
        }
#if defined(BOOST_TEXT_DOXYGEN) || defined(_MSC_VER)
        /** Stream inserter; performs formatted output, in UTF-16 encoding.
            Defined on Windows only. */
        friend std::wostream & operator<<(std::wostream & os, rope const & r)
        {
            if (os.good()) {
                auto const size = boost::text::estimated_width_of_graphemes(
                    r.begin().base(), r.end().base());
                detail::pad_width_before(os, size);
                if (os.good())
                    os << boost::text::as_utf16(r.rope_);
                if (os.good())
                    detail::pad_width_after(os, size);
            }
            return os;
        }
#endif

#ifndef BOOST_TEXT_DOXYGEN

    private:
        static const_iterator make_iter(
            unencoded_rope::const_iterator first,
            unencoded_rope::const_iterator it,
            unencoded_rope::const_iterator last) noexcept
        {
            return const_iterator{
                utf_8_to_32_iterator<unencoded_rope::const_iterator>{
                    first, first, last},
                utf_8_to_32_iterator<unencoded_rope::const_iterator>{
                    first, it, last},
                utf_8_to_32_iterator<unencoded_rope::const_iterator>{
                    first, last, last}};
        }

        using utf32_iter = utf_8_to_32_iterator<unencoded_rope::const_iterator>;

        replace_result<const_iterator> mutation_result(
            replace_result<unencoded_rope::const_iterator> rope_replacement)
        {
            auto const rope_first = rope_.begin();
            auto const rope_lo =
                rope_first + (rope_replacement.begin() - rope_.begin());
            auto const rope_hi =
                rope_first + (rope_replacement.end() - rope_.begin());
            auto const rope_last = rope_.end();

            // The insertion that just happened might be merged into the CP or
            // grapheme ending at the offset of the inserted char(s); if so,
            // back up and return an iterator to that.
            auto lo_cp_it = utf32_iter(rope_first, rope_lo, rope_last);
            auto const lo_grapheme_it = boost::text::prev_grapheme_break(
                begin().base(), lo_cp_it, end().base());

            // The insertion that just happened might be merged into the CP or
            // grapheme starting at the offset of the inserted char(s); if so,
            // move up and return an iterator to that.
            auto hi_cp_it = utf32_iter(rope_first, rope_hi, rope_last);
            auto hi_grapheme_it = hi_cp_it;
            if (!boost::text::at_grapheme_break(
                    begin().base(), hi_cp_it, end().base())) {
                hi_grapheme_it =
                    boost::text::next_grapheme_break(hi_cp_it, end().base());
            }

            return {
                make_iter(
                    begin().base().base(),
                    lo_grapheme_it.base(),
                    end().base().base()),
                make_iter(
                    begin().base().base(),
                    hi_grapheme_it.base(),
                    end().base().base())};
        }

        template<typename CUIter>
        replace_result<const_iterator> insert_impl(
            iterator at,
            CUIter first,
            CUIter last,
            insertion_normalization insertion_norm)
        {
            if (first == last)
                return {at, at};
            auto const rope_at = at.base().base();
            auto const insertion = boost::text::as_utf32(first, last);
            auto const retval = detail::replace_impl<true, nf::fcc>(
                rope_,
                rope_at,
                rope_at,
                insertion.begin(),
                insertion.end(),
                insertion_norm);
            return mutation_result(retval);
        }

        template<typename CUIter>
        replace_result<const_iterator> replace_impl(
            rope_view old_substr,
            CUIter first,
            CUIter last,
            insertion_normalization insertion_norm);

        unencoded_rope rope_;

        friend struct ::boost::text::rope_view;

#endif // Doxygen
    };

}}

#include <boost/text/text.hpp>
#include <boost/text/rope_view.hpp>
#include <boost/text/normalize_string.hpp>
#include <boost/text/detail/rope_iterator.hpp>

#ifndef BOOST_TEXT_DOXYGEN

namespace boost { namespace text {

    inline rope::rope(char const * c_str) : rope_(text(c_str).extract()) {}

#if defined(__cpp_char8_t)
    inline rope::rope(char8_t const * c_str) : rope((char const *)c_str) {}
#endif

    inline rope::rope(rope_view rv) :
        rope_(rv.begin().base().base(), rv.end().base().base())
    {}

    inline rope::rope(text t) : rope_(std::move(t).extract()) {}

    template<typename CharIter, typename Sentinel>
    rope::rope(
        CharIter first,
        Sentinel last,
        detail::char_iter_ret_t<void *, CharIter>) :
        rope_(text(first, last).extract())
    {}

    inline rope & rope::operator=(rope_view rv)
    {
        rope temp(rv);
        swap(temp);
        return *this;
    }

    inline rope & rope::operator=(text t)
    {
        rope temp(std::move(t));
        swap(temp);
        return *this;
    }

    inline replace_result<rope::const_iterator>
    rope::insert(const_iterator at, rope_view rv)
    {
        // TODO: It would be better if we shared nodes with copied nodes from
        // rv.  Same for unencoded_rope probably.
        return insert_impl(
            at,
            rv.begin().base().base(),
            rv.end().base().base(),
            insertion_normalized);
    }

    inline replace_result<rope::const_iterator>
    rope::insert(const_iterator at, grapheme const & g)
    {
        return insert(at, grapheme_ref<grapheme::const_iterator>(g));
    }

    template<typename CPIter>
    replace_result<rope::const_iterator>
    rope::insert(const_iterator at, grapheme_ref<CPIter> g)
    {
        if (g.empty())
            return replace_result<const_iterator>{at, at};
        std::array<char, 128> buf;
        auto out =
            boost::text::transcode_to_utf8(g.begin(), g.end(), buf.data());
        return insert_impl(at, buf.data(), out, insertion_not_normalized);
    }

    inline replace_result<rope::const_iterator> rope::erase(rope_view rv)
    {
        auto const rope_first = rv.begin().base().base().as_rope_iter();
        auto const rope_last = rv.end().base().base().as_rope_iter();
        auto const retval = detail::erase_impl<true, nf::fcc>(
            rope_, rope_first, rope_last);
        return mutation_result(retval);
    }

    inline replace_result<rope::const_iterator>
    rope::replace(rope_view old_substr, rope_view new_substr)
    {
        return replace_impl(
            old_substr,
            new_substr.begin().base().base(),
            new_substr.end().base().base(),
            insertion_normalized);
    }

    inline replace_result<rope::const_iterator>
    rope::replace(rope_view old_substr, string_view new_substr)
    {
        return replace_impl(
            old_substr,
            new_substr.begin(),
            new_substr.end(),
            insertion_not_normalized);
    }

    template<typename CharRange>
    auto rope::replace(rope_view old_substr, CharRange const & r) -> detail::
        rng_alg_ret_t<replace_result<rope::const_iterator>, CharRange>
    {
        return replace(old_substr, std::begin(r), std::end(r));
    }

    inline replace_result<rope::const_iterator>
    rope::replace(rope_view old_substr, char const * str)
    {
        return replace(old_substr, string_view(str));
    }

#if defined(__cpp_char8_t)
    inline replace_result<rope::const_iterator>
    rope::replace(rope_view old_substr, char8_t const * str)
    {
        return replace(old_substr, (char const *)str);
    }
#endif

    template<typename CharIter>
    auto rope::replace(rope_view old_substr, CharIter first, CharIter last)
        -> detail::
            char_iter_ret_t<replace_result<rope::const_iterator>, CharIter>
    {
        return replace_impl(old_substr, first, last, insertion_not_normalized);
    }

    inline replace_result<rope::const_iterator> rope::replace(
        rope_view old_substr, const_iterator first, const_iterator last)
    {
        return replace_impl(
            old_substr,
            first.base().base(),
            last.base().base(),
            insertion_normalized);
    }

    template<typename CUIter>
    replace_result<rope::const_iterator> rope::replace_impl(
        rope_view old_substr,
        CUIter first,
        CUIter last,
        insertion_normalization insertion_norm)
    {
        auto const rope_first = old_substr.begin().base().base().as_rope_iter();
        auto const rope_last = old_substr.end().base().base().as_rope_iter();
        auto const insertion = boost::text::as_utf32(first, last);
        auto const retval = detail::replace_impl<true, nf::fcc>(
            rope_,
            rope_first,
            rope_last,
            insertion.begin(),
            insertion.end(),
            insertion_norm);
        return mutation_result(retval);
    }

}}

#endif

namespace boost { namespace text {

    inline bool operator==(text const & lhs, rope_view rhs) noexcept
    {
        return algorithm::equal(
            lhs.begin().base().base(),
            lhs.end().base().base(),
            rhs.begin().base().base(),
            rhs.end().base().base());
    }
    inline bool operator==(rope_view lhs, text const & rhs) noexcept
    {
        return rhs == lhs;
    }

    inline bool operator!=(text const & lhs, rope_view rhs) noexcept
    {
        return !(lhs == rhs);
    }
    inline bool operator!=(rope_view lhs, text const & rhs) noexcept
    {
        return !(lhs == rhs);
    }

    inline bool operator==(text const & lhs, rope rhs) noexcept
    {
        return algorithm::equal(
            lhs.begin().base().base(),
            lhs.end().base().base(),
            rhs.begin().base().base(),
            rhs.end().base().base());
    }
    inline bool operator==(rope lhs, text const & rhs) noexcept
    {
        return rhs == lhs;
    }

    inline bool operator!=(text const & lhs, rope rhs) noexcept
    {
        return !(lhs == rhs);
    }
    inline bool operator!=(rope lhs, text const & rhs) noexcept
    {
        return !(lhs == rhs);
    }

    inline bool operator==(rope const & lhs, rope_view rhs) noexcept
    {
        return algorithm::equal(
            lhs.begin().base().base(),
            lhs.end().base().base(),
            rhs.begin().base().base(),
            rhs.end().base().base());
    }
    inline bool operator==(rope_view lhs, rope const & rhs) noexcept
    {
        return rhs == lhs;
    }

    inline bool operator!=(rope const & lhs, rope_view rhs) noexcept
    {
        return !(lhs == rhs);
    }
    inline bool operator!=(rope_view lhs, rope const & rhs) noexcept
    {
        return !(lhs == rhs);
    }

    inline bool operator==(rope const & lhs, rope const & rhs) noexcept
    {
        return algorithm::equal(
            lhs.begin().base().base(),
            lhs.end().base().base(),
            rhs.begin().base().base(),
            rhs.end().base().base());
    }

    inline bool operator!=(rope const & lhs, rope const & rhs) noexcept
    {
        return !(lhs == rhs);
    }

#if defined(__cpp_lib_concepts)

    /** Creates a new `rope` object that is the concatenation of `t` and some
        object `x` for which `r = x` is well-formed. */
    template<typename T>
    rope operator+(rope r, T const & x)
        // clang-format off
        requires requires { r = x; }
    // clang-format on
    {
        r.insert(r.end(), x);
        return r;
    }

    /** Creates a new `rope` object that is the concatenation
        of `x` and `t`, where `x` is an object for which `r = x` is
        well-formed. */
    template<typename T>
    rope operator+(T const & x, rope r)
        // clang-format off
        requires requires { r = x; } &&
            (!std::is_same_v<T, rope>)
    // clang-format on
    {
        r.insert(r.begin(), x);
        return r;
    }

#else

    /** Creates a new `rope` object that is the concatenation
        of `t` and some object `x` for which `r = x` is well-formed. */
    template<typename T>
    auto operator+(rope r, T const & x) -> decltype(r = x, rope{})
    {
        r.insert(r.end(), x);
        return r;
    }

    /** Creates a new `rope` object that is the concatenation
        of `x` and `t`, where `x` is an object for which `r = x` is
        well-formed. */
    template<typename T>
    auto operator+(T const & x, rope r) -> std::
        enable_if_t<!std::is_same<T, rope>::value, decltype(r = x, rope{})>
    {
        r.insert(r.begin(), x);
        return r;
    }

#endif


    template<nf Normalization, typename String>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::replace(
        const_iterator first, const_iterator last, rope_view new_substr)
    {
        return replace_impl(
            first,
            last,
            new_substr.begin().base().base(),
            new_substr.end().base().base(),
            insertion_normalized);
    }

}}

#ifndef BOOST_TEXT_DOXYGEN

namespace std {
    template<>
    struct hash<boost::text::rope>
    {
        using argument_type = boost::text::rope;
        using result_type = std::size_t;
        result_type operator()(argument_type const & r) const noexcept
        {
            return boost::text::detail::hash_grapheme_range(r);
        }
    };
}

#endif

#endif
