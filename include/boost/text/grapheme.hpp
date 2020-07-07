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


namespace boost { namespace text { inline namespace v1 {

    struct grapheme;
    template<typename CPIter>
    struct grapheme_ref;

    /** Returns the number of bytes controlled by g. */
    int storage_bytes(grapheme const & g) noexcept;

    /** An owning sequence of code points that comprise a grapheme in
        FCC-normalized text. */
    struct grapheme
    {
        using const_iterator = utf_8_to_32_iterator<char const *>;

        /** Default ctor. */
        grapheme() {}

        /** Constructs *this from the code points [first, last).

            \pre The code points in [first, last) comprise at most one
            grapheme.
            \pre [first, last) is normalized FCC. */
        template<typename CPIter>
        grapheme(CPIter first, CPIter last)
        {
            boost::text::v1::transcode_utf_32_to_8(
                first, last, std::back_inserter(chars_));
            BOOST_ASSERT(
                boost::text::v1::next_grapheme_break(begin(), end()) == end());
            BOOST_ASSERT(boost::text::v1::normalized<nf::fcc>(begin(), end()));
        }

        /** Constructs *this from the code point cp. */
        grapheme(uint32_t cp)
        {
            uint32_t cps[1] = {cp};
            boost::text::v1::transcode_utf_32_to_8(
                cps, cps + 1, std::back_inserter(chars_));
        }

        /** Constructs *this from r.

            \pre The code points in r comprise at most one grapheme.
            \pre The code points in [first, last) comprise at most one
            grapheme.
            \pre [first, last) is normalized FCC. */
        template<typename CPIter>
        grapheme(utf32_view<CPIter> r)
        {
            boost::text::v1::transcode_utf_32_to_8(
                r.begin(), r.end(), std::back_inserter(chars_));
            BOOST_ASSERT(
                boost::text::v1::next_grapheme_break(begin(), end()) == end());
            BOOST_ASSERT(boost::text::v1::normalized<nf::fcc>(begin(), end()));
        }

        /** Returns true if *this contains no code points. */
        bool empty() const noexcept { return chars_.empty(); }

        /** Returns the number of code points contained in *this.  This is an
            O(N) operation. */
        int distance() const noexcept { return std::distance(begin(), end()); }

        const_iterator begin() const noexcept
        {
            auto const first = &*chars_.begin();
            auto const last = first + chars_.size();
            return grapheme::const_iterator{first, first, last};
        }

        const_iterator end() const noexcept
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

        bool operator==(grapheme const & other) const noexcept
        {
            return chars_ == other.chars_;
        }
        bool operator!=(grapheme const & other) const noexcept
        {
            return chars_ != other.chars_;
        }

        friend int storage_bytes(grapheme const & g) noexcept
        {
            return g.chars_.size();
        }

    private:
        container::small_vector<char, 8> chars_;
    };


    /** A non-owning reference to a range of code points that comprise a
        grapheme. */
    template<typename CPIter>
    struct grapheme_ref : utf32_view<CPIter>
    {
        using iterator = CPIter;

        /** Default ctor. */
        constexpr grapheme_ref() noexcept = default;

        /** Constructs *this from the code points [first, last).

            \pre The code points in [first, last) comprise at most one
            grapheme. */
        constexpr grapheme_ref(CPIter first, CPIter last) noexcept :
            utf32_view<CPIter>(first, last)
        {
            BOOST_ASSERT(
                boost::text::v1::next_grapheme_break(first, last) == last);
        }

        /** Constructs *this from r.

            \pre The code points in r comprise at most one grapheme. */
        constexpr grapheme_ref(utf32_view<CPIter> r) noexcept :
            grapheme_ref(r.begin(), r.end())
        {}

        /** Constructs *this from g. */
        constexpr grapheme_ref(grapheme const & g) noexcept :
            utf32_view<CPIter>(g.begin(), g.end())
        {}

        /** Returns true if lhs the same sequence of code points as rhs. */
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator==(grapheme_ref lhs, grapheme_ref rhs) noexcept
        {
            return algorithm::equal(
                lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
        }
        friend BOOST_TEXT_CXX14_CONSTEXPR bool
        operator!=(grapheme_ref lhs, grapheme_ref rhs) noexcept
        {
            return !(lhs == rhs);
        }
    };

    /** Returns the number of bytes g refers to. */
    template<typename CPIter>
    int storage_bytes(grapheme_ref<CPIter> g) noexcept
    {
        return std::distance(g.begin().base(), g.end().base());
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter1, typename CPIter2>
    constexpr bool
    operator==(grapheme_ref<CPIter1> lhs, grapheme_ref<CPIter2> rhs) noexcept
    {
        return algorithm::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter1, typename CPIter2>
    constexpr bool
    operator!=(grapheme_ref<CPIter1> lhs, grapheme_ref<CPIter2> rhs) noexcept
    {
        return !(lhs == rhs);
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter>
    constexpr bool
    operator==(grapheme const & lhs, grapheme_ref<CPIter> rhs) noexcept
    {
        return algorithm::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter>
    constexpr bool
    operator==(grapheme_ref<CPIter> lhs, grapheme const & rhs) noexcept
    {
        return rhs == lhs;
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter>
    constexpr bool
    operator!=(grapheme const & lhs, grapheme_ref<CPIter> rhs) noexcept
    {
        return !(lhs == rhs);
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter>
    constexpr bool
    operator!=(grapheme_ref<CPIter> rhs, grapheme const & lhs) noexcept
    {
        return !(rhs == lhs);
    }

}}}

#endif
