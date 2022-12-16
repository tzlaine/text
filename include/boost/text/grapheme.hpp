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
#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I>
#else
    template<typename I>
#endif
    struct grapheme_ref;

    /** Returns the number of code_units controlled by g. */
    int storage_code_units(grapheme const & g);

    /** An owning sequence of code points that comprise an extended grapheme
        cluster. */
    struct grapheme
    {
        using iterator = utf_8_to_32_iterator<char const *>;
        using const_iterator = iterator;

        /** Default ctor. */
        grapheme() {}

        /** Constructs *this from the code points [first, last).

            \pre The code points in [first, last) comprise at most one
            grapheme. */
#if BOOST_TEXT_USE_CONCEPTS
        template<utf_iter I, std::sentinel_for<I> S>
#else
        template<typename I, typename S>
#endif
        grapheme(I first, S last)
        {
            boost::text::transcode_to_utf8(
                first, last, std::back_inserter(chars_));
            BOOST_ASSERT(
                boost::text::next_grapheme_break(begin(), end()) == end());
        }

        /** Constructs *this from the code point cp. */
        grapheme(uint32_t cp)
        {
            uint32_t cps[1] = {cp};
            boost::text::transcode_to_utf8(
                cps, cps + 1, std::back_inserter(chars_));
        }

        /** Constructs *this from r.

            \pre The code points in r comprise at most one grapheme. */
#if BOOST_TEXT_USE_CONCEPTS
        template<utf_range_like R>
#else
        template<typename R>
#endif
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
            auto const first = &*chars_.begin();
            auto const last = first + chars_.size();
            return grapheme::const_iterator{first, first, last};
        }

        const_iterator end() const
        {
            auto const first = &*chars_.begin();
            auto const last = first + chars_.size();
            return grapheme::const_iterator{first, last, last};
        }

        /** Stream inserter; performs unformatted output, in UTF-8 encoding. */
        friend std::ostream & operator<<(std::ostream & os, grapheme const & g)
        {
            return os.write(
                g.begin().base(), g.end().base() - g.begin().base());
        }

        bool operator==(grapheme const & other) const
        {
            return chars_ == other.chars_;
        }
        bool operator!=(grapheme const & other) const
        {
            return chars_ != other.chars_;
        }

        friend int storage_code_units(grapheme const & g)
        {
            return g.chars_.size();
        }

    private:
        container::small_vector<char, 8> chars_;
    };


    /** A non-owning reference to a range of code points that comprise a
        grapheme. */
#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I>
#else
    template<typename I>
#endif
    struct grapheme_ref : utf32_view<I>
    {
        /** Default ctor. */
        constexpr grapheme_ref() = default;

        /** Constructs *this from the code points [first, last).

            \pre The code points in [first, last) comprise at most one
            grapheme. */
        constexpr grapheme_ref(I first, I last) : utf32_view<I>(first, last)
        {
            BOOST_ASSERT(boost::text::next_grapheme_break(first, last) == last);
        }

        /** Constructs *this from r.

            \pre The code points in r comprise at most one grapheme. */
        constexpr grapheme_ref(utf32_view<I> r) :
            grapheme_ref(r.begin(), r.end())
        {}

        /** Constructs *this from g. */
        constexpr grapheme_ref(grapheme const & g)
#if BOOST_TEXT_USE_CONCEPTS
            requires std::same_as<I, grapheme::iterator>
#endif
            : utf32_view<I>(g.begin(), g.end())
        {}

        /** Returns true if lhs the same sequence of code points as rhs. */
        friend constexpr bool operator==(grapheme_ref lhs, grapheme_ref rhs)
        {
            return algorithm::equal(
                lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }
        friend constexpr bool operator!=(grapheme_ref lhs, grapheme_ref rhs)
        {
            return !(lhs == rhs);
        }
    };

#if defined(__cpp_deduction_guides)
#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I>
#else
    template<typename I>
#endif
    grapheme_ref(I, I) -> grapheme_ref<I>;

#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I>
#else
    template<typename I>
#endif
    grapheme_ref(utf32_view<I>) -> grapheme_ref<I>;

#if !defined(_MSC_VER)
    grapheme_ref(grapheme) -> grapheme_ref<grapheme::const_iterator>;
#endif
#endif

    /** Returns the number of bytes g refers to. */
#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I>
#else
    template<typename I>
#endif
    int storage_code_units(grapheme_ref<I> g)
    {
        return std::distance(g.begin().base(), g.end().base());
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I1, code_point_iter I2>
#else
    template<typename I1, typename I2>
#endif
    constexpr bool
    operator==(grapheme_ref<I1> lhs, grapheme_ref<I2> rhs)
    {
        return algorithm::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I1, code_point_iter I2>
#else
    template<typename I1, typename I2>
#endif
    constexpr bool
    operator!=(grapheme_ref<I1> lhs, grapheme_ref<I2> rhs)
    {
        return !(lhs == rhs);
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I>
#else
    template<typename I>
#endif
    constexpr bool
    operator==(grapheme const & lhs, grapheme_ref<I> rhs)
    {
        return algorithm::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I>
#else
    template<typename I>
#endif
    constexpr bool
    operator==(grapheme_ref<I> lhs, grapheme const & rhs)
    {
        return rhs == lhs;
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I>
#else
    template<typename I>
#endif
    constexpr bool
    operator!=(grapheme const & lhs, grapheme_ref<I> rhs)
    {
        return !(lhs == rhs);
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I>
#else
    template<typename I>
#endif
    constexpr bool
    operator!=(grapheme_ref<I> rhs, grapheme const & lhs)
    {
        return !(rhs == lhs);
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
