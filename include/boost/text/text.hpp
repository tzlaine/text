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
#include <boost/text/transcode_iterator.hpp>
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
        boost::text::normalize<nf::fcc>(str2);                                 \
        BOOST_ASSERT(str_ == str2);                                            \
    } while (false)
#else
#define BOOST_TEXT_CHECK_TEXT_NORMALIZATION()
#endif

#endif

namespace boost { namespace text {

    namespace detail {
        template<typename T, typename U, typename R>
        using enable_if_different =
            std::enable_if_t<!std::is_same<T, U>::value, R>;
    }

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
        requires u8_code_unit<std::ranges::range_value_t<String>> // TODO: Support for UTF-16
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
        // TODO: This should change once testing covers UTF-16.
        static_assert(
            utf_format == format::utf8 /* || utf_format == format::utf16*/, "");

        using value_type = grapheme;
        using size_type = std::size_t;
        using iterator = grapheme_iterator<utf_8_to_32_iterator<char_type *>>;
        using const_iterator =
            grapheme_iterator<utf_8_to_32_iterator<char_type const *>>;
        using reverse_iterator = stl_interfaces::reverse_iterator<iterator>;
        using const_reverse_iterator =
            stl_interfaces::reverse_iterator<const_iterator>;

        /** Default ctor. */
        basic_text() {}

        /** Constructs a basic_text from a pair of iterators. */
        basic_text(iterator first, iterator last);

        /** Constructs a basic_text from a pair of iterators. */
        basic_text(const_iterator first, const_iterator last);

        /** Constructs a basic_text from a null-terminated string. */
        basic_text(char_type const * c_str) : str_(c_str)
        {
            boost::text::normalize<nf::fcc>(str_);
        }

        /** Constructs a basic_text from a string. */
        explicit basic_text(string s);

        /** Constructs a basic_text from a text_view. */
        explicit basic_text(text_view tv);

