// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_TEXT_HPP
#define BOOST_TEXT_TEXT_HPP

#include <boost/text/estimated_width.hpp>
#include <boost/text/normalize_algorithm.hpp>
#include <boost/text/string_view.hpp>
#include <boost/text/text_fwd.hpp>
#include <boost/text/utf.hpp>
#include <boost/text/detail/make_string.hpp>
#include <boost/text/detail/utility.hpp>

#include <boost/algorithm/cxx14/equal.hpp>
#include <boost/container/small_vector.hpp>

#include <iterator>
#include <climits>


#ifndef BOOST_TEXT_DOXYGEN

#ifdef BOOST_TEXT_TESTING
#define BOOST_TEXT_CHECK_TEXT_NORMALIZATION()                                  \
    do {                                                                       \
        string str2(str_);                                                     \
        boost::text::normalize<normalization>(str2);                           \
        BOOST_ASSERT(str_ == str2);                                            \
    } while (false)
#else
#define BOOST_TEXT_CHECK_TEXT_NORMALIZATION()
#endif

#endif

namespace boost { namespace text {

    template<typename Iter>
    struct replace_result;
    struct rope_view;

    /** A mutable sequence of graphemes over an underlying container of
        contiguous null-terminated code units.  The underlying storage is a
        `String`, and is kept in normalization form `Normalization`.  The
        `String` is responsible for maintaining null-termination. */
    template<nf Normalization, typename String>
#if defined(__cpp_lib_concepts)
        // clang-format off
        requires utf8_code_unit<std::ranges::range_value_t<String>> ||
                 utf16_code_unit<std::ranges::range_value_t<String>>
#endif
    struct basic_text
    // clang-format on
    {
        /** The normalization form used in this basic_text. */
        static constexpr nf normalization = Normalization;

        /** The type of the container used as underlying storage. */
        using string = String;

        /** The type of code unit used in the underlying storage. */
        using char_type = detail::range_value_t<String>;

        /** The specialization of `std::basic_string_view` (or
            `boost::basic_string_view` in pre-C++17 code) compatible with
            `string`. */
        using string_view = basic_string_view<char_type>;

        /** The specialization of `basic_text_view` with the same
            normalizaation form and underlying code unit type. */
        using text_view = basic_text_view<Normalization, char_type>;

        /** The UTF format used in the underlying storage. */
        static constexpr format utf_format = detail::format_of<char_type>();

        BOOST_TEXT_STATIC_ASSERT_NORMALIZATION();
        static_assert(
            utf_format == format::utf8 || utf_format == format::utf16, "");

        using value_type = grapheme;
        using size_type = std::size_t;
        using iterator =
            grapheme_iterator<detail::text_transcode_iterator_t<char_type>>;
        using const_iterator = grapheme_iterator<
            detail::text_transcode_iterator_t<char_type const>>;
        using reverse_iterator = stl_interfaces::reverse_iterator<iterator>;
        using const_reverse_iterator =
            stl_interfaces::reverse_iterator<const_iterator>;

        /** Default ctor. */
        basic_text() {}

        basic_text(basic_text const &) = default;
        basic_text(basic_text &&) = default;

        /** Constructs a basic_text from a pair of iterators. */
        basic_text(const_iterator first, const_iterator last) :
            basic_text(text_view(first, last))
        {}

        /** Constructs a basic_text from a null-terminated string. */
        basic_text(char_type const * c_str) : str_(c_str)
        {
            boost::text::normalize<normalization>(str_);
        }

        /** Constructs a basic_text from a text_view. */
        explicit basic_text(text_view tv);

        /** Constructs a basic_text from a rope_view. */
        explicit basic_text(rope_view rv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Constructs a basic_text from a range of char.

            This function only participates in overload resolution if
            `CURange` models the CURange concept. */
        template<typename CURange>
        explicit basic_text(CURange const & r);

        /** Constructs a basic_text from a sequence of char.

            This function only participates in overload resolution if
            `CUIter` models the CUIter concept. */
        template<typename CUIter, typename Sentinel>
        basic_text(CUIter first, Iter CUlast);

        /** Constructs a basic_text from a range of graphemes over an underlying
            range of char.

            This function only participates in overload resolution if
            `GraphemeRange` models the GraphemeRange concept. */
        template<typename GraphemeRange>
        explicit basic_text(GraphemeRange const & r);

        // TODO: grapheme iterators.

#else

#if defined(__cpp_lib_concepts)
        template<range<basic_text<Normalization, String>::utf_format> R>
        explicit basic_text(R const & r) :
#else
        template<typename R>
        explicit basic_text(
            R const & r,
            detail::cu_rng_alg_ret_t<(int)utf_format, int *, R> = 0) :
#endif
            str_(detail::make_string<string>(r.begin(), r.end()))
        {
            boost::text::normalize<normalization>(str_);
        }

#if defined(__cpp_lib_concepts)
        template<boost::text::iterator<utf_format> I, std::sentinel_for<I> S>
        basic_text(I first, S last) :
#else
        template<typename I, typename S>
        basic_text(
            I first,
            S last,
            detail::cu_iter_ret_t<(int)utf_format, void *, I> = 0) :
#endif
            str_(detail::make_string<string>(first, last))
        {
            boost::text::normalize<normalization>(str_);
        }

#if defined(__cpp_lib_concepts)
        template<grapheme_range R>
        explicit basic_text(R const & r) :
#else
        template<typename R>
        explicit basic_text(
            R const & r, detail::graph_rng_alg_ret_t<int *, R> = 0) :
#endif
            str_(detail::make_string<string>(
                r.begin().base().base(), r.end().base().base()))
        {
            boost::text::normalize<normalization>(str_);
        }

#endif

        basic_text & operator=(basic_text const &) = default;
        basic_text & operator=(basic_text &&) = default;

        /** Assignment from a null-terminated string. */
        basic_text & operator=(char_type const * c_str)
        {
            return *this = string_view(c_str);
        }

        /** Assignment from a string_view. */
        basic_text & operator=(string_view sv)
        {
            str_.assign(sv.begin(), sv.end());
            boost::text::normalize<normalization>(str_);
            return *this;
        }

        /** Assignment from a text_view. */
        basic_text & operator=(text_view tv);

        /** Assignment from a rope_view. */
        basic_text & operator=(rope_view rv);


        iterator begin() noexcept
        {
            auto const first = const_cast<char_type *>(str_.data());
            auto const last = first + str_.size();
            return make_iter(first, first, last);
        }
        iterator end() noexcept
        {
            auto const first = const_cast<char_type *>(str_.data());
            auto const last = first + str_.size();
            return make_iter(first, last, last);
        }

        const_iterator begin() const noexcept
        {
            auto const first = str_.data();
            auto const last = first + str_.size();
            return make_iter(first, first, last);
        }
        const_iterator end() const noexcept
        {
            auto const first = str_.data();
            auto const last = first + str_.size();
            return make_iter(first, last, last);
        }

        const_iterator cbegin() const noexcept { return begin(); }
        const_iterator cend() const noexcept { return end(); }

        reverse_iterator rbegin() noexcept { return make_reverse_iter(end()); }
        reverse_iterator rend() noexcept { return make_reverse_iter(begin()); }

        const_reverse_iterator rbegin() const noexcept
        {
            return make_reverse_iter(end());
        }
        const_reverse_iterator rend() const noexcept
        {
            return make_reverse_iter(begin());
        }

        const_reverse_iterator crbegin() const noexcept { return rbegin(); }
        const_reverse_iterator crend() const noexcept { return rend(); }

        /** Returns true if size() == 0, false otherwise. */
        bool empty() const noexcept { return str_.empty(); }

        /** Returns the number of bytes controlled by `*this`, not including the
            null terminator. */
        size_type storage_bytes() const noexcept { return str_.size(); }

        /** Returns the number of bytes of storage currently in use by
            `*this`. */
        size_type capacity_bytes() const noexcept { return str_.capacity(); }

        /** Returns the number of graphemes in `*this`.  This operation is
            O(n). */
        size_type distance() const noexcept
        {
            return std::distance(begin(), end());
        }

        /** Returns the maximum size in bytes a basic_text can have. */
        size_type max_bytes() const noexcept { return PTRDIFF_MAX; }

        /** Clear.

            \post size() == 0 && capacity() == 0; begin(), end() delimit an
            empty string */
        void clear() noexcept { str_.clear(); }

        /** Erases the portion of `*this` delimited by `[first, last)`.

            \pre first <= last */
        replace_result<iterator> erase(iterator first, iterator last) noexcept
        {
            auto const lo = first.base().base() - str_.data();
            auto const hi = last.base().base() - str_.data();
            auto const retval = boost::text::normalize_erase<normalization>(
                str_, str_.begin() + lo, str_.begin() + hi);
            return mutation_result(retval);
        }

        /** Replaces the portion of `*this` delimited by `[first1, last1)`
            with the sequence `[first2, last2)`.

            \pre !std::less(first1.base().base(), begin().base().base()) &&
            !std::less(end().base().base(), last1.base().base()) */
        replace_result<iterator> replace(
            iterator first1,
            iterator last1,
            const_iterator first2,
            const_iterator last2)
        {
            return replace_impl(
                first1, last1, first2.base().base(), last2.base().base());
        }

        /** Replaces the portion of `*this` delimited by `[first, last)` with
            the sequence of char from `new_substr`.

            \pre !std::less(first.base().base(), begin().base().base()) &&
            !std::less(end().base().base(), last.base().base()) */
        replace_result<iterator>
        replace(iterator first, iterator last, char_type const * new_substr)
        {
            auto const insertion = string_view(new_substr);
            return replace_impl(
                first,
                last,
                insertion.begin(),
                insertion.end(),
                insertion_not_normalized);
        }

        /** Replaces the portion of `*this` delimited by `[first, last)` with the
            sequence of char from `new_substr`.

            \pre !std::less(first.base().base(), begin().base().base()) &&
            !std::less(end().base().base(), last.base().base()) */
        replace_result<iterator>
        replace(iterator first, iterator last, basic_text const & new_substr)
        {
            return replace_impl(
                first,
                last,
                new_substr.begin().base().base(),
                new_substr.end().base().base(),
                insertion_normalized);
        }

        /** Replaces the portion of `*this` delimited by `[first, last)` with the
            sequence of char from `new_substr`.

            \pre !std::less(first.base().base(), begin().base().base()) &&
            !std::less(end().base().base(), last.base().base()) */
        replace_result<iterator>
        replace(iterator first, iterator last, text_view new_substr);

        /** Replaves the  portion of `*this` delimited by `[first, last)` with the
            sequence of char from `new_substr`.

            \pre !std::less(first.base().base(), begin().base().base()) &&
            !std::less(end().base().base(), last.base().base()) */
        replace_result<iterator>
        replace(iterator first, iterator last, string_view new_substr)
        {
            return replace_impl(
                first,
                last,
                new_substr.begin(),
                new_substr.end(),
                insertion_not_normalized);
        }

        /** Replaces the portion of `*this` delimited by `[first, last)` with the
            sequence of char from `new_substr`.

            \pre !std::less(first.base().base(), begin().base().base()) &&
            !std::less(end().base().base(), last.base().base()) */
        replace_result<iterator>
        replace(iterator first, iterator last, rope_view new_substr);

#ifdef BOOST_TEXT_DOXYGEN

        /** Replaces the portion of `*this` delimited by `[first, last)` with the
            char range r.

            This function only participates in overload resolution if
            `CURange` models the CURange concept.

            \pre !std::less(first.base().base(), begin().base().base()) &&
            !std::less(end().base().base(), last.base().base()) */
        template<typename CURange>
        replace_result<iterator>
        replace(iterator first, iterator last, CURange const & r);

        /** Replaces the portion of `*this` delimited by `[first1, last1)` with
            the char sequence `[first2, last2)`.

            This function only participates in overload resolution if
            `CUIter` models the CUIter concept.

            \pre !std::less(first.base().base(), begin().base().base()) &&
            !std::less(end().base().base(), last.base().base()) */
        template<typename CUIter>
        replace_result<iterator>
        replace(iterator first1, iterator last1, CUIter first2, CUIter last2);

        // TODO: GraphemeRange
        // TODO: grapheme iterators.

#else

#if defined(__cpp_lib_concepts)
        template<range<utf_format> R>
        replace_result<iterator>
        replace(iterator first, iterator last, R const & r)
#else
        template<typename R>
        auto replace(iterator first, iterator last, R const & r) -> detail::
            cu_rng_alg_ret_t<(int)utf_format, replace_result<iterator>, R>
#endif
        {
            return replace(first, last, std::begin(r), std::end(r));
        }

#if defined(__cpp_lib_concepts)
        template<boost::text::iterator<utf_format> I>
        replace_result<iterator>
        replace(iterator first1, iterator last1, I first2, I last2)
#else
        template<typename I>
        auto replace(iterator first1, iterator last1, I first2, I last2)
            -> detail::
                cu_iter_ret_t<(int)utf_format, replace_result<iterator>, I>
#endif
        {
            return replace_impl(
                first1, last1, first2, last2, insertion_not_normalized);
        }

#endif

        /** Inserts the sequence [first, last) into `*this` starting at position
            at. */
        replace_result<iterator>
        insert(iterator at, const_iterator first, const_iterator last)
        {
            return insert(at, text_view(first, last));
        }

        /** Inserts the sequence of char_type from `c_str` into `*this`
            starting at position `at`. */
        replace_result<iterator> insert(iterator at, char_type const * c_str)
        {
            return insert(at, string_view(c_str));
        }

        /** Inserts the sequence of char_type from `x` into `*this` starting
            at position `at`. */
        template<typename T>
        auto insert(iterator at, T const & x) -> decltype(replace(at, at, x))
        {
            return replace(at, at, x);
        }

        /** Inserts the sequence `[first, last)` into `*this` starting at
            position `at`. */
        template<typename I>
        auto insert(iterator at, I first, I last)
            -> decltype(replace(at, at, first, last))
        {
            return replace(at, at, first, last);
        }

        // TODO: Change these to replace() overloads.
        /** Inserts the grapheme g into `*this` at position at. */
        replace_result<iterator> insert(iterator at, grapheme const & g);

        /** Inserts the grapheme g into `*this` at position at. */
        template<typename CPIter>
        replace_result<iterator> insert(iterator at, grapheme_ref<CPIter> g);

        /** Reserves storage enough for a string of at least new_size
            bytes.

            \post capacity() >= new_size + 1 */
        void reserve(size_type new_size) { str_.reserve(new_size); }

        /** Reduces storage used by `*this` to just the amount necessary to
            contain size() chars.

            \post capacity() == 0 || capacity() == size() + 1 */
        void shrink_to_fit() { str_.shrink_to_fit(); }

        /** Swaps `*this` with rhs. */
        void swap(basic_text & rhs) noexcept { str_.swap(rhs.str_); }

        /** Removes and returns the underlying string from `*this`. */
        string extract() && noexcept { return std::move(str_); }

        /** Replaces the underlying string in `*this`.

            \pre s is normalized FCC. */
        void replace(string && s) noexcept { str_ = std::move(s); }

        /** Appends c_str to `*this`. */
        basic_text & operator+=(char_type const * c_str)
        {
            return operator+=(string_view(c_str));
        }

        /** Appends `r` to `*this`.  `R` may be any type for which
            `insert(end(), r)` is well-formed. */
        template<typename T>
        auto operator+=(T const & x) -> decltype(insert(end(), x), *this)
        {
            insert(end(), x);
            return *this;
        }

        /** Stream inserter; performs formatted output, in UTF-8 encoding. */
        friend std::ostream &
        operator<<(std::ostream & os, basic_text const & t)
        {
            if (os.good()) {
                auto const size = boost::text::estimated_width_of_graphemes(
                    t.begin().base(), t.end().base());
                detail::pad_width_before(os, size);
                if (os.good())
                    os << boost::text::as_utf8(t.str_);
                if (os.good())
                    detail::pad_width_after(os, size);
            }
            return os;
        }
#if defined(BOOST_TEXT_DOXYGEN) || defined(_MSC_VER)
        /** Stream inserter; performs formatted output, in UTF-16 encoding.
            Defined on Windows only. */
        friend std::wostream &
        operator<<(std::wostream & os, basic_text const & t)
        {
            if (os.good()) {
                auto const size = boost::text::estimated_width_of_graphemes(
                    t.begin().base(), t.end().base());
                detail::pad_width_before(os, size);
                if (os.good())
                    os << boost::text::as_utf16(t.str_);
                if (os.good())
                    detail::pad_width_after(os, size);
            }
            return os;
        }
#endif

#ifndef BOOST_TEXT_DOXYGEN

    private:
        static iterator
        make_iter(char_type * first, char_type * it, char_type * last) noexcept
        {
            return iterator{
                detail::text_transcode_iterator_t<char_type>{
                    first, first, last},
                detail::text_transcode_iterator_t<char_type>{first, it, last},
                detail::text_transcode_iterator_t<char_type>{
                    first, last, last}};
        }

        static const_iterator make_iter(
            char_type const * first,
            char_type const * it,
            char_type const * last) noexcept
        {
            return const_iterator{
                detail::text_transcode_iterator_t<char_type const>{
                    first, first, last},
                detail::text_transcode_iterator_t<char_type const>{
                    first, it, last},
                detail::text_transcode_iterator_t<char_type const>{
                    first, last, last}};
        }

        template<typename Iter>
        static stl_interfaces::reverse_iterator<Iter>
        make_reverse_iter(Iter it) noexcept
        {
            return stl_interfaces::reverse_iterator<Iter>{it};
        }

        using mutable_utf32_iter = detail::text_transcode_iterator_t<char_type>;

        replace_result<iterator> mutation_result(
            replace_result<typename string::iterator> str_replacement)
        {
            auto const str_first = const_cast<char_type *>(str_.data());
            auto const str_lo =
                str_first + (str_replacement.begin() - str_.begin());
            auto const str_hi =
                str_first + (str_replacement.end() - str_.begin());
            auto const str_last = str_first + str_.size();

            // The insertion that just happened might be merged into the CP or
            // grapheme ending at the offset of the inserted char(s); if so,
            // back up and return an iterator to that.
            auto lo_cp_it = mutable_utf32_iter(str_first, str_lo, str_last);
            auto const lo_grapheme_it = boost::text::prev_grapheme_break(
                begin().base(), lo_cp_it, end().base());

            // The insertion that just happened might be merged into the CP or
            // grapheme starting at the offset of the inserted char(s); if so,
            // move up and return an iterator to that.
            auto hi_cp_it = mutable_utf32_iter(str_first, str_hi, str_last);
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
        replace_result<iterator> insert_impl(
            iterator at,
            CUIter first,
            CUIter last,
            insertion_normalization insertion_norm)
        {
            auto const str_at = str_.begin() + (at.base().base() - str_.data());
            auto const retval = boost::text::normalize_insert<normalization>(
                str_,
                str_at,
                boost::text::as_utf32(first, last),
                insertion_norm);
            return mutation_result(retval);
        }

        template<typename CUIter>
        replace_result<iterator> replace_impl(
            iterator first,
            iterator last,
            CUIter f,
            CUIter l,
            insertion_normalization insertion_norm)
        {
            auto const str_first =
                str_.begin() + (first.base().base() - str_.data());
            auto const str_last =
                str_.begin() + (last.base().base() - str_.data());
            auto const insertion = boost::text::as_utf32(f, l);
            auto const retval = boost::text::normalize_replace<normalization>(
                str_,
                str_first,
                str_last,
                insertion.begin(),
                insertion.end(),
                insertion_norm);
            return mutation_result(retval);
        }

        string str_;

#endif // Doxygen
    };

}}

#include <boost/text/text_view.hpp>
#include <boost/text/rope.hpp>
#include <boost/text/rope_view.hpp>

namespace boost { namespace text {

