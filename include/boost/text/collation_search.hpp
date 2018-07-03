#ifndef BOOST_TEXT_SEARCH_HPP
#define BOOST_TEXT_SEARCH_HPP

#include <boost/text/collate.hpp>

#include <boost/algorithm/cxx14/mismatch.hpp>
#include <boost/container/small_vector.hpp>

#include <deque>
#include <unordered_map>


namespace boost { namespace text {

    // TODO: Sentinels!

    /** TODO */
    template<typename CPIter, typename Searcher>
    CPIter
    collation_search(CPIter first, CPIter last, Searcher const & searcher)
    {
        return searcher(first, last).first;
    }

    /** TODO */
    template<typename CPRange, typename Searcher>
    auto collation_search(CPRange r, Searcher const & searcher)
        -> decltype(std::begin(r))
    {
        using std::begin;
        using std::end;
        return searcher(begin(r), end(r)).first;
    }

}}

// Le sigh.
namespace std {

    template<>
    struct hash<boost::text::detail::collation_element>
    {
        using argument_type = boost::text::detail::collation_element;
        using result_type = std::size_t;
        result_type operator()(argument_type ce) const noexcept
        {
            return (result_type(ce.l1_) << 32) | (ce.l2_ << 16) | ce.l3_;
        }
    };
}

namespace boost { namespace text {

    namespace detail {

        template<typename CPIter, typename Sentinel>
        struct dummy_prev_break
        {
            CPIter operator()(CPIter first, CPIter it, Sentinel last) const
                noexcept
            {
                return it;
            }
        };

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

        template<typename CPIter, std::size_t N>
        void get_search_ces(
            CPIter get_first,
            CPIter get_last,
            CPIter last,
            container::small_vector<collation_element, N> & ces,
            collation_table const & table,
            collation_strength strength,
            case_first case_1st,
            case_level case_lvl,
            variable_weighting weighting)
        {
            ces.clear();
            auto const next_contiguous_starter_it = std::find_if(
                get_last, last, [](uint32_t cp) { return ccc(cp) == 0; });

            table.copy_collation_elements(
                get_first,
                next_contiguous_starter_it,
                std::back_inserter(ces),
                strength,
                case_1st,
                case_lvl,
                weighting);

            for (auto & ce : ces) {
                ce = adjust_ce_for_search(ce, strength, case_1st, case_lvl);
            }
        }

        template<typename CPIter>
        void append_search_ces_and_sizes(
            CPIter get_first,
            CPIter get_last,
            CPIter last,
            std::deque<collation_element> & ces,
            std::deque<int> & ce_sizes,
            collation_table const & table,
            collation_strength strength,
            case_first case_1st,
            case_level case_lvl,
            variable_weighting weighting)
        {
            auto const next_contiguous_starter_it = std::find_if(
                get_last, last, [](uint32_t cp) { return ccc(cp) == 0; });

            auto ce_size_it = std::back_inserter(ce_sizes);
            auto const old_ces_size = ces.size();
            table.copy_collation_elements(
                get_first,
                next_contiguous_starter_it,
                std::back_inserter(ces),
                strength,
                case_1st,
                case_lvl,
                weighting,
                &ce_size_it);

            for (auto i = old_ces_size, end = ces.size(); i < end; ++i) {
                ces[i] =
                    adjust_ce_for_search(ces[i], strength, case_1st, case_lvl);
            }
        }

        template<typename Iter, typename IteratorCategory>
        Iter next_until(Iter it, std::ptrdiff_t n, Iter last, IteratorCategory)
        {
            assert(0 <= n);
            while (n && it != last) {
                --n;
                ++it;
            }
            return it;
        }

        template<typename Iter>
        Iter next_until(
            Iter it,
            std::ptrdiff_t n,
            Iter last,
            std::random_access_iterator_tag)
        {
            return std::next(it, (std::min)(n, last - it));
        }

