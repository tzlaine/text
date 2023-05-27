// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_GRAPHEME_HPP
#define BOOST_TEXT_GRAPHEME_HPP

#include <boost/text/grapheme_break.hpp>
#include <boost/text/normalize.hpp>
#include <boost/text/transcode_iterator.hpp>

#include <boost/algorithm/cxx14/equal.hpp>
#include <boost/container/small_vector.hpp>

#include <ostream>


namespace boost { namespace text {

    struct grapheme;
    template<code_point_iter I>
    struct grapheme_ref;

    /** An owning sequence of code points that comprise an extended grapheme
        cluster. */
    struct grapheme
    {
        using iterator = utf_8_to_32_iterator<char8_t const *>;
        using const_iterator = iterator;

        /** Default ctor. */
        grapheme() {}

        /** Constructs *this from the code points [first, last).

            \pre The code points in [first, last) comprise at most one
            grapheme. */
        template<utf_iter I, std::sentinel_for<I> S>
        grapheme(I first, S last)
        {
            boost::text::transcode_to_utf8(
                first, last, std::back_inserter(chars_));
            BOOST_ASSERT(
                boost::text::next_grapheme_break(begin(), end()) == end());
        }

        /** Constructs *this from the code point cp. */
        grapheme(char32_t cp)
        {
            char32_t cps[1] = {cp};
            boost::text::transcode_to_utf8(
                cps, cps + 1, std::back_inserter(chars_));
        }

        /** Constructs *this from r.

            \pre The code points in r comprise at most one grapheme. */
        template<utf_range_like R>
        grapheme(R && r)
        {
            if constexpr (std::is_pointer_v<std::remove_reference_t<R>>) {
                boost::text::transcode_to_utf8(
                    r, null_sentinel, std::back_inserter(chars_));
            } else {
                boost::text::transcode_to_utf8(
                    std::ranges::begin(r),
                    std::ranges::end(r),
                    std::back_inserter(chars_));
            }
            BOOST_ASSERT(
                boost::text::next_grapheme_break(begin(), end()) == end());
        }

        /** Returns true if *this contains no code points. */
        bool empty() const { return chars_.empty(); }

        /** Returns the number of code points contained in *this.  This is an
            O(N) operation. */
        int distance() const { return std::distance(begin(), end()); }

        const_iterator begin() const
        {
            auto const first = chars_.data();
            auto const last = first + chars_.size();
            return grapheme::const_iterator{first, first, last};
        }

        const_iterator end() const
        {
            auto const first = chars_.data();
            auto const last = first + chars_.size();
            return grapheme::const_iterator{first, last, last};
        }

        /** Stream inserter; performs unformatted output, in UTF-8 encoding. */
        friend std::ostream & operator<<(std::ostream & os, grapheme const & g)
        {
            return os << (g.chars_ | as_utf8);
        }

        bool operator==(grapheme const & other) const
        {
            return chars_ == other.chars_;
        }

        friend int storage_code_units(grapheme const & g)
        {
            return g.chars_.size();
        }

    private:
        container::small_vector<char8_t, 8> chars_;
    };


    /** A non-owning reference to a range of code points that comprise a
        grapheme. */
    template<code_point_iter I>
    struct grapheme_ref : utf32_view<std::ranges::subrange<I>>
    {
        /** Default ctor. */
        constexpr grapheme_ref() = default;

        /** Constructs *this from the code points [first, last).

            \pre The code points in [first, last) comprise at most one
            grapheme. */
        constexpr grapheme_ref(I first, I last) :
            utf32_view<std::ranges::subrange<I>>(
                std::ranges::subrange(first, last))
        {
            BOOST_ASSERT(boost::text::next_grapheme_break(first, last) == last);
        }

        /** Constructs *this from r.

            \pre The code points in r comprise at most one grapheme. */
        template<std::ranges::view R>
        // clang-format off
            requires std::same_as<std::ranges::iterator_t<R>, I> && utf_range<R> && std::ranges::common_range<R>
        // clang-format on
        constexpr grapheme_ref(R && r) :
             utf32_view<std::ranges::subrange<I>>(
                 std::ranges::subrange(std::ranges::begin(r), std::ranges::end(r)))
        {
            BOOST_ASSERT(
                boost::text::next_grapheme_break(
                    std::ranges::begin(r), std::ranges::end(r)) ==
                std::ranges::end(r));
        }

        /** Constructs *this from g. */
        constexpr grapheme_ref(grapheme const & g)
            requires std::same_as<grapheme::iterator, I>
            : utf32_view<std::ranges::subrange<I>>(g)
        {}
    };

#if defined(__cpp_deduction_guides)
    template<class I>
    grapheme_ref(I, I) -> grapheme_ref<I>;

    template<class R>
    grapheme_ref(R &&) -> grapheme_ref<std::ranges::iterator_t<R>>;

#if !defined(_MSC_VER)
    grapheme_ref(grapheme) -> grapheme_ref<grapheme::const_iterator>;
#endif
#endif

    /** Returns the number of bytes g refers to. */
    template<code_point_iter I>
    int storage_code_units(grapheme_ref<I> g)
    {
        return std::distance(g.begin().base(), g.end().base());
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<class I1, class I2>
    constexpr bool operator==(grapheme_ref<I1> lhs, grapheme_ref<I2> rhs)
    {
        return algorithm::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

}}

#if BOOST_TEXT_USE_CONCEPTS

namespace std::ranges {
    template<boost::text::code_point_iter I>
    inline constexpr bool enable_borrowed_range<boost::text::grapheme_ref<I>> =
        true;
}

#endif

#endif