    namespace literals {
        /** Creates a `text` from a `char` string literal. */
        inline text operator"" _t(char const * str, std::size_t len)
        {
            return text(str, str + len);
        }
    }

#ifndef BOOST_TEXT_DOXYGEN

    template<nf Normalization, typename String>
    basic_text<Normalization, String>::basic_text(text_view tv) :
        str_(tv.begin().base().base(), tv.end().base().base())
    {}

    template<nf Normalization, typename String>
    basic_text<Normalization, String>::basic_text(rope_view rv) :
        str_(rv.begin().base().base(), rv.end().base().base())
    {}

    template<nf Normalization, typename String>
    basic_text<Normalization, String> &
    basic_text<Normalization, String>::operator=(text_view tv)
    {
        str_.assign(tv.begin().base().base(), tv.end().base().base());
        return *this;
    }

    template<nf Normalization, typename String>
    basic_text<Normalization, String> &
    basic_text<Normalization, String>::operator=(rope_view rv)
    {
        str_.assign(rv.begin().base().base(), rv.end().base().base());
        return *this;
    }

    template<nf Normalization, typename String>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::insert(iterator at, grapheme const & g)
    {
        return insert(at, grapheme_ref<grapheme::const_iterator>(g));
    }

    template<nf Normalization, typename String>
    template<typename CPIter>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::insert(
        iterator at, grapheme_ref<CPIter> g)
    {
        if (g.empty())
            return replace_result<iterator>{at, at};
        std::array<char_type, 128> buf;
        auto out =
            boost::text::transcode_to_utf8(g.begin(), g.end(), buf.data());
        return insert_impl(at, buf.data(), out, insertion_not_normalized);
    }

