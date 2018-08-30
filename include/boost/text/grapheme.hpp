#ifndef BOOST_TEXT_GRAPHEME_HPP
#define BOOST_TEXT_GRAPHEME_HPP

#include <boost/text/utility.hpp>

#include <boost/algorithm/cxx14/equal.hpp>
#include <boost/container/small_vector.hpp>


namespace boost { namespace text {

    namespace detail {
        using grapheme_storage_t = container::small_vector<uint32_t, 4>;
    }

    template<typename CPIter = detail::grapheme_storage_t::const_iterator>
    struct grapheme_view;

    /** An owning sequence of code points that comprise a grapheme. */
    struct grapheme
    {
        using storage_t = detail::grapheme_storage_t;
        using iterator = storage_t::iterator;
        using const_iterator = storage_t::const_iterator;

        /** Default ctor. */
        grapheme() {}

        /** Constructs *this from the code points [first, last).

            \pre The code points in [first, last) comprise at most one
            grapheme. */
        template<typename CPIter>
        grapheme(CPIter first, CPIter last) : cps_(first, last)
        {}

        /** Constructs *this from r.

            \pre The code points in r comprise at most one grapheme. */
        template<typename CPIter>
        grapheme(cp_range<CPIter> r) : cps_(r.begin(), r.end())
        {}

        /** Constructs *this from g. */
        template<typename CPIter>
        grapheme(grapheme_view<CPIter> g) : cps_(g.begin(), g.end())
        {}

        /** Returns true of *this contains no code points. */
        bool empty() const noexcept { return cps_.empty(); }

        /** Returns the number of code points contained in *this. */
        int size() const noexcept { return cps_.size(); }

        const_iterator begin() const noexcept { return cps_.begin(); }
        const_iterator end() const noexcept { return cps_.end(); }

        iterator begin() noexcept { return cps_.begin(); }
        iterator end() noexcept { return cps_.end(); }

    private:
        container::small_vector<uint32_t, 4> cps_;
    };

    /** A non-owning view of a range of code points that conprise a
        grapheme. */
    template<typename CPIter>
    struct grapheme_view
    {
        using iterator = CPIter;

        /** Default ctor. */
        grapheme_view() noexcept : first_(), last_() {}

        /** Constructs *this from the code points [first, last).

            \pre The code points in [first, last) comprise at most one
            grapheme. */
        grapheme_view(CPIter first, CPIter last) noexcept :
            first_(first),
            last_(last)
        {}

        /** Constructs *this from r.

            \pre The code points in r comprise at most one grapheme. */
        grapheme_view(cp_range<CPIter> r) noexcept :
            first_(r.begin()),
            last_(r.end())
        {}

        /** Constructs *this from g. */
        grapheme_view(struct grapheme const & g) noexcept :
            first_(g.begin()),
            last_(g.end())
        {}

        /** Returns true of *this contains no code points. */
        bool empty() const noexcept { return first_ == last_; }

        iterator begin() const noexcept { return first_; }
        iterator end() const noexcept { return last_; }

    private:
        iterator first_;
        iterator last_;
    };

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
    bool operator==(struct grapheme const & lhs, grapheme_view<CPIter> rhs)
    {
        return algorithm::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter>
    bool operator==(grapheme_view<CPIter> lhs, struct grapheme const & rhs)
    {
        return rhs == lhs;
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter>
    bool operator!=(struct grapheme const & lhs, grapheme_view<CPIter> rhs)
    {
        return !(lhs == rhs);
    }

    /** Returns true if lhs the same sequence of code points as rhs. */
    template<typename CPIter>
    bool operator!=(grapheme_view<CPIter> rhs, struct grapheme const & lhs)
    {
        return !(lhs == rhs);
    }

}}

#endif
