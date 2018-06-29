#ifndef BOOST_TEXT_SEARCH_HPP
#define BOOST_TEXT_SEARCH_HPP

#include <boost/text/collate.hpp>

#include <boost/algorithm/cxx14/mismatch.hpp>
#include <boost/container/small_vector.hpp>

#include <deque>


namespace boost { namespace text {

    // TODO: Sentinels!

    namespace detail {

        template<typename CPIter, typename Sentinel>
        CPIter dummy_prev_break(CPIter first, CPIter it, Sentinel last) noexcept
        {
            return it;
        }

        collation_element adjust_ce_for_search(
            collation_element ce,
            collation_strength strength,
            case_first case_1st,
            case_level case_lvl) noexcept
        {
            ce = modify_for_case(ce, strength, case_1st, case_lvl);
            if (strength < collation_strength::quaternary) {
                ce.l4_ = 0;
                if (strength < collation_strength::tertiary) {
                    ce.l3_ = 0;
                    if (strength < collation_strength::secondary)
                        ce.l2_ = 0;
                }
            }
            return ce;
        }

        // Returns the iterator in [first, last) at which the match is found, or
        // first otherwise.
        template<typename CPIter1>
        CPIter1 search_mismatch(
            CPIter1 first,
            CPIter1 last,
            std::deque<collation_element> & str_ces,
            std::deque<int> & str_ce_sizes,
            container::small_vector<collation_element, 1024> const & sub_ces,
            collation_table const & table,
            collation_strength strength,
            case_first case_1st,
            case_level case_lvl,
            variable_weighting weighting)
        {
            auto sub_ces_it = sub_ces.begin();
            auto const sub_ces_end = sub_ces.end();

            auto mismatch = algorithm::mismatch(
                str_ces.begin(), str_ces.end(), sub_ces_it, sub_ces_end);

            if (mismatch.first == str_ces.end()) {
                if (mismatch.second == sub_ces_end) {
                    return std::next(first, str_ce_sizes.size());
                } else {
                    sub_ces_it = mismatch.second;
                    while (sub_ces_it != sub_ces_end) {
                        if (first == last)
                            return first;

                        auto const str_ces_old_size = str_ces.size();
                        table.copy_collation_elements(
                            first,
                            std::next(first),
                            std::back_inserter(str_ces),
                            strength,
                            case_1st,
                            case_lvl,
                            weighting);
                        str_ce_sizes.push_back(
                            str_ces.size() - str_ces_old_size);

                        bool mismatch_found = false;
                        for (std::size_t i = str_ces_old_size,
                                         end = str_ces.size();
                             i < end;
                             ++i) {
                            str_ces[i] = adjust_ce_for_search(
                                str_ces[i], strength, case_1st, case_lvl);
                            if (str_ces[i] != *sub_ces_it++)
                                mismatch_found = true;
                        }

                        if (mismatch_found)
                            return first;
                    }
                }

                return std::next(first, str_ce_sizes.size());
            } else {
                return first;
            }
        }

        // TODO: Document the requirements of BreakFunc.
        template<typename CPIter1, typename CPIter2, typename BreakFunc>
        CPIter1 search_impl(
            CPIter1 first,
            CPIter1 last,
            CPIter2 sub_first,
            CPIter2 sub_last,
            BreakFunc break_fn,
            collation_table const & table,
            collation_strength strength,
            case_first case_1st,
            case_level case_lvl,
            variable_weighting weighting)
        {
            container::small_vector<collation_element, 1024> sub_ces;
            table.copy_collation_elements(
                sub_first,
                sub_last,
                std::back_inserter(sub_ces),
                strength,
                case_1st,
                case_lvl,
                weighting);

            for (auto & ce : sub_ces) {
                ce = adjust_ce_for_search(ce, strength, case_1st, case_lvl);
            }

            std::deque<collation_element> str_ces;
            std::deque<int> str_ce_sizes;

            auto pop_front = [&str_ces, &str_ce_sizes]() {
                str_ces.erase(
                    str_ces.begin(), str_ces.begin() + str_ce_sizes.front());
                str_ce_sizes.pop_front();
            };

            auto it = first;
            while (it != last) {
                if (break_fn(first, it, last) == it) {
                    auto mismatch_it = search_mismatch(
                        it,
                        last,
                        str_ces,
                        str_ce_sizes,
                        sub_ces,
                        table,
                        strength,
                        case_1st,
                        case_lvl,
                        weighting);
                    if (mismatch_it != it &&
                        break_fn(first, mismatch_it, last) == mismatch_it) {
                        return it;
                    }
                    pop_front();
                }
            }

            return it;
        }
    }

    /** Returns the first occurrence of the subsequence [sub_first, sub_last)
        in the range [first, last), or a value equal to last if no such
        occurrence is found. */
    template<typename CPIter1, typename CPIter2, typename BreakFunc>
    CPIter1 search(
        CPIter1 first,
        CPIter1 last,
        CPIter2 sub_first,
        CPIter2 sub_last,
        BreakFunc break_fn,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
    {
        return detail::search_impl(
            first,
            last,
            sub_first,
            sub_last,
            break_fn,
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
    }

    /** Returns the first occurrence of the subsequence r2 in the range r1, or
        a value equal to r1.end() if no such occurrence is found. */
    template<typename CPRange1, typename CPRange2, typename BreakFunc>
    auto search(
        CPRange1 & r1,
        CPRange2 & r2,
        BreakFunc break_fn,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
        -> decltype(r1.begin())
    {
        using std::begin;
        using std::end;
        return search(
            begin(r1),
            end(r1),
            begin(r2),
            end(r2),
            break_fn,
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
    }

    /** Returns the first occurrence of the subsequence [sub_first, sub_last)
        in the range [first, last), or a value equal to last if no such
        occurrence is found. */
    template<typename CPIter1, typename CPIter2>
    CPIter1 search(
        CPIter1 first,
        CPIter1 last,
        CPIter2 sub_first,
        CPIter2 sub_last,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
    {
        return detail::search_impl(
            first,
            last,
            sub_first,
            sub_last,
            detail::dummy_prev_break<CPIter1, CPIter1>,
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
    }

    /** Returns the first occurrence of the subsequence r2 in the range r1, or
        a value equal to r1.end() if no such occurrence is found. */
    template<typename CPRange1, typename CPRange2>
    auto search(
        CPRange1 & r1,
        CPRange2 & r2,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
        -> decltype(r1.begin())
    {
        using std::begin;
        using std::end;
        return search(
            begin(r1),
            end(r1),
            begin(r2),
            end(r2),
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
    }

}}

#endif