        template<typename Iter>
        Iter next_until(Iter it, std::ptrdiff_t n, Iter last)
        {
            return next_until(
                it,
                n,
                last,
                typename std::iterator_traits<Iter>::iterator_category{});
        }

        enum { search_eos = -1 };

        // Returns a pair, which consists of: a) an iterator to the CP in
        // [first, last) at which the end of the match is found, or first
        // otherwise; and b) the index of the first mismatched CE in str_ces.
        // b) is only relevant if a) is equal to the 'last' parameter.  If a)
        // is last and b) is search_eos, we're done and the search has failed
        // to find a match.
        template<typename CPIter1, std::size_t N>
        std::pair<CPIter1, std::ptrdiff_t> search_mismatch(
            CPIter1 first,
            CPIter1 last,
            std::deque<collation_element> & str_ces,
            std::deque<int> & str_ce_sizes,
            container::small_vector<collation_element, N> const & pattern_ces,
            collation_table const & table,
            collation_strength strength,
            case_first case_1st,
            case_level case_lvl,
            variable_weighting weighting)
        {
            // TODO: Only return a valid match if the CE we end up on is also
            // at the end of some CP's CEs.

            auto pattern_ces_it = pattern_ces.begin();
            auto const pattern_ces_end = pattern_ces.end();

            constexpr int cps_per_append = 16; // TODO: Tune this value?

            auto match = [first, &str_ce_sizes]() {
                return std::make_pair(std::next(first, str_ce_sizes.size()), 0);
            };
            auto mismatch_at =
                [first, &str_ces](
                    typename std::deque<collation_element>::iterator at) {
                    return std::make_pair(first, at - str_ces.begin());
                };

            auto mismatch = algorithm::mismatch(
                str_ces.begin(),
                str_ces.end(),
                pattern_ces_it,
                pattern_ces_end);

            // If the entirety of str_ces matches pattern_ces, then keep
            // looking....
            if (mismatch.first == str_ces.end()) {
                // If the match is also at the end of pattern_ces, that's our
                // full match.
                if (mismatch.second == pattern_ces_end) {
                    return match();
                } else {
                    // Othwerwise, take CEs a few CPs at a time and see if
                    // they match.
                    pattern_ces_it = mismatch.second;
                    while (pattern_ces_it != pattern_ces_end) {
                        auto const it = std::next(first, str_ce_sizes.size());
                        if (it == last)
                            return std::make_pair(first, search_eos);

                        auto const str_ces_old_size = str_ces.size();
                        append_search_ces_and_sizes(
                            it,
                            next_until(it, cps_per_append, last),
                            last,
                            str_ces,
                            str_ce_sizes,
                            table,
                            strength,
                            case_1st,
                            case_lvl,
                            weighting);

                        mismatch = algorithm::mismatch(
                            str_ces.begin() + str_ces_old_size,
                            str_ces.begin() + str_ces.size(),
                            pattern_ces_it,
                            pattern_ces_end);

                        if (mismatch.first == str_ces.end()) {
                            if (mismatch.second == pattern_ces_end) {
                                return match();
                            } else {
                                pattern_ces_it = mismatch.second;
                                continue;
                            }
                        } else {
                            if (mismatch.second == pattern_ces_end)
                                return match();
                            else
                                return mismatch_at(mismatch.first);
                        }
                    }

                    return match();
                }
            } else {
                return mismatch_at(mismatch.first);
            }
        }

        struct search_skip_table
        {
            search_skip_table() : default_() {}

            search_skip_table(
                std::ptrdiff_t pattern_ces, std::ptrdiff_t default_value) :
                default_(default_value),
                map_(pattern_ces)
            {}

            void insert(detail::collation_element key, std::ptrdiff_t value)
            {
                map_[key] = value;
            }

            bool empty() const { return map_.empty(); }

            std::ptrdiff_t operator[](detail::collation_element key) const
            {
                auto const it = map_.find(key);
                return it == map_.end() ? default_ : it->second;
            }

