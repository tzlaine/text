#ifndef BOOST_TEXT_GRAPHEME_HPP
#define BOOST_TEXT_GRAPHEME_HPP

#include <boost/text/grapheme_break.hpp>
#include <boost/text/normalize.hpp>
#include <boost/text/transcode_iterator.hpp>
#include <boost/text/utility.hpp>

#include <boost/algorithm/cxx14/equal.hpp>
#include <boost/container/small_vector.hpp>

#include <ostream>


namespace boost { namespace text {

    struct grapheme;
    template<typename CPIter>
    struct grapheme_view;
    int storage_bytes(grapheme const & g) noexcept;

    /** An owning sequence of code points that comprise a grapheme. */
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
            transcode_utf_32_to_8(first, last, std::back_inserter(chars_));
            BOOST_ASSERT(next_grapheme_break(begin(), end()) == end());
            BOOST_ASSERT(fcd_form(begin(), end()));
        }

        /** Constructs *this from the code point cp. */
        grapheme(uint32_t cp)
        {
            uint32_t cps[1] = {cp};
            transcode_utf_32_to_8(cps, cps + 1, std::back_inserter(chars_));
        }

        /** Constructs *this from r.

            \pre The code points in r comprise at most one grapheme.
            \pre The code points in [first, last) comprise at most one
            grapheme.
            \pre [first, last) is normalized FCC. */
        template<typename CPIter>
        grapheme(cp_range<CPIter> r)
        {
            transcode_utf_32_to_8(
                r.begin(), r.end(), std::back_inserter(chars_));
            BOOST_ASSERT(next_grapheme_break(begin(), end()) == end());
            BOOST_ASSERT(fcd_form(begin(), end()));
        }

        /** Constructs *this from g. */
        template<typename CPIter>
        grapheme(grapheme_view<CPIter> g)
        {
            transcode_utf_32_to_8(
                g.begin(), g.end(), std::back_inserter(chars_));
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

        /** Returns the number of bytes controlled by g. */
        friend int storage_bytes(grapheme const & g) noexcept
        {
            return g.chars_.size();
        }

    private:
        container::small_vector<char, 8> chars_;
    };


    /** A non-owning view of a range of code points that comprise a
        grapheme. */
    template<typename CPIter>
    struct grapheme_view
    {
        using iterator = CPIter;

        /** Default ctor. */
        grapheme_view() noexcept : first_(), last_() {}

        /** Constructs *this from the code points [first, last).

            \pre The code points in [first, last) comprise at most one
            grapheme.
            \pre [first, last) is normalized FCC. */
        grapheme_view(CPIter first, CPIter last) noexcept :
            first_(first),
            last_(last)
        {
            BOOST_ASSERT(next_grapheme_break(first_, last_) == last_);
            BOOST_ASSERT(fcd_form(first_, last_));
        }

        /** Constructs *this from r.

            \pre The code points in r comprise at most one grapheme.
            \pre The code points in r are normalized FCC. */
        grapheme_view(cp_range<CPIter> r) noexcept :
            first_(r.begin()),
            last_(r.end())
        {
            BOOST_ASSERT(next_grapheme_break(first_, last_) == last_);
            BOOST_ASSERT(fcd_form(first_, last_));
        }

        /** Constructs *this from g. */
        grapheme_view(grapheme const & g) noexcept :
            first_(g.begin()),
            last_(g.end())
        {}

        /** Returns true of *this contains no code points. */
        bool empty() const noexcept { return first_ == last_; }

        iterator begin() const noexcept { return first_; }
        iterator end() const noexcept { return last_; }

        /** Stream inserter; performs unformatted output, in UTF-8 encoding. */
        friend std::ostream & operator<<(std::ostream & os, grapheme_view gv)
        {
            std::array<char, 1024> buf;
            auto out = transcode_utf_32_to_8(gv.begin(), gv.end(), buf.data());
            os.write(buf.data(), out - buf.data());
            return os;
        }

    private:
        iterator first_;
        iterator last_;
    };

    /** Returns the number of bytes g refers to. */
    template<typename CPIter>
    int storage_bytes(grapheme_view<CPIter> g) noexcept
    {
        return std::distance(g.begin().base(), g.end().base());
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter1, typename CPIter2>
    bool operator==(grapheme_view<CPIter1> lhs, grapheme_view<CPIter2> rhs)
    {
        return algorithm::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter1, typename CPIter2>
    bool operator!=(grapheme_view<CPIter1> lhs, grapheme_view<CPIter2> rhs)
    {
        return !(lhs == rhs);
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter1, typename CPIter2>
    bool operator==(cp_range<CPIter1> lhs, grapheme_view<CPIter2> rhs)
    {
        return algorithm::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter1, typename CPIter2>
    bool operator==(grapheme_view<CPIter1> lhs, cp_range<CPIter2> rhs)
    {
        return rhs == lhs;
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter1, typename CPIter2>
    bool operator!=(cp_range<CPIter1> lhs, cp_range<CPIter2> rhs)
    {
        return !(lhs == rhs);
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter1, typename CPIter2>
    bool operator!=(grapheme_view<CPIter1> lhs, cp_range<CPIter2> rhs)
    {
        return !(lhs == rhs);
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter>
    bool operator==(grapheme const & lhs, grapheme_view<CPIter> rhs)
    {
        return algorithm::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter>
    bool operator==(grapheme_view<CPIter> lhs, grapheme const & rhs)
    {
        return rhs == lhs;
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter>
    bool operator!=(grapheme const & lhs, grapheme_view<CPIter> rhs)
    {
        return !(lhs == rhs);
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter>
    bool operator!=(grapheme_view<CPIter> rhs, grapheme const & lhs)
    {
        return !(lhs == rhs);
    }

}}

#endif