        /** Constructs a basic_text from a string_view. */
        explicit basic_text(string_view sv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Constructs a basic_text from a range of char.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept. */
        template<typename CharRange>
        explicit basic_text(CharRange const & r);

        /** Constructs a basic_text from a sequence of char.

            This function only participates in overload resolution if
            `CharIter` models the CharIter concept. */
        template<typename CharIter, typename Sentinel>
        basic_text(CharIter first, Iter Charlast);

        /** Constructs a basic_text from a range of graphemes over an underlying
            range of char.

            This function only participates in overload resolution if
            `GraphemeRange` models the GraphemeRange concept. */
        template<typename GraphemeRange>
        explicit basic_text(GraphemeRange const & r);

#else

        template<typename CharRange>
        explicit basic_text(
            CharRange const & r, detail::rng_alg_ret_t<int *, CharRange> = 0);

        template<typename CharIter, typename Sentinel>
        basic_text(
            CharIter first,
            Sentinel last,
            detail::char_iter_ret_t<void *, CharIter> = 0);

        template<typename GraphemeRange>
        explicit basic_text(
            GraphemeRange const & r,
            detail::graph_rng_alg_ret_t<int *, GraphemeRange> = 0);

#endif
        /** Assignment from a null-terminated string. */
        basic_text & operator=(char_type const * c_str)
        {
            str_ = c_str;
            boost::text::normalize<nf::fcc>(str_);
            return *this;
        }

        /** Assignment from a string. */
        basic_text & operator=(string s);

        /** Assignment from a text_view. */
        basic_text & operator=(text_view tv);

        /** Assignment from a string_view. */
        basic_text & operator=(string_view sv);


#ifdef BOOST_TEXT_DOXYGEN

        /** Assignment from a range of char.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept. */
        template<typename CharRange>
        basic_text & operator=(CharRange const & r);

        /** Assignment from a range of graphemes over an underlying range of
            char.

            This function only participates in overload resolution if
            `GraphemeRange` models the GraphemeRange concept. */
        template<typename GraphemeRange>
        basic_text & operator=(GraphemeRange const & r);

#else

        template<typename CharRange>
        auto operator=(CharRange const & r)
            -> detail::rng_alg_ret_t<basic_text &, CharRange>;

        template<typename GraphemeRange>
        auto operator=(GraphemeRange const & r)
            -> detail::graph_rng_alg_ret_t<basic_text &, GraphemeRange>;

#endif

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

        /** Returns the number of bytes controlled by *this, not including the
            null terminator. */
        size_type storage_bytes() const noexcept { return str_.size(); }

        /** Returns the number of bytes of storage currently in use by
         *this. */
        size_type capacity_bytes() const noexcept { return str_.capacity(); }

        /** Returns the number of graphemes in *this.  This operation is
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

        /** Inserts the sequence of char from c_str into *this starting at
            position at. */
        replace_result<iterator> insert(iterator at, char_type const * c_str);

        /** Inserts the sequence of char from t into *this starting at
            position at. */
        replace_result<iterator> insert(iterator at, basic_text const & t);

        /** Inserts the sequence of char from tv into *this starting at
            position at. */
        replace_result<iterator> insert(iterator at, text_view tv);

        /** Inserts the sequence of char from sv into *this starting at
            position at. */
        replace_result<iterator> insert(iterator at, string_view sv);

        /** Inserts the sequence of char from rv into *this starting at
            position at. */
        replace_result<iterator> insert(iterator at, rope_view rv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Inserts the char range r into *this starting at position at.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept. */
        template<typename CharRange>
        replace_result<iterator> insert(iterator at, CharRange const & r);

        /** Inserts the char sequence [first, last) into *this starting at
            position at.

            This function only participates in overload resolution if
            `CharIter` models the CharIter concept. */
        template<typename CharIter, typename Sentinel>
        replace_result<iterator>
        insert(iterator at, CharIter first, Sentinel last);

#else

        template<typename CharRange>
        auto insert(iterator at, CharRange const & r)
            -> detail::rng_alg_ret_t<replace_result<iterator>, CharRange>;

        template<typename CharIter, typename Sentinel>
        auto insert(iterator at, CharIter first, Sentinel last)
            -> detail::char_iter_ret_t<replace_result<iterator>, CharIter>;

#endif

        /** Inserts the sequence [first, last) into *this starting at position
            at. */
        replace_result<iterator>
        insert(iterator at, const_iterator first, const_iterator last);

        /** Inserts the grapheme g into *this at position at. */
        replace_result<iterator> insert(iterator at, grapheme const & g);

        /** Inserts the grapheme g into *this at position at. */
        template<typename CPIter>
        replace_result<iterator> insert(iterator at, grapheme_ref<CPIter> g);

        /** Erases the portion of *this delimited by tv.

            \pre !std::less(tv.begin().base().base(), begin().base().base()) &&
            !std::less(end().base().base(), tv.end().base().base()) */
        replace_result<iterator> erase(text_view tv) noexcept;

        /** Erases the portion of *this delimited by [first, last).

            \pre first <= last */
        replace_result<iterator> erase(iterator first, iterator last) noexcept;

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        replace_result<iterator>
        replace(text_view old_substr, char_type const * new_substr);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        replace_result<iterator>
        replace(text_view old_substr, basic_text const & new_substr);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        replace_result<iterator>
        replace(text_view old_substr, text_view new_substr);

        /** Replaves the  portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        replace_result<iterator>
        replace(text_view old_substr, string_view new_substr);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        replace_result<iterator>
        replace(text_view old_substr, rope_view new_substr);

#ifdef BOOST_TEXT_DOXYGEN

        /** Replaces the portion of *this delimited by old_substr with the
            char range r.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        template<typename CharRange>
        replace_result<iterator>
        replace(text_view old_substr, CharRange const & r);

        /** Replaces the portion of *this delimited by old_substr with the
            char sequence [first, last).

            This function only participates in overload resolution if
            `CharIter` models the CharIter concept.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        template<typename CharIter, typename Sentinel>
        replace_result<iterator>
        replace(text_view old_substr, CharIter first, Sentinel last);

#else

        template<typename CharRange>
        auto replace(text_view old_substr, CharRange const & r)
            -> detail::rng_alg_ret_t<replace_result<iterator>, CharRange>;

        template<typename CharIter, typename Sentinel>
        auto replace(text_view old_substr, CharIter first, Sentinel last)
            -> detail::char_iter_ret_t<replace_result<iterator>, CharIter>;

#endif

        /** Replaces the portion of *this delimited by old_substr with the
            sequence [first, last).

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        replace_result<iterator> replace(
            text_view old_substr, const_iterator first, const_iterator last);

        /** Reserves storage enough for a string of at least new_size
            bytes.

            \post capacity() >= new_size + 1 */
        void reserve(size_type new_size) { str_.reserve(new_size); }

        /** Reduces storage used by *this to just the amount necessary to
            contain size() chars.

            \post capacity() == 0 || capacity() == size() + 1 */
        void shrink_to_fit() { str_.shrink_to_fit(); }

        /** Swaps *this with rhs. */
        void swap(basic_text & rhs) noexcept { str_.swap(rhs.str_); }

        /** Removes and returns the underlying string from *this. */
        string extract() && noexcept { return std::move(str_); }

        /** Replaces the underlying string in *this.

            \pre s is normalized FCC. */
        void replace(string && s) noexcept { str_ = std::move(s); }

        /** Appends c_str to *this. */
        basic_text & operator+=(char_type const * c_str);

        /** Appends tv to *this. */
        basic_text & operator+=(string_view sv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Appends the char range r to *this.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept. */
        template<typename CharRange>
        basic_text & operator+=(CharRange const & r);

#else

        template<typename CharRange>
        auto operator+=(CharRange const & r)
            -> detail::rng_alg_ret_t<basic_text &, CharRange>;

#endif

        /** Stream inserter; performs formatted output, in UTF-8 encoding. */
        friend std::ostream &
        operator<<(std::ostream & os, basic_text const & t)
        {
            if (os.good()) {
                auto const size = boost::text::estimated_width_of_graphemes(
                    t.begin().base(), t.end().base());
                detail::pad_width_before(os, size);
                os << t.str_;
                if (os.good())
                    detail::pad_width_after(os, size);
            }
            return os;
        }

#ifndef BOOST_TEXT_DOXYGEN

    private:
        static iterator
        make_iter(char_type * first, char_type * it, char_type * last) noexcept
        {
            return iterator{
                utf_8_to_32_iterator<char_type *>{first, first, last},
                utf_8_to_32_iterator<char_type *>{first, it, last},
                utf_8_to_32_iterator<char_type *>{first, last, last}};
        }

        static const_iterator make_iter(
            char_type const * first,
            char_type const * it,
            char_type const * last) noexcept
        {
            return const_iterator{
                utf_8_to_32_iterator<char_type const *>{first, first, last},
                utf_8_to_32_iterator<char_type const *>{first, it, last},
                utf_8_to_32_iterator<char_type const *>{first, last, last}};
        }

        template<typename Iter>
        static stl_interfaces::reverse_iterator<Iter>
        make_reverse_iter(Iter it) noexcept
        {
            return stl_interfaces::reverse_iterator<Iter>{it};
        }

        using mutable_utf32_iter = utf_8_to_32_iterator<char_type *>;

        replace_result<iterator> mutation_result(
            replace_result<typename string::iterator> str_replacement);

        template<typename CharIter>
        replace_result<iterator> insert_impl(
            iterator at,
            CharIter first,
            CharIter last,
            insertion_normalization insertion_norm);

        template<typename CharIter>
        replace_result<iterator> replace_impl(
            text_view old_substr,
            CharIter first,
            CharIter last,
            insertion_normalization insertion_norm);

        template<typename CPIter>
        struct insert_grapheme_ref_impl;

        string str_;

        template<typename CPIter>
        friend struct insert_grapheme_ref_impl;

#endif // Doxygen
    };

}}

#include <boost/text/text_view.hpp>
#include <boost/text/rope.hpp>
#include <boost/text/detail/make_string.hpp>

namespace boost { namespace text {

