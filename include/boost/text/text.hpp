// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_TEXT_HPP
#define BOOST_TEXT_TEXT_HPP

#include <boost/text/grapheme.hpp>
#include <boost/text/grapheme_iterator.hpp>
#include <boost/text/normalize_string.hpp>
#include <boost/text/string_view.hpp>
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
        std::string str2(str_);                                                \
        normalize<nf::fcc>(str2);                                              \
        BOOST_ASSERT(str_ == str2);                                            \
    } while (false)
#else
#define BOOST_TEXT_CHECK_TEXT_NORMALIZATION()
#endif

#endif

namespace boost { namespace text {

    struct text_view;
    struct rope_view;

    /** A mutable contiguous null-terminated sequence of graphemes.  The
        underlying storage is a std::string that is UTF-8-encoded and
        FCC-normalized. */
    struct text
    {
        using value_type = grapheme;
        using size_type = std::size_t;
        using iterator = grapheme_iterator<utf_8_to_32_iterator<char *>>;
        using const_iterator =
            grapheme_iterator<utf_8_to_32_iterator<char const *>>;
        using reverse_iterator = stl_interfaces::reverse_iterator<iterator>;
        using const_reverse_iterator =
            stl_interfaces::reverse_iterator<const_iterator>;

        /** Default ctor. */
        text() {}

        /** Constructs a text from a pair of iterators. */
        text(iterator first, iterator last);

        /** Constructs a text from a pair of iterators. */
        text(const_iterator first, const_iterator last);

        /** Constructs a text from a null-terminated string. */
        text(char const * c_str);

#if defined(__cpp_char8_t)
        /** Constructs a text from a null-terminated string. */
        text(char8_t const * c_str);
#endif

        /** Constructs a text from a string. */
        explicit text(std::string s);

        /** Constructs a text from a text_view. */
        explicit text(text_view tv);

