// Copyright (C) 2022 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_DETAIL_BREAKS_IMPL_HPP
#define BOOST_TEXT_DETAIL_BREAKS_IMPL_HPP

#include <boost/text/break_view.hpp>
#include <boost/text/detail/algorithm.hpp>


namespace boost { namespace text { namespace detail {

    template<
        typename GraphemeRange,
#if BOOST_TEXT_USE_CONCEPTS
        bool UseSentinel = cp_sentinel_gr_rng<GraphemeRange>
#else
        bool UseSentinel = is_cp_sentinel_gr_rng_v<GraphemeRange>
#endif
        >
    struct gr_rng_cp_last
    {
        static auto call(GraphemeRange && r) { return r.end(); }
    };

    template<typename GraphemeRange>
    struct gr_rng_cp_last<GraphemeRange, false>
    {
        static auto call(GraphemeRange && r) { return r.end().base(); }
    };

    template<
        template<class>
        class PrevFunc,
        template<class, class>
        class NextFunc,
        typename CPIter,
        typename Sentinel>
    break_view<CPIter, Sentinel, PrevFunc<CPIter>, NextFunc<CPIter, Sentinel>>
    breaks_impl(CPIter first, Sentinel last) noexcept
    {
        return {first, last, {}, {}};
    }

    template<
        template<class>
        class PrevFunc,
        template<class, class>
        class NextFunc,
        typename CPRange>
    break_view<
        iterator_t<CPRange>,
        sentinel_t<CPRange>,
        PrevFunc<iterator_t<CPRange>>,
        NextFunc<iterator_t<CPRange>, sentinel_t<CPRange>>>
    breaks_cr_impl(CPRange && range) noexcept
    {
        return {detail::begin(range), detail::end(range), {}, {}};
    }

    template<
        template<class>
        class PrevFunc,
        template<class, class>
        class NextFunc,
        typename GraphemeRange>
    break_view<
        gr_rng_cp_iter_t<GraphemeRange>,
        gr_rng_cp_sent_t<GraphemeRange>,
        PrevFunc<gr_rng_cp_iter_t<GraphemeRange>>,
        NextFunc<
            gr_rng_cp_iter_t<GraphemeRange>,
            gr_rng_cp_sent_t<GraphemeRange>>,
        grapheme_view<gr_rng_cp_iter_t<GraphemeRange>>>
    breaks_gr_impl(GraphemeRange && range) noexcept
    {
        return {
            range.begin().base(),
            gr_rng_cp_last<GraphemeRange>::call(range),
            {},
            {}};
    }

}}}

#endif