    namespace literals {
        /** Creates a `text` from a `char` string literal. */
        inline text operator"" _t(char const * str, std::size_t len)
        {
            return text(str, str + len);
        }

#if defined(__cpp_char8_t)
        /** Creates a `text` from a `char8_t` string literal. */
        inline text operator"" _t(char8_t const * str, std::size_t len)
        {
            return text(str, str + len);
        }
#endif
    }

#ifndef BOOST_TEXT_DOXYGEN

    template<nf Normalization, typename String>
    basic_text<Normalization, String>::basic_text(
        iterator first, iterator last) :
        basic_text(text_view(first, last))
    {}

    template<nf Normalization, typename String>
    basic_text<Normalization, String>::basic_text(
        const_iterator first, const_iterator last) :
        basic_text(text_view(first, last))
    {}

    template<nf Normalization, typename String>
    basic_text<Normalization, String>::basic_text(string s) : str_(std::move(s))
    {
        boost::text::normalize<nf::fcc>(str_);
    }

    template<nf Normalization, typename String>
    basic_text<Normalization, String>::basic_text(text_view tv) :
        str_(tv.begin().base().base(), tv.end().base().base())
    {
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();
    }

    template<nf Normalization, typename String>
    basic_text<Normalization, String>::basic_text(string_view sv) :
        str_(sv.begin(), sv.end())
    {
        boost::text::normalize<nf::fcc>(str_);
    }

