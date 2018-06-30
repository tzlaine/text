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
        template<typename CPIter1, std::size_t N>
        CPIter1 default_search_mismatch(
            CPIter1 first,
            CPIter1 last,
            std::deque<collation_element> & str_ces,
            std::deque<int> & str_ce_sizes,
            container::small_vector<collation_element, N> const & sub_ces,
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
    }

    /** TODO */
    template<typename CPIter, typename BreakFunc>
    struct default_collation_searcher
    {
        default_collation_searcher(
            CPIter sub_first,
            CPIter sub_last,
            BreakFunc break_fn,
            collation_table const & table,
            collation_strength strength = collation_strength::tertiary,
            case_first case_1st = case_first::off,
            case_level case_lvl = case_level::off,
            variable_weighting weighting = variable_weighting::non_ignorable) :
            table_(table),
            strength_(strength),
            case_first_(case_1st),
            case_level_(case_lvl),
            weighting_(weighting),
            break_fn_(break_fn),
            sub_size_(0)
        {
            table.copy_collation_elements(
                sub_first,
                sub_last,
                std::back_inserter(sub_ces_),
                strength_,
                case_first_,
                case_level_,
                weighting_);

            for (auto & ce : sub_ces_) {
                ce = adjust_ce_for_search(
                    ce, strength_, case_first_, case_level_);
            }

            sub_size_ = std::distance(sub_first, sub_last);
        }

        template<typename CPIter2>
        std::pair<CPIter2, CPIter2>
        operator()(CPIter2 first, CPIter2 last) const
        {
            if (first == last || sub_ces_.empty())
                return std::pair<CPIter2, CPIter2>(first, first);

            std::deque<detail::collation_element> str_ces;
            std::deque<int> str_ce_sizes;

            auto pop_front = [&str_ces, &str_ce_sizes]() {
                str_ces.erase(
                    str_ces.begin(), str_ces.begin() + str_ce_sizes.front());
                str_ce_sizes.pop_front();
            };

            auto it = first;
            auto const match_end = std::prev(last, sub_size_);
            while (it != match_end) {
                if (break_fn_(first, it, last) == it) {
                    auto mismatch_it = detail::default_search_mismatch(
                        it,
                        last,
                        str_ces,
                        str_ce_sizes,
                        sub_ces_,
                        table_,
                        strength_,
                        case_first_,
                        case_level_,
                        weighting_);
                    if (mismatch_it != it &&
                        break_fn_(first, mismatch_it, last) == mismatch_it) {
                        return std::pair<CPIter2, CPIter2>(it, mismatch_it);
                    }
                    pop_front();
                }
            }

            return std::pair<CPIter2, CPIter2>(last, last);
        }

    private:
        collation_table table_;
        collation_strength strength_;
        case_first case_first_;
        case_level case_level_;
        variable_weighting weighting_;
        container::small_vector<detail::collation_element, 256> sub_ces_;
        BreakFunc break_fn_;
        int sub_size_;
    };

    // TODO: Document the requirements of BreakFunc.

    /** TODO */
    template<typename CPIter, typename BreakFunc>
    default_collation_searcher<CPIter, BreakFunc>
    make_default_collation_searcher(
        CPIter first,
        CPIter last,
        BreakFunc break_fn,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
    {
        return default_collation_searcher<CPIter, BreakFunc>(
            first,
            last,
            break_fn,
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
    }

    /** TODO */
    template<typename CPIter, typename Searcher>
    CPIter
    collation_search(CPIter first, CPIter last, Searcher const & searcher)
    {
        return searcher(first, last).first;
    }

    /** Returns the first occurrence of the subsequence [sub_first, sub_last)
        in the range [first, last), or a value equal to last if no such
        occurrence is found. */
    template<typename CPIter1, typename CPIter2, typename BreakFunc>
    CPIter1 collation_search(
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
        auto const s = make_default_collation_searcher(
            sub_first,
            sub_last,
            break_fn,
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
        return collation_search(first, last, s);
    }

    /** Returns the first occurrence of the subsequence substr in the range str, or
        a value equal to str.end() if no such occurrence is found. */
    template<typename CPRange1, typename CPRange2, typename BreakFunc>
    auto collation_search(
        CPRange1 & str,
        CPRange2 & substr,
        BreakFunc break_fn,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
        -> decltype(str.begin())
    {
        using std::begin;
        using std::end;
        auto const s = make_default_collation_searcher(
            begin(substr),
            end(substr),
            break_fn,
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
        return collation_search(begin(str), end(str), s);
    }

    /** Returns the first occurrence of the subsequence [sub_first, sub_last)
        in the range [first, last), or a value equal to last if no such
        occurrence is found. */
    template<typename CPIter1, typename CPIter2>
    CPIter1 collation_search(
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
        auto const s = make_default_collation_searcher(
            sub_first,
            sub_last,
            detail::dummy_prev_break<CPIter1, CPIter1>,
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
        return collation_search(first, last, s);
    }

    /** Returns the first occurrence of the subsequence substr in the range str, or
        a value equal to str.end() if no such occurrence is found. */
    template<typename CPRange1, typename CPRange2>
    auto collation_search(
        CPRange1 & str,
        CPRange2 & substr,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
        -> decltype(str.begin())
    {
        using result_t = decltype(str.begin());
        using std::begin;
        using std::end;
        auto const s = make_default_collation_searcher(
            begin(substr),
            end(substr),
            detail::dummy_prev_break<result_t, result_t>,
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
        return collation_search(begin(str), end(str), s);
    }

}}

#endif
