// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_TEST_BIDI_TESTS_HPP
#define BOOST_TEXT_TEST_BIDI_TESTS_HPP

#include <boost/text/bidirectional.hpp>

#include <iostream>


template<typename CPIter>
std::vector<int>
bidi_levels(CPIter first, CPIter last, int paragraph_embedding_level = -1)
{
    using namespace boost::text;

    std::vector<int> retval;

#if BOOST_TEXT_USE_COROUTINES

    auto r = detail::bidi_subranges<int, detail::bidi_mode::level_test>(
        first,
        last,
        paragraph_embedding_level,
        detail::bidi_next_hard_line_break_callable{});

    for (auto x : r) {
        retval.push_back(x);
    }

#else

    detail::bidi_subrange_state<
        CPIter,
        CPIter,
        detail::bidi_next_hard_line_break_callable,
        int,
        detail::bidi_mode::level_test>
        state{first,
              last,
              paragraph_embedding_level,
              detail::bidi_next_hard_line_break_callable{}};

    auto out = std::back_inserter(retval);
    while (!state.at_end()) {
        *out = state.get_value();
        ++out;
    }

#endif

    return retval;
}

template<typename CPIter>
std::vector<int> bidi_reordered_indices(
    CPIter first, CPIter last, int paragraph_embedding_level = -1)
{
    using namespace boost::text;

    std::vector<int> retval;

#if BOOST_TEXT_USE_COROUTINES

    auto r = detail::bidi_subranges<
        bidirectional_cp_subrange<CPIter>,
        detail::bidi_mode::reorder_test>(
        first,
        last,
        paragraph_embedding_level,
        detail::bidi_next_hard_line_break_callable{});

    for (auto subrange : r) {
        for (auto cp : subrange) {
            retval.push_back(cp);
        }
    }

#else

    detail::bidi_subrange_state<
        CPIter,
        CPIter,
        detail::bidi_next_hard_line_break_callable,
        bidirectional_cp_subrange<CPIter>,
        detail::bidi_mode::reorder_test>
        state{
            first,
            last,
            paragraph_embedding_level,
            detail::bidi_next_hard_line_break_callable{}};

    while (!state.at_end()) {
        auto const subrange = state.get_value();
        for (auto cp : subrange) {
            retval.push_back(cp);
        }
    }

#endif

    return retval;
}

namespace std {
    ostream & operator<<(ostream & os, vector<int> const & levels)
    {
        os << "[ ";
        for (int l : levels) {
            os << l << " ";
        }
        os << ']' << endl;
        return os;
    }
}

#endif