    template<nf Normalization, typename String>
    template<typename CharRange>
    basic_text<Normalization, String>::basic_text(
        CharRange const & r, detail::rng_alg_ret_t<int *, CharRange>) :
        str_(detail::make_string(r.begin(), r.end()))
    {
        boost::text::normalize<nf::fcc>(str_);
    }

    template<nf Normalization, typename String>
    template<typename CharIter, typename Sentinel>
    basic_text<Normalization, String>::basic_text(
        CharIter first,
        Sentinel last,
        detail::char_iter_ret_t<void *, CharIter>) :
        str_(detail::make_string(first, last))
    {
        boost::text::normalize<nf::fcc>(str_);
    }

    template<nf Normalization, typename String>
    template<typename GraphemeRange>
    basic_text<Normalization, String>::basic_text(
        GraphemeRange const & r,
        detail::graph_rng_alg_ret_t<int *, GraphemeRange>) :
        str_(
            detail::make_string(r.begin().base().base(), r.end().base().base()))
    {}

    template<nf Normalization, typename String>
    basic_text<Normalization, String> &
    basic_text<Normalization, String>::operator=(string s)
    {
        str_ = std::move(s);
        boost::text::normalize<nf::fcc>(str_);
        return *this;
    }

    template<nf Normalization, typename String>
    basic_text<Normalization, String> &
    basic_text<Normalization, String>::operator=(text_view tv)
    {
        str_.assign(tv.begin().base().base(), tv.end().base().base());
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();
        return *this;
    }

    template<nf Normalization, typename String>
    basic_text<Normalization, String> &
    basic_text<Normalization, String>::operator=(string_view sv)
    {
        str_.assign(sv.begin(), sv.end());
        boost::text::normalize<nf::fcc>(str_);
        return *this;
    }

