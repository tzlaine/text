#ifndef BOOST_TEXT_TEST_BIDI_TESTS_HPP
#define BOOST_TEXT_TEST_BIDI_TESTS_HPP

#include <boost/text/bidirectional.hpp>


using str_iter_t = uint32_t const *;

inline std::vector<int> bidi_levels(
    str_iter_t first, str_iter_t last, int paragraph_embedding_level = -1)
{
    using namespace boost::text;

    std::vector<int> retval;

    detail::bidi_subrange_state<
        str_iter_t,
        str_iter_t,
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

    return retval;
}

inline std::vector<int> bidi_reordered_indices(
    str_iter_t first, str_iter_t last, int paragraph_embedding_level = -1)
{
    using namespace boost::text;

    std::vector<int> retval;

    detail::bidi_subrange_state<
        str_iter_t,
        str_iter_t,
        detail::bidi_next_hard_line_break_callable,
        bidirectional_subrange<str_iter_t>,
        detail::bidi_mode::reorder_test>
        state{first,
              last,
              paragraph_embedding_level,
              detail::bidi_next_hard_line_break_callable{}};

    while (!state.at_end()) {
        auto const subrange = state.get_value();
        for (auto cp : subrange) {
            retval.push_back(cp);
        }
    }

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
