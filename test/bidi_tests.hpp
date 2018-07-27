#ifndef BOOST_TEXT_TEST_BIDI_TESTS_HPP
#define BOOST_TEXT_TEST_BIDI_TESTS_HPP

#include <boost/text/bidirectional.hpp>


using str_iter_t = uint32_t const *;

inline std::vector<int> bidi_levels(
    str_iter_t first, str_iter_t last, int paragraph_embedding_level = -1)
{
    using namespace boost::text;

    std::vector<int> retval;

    detail::bidirectional_order_impl(
        first,
        last,
        std::back_inserter(retval),
        next_hard_line_break_callable{},
        paragraph_embedding_level,
        [](detail::props_and_embeddings_t<str_iter_t> & props_and_embeddings,
           int paragraph_embedding_level,
           next_hard_line_break_callable & next_line_break,
           std::back_insert_iterator<std::vector<int>> out) {

            // Do L1, but not the rest of the line-based processing, to get
            // the final embeddings that match the test data.

            using pae_cp_iterator =
                detail::props_and_embeddings_cp_iterator<str_iter_t>;

            lazy_segment_range<
                line_break_result<pae_cp_iterator>,
                pae_cp_iterator,
                detail::next_line_break_t<
                    next_hard_line_break_callable,
                    line_break_result<pae_cp_iterator>,
                    pae_cp_iterator>,
                line_break_cp_range<pae_cp_iterator>>
                lines{
                    {line_break_result<pae_cp_iterator>{
                         pae_cp_iterator{props_and_embeddings.begin()}, false},
                     pae_cp_iterator{props_and_embeddings.end()}},
                    {pae_cp_iterator{props_and_embeddings.end()}}};

            for (auto line : lines) {
                detail::l1(line, paragraph_embedding_level);
                for (auto it = line.begin(), end = line.end(); it != end;
                     ++it) {
                    *out = it.it_->embedding_;
                    ++out;
                }
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