    template<nf Normalization, typename String>
    template<typename CharRange>
    auto basic_text<Normalization, String>::operator=(CharRange const & r)
        -> detail::rng_alg_ret_t<basic_text<Normalization, String> &, CharRange>
    {
        str_.assign(r.begin(), r.end());
        boost::text::normalize<nf::fcc>(str_);
        return *this;
    }

    template<nf Normalization, typename String>
    template<typename GraphemeRange>
    auto basic_text<Normalization, String>::operator=(GraphemeRange const & r)
        -> detail::graph_rng_alg_ret_t<
            basic_text<Normalization, String> &,
            GraphemeRange>
    {
        str_.assign(r.begin().base().base(), r.begin().base().base());
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();
        return *this;
    }

    template<nf Normalization, typename String>
    template<typename CharRange>
    auto
    basic_text<Normalization, String>::insert(iterator at, CharRange const & r)
        -> detail::rng_alg_ret_t<replace_result<iterator>, CharRange>
    {
        return insert(at, std::begin(r), std::end(r));
    }

    template<nf Normalization, typename String>
    template<typename CharIter, typename Sentinel>
    auto basic_text<Normalization, String>::insert(
        iterator at, CharIter first, Sentinel last)
        -> detail::char_iter_ret_t<replace_result<iterator>, CharIter>
    {
        return insert_impl(at, first, last, insertion_not_normalized);
    }

    template<nf Normalization, typename String>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::insert(
        iterator at, char_type const * c_str)
    {
        return insert(at, string_view(c_str));
    }

    template<nf Normalization, typename String>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::insert(iterator at, basic_text const & t)
    {
        return insert_impl(
            at, t.str_.begin(), t.str_.end(), insertion_normalized);
    }

    template<nf Normalization, typename String>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::insert(iterator at, text_view tv)
    {
        auto const first = tv.begin().base().base();
        auto const last = tv.end().base().base();
        return insert_impl(at, first, last, insertion_normalized);
    }

    template<nf Normalization, typename String>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::insert(iterator at, string_view sv)
    {
        return insert_impl(at, sv.begin(), sv.end(), insertion_not_normalized);
    }

    template<nf Normalization, typename String>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::erase(text_view tv) noexcept
    {
        auto const lo = tv.begin().base().base() - str_.data();
        auto const hi = tv.end().base().base() - str_.data();
        auto const retval = boost::text::normalize_erase<nf::fcc>(
            str_, str_.begin() + lo, str_.begin() + hi);
        return mutation_result(retval);
    }

    template<nf Normalization, typename String>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::insert(
        iterator at, const_iterator first, const_iterator last)
    {
        return insert(at, text_view(first, last));
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
        return insert_grapheme_ref_impl<CPIter>::call(*this, at, g);
    }

    template<nf Normalization, typename String>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::erase(
        iterator first, iterator last) noexcept
    {
        auto const lo = first.base().base() - str_.data();
        auto const hi = last.base().base() - str_.data();
        auto const retval = boost::text::normalize_erase<nf::fcc>(
            str_, str_.begin() + lo, str_.begin() + hi);
        return mutation_result(retval);
    }

    template<nf Normalization, typename String>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::replace(
        text_view old_substr, char_type const * new_substr)
    {
        auto const insertion = string_view(new_substr);
        return replace_impl(
            old_substr,
            insertion.begin(),
            insertion.end(),
            insertion_not_normalized);
    }

    template<nf Normalization, typename String>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::replace(
        text_view old_substr, basic_text const & new_substr)
    {
        return replace_impl(
            old_substr,
            new_substr.begin().base().base(),
            new_substr.end().base().base(),
            insertion_normalized);
    }

    template<nf Normalization, typename String>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::replace(
        text_view old_substr, text_view new_substr)
    {
        return replace_impl(
            old_substr,
            new_substr.begin().base().base(),
            new_substr.end().base().base(),
            insertion_normalized);
    }