        private:
            using map_t =
                std::unordered_map<detail::collation_element, std::ptrdiff_t>;

            std::ptrdiff_t default_;
            map_t map_;
        };
    }

    // TODO: Refactor breaking searches to use next_break() like:
    //   next = next_break(it, last);
    //   search_in(it, next);
    // ?  search_in() should single step through CEs when (next - it) >
    // some-heuristic-threshold, and grab all CEs in [it, next) at once
    // otherwise.

    // Searchers

    /** TODO */
    template<typename CPIter, typename BreakFunc>
    struct default_collation_searcher
    {
        default_collation_searcher(
            CPIter pattern_first,
            CPIter pattern_last,
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
            break_fn_(break_fn)
        {
            detail::get_search_ces(
                pattern_first,
                pattern_last,
                pattern_last,
                pattern_ces_,
                table_,
                strength_,
                case_first_,
                case_level_,
                weighting_);
        }

        template<typename CPIter2>
        std::pair<CPIter2, CPIter2>
        operator()(CPIter2 first, CPIter2 last) const
        {
            if (first == last || pattern_ces_.empty())
                return std::pair<CPIter2, CPIter2>(first, first);

            std::deque<detail::collation_element> str_ces;
            std::deque<int> str_ce_sizes;

            auto pop_front = [&str_ces, &str_ce_sizes]() {
                str_ces.erase(
                    str_ces.begin(), str_ces.begin() + str_ce_sizes.front());
                str_ce_sizes.pop_front();
            };

            auto at_break = [first, last, this](CPIter2 it) {
                return break_fn_(first, it, last) == it;
            };

            auto it = first;
            while (it != last) {
                if (at_break(it)) {
                    auto mismatch_it = detail::search_mismatch(
                                           it,
                                           last,
                                           str_ces,
                                           str_ce_sizes,
                                           pattern_ces_,
                                           table_,
                                           strength_,
                                           case_first_,
                                           case_level_,
                                           weighting_)
                                           .first;
                    if (mismatch_it != it && at_break(mismatch_it))
                        return std::pair<CPIter2, CPIter2>(it, mismatch_it);
                }
                if (!str_ces.empty())
                    pop_front();
                ++it;
            }

            return std::pair<CPIter2, CPIter2>(last, last);
        }

    private:
        collation_table table_;
        collation_strength strength_;
        case_first case_first_;
        case_level case_level_;
        variable_weighting weighting_;
        container::small_vector<detail::collation_element, 256> pattern_ces_;
        BreakFunc break_fn_;
    };

    // TODO: Document the requirements of BreakFunc.