    template<nf Normalization, typename String>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::replace(
        iterator first, iterator last, text_view new_substr)
    {
        return replace_impl(
            first,
            last,
            new_substr.begin().base().base(),
            new_substr.end().base().base(),
            insertion_normalized);
    }

#endif // Doxygen

    template<nf Normalization, typename String>
    bool operator==(
        basic_text<Normalization, String> const & lhs,
        typename basic_text<Normalization, String>::text_view rhs) noexcept
    {
        return algorithm::equal(
            lhs.begin().base().base(),
            lhs.end().base().base(),
            rhs.begin().base().base(),
            rhs.end().base().base());
    }
    template<nf Normalization, typename String>
    bool operator==(
        typename basic_text<Normalization, String>::text_view lhs,
        basic_text<Normalization, String> const & rhs) noexcept
    {
        return rhs == lhs;
    }

    template<nf Normalization, typename String>
    bool operator!=(
        basic_text<Normalization, String> const & lhs,
        typename basic_text<Normalization, String>::text_view rhs) noexcept
    {
        return !(lhs == rhs);
    }
    template<nf Normalization, typename String>
    bool operator!=(
        typename basic_text<Normalization, String>::text_view lhs,
        basic_text<Normalization, String> const & rhs) noexcept
    {
        return !(lhs == rhs);
    }

