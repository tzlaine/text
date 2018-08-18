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
           next_hard_line_break_callable &,
           std::back_insert_iterator<std::vector<int>> out) {

            // Do L1, but not the rest of the line-based processing, to get
            // the final embeddings that match the test data.

            using pae_cp_iterator =
                detail::props_and_embeddings_cp_iterator<str_iter_t>;

            detail::next_line_break_t<
                next_hard_line_break_callable,
                line_break_result<pae_cp_iterator>,
                pae_cp_iterator>
                next;
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
                     pae_cp_iterator{props_and_embeddings.end()},
                     std::move(next)},
                    {pae_cp_iterator{props_and_embeddings.end()}}};

            for (auto line : lines) {
                detail::l1(line, paragraph_embedding_level);
                for (auto it = line.begin(), end = line.end(); it != end;
                     ++it) {
                    *out = it.base()->embedding_;
                    ++out;
                }
            }

            return out;
        });


    return retval;
}

inline std::vector<int> bidi_reordered_indices(
    str_iter_t first, str_iter_t last, int paragraph_embedding_level = -1)
{
    using namespace boost::text;

    std::vector<int> retval;

    detail::bidirectional_order_impl(
        first,
        last,
        bidirectional_subrange_copier(std::back_inserter(retval)),
        next_hard_line_break_callable{},
        paragraph_embedding_level,
        [](detail::props_and_embeddings_t<str_iter_t> & props_and_embeddings,
           int paragraph_embedding_level,
           next_hard_line_break_callable &,
           bidirectional_subrange_copy_iterator<
               std::back_insert_iterator<std::vector<int>>> out) {

            // Do everything through L2 to match the test data.

            using pae_cp_iterator =
                detail::props_and_embeddings_cp_iterator<str_iter_t>;

            detail::all_runs_t<str_iter_t> all_runs;

            detail::next_line_break_t<
                next_hard_line_break_callable,
                line_break_result<pae_cp_iterator>,
                pae_cp_iterator>
                next;
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
                     pae_cp_iterator{props_and_embeddings.end()},
                     std::move(next)},
                    {pae_cp_iterator{props_and_embeddings.end()}}};

            for (auto line : lines) {
                detail::l1(line, paragraph_embedding_level);

                // https://unicode.org/reports/tr9/#L2
                all_runs = detail::find_all_runs<str_iter_t>(
                    line.begin().base(), line.end().base(), true);
                auto reordered_runs = detail::l2(all_runs);

                // Output the reordered subranges.
                for (auto run : reordered_runs) {
                    auto const cp_first = run.begin()->it_;
                    auto const cp_last =
                        run.begin() == run.end()
                            ? cp_first
                            : std::next(std::prev(run.end())->it_);
                    if (run.reversed()) {
                        auto out_value = bidirectional_subrange<str_iter_t>{
                            detail::make_reverse_iterator(cp_last),
                            detail::make_reverse_iterator(cp_first)};
                        *out = out_value;
                        ++out;
                    } else {
                        auto out_value =
                            bidirectional_subrange<str_iter_t>{cp_first, cp_last};
                        *out = out_value;
                        ++out;
                    }
                }

                bidirectional_subrange<str_iter_t> const line_break_range{
                    line.hard_break() ? detail::bidi_line_break_kind::hard
                                      : detail::bidi_line_break_kind::possible};
                *out = line_break_range;
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
