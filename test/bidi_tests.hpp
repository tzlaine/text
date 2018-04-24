#ifndef BOOST_TEXT_TEST_BIDI_TESTS_HPP
#define BOOST_TEXT_TEST_BIDI_TESTS_HPP

#include <boost/text/bidirectional.hpp>


using str_iter_t = uint32_t const *;

inline std::vector<int> bidi_levels(str_iter_t first, str_iter_t last)
{
    using namespace boost::text;

    std::vector<int> retval;

    detail::bidirectional_order_impl(
        first,
        last,
        std::back_inserter(retval),
        next_possible_line_break_callable{},
        [](detail::props_and_embeddings_t<str_iter_t> & props_and_embeddings,
           int paragraph_embedding_level,
           next_possible_line_break_callable & next_line_break,
           std::back_insert_iterator<std::vector<int>> out) {
            for (auto pae : props_and_embeddings) {
                *out = pae.embedding_;
                ++out;
            }
            return out;
        });


    return retval;
}

namespace std {
    ostream &
    operator<<(ostream & os, vector<int> const & levels)
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