    template<nf Normalization, typename String>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::replace(
        text_view old_substr, string_view new_substr)
    {
        return replace_impl(
            old_substr,
            new_substr.begin(),
            new_substr.end(),
            insertion_not_normalized);
    }

    template<nf Normalization, typename String>
    template<typename CharRange>
    auto basic_text<Normalization, String>::replace(
        text_view old_substr, CharRange const & r)
        -> detail::rng_alg_ret_t<replace_result<iterator>, CharRange>
    {
        return replace(old_substr, std::begin(r), std::end(r));
    }

    template<nf Normalization, typename String>
    template<typename CharIter, typename Sentinel>
    auto basic_text<Normalization, String>::replace(
        text_view old_substr, CharIter first, Sentinel last)
        -> detail::char_iter_ret_t<replace_result<iterator>, CharIter>
    {
        return replace_impl(old_substr, first, last, insertion_not_normalized);
    }

    template<nf Normalization, typename String>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::replace(
        text_view old_substr, const_iterator first, const_iterator last)
    {
        return replace(old_substr, text_view(first, last));
    }

    template<nf Normalization, typename String>
    basic_text<Normalization, String> &
    basic_text<Normalization, String>::operator+=(char_type const * c_str)
    {
        return operator+=(string_view(c_str));
    }

    template<nf Normalization, typename String>
    basic_text<Normalization, String> &
    basic_text<Normalization, String>::operator+=(string_view sv)
    {
        insert(end(), sv);
        return *this;
    }

    template<nf Normalization, typename String>
    template<typename CharRange>
    auto basic_text<Normalization, String>::operator+=(CharRange const & r)
        -> detail::rng_alg_ret_t<basic_text<Normalization, String> &, CharRange>
    {
        insert(this->end(), std::begin(r), std::end(r));
        return *this;
    }

    template<nf Normalization, typename String>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::mutation_result(
        replace_result<typename string::iterator> str_replacement)
    {
        auto const str_first = const_cast<char_type *>(str_.data());
        auto const str_lo =
            str_first + (str_replacement.begin() - str_.begin());
        auto const str_hi = str_first + (str_replacement.end() - str_.begin());
        auto const str_last = str_first + str_.size();

        // The insertion that just happened might be merged into the CP or
        // grapheme ending at the offset of the inserted char(s); if so, back
        // up and return an iterator to that.
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
            iterator(begin().base(), lo_grapheme_it, end().base()),
            iterator(begin().base(), hi_grapheme_it, end().base())};
    }

    template<nf Normalization, typename String>
    template<typename CharIter>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::insert_impl(
        iterator at,
        CharIter first,
        CharIter last,
        insertion_normalization insertion_norm)
    {
        auto const str_at = str_.begin() + (at.base().base() - str_.data());
        auto retval = boost::text::normalize_insert<nf::fcc>(
            str_, str_at, boost::text::as_utf32(first, last), insertion_norm);
        return mutation_result(retval);
    }

    template<nf Normalization, typename String>
    template<typename CharIter>
    replace_result<typename basic_text<Normalization, String>::iterator>
    basic_text<Normalization, String>::replace_impl(
        text_view old_substr,
        CharIter first,
        CharIter last,
        insertion_normalization insertion_norm)
    {
        auto const str_first =
            str_.begin() + (old_substr.begin().base().base() - str_.data());
        auto const str_last =
            str_.begin() + (old_substr.end().base().base() - str_.data());
        auto const insertion = boost::text::as_utf32(first, last);
        auto retval = boost::text::normalize_replace<nf::fcc>(
            str_,
            str_first,
            str_last,
            insertion.begin(),
            insertion.end(),
            insertion_norm);
        return mutation_result(retval);
    }

    template<nf Normalization, typename String>
    template<typename CPIter>
    struct basic_text<Normalization, String>::insert_grapheme_ref_impl
    {
        static auto call(basic_text & t, iterator at, grapheme_ref<CPIter> g)
        {
            if (g.empty())
                return replace_result<iterator>{at, at};

            std::array<char_type, 1024> buf;
            auto out =
                boost::text::transcode_to_utf8(g.begin(), g.end(), buf.data());
            return t.insert_impl(at, buf.data(), out, insertion_normalized);
        }
    };