    /** TODO */
    template<typename CPIter>
    default_collation_searcher<CPIter, detail::dummy_prev_break<CPIter, CPIter>>
    make_default_collation_searcher(
        CPIter first,
        CPIter last,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
    {
        return default_collation_searcher<
            CPIter,
            detail::dummy_prev_break<CPIter, CPIter>>(
            first,
            last,
            detail::dummy_prev_break<CPIter, CPIter>{},
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
    }

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
    template<typename CPRange>
    auto make_default_collation_searcher(
        CPRange r,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
        -> default_collation_searcher<
            decltype(std::begin(r)),
            detail::dummy_prev_break<
                decltype(std::begin(r)),
                decltype(std::begin(r))>>
    {
        using std::begin;
        using std::end;
        using r_iter = decltype(std::begin(r));
        return default_collation_searcher<
            r_iter,
            detail::dummy_prev_break<r_iter, r_iter>>(
            begin(r),
            end(r),
            detail::dummy_prev_break<r_iter, r_iter>{},
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
    }

    /** TODO */
    template<typename CPRange, typename BreakFunc>
    auto make_default_collation_searcher(
        CPRange r,
        BreakFunc break_fn,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
        -> default_collation_searcher<decltype(std::begin(r)), BreakFunc>
    {
        using std::begin;
        using std::end;
        using r_iter = decltype(std::begin(r));
        return default_collation_searcher<r_iter, BreakFunc>(
            begin(r),
            end(r),
            break_fn,
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
    }

    /** TODO */
    template<typename CPIter, typename BreakFunc>
    struct boyer_moore_horspool_collation_searcher
    {
        boyer_moore_horspool_collation_searcher(
            CPIter pattern_first,
            CPIter pattern_last,
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
            break_fn_(break_fn)
        {
            detail::get_search_ces(
                pattern_first,
                pattern_last,
                pattern_last,
                pattern_ces_,
                table_,
                strength_,
                case_first_,
                case_level_,
                weighting_);

            skips_ =
                detail::search_skip_table(pattern_ces_.size(), pattern_ces_.size());

            std::ptrdiff_t i = 0;
            for (auto ce : pattern_ces_) {
                skips_.insert(ce, pattern_ces_.size() - 1 - i++);
            }
        }

        template<typename CPIter2>
        std::pair<CPIter2, CPIter2>
        operator()(CPIter2 first, CPIter2 last) const
        {
            if (first == last || !skips_.empty())
                return std::pair<CPIter2, CPIter2>(first, first);

            std::deque<detail::collation_element> str_ces;
            std::deque<int> str_ce_sizes;

            auto it = first;

            auto pop_front = [&str_ces, &str_ce_sizes, &it](int ces) {
                str_ces.erase(str_ces.begin(), str_ces.begin() + ces);
                while (0 <= ces) {
                    ces -= str_ce_sizes.front();
                    str_ce_sizes.pop_front();
                    ++it;
                }
                str_ces.erase(str_ces.begin(), str_ces.begin() - ces);
            };

            auto at_break = [first, last, this](CPIter2 it) {
                return break_fn_(first, it, last) == it;
            };

            while (it != last) {
                if (at_break(it)) {
                    auto pair = detail::search_mismatch(
                        it,
                        last,
                        str_ces,
                        str_ce_sizes,
                        pattern_ces_,
                        table_,
                        strength_,
                        case_first_,
                        case_level_,
                        weighting_);
                    if (pair.first == it) {
                        if (pair.second == detail::search_eos)
                            return std::pair<CPIter2, CPIter2>(last, last);
                        std::ptrdiff_t const pattern_length =
                            pattern_ces_.size();
                        std::ptrdiff_t const str_ce_for_skip =
                            pair.second + pattern_length - 1;
                        // We need to have sufficient lookahead (at least
                        // pattern_length extra CEs) for the skip below to
                        // work.
                        if ((std::ptrdiff_t)str_ces.size() < str_ce_for_skip) {
                            auto const append_it =
                                std::next(it, str_ce_sizes.size());
                            detail::append_search_ces_and_sizes(
                                append_it,
                                detail::next_until(
                                    append_it,
                                    (str_ce_for_skip -
                                     (std::ptrdiff_t)str_ces.size()),
                                    last),
                                last,
                                str_ces,
                                str_ce_sizes,
                                table_,
                                strength_,
                                case_first_,
                                case_level_,
                                weighting_);
                        }
                        pop_front(skips_[str_ces[str_ce_for_skip]]);
                    } else {
                        if (break_fn_(first, pair.first, last) == pair.first)
                            return std::pair<CPIter2, CPIter2>(it, pair.first);
                        else
                            pop_front(str_ce_sizes.front());
                    }
                } else {
                    if (!str_ces.empty())
                        pop_front(str_ce_sizes.front());
                    else
                        ++it;
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
        detail::search_skip_table skips_;
        container::small_vector<detail::collation_element, 256> pattern_ces_;
        BreakFunc break_fn_;
    };

    /** TODO */
    template<typename CPIter>
    boyer_moore_horspool_collation_searcher<
        CPIter,
        detail::dummy_prev_break<CPIter, CPIter>>
    make_boyer_moore_horspool_collation_searcher(
        CPIter first,
        CPIter last,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
    {
        return boyer_moore_horspool_collation_searcher<
            CPIter,
            detail::dummy_prev_break<CPIter, CPIter>>(
            first,
            last,
            detail::dummy_prev_break<CPIter, CPIter>{},
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
    }

    /** TODO */
    template<typename CPIter, typename BreakFunc>
    boyer_moore_horspool_collation_searcher<CPIter, BreakFunc>
    make_boyer_moore_horspool_collation_searcher(
        CPIter first,
        CPIter last,
        BreakFunc break_fn,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
    {
        return boyer_moore_horspool_collation_searcher<CPIter, BreakFunc>(
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
    template<typename CPRange>
    auto make_boyer_moore_horspool_collation_searcher(
        CPRange r,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
        -> boyer_moore_horspool_collation_searcher<
            decltype(std::begin(r)),
            detail::dummy_prev_break<
                decltype(std::begin(r)),
                decltype(std::begin(r))>>
    {
        using std::begin;
        using std::end;
        using r_iter = decltype(std::begin(r));
        return boyer_moore_horspool_collation_searcher<
            r_iter,
            detail::dummy_prev_break<r_iter, r_iter>>(
            begin(r),
            end(r),
            detail::dummy_prev_break<r_iter, r_iter>{},
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
    }

    /** TODO */
    template<typename CPRange, typename BreakFunc>
    auto make_boyer_moore_horspool_collation_searcher(
        CPRange r,
        BreakFunc break_fn,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
        -> boyer_moore_horspool_collation_searcher<
            decltype(std::begin(r)),
            BreakFunc>
    {
        using std::begin;
        using std::end;
        using r_iter = decltype(std::begin(r));
        return boyer_moore_horspool_collation_searcher<r_iter, BreakFunc>(
            begin(r),
            end(r),
            break_fn,
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
    }

    // Convenience overloads

    /** Returns the first occurrence of the subsequence [pattern_first,
        pattern_last) in the range [first, last), or a value equal to last if
        no such occurrence is found. */
    template<typename CPIter1, typename CPIter2, typename BreakFunc>
    CPIter1 collation_search(
        CPIter1 first,
        CPIter1 last,
        CPIter2 pattern_first,
        CPIter2 pattern_last,
        BreakFunc break_fn,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
    {
        auto const s = make_default_collation_searcher(
            pattern_first,
            pattern_last,
            break_fn,
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
        return collation_search(first, last, s);
    }

    /** Returns the first occurrence of pattern in str, or a value equal to
        str.end() if no such occurrence is found. */
    template<typename CPRange1, typename CPRange2, typename BreakFunc>
    auto collation_search(
        CPRange1 & str,
        CPRange2 & pattern,
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
            pattern, break_fn, table, strength, case_1st, case_lvl, weighting);
        return collation_search(begin(str), end(str), s);
    }

    /** Returns the first occurrence of the subsequence [pattern_first,
        pattern_last) in the range [first, last), or a value equal to last if
        no such occurrence is found. */
    template<typename CPIter1, typename CPIter2>
    CPIter1 collation_search(
        CPIter1 first,
        CPIter1 last,
        CPIter2 pattern_first,
        CPIter2 pattern_last,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
    {
        auto const s = make_default_collation_searcher(
            pattern_first,
            pattern_last,
            detail::dummy_prev_break<CPIter1, CPIter1>{},
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
        return collation_search(first, last, s);
    }

    /** Returns the first occurrence of pattern in str, or a value equal to
        str.end() if no such occurrence is found. */
    template<typename CPRange1, typename CPRange2>
    auto collation_search(
        CPRange1 & str,
        CPRange2 & pattern,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
        -> decltype(std::begin(str))
    {
        using std::begin;
        using std::end;
        using str_iter = decltype(std::begin(str));
        auto const s = make_default_collation_searcher(
            pattern,
            detail::dummy_prev_break<str_iter, str_iter>{},
            table,
            strength,
            case_1st,
            case_lvl,
            weighting);
        return collation_search(begin(str), end(str), s);
    }

}}

#endif
