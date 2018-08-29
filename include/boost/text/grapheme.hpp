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

    /** TODO */
    struct grapheme
    {
        using storage_t = detail::grapheme_storage_t;
        using iterator = storage_t::iterator;
        using const_iterator = storage_t::const_iterator;

        grapheme() {}

        template<typename CPIter>
        grapheme(CPIter first, CPIter last) : cps_(first, last)
        {}

        template<typename CPIter>
        grapheme(cp_range<CPIter> r) : cps_(r.begin(), r.end())
        {}

        template<typename CPIter>
        grapheme(grapheme_view<CPIter> g) : cps_(g.begin(), g.end())
        {}

        bool empty() const noexcept { return cps_.empty(); }
        int size() const noexcept { return cps_.size(); }

        const_iterator begin() const noexcept { return cps_.begin(); }
        const_iterator end() const noexcept { return cps_.end(); }

        iterator begin() noexcept { return cps_.begin(); }
        iterator end() noexcept { return cps_.end(); }

    private:
        container::small_vector<uint32_t, 4> cps_;
    };

    /** TODO */
    template<typename CPIter>
    struct grapheme_view
    {
        using iterator = CPIter;

        grapheme_view() noexcept : first_(), last_() {}
        grapheme_view(CPIter first, CPIter last) noexcept :
            first_(first),
            last_(last)
        {}
        grapheme_view(cp_range<CPIter> r) noexcept :
            first_(r.begin()),
            last_(r.end())
        {}
        grapheme_view(struct grapheme const & g) noexcept :
            first_(g.begin()),
            last_(g.end())
        {}

        bool empty() const noexcept { return first_ == last_; }
        iterator begin() const noexcept { return first_; }
        iterator end() const noexcept { return last_; }

    private:
        iterator first_;
        iterator last_;
    };

    template<typename CPIter1, typename CPIter2>
    bool operator==(grapheme_view<CPIter1> lhs, grapheme_view<CPIter2> rhs)
    {
        return algorithm::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    template<typename CPIter1, typename CPIter2>
    bool operator!=(grapheme_view<CPIter1> lhs, grapheme_view<CPIter2> rhs)
    {
        return !(lhs == rhs);
    }

    template<typename CPIter1, typename CPIter2>
    bool operator==(cp_range<CPIter1> lhs, grapheme_view<CPIter2> rhs)
    {
        return algorithm::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    template<typename CPIter1, typename CPIter2>
    bool operator==(grapheme_view<CPIter1> lhs, cp_range<CPIter2> rhs)
    {
        return rhs == lhs;
    }

    template<typename CPIter1, typename CPIter2>
    bool operator!=(cp_range<CPIter1> lhs, cp_range<CPIter2> rhs)
    {
        return !(lhs == rhs);
    }

    template<typename CPIter1, typename CPIter2>
    bool operator!=(grapheme_view<CPIter1> lhs, cp_range<CPIter2> rhs)
    {
        return !(lhs == rhs);
    }

    template<typename CPIter>
    bool operator==(struct grapheme const & lhs, grapheme_view<CPIter> rhs)
    {
        return algorithm::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    template<typename CPIter>
    bool operator==(grapheme_view<CPIter> lhs, struct grapheme const & rhs)
    {
        return rhs == lhs;
    }

    template<typename CPIter>
    bool operator!=(struct grapheme const & lhs, grapheme_view<CPIter> rhs)
    {
        return !(lhs == rhs);
    }

    template<typename CPIter>
    bool operator!=(grapheme_view<CPIter> rhs, struct grapheme const & lhs)
    {
        return !(lhs == rhs);
    }

}}

#endif