        /** Constructs a text from a string_view. */
        explicit text(string_view sv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Constructs a text from a range of char.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept. */
        template<typename CharRange>
        explicit text(CharRange const & r);

        /** Constructs a text from a sequence of char.

            This function only participates in overload resolution if
            `CharIter` models the CharIter concept. */
        template<typename CharIter, typename Sentinel>
        text(CharIter first, Iter Charlast);

        /** Constructs a text from a range of graphemes over an underlying
            range of char.

            This function only participates in overload resolution if
            `GraphemeRange` models the GraphemeRange concept. */
        template<typename GraphemeRange>
        explicit text(GraphemeRange const & r);

#else

        template<typename CharRange>
        explicit text(
            CharRange const & r, detail::rng_alg_ret_t<int *, CharRange> = 0);

        template<typename CharIter, typename Sentinel>
        text(
            CharIter first,
            Sentinel last,
            detail::char_iter_ret_t<void *, CharIter> = 0);

        template<typename GraphemeRange>
        explicit text(
            GraphemeRange const & r,
            detail::graph_rng_alg_ret_t<int *, GraphemeRange> = 0);

#endif
        /** Assignment from a null-terminated string. */
        text & operator=(char const * c_str);

#if defined(__cpp_char8_t)
        /** Assignment from a null-terminated string. */
        text & operator=(char8_t const * c_str);
#endif

        /** Assignment from a string. */
        text & operator=(std::string s);

        /** Assignment from a text_view. */
        text & operator=(text_view tv);

        /** Assignment from a string_view. */
        text & operator=(string_view sv);


#ifdef BOOST_TEXT_DOXYGEN

        /** Assignment from a range of char.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept. */
        template<typename CharRange>
        text & operator=(CharRange const & r);

        /** Assignment from a range of graphemes over an underlying range of
            char.

            This function only participates in overload resolution if
            `GraphemeRange` models the GraphemeRange concept. */
        template<typename GraphemeRange>
        text & operator=(GraphemeRange const & r);

#else

        template<typename CharRange>
        auto operator=(CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        template<typename GraphemeRange>
        auto operator=(GraphemeRange const & r)
            -> detail::graph_rng_alg_ret_t<text &, GraphemeRange>;

#endif

        iterator begin() noexcept
        {
            auto const first = const_cast<char *>(str_.data());
            auto const last = first + str_.size();
            return make_iter(first, first, last);
        }
        iterator end() noexcept
        {
            auto const first = const_cast<char *>(str_.data());
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
        size_type distance() const noexcept { return std::distance(begin(), end()); }

        /** Returns the maximum size in bytes a text can have. */
        size_type max_bytes() const noexcept { return PTRDIFF_MAX; }

        /** Clear.

            \post size() == 0 && capacity() == 0; begin(), end() delimit an
            empty string */
        void clear() noexcept { str_.clear(); }

        /** Inserts the sequence of char from c_str into *this starting at
            position at. */
        iterator insert(iterator at, char const * c_str);

#if defined(__cpp_char8_t)
        /** Inserts the sequence of char from c_str into *this starting at
            position at. */
        iterator insert(iterator at, char8_t const * c_str);
#endif

        /** Inserts the sequence of char from t into *this starting at
            position at. */
        iterator insert(iterator at, text const & t);

        /** Inserts the sequence of char from tv into *this starting at
            position at. */
        iterator insert(iterator at, text_view tv);

        /** Inserts the sequence of char from sv into *this starting at
            position at. */
        iterator insert(iterator at, string_view sv);

        /** Inserts the sequence of char from rv into *this starting at
            position at. */
        iterator insert(iterator at, rope_view rv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Inserts the char range r into *this starting at position at.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept. */
        template<typename CharRange>
        iterator insert(iterator at, CharRange const & r);

        /** Inserts the char sequence [first, last) into *this starting at
            position at.

            This function only participates in overload resolution if
            `CharIter` models the CharIter concept. */
        template<typename CharIter, typename Sentinel>
        iterator insert(iterator at, CharIter first, Sentinel last);

#else

        template<typename CharRange>
        auto insert(iterator at, CharRange const & r)
            -> detail::rng_alg_ret_t<iterator, CharRange>;

        template<typename CharIter, typename Sentinel>
        auto insert(iterator at, CharIter first, Sentinel last)
            -> detail::char_iter_ret_t<iterator, CharIter>;

#endif

        /** Inserts the sequence [first, last) into *this starting at position
            at. */
        iterator insert(iterator at, const_iterator first, const_iterator last);

        /** Inserts the grapheme g into *this at position at. */
        iterator insert(iterator at, grapheme const & g);

        /** Inserts the grapheme g into *this at position at. */
        template<typename CPIter>
        iterator insert(iterator at, grapheme_ref<CPIter> g);

        /** Erases the portion of *this delimited by tv.

            \pre !std::less(tv.begin().base().base(), begin().base().base()) &&
            !std::less(end().base().base(), tv.end().base().base()) */
        text & erase(text_view tv) noexcept;

        /** Erases the portion of *this delimited by [first, last).

            \pre first <= last */
        iterator erase(iterator first, iterator last) noexcept;

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        text & replace(text_view old_substr, char const * new_substr);

#if defined(__cpp_char8_t)
        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        text & replace(text_view old_substr, char8_t const * new_substr);
#endif

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        text & replace(text_view old_substr, text const & new_substr);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        text & replace(text_view old_substr, text_view new_substr);

        /** Replaves the  portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        text & replace(text_view old_substr, string_view new_substr);

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of char from new_substr.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        text & replace(text_view old_substr, rope_view new_substr);

#ifdef BOOST_TEXT_DOXYGEN

        /** Replaces the portion of *this delimited by old_substr with the
            char range r.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        template<typename CharRange>
        text & replace(text_view old_substr, CharRange const & r);

        /** Replaces the portion of *this delimited by old_substr with the
            char sequence [first, last).

            This function only participates in overload resolution if
            `CharIter` models the CharIter concept.

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        template<typename CharIter, typename Sentinel>
        text & replace(text_view old_substr, CharIter first, Sentinel last);

#else

        template<typename CharRange>
        auto replace(text_view old_substr, CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

        template<typename CharIter, typename Sentinel>
        auto replace(text_view old_substr, CharIter first, Sentinel last)
            -> detail::char_iter_ret_t<text &, CharIter>;

#endif

        /** Replaces the portion of *this delimited by old_substr with the
            sequence [first, last).

            \pre !std::less(old_substr.begin().base().base(),
            begin().base().base()) && !std::less(end().base().base(),
            old_substr.end().base().base()) */
        text & replace(
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
        void swap(text & rhs) noexcept { str_.swap(rhs.str_); }

        /** Removes and returns the underlying string from *this. */
        std::string extract() && noexcept { return std::move(str_); }

        /** Replaces the underlying string in *this.

            \pre s is normalized FCC. */
        void replace(std::string && s) noexcept { str_ = std::move(s); }

        /** Appends c_str to *this. */
        text & operator+=(char const * c_str);

#if defined(__cpp_char8_t)
        /** Appends c_str to *this. */
        text & operator+=(char8_t const * c_str);
#endif

        /** Appends tv to *this. */
        text & operator+=(string_view sv);

#ifdef BOOST_TEXT_DOXYGEN

        /** Appends the char range r to *this.

            This function only participates in overload resolution if
            `CharRange` models the CharRange concept. */
        template<typename CharRange>
        text & operator+=(CharRange const & r);

#else

        template<typename CharRange>
        auto operator+=(CharRange const & r)
            -> detail::rng_alg_ret_t<text &, CharRange>;

#endif

        /** Stream inserter; performs formatted output, in UTF-8 encoding. */
        friend std::ostream & operator<<(std::ostream & os, text const & t)
        {
            if (os.good()) {
                auto const size = t.distance();
                detail::pad_width_before(os, size);
                os << t.str_;
                if (os.good())
                    detail::pad_width_after(os, size);
            }
            return os;
        }

#ifndef BOOST_TEXT_DOXYGEN

    private:
        static iterator make_iter(char * first, char * it, char * last) noexcept
        {
            return iterator{utf_8_to_32_iterator<char *>{first, first, last},
                            utf_8_to_32_iterator<char *>{first, it, last},
                            utf_8_to_32_iterator<char *>{first, last, last}};
        }

        static const_iterator make_iter(
            char const * first, char const * it, char const * last) noexcept
        {
            return const_iterator{
                utf_8_to_32_iterator<char const *>{first, first, last},
                utf_8_to_32_iterator<char const *>{first, it, last},
                utf_8_to_32_iterator<char const *>{first, last, last}};
        }

        template<typename Iter>
        static stl_interfaces::reverse_iterator<Iter>
        make_reverse_iter(Iter it) noexcept
        {
            return stl_interfaces::reverse_iterator<Iter>{it};
        }

        using mutable_utf32_iter = utf_8_to_32_iterator<char *>;

        mutable_utf32_iter prev_stable_cp(mutable_utf32_iter last) noexcept;
        mutable_utf32_iter next_stable_cp(mutable_utf32_iter first) noexcept;

        // https://www.unicode.org/reports/tr15/#Concatenation
        size_type normalize_subrange(size_type from_near_offset, size_type to_near_offset);

        iterator insert_impl_suffix(size_type lo, size_type hi, bool normalized);

        template<typename CharIter, typename Sentinel>
        iterator insert_impl(
            iterator at,
            CharIter first,
            Sentinel last,
            bool first_last_normalized);
        iterator insert_impl(iterator at, string_view sv, bool sv_normalized);

        template<typename CharIter, typename Sentinel>
        text & replace_impl(
            text_view old_substr,
            CharIter first,
            Sentinel last,
            bool first_last_normalized);
        text & replace_impl(
            text_view old_substr,
            string_view new_substr,
            bool new_substr_normalized);

        template<typename CPIter>
        struct insert_grapheme_ref_impl;

        std::string str_;

        template<typename CPIter>
        friend struct insert_grapheme_ref_impl;

#endif // Doxygen
    };

#ifndef BOOST_TEXT_DOXYGEN

    template<typename CPIter>
    struct text::insert_grapheme_ref_impl
    {
        static text::iterator
        call(text & t, text::iterator at, grapheme_ref<CPIter> g)
        {
            if (g.empty())
                return at;

            std::array<char, 1024> buf;
            auto out =
                boost::text::transcode_to_utf8(g.begin(), g.end(), buf.data());
            return t.insert_impl(
                at, string_view(buf.data(), out - buf.data()), true);
        }
    };

    template<typename Sentinel, typename ErrorHandler>
    struct text::insert_grapheme_ref_impl<
        utf_8_to_32_iterator<char const *, Sentinel, ErrorHandler>>
    {
        static text::iterator call(
            text & t,
            text::iterator at,
            grapheme_ref<
                utf_8_to_32_iterator<char const *, Sentinel, ErrorHandler>> g)
        {
            return t.insert_impl(
                at,
                string_view(
                    g.begin().base(), g.end().base() - g.begin().base()),
                true);
        }
    };

    template<typename Sentinel, typename ErrorHandler>
    struct text::insert_grapheme_ref_impl<
        utf_8_to_32_iterator<char *, Sentinel, ErrorHandler>>
    {
        static text::iterator call(
            text & t,
            text::iterator at,
            grapheme_ref<utf_8_to_32_iterator<char *, Sentinel, ErrorHandler>>
                g)
        {
            return t.insert_impl(
                at,
                string_view(
                    g.begin().base(), g.end().base() - g.begin().base()),
                true);
        }
    };

#endif

    inline text::iterator begin(text & t) noexcept { return t.begin(); }
    inline text::iterator end(text & t) noexcept { return t.end(); }
    inline text::const_iterator begin(text const & t) noexcept
    {
        return t.begin();
    }
    inline text::const_iterator end(text const & t) noexcept { return t.end(); }
    inline text::const_iterator cbegin(text const & t) noexcept
    {
        return t.cbegin();
    }
    inline text::const_iterator cend(text const & t) noexcept
    {
        return t.cend();
    }

    inline text::reverse_iterator rbegin(text & t) noexcept
    {
        return t.rbegin();
    }
    inline text::reverse_iterator rend(text & t) noexcept { return t.rend(); }
    inline text::const_reverse_iterator rbegin(text const & t) noexcept
    {
        return t.rbegin();
    }
    inline text::const_reverse_iterator rend(text const & t) noexcept
    {
        return t.rend();
    }
    inline text::const_reverse_iterator crbegin(text const & t) noexcept
    {
        return t.crbegin();
    }
    inline text::const_reverse_iterator crend(text const & t) noexcept
    {
        return t.crend();
    }

}}

#include <boost/text/text_view.hpp>
#include <boost/text/rope.hpp>
#include <boost/text/normalize.hpp>

namespace boost { namespace text {

    namespace literals {
        /** Creates a text from a char string literal. */
        inline text operator"" _t(char const * str, std::size_t len)
        {
            return text(str, str + len);
        }

#if defined(__cpp_char8_t)
        /** Creates a text from a char8_t string literal. */
        inline text operator"" _t(char8_t const * str, std::size_t len)
        {
            return text(str, str + len);
        }
#endif
    }

#ifndef BOOST_TEXT_DOXYGEN

    inline text::text(iterator first, iterator last) :
        text(text_view(first, last))
    {}

    inline text::text(const_iterator first, const_iterator last) :
        text(text_view(first, last))
    {}

    inline text::text(char const * c_str) : str_(c_str)
    {
        normalize<nf::fcc>(str_);
    }

#if defined(__cpp_char8_t)
    inline text::text(char8_t const * c_str) : str_((char const *)c_str)
    {
        normalize<nf::fcc>(str_);
    }
#endif

    inline text::text(std::string s) : str_(std::move(s))
    {
        normalize<nf::fcc>(str_);
    }

    inline text::text(text_view tv) :
        str_(tv.begin().base().base(), tv.end().base().base())
    {
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();
    }

    inline text::text(string_view sv) : str_(sv.begin(), sv.end())
    {
        normalize<nf::fcc>(str_);
    }

    template<typename CharRange>
    text::text(CharRange const & r, detail::rng_alg_ret_t<int *, CharRange>) :
        str_(detail::make_string(r.begin(), r.end()))
    {
        normalize<nf::fcc>(str_);
    }

    template<typename CharIter, typename Sentinel>
    text::text(
        CharIter first,
        Sentinel last,
        detail::char_iter_ret_t<void *, CharIter>) :
        str_(detail::make_string(first, last))
    {
        normalize<nf::fcc>(str_);
    }

    template<typename GraphemeRange>
    text::text(
        GraphemeRange const & r,
        detail::graph_rng_alg_ret_t<int *, GraphemeRange>) :
        str_(
            detail::make_string(r.begin().base().base(), r.end().base().base()))
    {}

    inline text & text::operator=(char const * c_str)
    {
        str_ = c_str;
        normalize<nf::fcc>(str_);
        return *this;
    }

#if defined(__cpp_char8_t)
    inline text & text::operator=(char8_t const * c_str)
    {
        return *this = (char const *)c_str;
    }
#endif

    inline text & text::operator=(std::string s)
    {
        str_ = std::move(s);
        normalize<nf::fcc>(str_);
        return *this;
    }

    inline text & text::operator=(text_view tv)
    {
        str_.assign(tv.begin().base().base(), tv.end().base().base());
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();
        return *this;
    }

    inline text & text::operator=(string_view sv)
    {
        str_.assign(sv.begin(), sv.end());
        normalize<nf::fcc>(str_);
        return *this;
    }

    template<typename CharRange>
    auto text::operator=(CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        str_.assign(r.begin(), r.end());
        normalize<nf::fcc>(str_);
        return *this;
    }

    template<typename GraphemeRange>
    auto text::operator=(GraphemeRange const & r)
        -> detail::graph_rng_alg_ret_t<text &, GraphemeRange>
    {
        str_.assign(r.begin().base().base(), r.begin().base().base());
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();
        return *this;
    }

    template<typename CharRange>
    auto text::insert(iterator at, CharRange const & r)
        -> detail::rng_alg_ret_t<iterator, CharRange>
    {
        return insert(at, std::begin(r), std::end(r));
    }

    template<typename CharIter, typename Sentinel>
    auto text::insert(iterator at, CharIter first, Sentinel last)
        -> detail::char_iter_ret_t<iterator, CharIter>
    {
        return insert_impl(at, first, last, false);
    }

    inline text::iterator text::insert(iterator at, char const * c_str)
    {
        return insert(at, string_view(c_str));
    }

#if defined(__cpp_char8_t)
    inline text::iterator text::insert(iterator at, char8_t const * c_str)
    {
        return insert(at, string_view((char const *)c_str));
    }
#endif

    inline text::iterator text::insert(iterator at, text const & t)
    {
        return insert_impl(at, string_view(t.str_), true);
    }

    inline text::iterator text::insert(iterator at, text_view tv)
    {
        auto const first = tv.begin().base().base();
        auto const last = tv.end().base().base();
        return insert_impl(at, string_view(first, last - first), true);
    }

    inline text::iterator text::insert(iterator at, string_view sv)
    {
        return insert_impl(at, sv, false);
    }

    inline text & text::erase(text_view tv) noexcept
    {
        auto const at = tv.begin().base().base() - str_.data();
        str_.erase(str_.begin() + at, str_.begin() + at + tv.storage_bytes());
        normalize_subrange(at, at);
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();
        return *this;
    }

    inline text::iterator
    text::insert(iterator at, const_iterator first, const_iterator last)
    {
        return insert(at, text_view(first, last));
    }

    inline text::iterator text::insert(iterator at, grapheme const & g)
    {
        return insert(at, grapheme_ref<grapheme::const_iterator>(g));
    }

    template<typename CPIter>
    text::iterator text::insert(iterator at, grapheme_ref<CPIter> g)
    {
        return insert_grapheme_ref_impl<CPIter>::call(*this, at, g);
    }

    inline text::iterator text::erase(iterator first, iterator last) noexcept
    {
        size_type const offset = first.base().base() - str_.c_str();
        erase(text_view(first, last));
        char * str_first = const_cast<char *>(str_.data());
        return make_iter(
            str_first, str_first + offset, str_first + str_.size());
    }

    inline text & text::replace(text_view old_substr, char const * new_substr)
    {
        return replace_impl(old_substr, string_view(new_substr), false);
    }

#if defined(__cpp_char8_t)
    inline text &
    text::replace(text_view old_substr, char8_t const * new_substr)
    {
        return replace_impl(
            old_substr, string_view((char const *)new_substr), false);
    }
#endif

    inline text & text::replace(text_view old_substr, text const & new_substr)
    {
        return replace_impl(
            old_substr,
            string_view(
                new_substr.begin().base().base(),
                new_substr.end().base().base() -
                    new_substr.begin().base().base()),
            true);
    }

    inline text & text::replace(text_view old_substr, text_view new_substr)
    {
        return replace_impl(
            old_substr,
            string_view(
                new_substr.begin().base().base(),
                new_substr.end().base().base() -
                    new_substr.begin().base().base()),
            true);
    }

    inline text & text::replace(text_view old_substr, string_view new_substr)
    {
        return replace_impl(old_substr, new_substr, false);
    }

    template<typename CharRange>
    auto text::replace(text_view old_substr, CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        return replace(old_substr, std::begin(r), std::end(r));
    }

    template<typename CharIter, typename Sentinel>
    auto text::replace(text_view old_substr, CharIter first, Sentinel last)
        -> detail::char_iter_ret_t<text &, CharIter>
    {
        return replace_impl(old_substr, first, last, false);
    }

    inline text & text::replace(
        text_view old_substr, const_iterator first, const_iterator last)
    {
        return replace(old_substr, text_view(first, last));
    }

    inline text & text::operator+=(char const * c_str)
    {
        return operator+=(string_view(c_str));
    }

#if defined(__cpp_char8_t)
    inline text & text::operator+=(char8_t const * c_str)
    {
        return operator+=(string_view((char const *)c_str));
    }
#endif

    inline text & text::operator+=(string_view sv)
    {
        insert(end(), sv);
        return *this;
    }

    template<typename CharRange>
    auto text::operator+=(CharRange const & r)
        -> detail::rng_alg_ret_t<text &, CharRange>
    {
        insert(this->end(), std::begin(r), std::end(r));
        return *this;
    }

    inline text::mutable_utf32_iter
    text::prev_stable_cp(mutable_utf32_iter last) noexcept
    {
        auto const str_first = const_cast<char *>(str_.data());
        auto const first =
            mutable_utf32_iter(str_first, str_first, str_first + str_.size());
        auto const it =
            find_if_backward(first, last, detail::stable_code_point<nf::fcc>);
        if (it == last)
            return first;
        return it;
    }

    inline text::mutable_utf32_iter
    text::next_stable_cp(mutable_utf32_iter first) noexcept
    {
        auto const str_first = const_cast<char *>(str_.data());
        auto const str_last = str_first + str_.size();
        auto const last = mutable_utf32_iter(str_first, str_last, str_last);
        auto const it =
            find_if(first, last, detail::stable_code_point<nf::fcc>);
        return it;
    }

    inline text::size_type text::normalize_subrange(
        size_type from_near_offset, size_type to_near_offset)
    {
        auto const str_first = const_cast<char *>(str_.data());
        auto const str_last = str_first + str_.size();

        mutable_utf32_iter first(
            str_first, str_first + from_near_offset, str_last);
        mutable_utf32_iter last(
            str_first, str_first + to_near_offset, str_last);
        first = prev_stable_cp(first);
        last = next_stable_cp(last);

        if (first == last)
            return from_near_offset;

        container::small_vector<char, 256> buf;
        normalize<nf::fcc>(first, last, from_utf32_back_inserter(buf));
        auto const initial_cp = *boost::text::utf32_iterator(
            &buf[0], &buf[0], &buf[0] + buf.size());
        auto const prev_initial_cp = *first;

        str_.replace(
            str_.begin() + (first.base() - str_.data()),
            str_.begin() + (last.base() - str_.data()),
            buf.begin(),
            buf.end());

        // Return the new lo offset if the first normalized CP changed.
        return initial_cp == prev_initial_cp ? from_near_offset
                                             : first.base() - str_first;
    }

    inline text::iterator
    text::insert_impl_suffix(size_type lo, size_type hi, bool normalized)
    {
        size_type new_lo = lo;
        if (normalized) {
            if (hi < str_.size())
                normalize_subrange(hi, hi);
            if (lo)
                new_lo = normalize_subrange(lo, lo);
        } else {
            new_lo = normalize_subrange(lo, hi);
        }
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();

        // The insertion that just happened might be merged into the CP or
        // grapheme ending at the offset of the inserted char(s); if so, back
        // up and return an iterator to that.
        auto const str_first = const_cast<char *>(str_.data());
        auto const str_last = str_first + str_.size();
        auto insertion_cp_it =
            mutable_utf32_iter(str_first, str_first + new_lo, str_last);
        auto const first_cp_of_grapheme_it =
            prev_grapheme_break(begin().base(), insertion_cp_it, end().base());

        return make_iter(str_first, first_cp_of_grapheme_it.base(), str_last);
    }

    template<typename CharIter, typename Sentinel>
    text::iterator text::insert_impl(
        iterator at, CharIter first, Sentinel last, bool first_last_normalized)
    {
        size_type const lo = at.base().base() - str_.data();
        auto const insertion_it = str_.insert(
            str_.begin() + (at.base().base() - str_.data()), first, last);
        size_type const hi = insertion_it - str_.begin();
        return insert_impl_suffix(lo, hi, first_last_normalized);
    }

    inline text::iterator
    text::insert_impl(iterator at, string_view sv, bool sv_normalized)
    {
        bool const sv_null_terminated = !sv.empty() && sv.back() == '\0';
        if (sv_null_terminated)
            sv = detail::substring(sv, 0, -1);
        size_type const lo = at.base().base() - str_.data();
        auto const insertion_it = str_.insert(
            str_.begin() + (at.base().base() - str_.data()),
            sv.begin(),
            sv.end());
        size_type const hi = insertion_it - str_.begin();
        return insert_impl_suffix(lo, hi, sv_normalized);
    }

    template<typename CharIter, typename Sentinel>
    text & text::replace_impl(
        text_view old_substr,
        CharIter first,
        Sentinel last,
        bool first_last_normalized)
    {
        size_type const lo = old_substr.begin().base().base() - str_.data();
        size_type const old_size = storage_bytes();
        size_type const old_substr_size = old_substr.storage_bytes();
        str_.replace(
            str_.begin() + (old_substr.begin().base().base() - str_.data()),
            str_.begin() + (old_substr.end().base().base() - str_.data()),
            first,
            last);
        size_type const new_size = storage_bytes();
        size_type const hi = lo + old_substr_size + (new_size - old_size);
        if (first_last_normalized) {
            if (hi < str_.size())
                normalize_subrange(hi, hi);
            if (lo)
                normalize_subrange(lo, lo);
        } else {
            normalize_subrange(lo, hi);
        }
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();
        return *this;
    }

    inline text & text::replace_impl(
        text_view old_substr,
        string_view new_substr,
        bool new_substr_normalized)
    {
        bool const new_substr_null_terminated =
            !new_substr.empty() && new_substr.back() == '\0';
        if (new_substr_null_terminated)
            new_substr = detail::substring(new_substr, 0, -1);

        size_type const lo = old_substr.begin().base().base() - str_.data();
        size_type const hi = lo + old_substr.storage_bytes() +
                             (new_substr.size() - old_substr.storage_bytes());
        str_.replace(
            str_.begin() + (old_substr.begin().base().base() - str_.data()),
            str_.begin() + (old_substr.end().base().base() - str_.data()),
            new_substr.begin(),
            new_substr.end());
        if (new_substr_normalized) {
            if (hi < str_.size())
                normalize_subrange(hi, hi);
            if (lo)
                normalize_subrange(lo, lo);
        } else {
            normalize_subrange(lo, hi);
        }
        BOOST_TEXT_CHECK_TEXT_NORMALIZATION();
        return *this;
    }

#endif // Doxygen

    inline bool
    operator==(text const & lhs, char const * rhs) noexcept = delete;
    inline bool
    operator==(char const * lhs, text const & rhs) noexcept = delete;

    inline bool
    operator!=(text const & lhs, char const * rhs) noexcept = delete;
    inline bool
    operator!=(char const * lhs, text const & rhs) noexcept = delete;

#if defined(__cpp_char8_t)
    inline bool
    operator==(text const & lhs, char8_t const * rhs) noexcept = delete;
    inline bool
    operator==(char8_t const * lhs, text const & rhs) noexcept = delete;

    inline bool
    operator!=(text const & lhs, char8_t const * rhs) noexcept = delete;
    inline bool
    operator!=(char8_t const * lhs, text const & rhs) noexcept = delete;
#endif

    inline bool operator==(text const & lhs, text_view rhs) noexcept
    {
        return algorithm::equal(
            lhs.begin().base().base(),
            lhs.end().base().base(),
            rhs.begin().base().base(),
            rhs.end().base().base());
    }
    inline bool operator==(text_view lhs, text const & rhs) noexcept
    {
        return rhs == lhs;
    }

    inline bool operator!=(text const & lhs, text_view rhs) noexcept
    {
        return !(lhs == rhs);
    }
    inline bool operator!=(text_view lhs, text const & rhs) noexcept
    {
        return !(lhs == rhs);
    }

    inline bool operator==(text const & lhs, text const & rhs) noexcept
    {
        return algorithm::equal(
            lhs.begin().base().base(),
            lhs.end().base().base(),
            rhs.begin().base().base(),
            rhs.end().base().base());
    }

    inline bool operator!=(text const & lhs, text const & rhs) noexcept
    {
        return !(lhs == rhs);
    }

    /** Creates a new text object that is the concatenation of t and t2. */
    inline text operator+(text t, text const & t2)
    {
        t.insert(t.end(), t2);
        return t;
    }

    /** Creates a new text object that is the concatenation of t and c_str. */
    inline text operator+(text t, char const * c_str) { return t += c_str; }

    /** Creates a new text object that is the concatenation of c_str and t. */
    inline text operator+(char const * c_str, text const & t)
    {
        return text(c_str) + t;
    }

#if defined(__cpp_char8_t)
    /** Creates a new text object that is the concatenation of t and c_str. */
    inline text operator+(text t, char8_t const * c_str)
    {
        return t += (char const *)c_str;
    }

    /** Creates a new text object that is the concatenation of c_str and t. */
    inline text operator+(char8_t const * c_str, text const & t)
    {
        return text((char const *)c_str) + t;
    }
#endif

    /** Creates a new text object that is the concatenation of t and tv. */
    inline text operator+(text t, text_view tv)
    {
        t.insert(t.end(), tv);
        return t;
    }

    /** Creates a new text object that is the concatenation of tv and t. */
    inline text operator+(text_view tv, text const & t) { return text(tv) + t; }

#ifdef BOOST_TEXT_DOXYGEN

    /** Creates a new text object that is the concatenation of t and r.

        This function only participates in overload resolution if `CharRange`
        models the CharRange concept. */
    template<typename CharRange>
    text operator+(text t, CharRange const & r);

    /** Creates a new text object that is the concatenation of r and t.

        This function only participates in overload resolution if `CharRange`
        models the CharRange concept. */
    template<typename CharRange>
    text operator+(CharRange const & r, text const & t);

#else

    template<typename CharRange>
    auto operator+(text t, CharRange const & r)
        -> detail::rng_alg_ret_t<text, CharRange>
    {
        return t += r;
    }

    template<typename CharRange>
    auto operator+(CharRange const & r, text const & t)
        -> detail::rng_alg_ret_t<text, CharRange>
    {
        return text(r) + t;
    }

#endif

}}

#ifndef BOOST_TEXT_DOXYGEN

namespace std {
    template<>
    struct hash<boost::text::text>
    {
        using argument_type = boost::text::text;
        using result_type = std::size_t;
        result_type operator()(argument_type const & t) const noexcept
        {
            return boost::text::detail::hash_grapheme_range(t);
        }
    };
}

#endif

#endif