    template<nf Normalization, typename String>
    bool operator==(
        basic_text<Normalization, String> const & lhs,
        basic_text<Normalization, String> const & rhs) noexcept
    {
        return algorithm::equal(
            lhs.begin().base().base(),
            lhs.end().base().base(),
            rhs.begin().base().base(),
            rhs.end().base().base());
    }

    template<nf Normalization, typename String>
    bool operator!=(
        basic_text<Normalization, String> const & lhs,
        basic_text<Normalization, String> const & rhs) noexcept
    {
        return !(lhs == rhs);
    }

#if defined(__cpp_lib_concepts)

    /** Creates a new `basic_text` object that is the concatenation of `t` and
        some object `x` for which `t.insert(t.end(), x)` is well-formed. */
    template<nf Normalization, typename String, typename T>
    basic_text<Normalization, String>
    operator+(basic_text<Normalization, String> t, T const & x)
        // clang-format off
        requires requires { t.insert(t.end(), x); }
    // clang-format on
    {
        t.insert(t.end(), x);
        return t;
    }

    /** Creates a new `basic_text` object that is the concatenation of `x` and
        `t`, where `x` is an object for which `t.insert(t.begin(), x)` is
        well-formed. */
    template<nf Normalization, typename String, typename T>
    auto operator+(T const & x, basic_text<Normalization, String> t)
        // clang-format off
        requires requires { t.insert(t.begin(), x); } &&
            (!std::is_same_v<T, basic_text<Normalization, String>>)
    // clang-format on
    {
        t.insert(t.begin(), x);
        return t;
    }

#else

