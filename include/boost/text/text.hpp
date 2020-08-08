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

    template<nf Normalization, typename Char, typename String>
#if defined(__cpp_lib_concepts)
        // clang-format off
        requires (utf8_code_unit<Char> || utf16_code_unit<Char>) &&
            std::is_same_v<Char, std::ranges::range_value_t<String>>
#endif
    struct basic_text
    // clang-format on
    {
        /** The normalization form used in this `basic_text`. */
        static constexpr nf normalization = Normalization;

        /** The type of code unit used in the underlying storage. */
        using char_type = Char;

        /** The type of the container used as underlying storage. */
        using string = String;

        /** The specialization of `std::basic_string_view` (or
            `boost::basic_string_view` in pre-C++17 code) compatible with
            `string`. */
        using string_view = basic_string_view<char_type>;

        /** The specialization of `basic_text_view` with the same
            normalization form and underlying code unit type. */
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

        /** Constructs a `basic_text` from a pair of iterators. */
        basic_text(const_iterator first, const_iterator last) :
            basic_text(text_view(first, last))
        {}

        /** Constructs a `basic_text` from a null-terminated string. */
        basic_text(char_type const * c_str) : str_(c_str)
        {
            boost::text::normalize<normalization>(str_);
        }

        /** Constructs a `basic_text` from a `text_view`. */
        explicit basic_text(text_view tv);

        /** Constructs a `basic_text` from a `rope_view`. */
        explicit basic_text(rope_view rv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Constructs a `basic_text` from a range of `char_type`.

            This function only participates in overload resolution if
            `CURange` models the CURange concept. */
        template<typename CURange>
        explicit basic_text(CURange const & r);

        /** Constructs a `basic_text` from a sequence of `char_type`.

            This function only participates in overload resolution if
            `CUIter` models the CUIter concept. */
        template<typename CUIter, typename Sentinel>
        basic_text(CUIter first, Iter CUlast);

        /** Constructs a `basic_text` from a range of graphemes.

            This function only participates in overload resolution if
            `GraphemeCURange` models the GraphemeCURange concept. */
        template<typename GraphemeCURange>
        explicit basic_text(GraphemeCURange const & r);

        /** Constructs a `basic_text` from a sequence of graphemes.

            This function only participates in overload resolution if
            `GraphemeCUIter` models the GraphemeCUIter concept. */
        template<typename GraphemeCUIter>
        explicit basic_text(GraphemeCUIter first, GraphemeCUIter last);

#else

#if defined(__cpp_lib_concepts)
        template<code_unit_range<utf_format> R>
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
        template<code_unit_iterator<utf_format> I, std::sentinel_for<I> S>
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
        template<grapheme_range_code_unit<utf_format> R>
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

#if defined(__cpp_lib_concepts)
        template<grapheme_iter_code_unit<utf_format> I>
        explicit basic_text(I first, I last) :
#else
        template<typename I>
        explicit basic_text(
            I first,
            I last,
            detail::graph_iter_alg_cu_ret_t<(int)utf_format, int *, I> = 0) :
#endif
            str_(detail::make_string<string>(
                first.base().base(), last.base().base()))
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

        /** Assignment from a `string_view`. */
        basic_text & operator=(string_view sv)
        {
            str_.assign(sv.begin(), sv.end());
            boost::text::normalize<normalization>(str_);
            return *this;
        }

        /** Assignment from a `text_view`. */
        basic_text & operator=(text_view tv);

        /** Assignment from a `rope_view`. */
        basic_text & operator=(rope_view rv);

        operator text_view() const noexcept;


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

        /** Returns true iff `size() == 0`. */
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

        /** Returns the maximum size in bytes a `basic_text` can have. */
        size_type max_bytes() const noexcept { return PTRDIFF_MAX; }

        /** Clear.

            \post size() == 0 && capacity() == 0; begin(), end() delimit an
            empty string */
        void clear() noexcept { str_.clear(); }

        /** Erases the portion of `*this` delimited by `[first, last)`.

            \pre first <= last */
        replace_result<iterator>
        erase(const_iterator first, const_iterator last) noexcept
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
            const_iterator first1,
            const_iterator last1,
            const_iterator first2,
            const_iterator last2)
        {
            return replace_impl(
                first1,
                last1,
                first2.base().base(),
                last2.base().base(),
                insertion_normalized);
        }

        /** Replaces the portion of `*this` delimited by `[first, last)` with
            `new_substr`.

            \pre !std::less(first.base().base(), begin().base().base()) &&
            !std::less(end().base().base(), last.base().base()) */
        replace_result<iterator> replace(
            const_iterator first,
            const_iterator last,
            char_type const * new_substr)
        {
            auto const insertion = string_view(new_substr);
            return replace_impl(
                first,
                last,
                insertion.begin(),
                insertion.end(),
                insertion_not_normalized);
        }

        /** Replaces the portion of `*this` delimited by `[first, last)` with
            `new_substr`.

            \pre !std::less(first.base().base(), begin().base().base()) &&
            !std::less(end().base().base(), last.base().base()) */
        replace_result<iterator> replace(
            const_iterator first,
            const_iterator last,
            basic_text const & new_substr)
        {
            return replace_impl(
                first,
                last,
                new_substr.begin().base().base(),
                new_substr.end().base().base(),
                insertion_normalized);
        }

        /** Replaces the portion of `*this` delimited by `[first, last)` with
            `new_substr`.

            \pre !std::less(first.base().base(), begin().base().base()) &&
            !std::less(end().base().base(), last.base().base()) */
        replace_result<iterator> replace(
            const_iterator first, const_iterator last, text_view new_substr);

        /** Replaves the portion of `*this` delimited by `[first, last)` with
            `new_substr`.

            \pre !std::less(first.base().base(), begin().base().base()) &&
            !std::less(end().base().base(), last.base().base()) */
        replace_result<iterator> replace(
            const_iterator first, const_iterator last, string_view new_substr)
        {
            return replace_impl(
                first,
                last,
                new_substr.begin(),
                new_substr.end(),
                insertion_not_normalized);
        }

        /** Replaces the portion of `*this` delimited by `[first, last)` with
            `new_substr`.

            \pre !std::less(first.base().base(), begin().base().base()) &&
            !std::less(end().base().base(), last.base().base()) */
        replace_result<iterator> replace(
            const_iterator first, const_iterator last, rope_view new_substr);

#ifdef BOOST_TEXT_DOXYGEN

        /** Replaces the portion of `*this` delimited by `[first, last)` with
            `r`.

            This function only participates in overload resolution if
            `CURange` models the CURange concept.

            \pre !std::less(first.base().base(), begin().base().base()) &&
            !std::less(end().base().base(), last.base().base()) */
        template<typename CURange>
        replace_result<iterator>
        replace(const_iterator first, const_iterator last, CURange const & r);

        /** Replaces the portion of `*this` delimited by `[first1, last1)`
            with `[first2, last2)`.

            This function only participates in overload resolution if
            `CUIter` models the CUIter concept.

            \pre !std::less(first.base().base(), begin().base().base()) &&
            !std::less(end().base().base(), last.base().base()) */
        template<typename CUIter>
        replace_result<iterator> replace(
            const_iterator first1,
            const_iterator last1,
            CUIter first2,
            CUIter last2);

        /** Replaces the portion of `*this` delimited by `[first, last)` with
            `r`.

            This function only participates in overload resolution if
            `GraphemeCURange` models the GraphemeCURange concept.

            \pre !std::less(first.base().base(), begin().base().base()) &&
            !std::less(end().base().base(), last.base().base()) */
        template<typename GraphemeCURange>
        replace_result<iterator> replace(
            const_iterator first,
            const_iterator last,
            GraphemeCURange const & r);

        /** Replaces the portion of `*this` delimited by `[first1, last1)`
            with `[first2, last2)`.

            This function only participates in overload resolution if
            `GraphemeCUIter` models the GraphemeCUIter concept.

            \pre !std::less(first.base().base(), begin().base().base()) &&
            !std::less(end().base().base(), last.base().base()) */
        template<typename GraphemeCUIter>
        replace_result<iterator> replace(
            const_iterator first1,
            const_iterator last1,
            GraphemeCUIter first2,
            GraphemeCUIter last2);

#else

#if defined(__cpp_lib_concepts)
        template<code_unit_range<utf_format> R>
        replace_result<iterator>
        replace(const_iterator first, const_iterator last, R const & r)
        {
            return replace(
                first, last, std::ranges::begin(r), std::ranges::end(r));
        }
#else
        template<typename R>
        auto replace(const_iterator first, const_iterator last, R const & r)
            -> detail::
                cu_rng_alg_ret_t<(int)utf_format, replace_result<iterator>, R>
        {
            return replace(first, last, std::begin(r), std::end(r));
        }
#endif

#if defined(__cpp_lib_concepts)
        template<code_unit_iterator<utf_format> I>
        replace_result<iterator>
        replace(const_iterator first1, const_iterator last1, I first2, I last2)
#else
        template<typename I>
        auto
        replace(const_iterator first1, const_iterator last1, I first2, I last2)
            -> detail::
                cu_iter_ret_t<(int)utf_format, replace_result<iterator>, I>
#endif
        {
            return replace_impl(
                first1, last1, first2, last2, insertion_not_normalized);
        }

#if defined(__cpp_lib_concepts)
        template<grapheme_range_code_unit<utf_format> R>
        replace_result<iterator>
        replace(const_iterator first, const_iterator last, R const & r)
        {
            return replace(
                first,
                last,
                std::ranges::begin(r).base().base(),
                std::ranges::end(r).base().base());
        }
#else
        template<typename R>
        auto replace(const_iterator first, const_iterator last, R const & r)
            -> detail::graph_rng_alg_ret_t<replace_result<iterator>, R>
        {
            return replace(first, last, std::begin(r), std::end(r));
        }
#endif

#if defined(__cpp_lib_concepts)
        template<grapheme_iter_code_unit<utf_format> I>
        replace_result<iterator>
        replace(const_iterator first1, const_iterator last1, I first2, I last2)
#else
        template<typename I>
        auto
        replace(const_iterator first1, const_iterator last1, I first2, I last2)
            -> detail::graph_iter_alg_cu_ret_t<
                (int)utf_format,
                replace_result<iterator>,
                I>
#endif
        {
            return replace_impl(
                first1,
                last1,
                first2.base().base(),
                last2.base().base(),
                insertion_not_normalized);
        }

#endif

        /** Replaces the portion of `*this` delimited by `[first, last)` with
            `g`. */
        replace_result<iterator>
        replace(const_iterator first, const_iterator last, grapheme const & g);

        /** Replaces the portion of `*this` delimited by `[first, last)` with
            `g`. */
        template<typename CPIter>
        replace_result<iterator> replace(
            const_iterator first, const_iterator last, grapheme_ref<CPIter> g);

        /** Inserts the sequence `[first, last)` into `*this` starting at
            position `at`. */
        replace_result<iterator>
        insert(const_iterator at, const_iterator first, const_iterator last)
        {
            return replace(at, at, first, last);
        }

        /** Inserts the sequence of `char_type` from `x` into `*this` starting
            at position `at`. */
        template<typename T>
        auto insert(const_iterator at, T const & x)
            -> decltype(replace(at, at, x))
        {
            return replace(at, at, x);
        }

        /** Inserts the sequence `[first, last)` into `*this` starting at
            position `at`. */
        template<typename I>
        auto insert(const_iterator at, I first, I last)
            -> decltype(replace(at, at, first, last))
        {
            return replace(at, at, first, last);
        }

        /** Reserves storage enough for a string of at least `new_size` bytes.

            \post capacity() >= new_size + 1 */
        void reserve(size_type new_size) { str_.reserve(new_size); }

        /** Reduces storage used by `*this` to just the amount necessary to
            contain `size()` chars.

            \post capacity() == 0 || capacity() == size() + 1 */
        void shrink_to_fit() { str_.shrink_to_fit(); }

        /** Swaps `*this` with rhs. */
        void swap(basic_text & rhs) noexcept { str_.swap(rhs.str_); }

        /** Removes and returns the underlying string from `*this`. */
        string extract() && noexcept { return std::move(str_); }

        /** Replaces the underlying string in `*this`.

            \pre s is normalized FCC. */
        void replace(string && s) noexcept { str_ = std::move(s); }

        /** Appends `x` to `*this`.  `T` may be any type for which `*this = x`
            is well-formed. */
        template<typename T>
        auto operator+=(T const & x) -> decltype(*this = x)
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

        template<typename Iter>
        replace_result<iterator>
        mutation_result(replace_result<Iter> str_replacement)
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
        replace_result<iterator> replace_impl(
            const_iterator first,
            const_iterator last,
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

    template<nf Normalization, typename Char, typename String>
    basic_text<Normalization, Char, String>::basic_text(text_view tv) :
        str_(tv.begin().base().base(), tv.end().base().base())
    {}

    template<nf Normalization, typename Char, typename String>
    basic_text<Normalization, Char, String>::basic_text(rope_view rv) :
        str_(rv.begin().base().base(), rv.end().base().base())
    {}

    template<nf Normalization, typename Char, typename String>
    basic_text<Normalization, Char, String> &
    basic_text<Normalization, Char, String>::operator=(text_view tv)
    {
        str_.assign(tv.begin().base().base(), tv.end().base().base());
        return *this;
    }

    template<nf Normalization, typename Char, typename String>
    basic_text<Normalization, Char, String> &
    basic_text<Normalization, Char, String>::operator=(rope_view rv)
    {
        str_.assign(rv.begin().base().base(), rv.end().base().base());
        return *this;
    }

    template<nf Normalization, typename Char, typename String>
    basic_text<Normalization, Char, String>::
    operator basic_text<Normalization, Char, String>::text_view() const noexcept
    {
        return {begin(), end()};
    }


    template<nf Normalization, typename Char, typename String>
    replace_result<typename basic_text<Normalization, Char, String>::iterator>
    basic_text<Normalization, Char, String>::replace(
        const_iterator first, const_iterator last, grapheme const & g)
    {
        return replace(first, last, grapheme_ref<grapheme::const_iterator>(g));
    }

    template<nf Normalization, typename Char, typename String>
    template<typename CPIter>
    replace_result<typename basic_text<Normalization, Char, String>::iterator>
    basic_text<Normalization, Char, String>::replace(
        const_iterator first, const_iterator last, grapheme_ref<CPIter> g)
    {
        if (g.empty() && first == last) {
            auto const first_ptr = const_cast<char_type *>(str_.data());
            auto const it = make_iter(
                first_ptr,
                const_cast<char_type *>(first.base().base()),
                first_ptr + str_.size());
            return replace_result<iterator>{it, it};
        }
        std::array<char_type, 128> buf;
        auto out =
            boost::text::transcode_to_utf8(g.begin(), g.end(), buf.data());
        return replace_impl(
            first, last, buf.data(), out, insertion_not_normalized);
    }

    template<nf Normalization, typename Char, typename String>
    replace_result<typename basic_text<Normalization, Char, String>::iterator>
    basic_text<Normalization, Char, String>::replace(
        const_iterator first, const_iterator last, text_view new_substr)
    {
        return replace_impl(
            first,
            last,
            new_substr.begin().base().base(),
            new_substr.end().base().base(),
            insertion_normalized);
    }

#endif // Doxygen

    template<nf Normalization, typename Char, typename String>
    bool operator==(
        basic_text<Normalization, Char, String> const & lhs,
        typename basic_text<Normalization, Char, String>::text_view
            rhs) noexcept
    {
        return algorithm::equal(
            lhs.begin().base().base(),
            lhs.end().base().base(),
            rhs.begin().base().base(),
            rhs.end().base().base());
    }
    template<nf Normalization, typename Char, typename String>
    bool operator==(
        typename basic_text<Normalization, Char, String>::text_view lhs,
        basic_text<Normalization, Char, String> const & rhs) noexcept
    {
        return rhs == lhs;
    }

    template<nf Normalization, typename Char, typename String>
    bool operator!=(
        basic_text<Normalization, Char, String> const & lhs,
        typename basic_text<Normalization, Char, String>::text_view
            rhs) noexcept
    {
        return !(lhs == rhs);
    }
    template<nf Normalization, typename Char, typename String>
    bool operator!=(
        typename basic_text<Normalization, Char, String>::text_view lhs,
        basic_text<Normalization, Char, String> const & rhs) noexcept
    {
        return !(lhs == rhs);
    }

    template<nf Normalization, typename Char, typename String>
    bool operator==(
        basic_text<Normalization, Char, String> const & lhs,
        basic_text<Normalization, Char, String> const & rhs) noexcept
    {
        return algorithm::equal(
            lhs.begin().base().base(),
            lhs.end().base().base(),
            rhs.begin().base().base(),
            rhs.end().base().base());
    }

    template<nf Normalization, typename Char, typename String>
    bool operator!=(
        basic_text<Normalization, Char, String> const & lhs,
        basic_text<Normalization, Char, String> const & rhs) noexcept
    {
        return !(lhs == rhs);
    }

#if defined(__cpp_lib_concepts)

    /** Creates a new `basic_text` object that is the concatenation of `t` and
        some object `x` for which `t = x` is well-formed. */
    template<nf Normalization, typename Char, typename String, typename T>
    basic_text<Normalization, Char, String>
    operator+(basic_text<Normalization, Char, String> t, T const & x)
        // clang-format off
        requires requires { t = x; }
    // clang-format on
    {
        t.insert(t.end(), x);
        return t;
    }

    /** Creates a new `basic_text` object that is the concatenation of `x` and
        `t`, where `x` is an object for which `t = x` is well-formed. */
    template<nf Normalization, typename Char, typename String, typename T>
    basic_text<Normalization, Char, String>
    operator+(T const & x, basic_text<Normalization, Char, String> t)
        // clang-format off
        requires requires { t = x; } &&
            (!std::is_same_v<T, basic_text<Normalization, Char, String>>)
    // clang-format on
    {
        t.insert(t.begin(), x);
        return t;
    }

#else

    /** Creates a new `basic_text` object that is the concatenation of `t` and
        some object `x` for which `t = x` is well-formed. */
    template<nf Normalization, typename Char, typename String, typename T>
    auto operator+(basic_text<Normalization, Char, String> t, T const & x)
        -> decltype(t = x, basic_text<Normalization, Char, String>{})
    {
        t.insert(t.end(), x);
        return t;
    }

    /** Creates a new `basic_text` object that is the concatenation of `x` and
        `t`, where `x` is an object for which `t = x` is well-formed. */
    template<nf Normalization, typename Char, typename String, typename T>
    auto operator+(T const & x, basic_text<Normalization, Char, String> t)
        -> std::enable_if_t<
            !std::is_same<T, basic_text<Normalization, Char, String>>::value,
            decltype(t = x, basic_text<Normalization, Char, String>{})>
    {
        t.insert(t.begin(), x);
        return t;
    }

#endif

}}

#ifndef BOOST_TEXT_DOXYGEN

namespace std {
    template<boost::text::nf Normalization, typename Char, typename String>
    struct hash<boost::text::basic_text<Normalization, Char, String>>
    {
        using argument_type =
            boost::text::basic_text<Normalization, Char, String>;
        using result_type = std::size_t;
        result_type operator()(argument_type const & t) const noexcept
        {
            return boost::text::detail::hash_grapheme_range(t);
        }
    };
}

#endif

#endif
