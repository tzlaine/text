#ifndef BOOST_TEXT_GRAPHEME_HPP
#define BOOST_TEXT_GRAPHEME_HPP

#include <boost/text/utility.hpp>

#include <boost/container/small_vector.hpp>


namespace boost { namespace text {

    template<typename CPIter = grapheme::const_iterator>
    struct grapheme_view;

    struct grapheme
    {
        using storage_t = container::small_vector<uint32_t, 4>;
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
        grapheme_view(grapheme const & g) noexcept :
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

    // TODO: operator== between grapheme/grapheme_view/cp_range and
    // grapheme/grapheme_view/cp_range, with different CPIter params

    // TODO: Tests with the standard algorithms and Boost.StringAlgo

}}

#endif