    /** Creates a new `basic_text` object that is the concatenation of `t` and
        some object `x` for which `t.insert(t.end(), x) well-formed. */
    template<nf Normalization, typename String, typename T>
    auto operator+(basic_text<Normalization, String> t, T const & x)
        -> decltype(t.insert(t.end(), x), basic_text<Normalization, String>{})
    {
        t.insert(t.end(), x);
        return t;
    }

    /** Creates a new `basic_text` object that is the concatenation of `x` and
        `t`, where `x` is an object for which `t.insert(t.begin(), x)` is
        well-formed. */
    template<nf Normalization, typename String, typename T>
    auto operator+(T const & x, basic_text<Normalization, String> t)
        -> std::enable_if_t<
            !std::is_same<T, basic_text<Normalization, String>>::value,
            decltype(
                t.insert(t.begin(), x), basic_text<Normalization, String>{})>
    {
        t.insert(t.begin(), x);
        return t;
    }

#endif

    /** Creates a new basic_text object that is the concatenation of t and
        c_str. */
    template<nf Normalization, typename String>
    basic_text<Normalization, String>
    operator+(basic_text<Normalization, String> t, char const * c_str)
    {
        return t += c_str;
    }

    /** Creates a new text object that is the concatenation of c_str and t. */
    template<nf Normalization, typename String>
    basic_text<Normalization, String>
    operator+(char const * c_str, basic_text<Normalization, String> const & t)
    {
        return basic_text<Normalization, String>(c_str) + t;
    }

}}

#ifndef BOOST_TEXT_DOXYGEN

namespace std {
    template<boost::text::nf Normalization, typename String>
    struct hash<boost::text::basic_text<Normalization, String>>
    {
        using argument_type = boost::text::basic_text<Normalization, String>;
        using result_type = std::size_t;
        result_type operator()(argument_type const & t) const noexcept
        {
            return boost::text::detail::hash_grapheme_range(t);
        }
    };
}

#endif

#endif