    template<nf Normalization, typename String>
    template<typename Sentinel, typename ErrorHandler>
    struct basic_text<Normalization, String>::insert_grapheme_ref_impl<
        utf_8_to_32_iterator<
            typename basic_text<Normalization, String>::char_type const *,
            Sentinel,
            ErrorHandler>>
    {
        static auto call(
            basic_text & t,
            basic_text<Normalization, String>::iterator at,
            grapheme_ref<
                utf_8_to_32_iterator<char_type const *, Sentinel, ErrorHandler>>
                g)
        {
            return t.insert_impl(
                at, g.begin().base(), g.end().base(), insertion_normalized);
        }
    };

    template<nf Normalization, typename String>
    template<typename Sentinel, typename ErrorHandler>
    struct basic_text<Normalization, String>::insert_grapheme_ref_impl<
        utf_8_to_32_iterator<
            typename basic_text<Normalization, String>::char_type *,
            Sentinel,
            ErrorHandler>>
    {
        static auto call(
            basic_text & t,
            basic_text<Normalization, String>::iterator at,
            grapheme_ref<
                utf_8_to_32_iterator<char_type *, Sentinel, ErrorHandler>> g)
        {
            return t.insert_impl(
                at, g.begin().base(), g.end().base(), insertion_normalized);
        }
    };

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

    /** Creates a new basic_text object that is the concatenation of t and t2.
     */
    template<nf Normalization, typename String>
    basic_text<Normalization, String> operator+(
        basic_text<Normalization, String> t,
        basic_text<Normalization, String> const & t2)
    {
        t.insert(t.end(), t2);
        return t;
    }

    /** Creates a new basic_text object that is the concatenation of t and
     * c_str. */
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

    /** Creates a new basic_text object that is the concatenation of t and tv.
     */
    template<nf Normalization, typename String>
    basic_text<Normalization, String> operator+(
        basic_text<Normalization, String> t,
        typename basic_text<Normalization, String>::text_view tv)
    {
        t.insert(t.end(), tv);
        return t;
    }

    /** Creates a new basic_text object that is the concatenation of tv and t.
     */
    template<nf Normalization, typename String>
    basic_text<Normalization, String> operator+(
        typename basic_text<Normalization, String>::text_view tv,
        basic_text<Normalization, String> const & t)
    {
        return basic_text<Normalization, String>(tv) + t;
    }

#ifdef BOOST_TEXT_DOXYGEN

    /** Creates a new basic_text object that is the concatenation of t and r.

        This function only participates in overload resolution if `CharRange`
        models the CharRange concept. */
    template<nf Normalization, typename String, typename CharRange>
    basic_text<Normalization, String>
    operator+(basic_text<Normalization, String> t, CharRange const & r);

    /** Creates a new basic_text object that is the concatenation of r and t.

        This function only participates in overload resolution if `CharRange`
        models the CharRange concept. */
    template<nf Normalization, typename String, typename CharRange>
    basic_text<Normalization, String>
    operator+(CharRange const & r, basic_text<Normalization, String> const & t);

#else

    template<nf Normalization, typename String, typename CharRange>
    auto operator+(basic_text<Normalization, String> t, CharRange const & r)
        -> detail::rng_alg_ret_t<basic_text<Normalization, String>, CharRange>
    {
        return t += r;
    }

    template<nf Normalization, typename String, typename CharRange>
    auto
    operator+(CharRange const & r, basic_text<Normalization, String> const & t)
        -> detail::rng_alg_ret_t<basic_text<Normalization, String>, CharRange>
    {
        return basic_text<Normalization, String>(r) + t;
    }

#endif

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
