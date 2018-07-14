#ifndef BOOST_TEXT_SEARCH_HPP
#define BOOST_TEXT_SEARCH_HPP

#include <boost/text/collate.hpp>

#include <boost/algorithm/cxx14/mismatch.hpp>
#include <boost/container/small_vector.hpp>

#include <deque>
#include <unordered_map>


#ifndef BOOST_TEXT_COLLATION_SEARCH_INSTRUMENTATION
#define BOOST_TEXT_COLLATION_SEARCH_INSTRUMENTATION 0
#endif

namespace boost { namespace text {

    // TODO: Sentinels!

    // TODO: Remove all the using std::begin business.  Just use std::begin()
    // directly.  (This should be done in all files, not just this one.)

    /** TODO */
    template<typename CPIter, typename Searcher>
    auto collation_search(CPIter first, CPIter last, Searcher const & searcher)
        -> decltype(searcher(first, last))
    {
        return searcher(first, last);
    }

    /** TODO */
    template<typename CPRange, typename Searcher>
    auto collation_search(CPRange r, Searcher const & searcher)
        -> decltype(searcher(std::begin(r), std::end(r)))
    {
        using std::begin;
        using std::end;
        return searcher(begin(r), end(r));
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

#if BOOST_TEXT_COLLATION_SEARCH_INSTRUMENTATION
        std::ostream & operator<<(std::ostream & os, collation_element ce)
        {
            os << "[" << std::hex;
            os << "0x" << std::setfill('0') << std::setw(4) << ce.l1_ << ", ";
            os << "0x" << std::setfill('0') << std::setw(2) << ce.l2_ << ", ";
            os << "0x" << std::setfill('0') << std::setw(2) << ce.l3_ << ", ";
            os << "0x" << std::setfill('0') << std::setw(4) << ce.l4_;
            os << "]" << std::dec;
            return os;
        }
#endif

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
            std::ptrdiff_t n = std::distance(get_first, get_last);
            auto const next_contiguous_starter_it =
                std::find_if(get_last, last, [&n](uint32_t cp) {
                    auto const retval = ccc(cp) == 0;
                    if (!retval)
                        ++n;
                    return retval;
                });

            container::small_vector<uint32_t, 1024> buf(n);
            std::copy(get_first, next_contiguous_starter_it, buf.begin());

#if BOOST_TEXT_COLLATION_SEARCH_INSTRUMENTATION
            std::cout << "get_search_ces(): Gathering CEs for [" << std::hex;
            bool first_cp = true;
            for (auto cp : buf) {
                if (!first_cp)
                    std::cout << ", ";
                std::cout << "0x" << std::setw(4) << std::setfill('0') << cp;
                if (cp < 0x80)
                    std::cout << " '" << (char)cp << "'";
                first_cp = false;
            }
            std::cout << std::dec << "]\n";
#endif

            table.copy_collation_elements(
                buf.begin(),
                buf.end(),
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
            std::ptrdiff_t n = std::distance(get_first, get_last);
            auto const next_contiguous_starter_it =
                std::find_if(get_last, last, [&n](uint32_t cp) {
                    auto const retval = ccc(cp) == 0;
                    if (!retval)
                        ++n;
                    return retval;
                });

            container::small_vector<uint32_t, 1024> buf(n);
            std::copy(get_first, next_contiguous_starter_it, buf.begin());

#if BOOST_TEXT_COLLATION_SEARCH_INSTRUMENTATION
            auto const old_ce_sizes_size = ce_sizes.size();

            std::cout << "append_search_ces_and_sizes(): Gathering CEs for ["
                      << std::hex;
            bool first_cp = true;
            for (auto cp : buf) {
                if (!first_cp)
                    std::cout << ", ";
                std::cout << "0x" << std::setw(4) << std::setfill('0') << cp;
                if (cp < 0x80)
                    std::cout << " '" << (char)cp << "'";
                first_cp = false;
            }
            std::cout << std::dec << "]\n";
#endif

            auto ce_size_it = std::back_inserter(ce_sizes);
            auto const old_ces_size = ces.size();
            table.copy_collation_elements(
                buf.begin(),
                buf.end(),
                std::back_inserter(ces),
                strength,
                case_1st,
                case_lvl,
                weighting,
                &ce_size_it);

#if BOOST_TEXT_COLLATION_SEARCH_INSTRUMENTATION
#if 0
            std::cout << "    ces gathered: [ ";
            for (auto i = old_ces_size, end = ces.size(); i < end; ++i) {
                std::cout << ces[i] << ' ';
            }
            std::cout << "]\n";
#endif
#endif

            for (auto i = old_ces_size, end = ces.size(); i < end; ++i) {
                ces[i] =
                    adjust_ce_for_search(ces[i], strength, case_1st, case_lvl);
            }

#if BOOST_TEXT_COLLATION_SEARCH_INSTRUMENTATION
            std::cout << "    modified ces: [ ";
            for (auto i = old_ces_size, end = ces.size(); i < end; ++i) {
                std::cout << ces[i] << ' ';
            }
            std::cout << "]\n";

            std::cout << "    ce_sizes appended: [ ";
            for (auto i = old_ce_sizes_size, end = ce_sizes.size(); i < end;
                 ++i) {
                std::cout << ce_sizes[i] << ' ';
            }
            std::cout << "]\n";
#endif
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

        struct search_skip_table
        {
            search_skip_table() : default_() {}

            search_skip_table(
                std::ptrdiff_t pattern_ces, std::ptrdiff_t default_value) :
                default_(default_value),
                map_(pattern_ces)
            {}

            void insert(collation_element key, std::ptrdiff_t value)
            {
                map_[key] = value;
            }

            bool empty() const { return map_.empty(); }

            std::ptrdiff_t operator[](collation_element key) const
            {
                auto const it = map_.find(key);
                return it == map_.end() ? default_ : it->second;
            }

        private:
            using map_t = std::unordered_map<collation_element, std::ptrdiff_t>;

            std::ptrdiff_t default_;
            map_t map_;
        };

#if BOOST_TEXT_COLLATION_SEARCH_INSTRUMENTATION
        struct collation_element_dumper
        {
            template<typename Range>
            collation_element_dumper(Range const & r) :
                first_(&*r.begin()),
                last_(&*r.end())
            {}

            friend std::ostream &
            operator<<(std::ostream & os, collation_element_dumper d)
            {
                for (auto it = d.first_; it != d.last_; ++it) {
                    if (it != d.first_)
                        os << ", ";
                    os << *it;
                }
                return os;
            }

        private:
            collation_element const * first_;
            collation_element const * last_;
        };
#endif

        // O, to have constexpr if!
        template<
            typename CPIter,
            std::size_t N,
            typename StrCEsIter,
            typename PatternCEsIter,
            typename AtBreakFunc,
            typename PopFrontFunc,
            typename PopsFunc>
        text::cp_range<CPIter> search_mismatch_impl(
            CPIter it,
            container::small_vector<collation_element, N> const & pattern_ces,
            std::deque<collation_element> const & str_ces,
            std::deque<int> const & str_ce_sizes,
            StrCEsIter str_ces_first,
            StrCEsIter str_ces_last,
            PatternCEsIter pattern_ces_first,
            AtBreakFunc at_break,
            PopFrontFunc pop_front,
            PopsFunc pops_on_mismatch)
        {
            auto const pattern_length = pattern_ces.size();

#if BOOST_TEXT_COLLATION_SEARCH_INSTRUMENTATION
            std::cout << "Comparing str="
                      << collation_element_dumper(
                             std::vector<collation_element>(
                                 str_ces_first, str_ces_last))
                      << "\n";
            std::cout << "To     substr="
                      << collation_element_dumper(
                             std::vector<collation_element>(
                                 pattern_ces_first,
                                 std::next(pattern_ces_first, pattern_length)))
                      << "\n";
#endif

            auto const mismatch =
                std::mismatch(str_ces_first, str_ces_last, pattern_ces_first);
            if (mismatch.first == str_ces_last) {
#if BOOST_TEXT_COLLATION_SEARCH_INSTRUMENTATION
                std::cout << "*** == ***\n";
#endif
                std::ptrdiff_t remainder = pattern_length;
                auto match_end = it;
                for (auto size : str_ce_sizes) {
                    remainder -= size;
                    ++match_end;
                    if (remainder == 0)
                        break;
                    if (remainder < 0) {
                        match_end = it;
                        break;
                    }
                }
                if (match_end != it && at_break(match_end))
                    return text::cp_range<CPIter>(it, match_end);
                else
                    pop_front(str_ce_sizes.front());
            } else {
#if BOOST_TEXT_COLLATION_SEARCH_INSTRUMENTATION
                std::cout << "*** != ***\n";
#endif
                auto const ces_to_pop = pops_on_mismatch(mismatch, str_ces);
                pop_front(ces_to_pop);
            }

            return text::cp_range<CPIter>(it, it);
        }

        enum class mismatch_dir { fwd, rev };

        template<mismatch_dir MismatchDir>
        struct search_mismatch_t
        {
            template<
                typename CPIter,
                std::size_t N,
                typename AtBreakFunc,
                typename PopFrontFunc,
                typename PopsFunc>
            text::cp_range<CPIter> operator()(
                CPIter it,
                container::small_vector<collation_element, N> const &
                    pattern_ces,
                std::deque<collation_element> const & str_ces,
                std::deque<int> const & str_ce_sizes,
                AtBreakFunc at_break,
                PopFrontFunc pop_front,
                PopsFunc pops_on_mismatch)
            {
                return search_mismatch_impl(
                    it,
                    pattern_ces,
                    str_ces,
                    str_ce_sizes,
                    str_ces.begin(),
                    str_ces.begin() + pattern_ces.size(),
                    pattern_ces.begin(),
                    at_break,
                    pop_front,
                    pops_on_mismatch);
            }
        };

        template<>
        struct search_mismatch_t<mismatch_dir::rev>
        {
            template<
                typename CPIter,
                std::size_t N,
                typename AtBreakFunc,
                typename PopFrontFunc,
                typename PopsFunc>
            text::cp_range<CPIter> operator()(
                CPIter it,
                container::small_vector<collation_element, N> const &
                    pattern_ces,
                std::deque<collation_element> const & str_ces,
                std::deque<int> const & str_ce_sizes,
                AtBreakFunc at_break,
                PopFrontFunc pop_front,
                PopsFunc pops_on_mismatch)
            {
                return search_mismatch_impl(
                    it,
                    pattern_ces,
                    str_ces,
                    str_ce_sizes,
                    str_ces.rend() - pattern_ces.size(),
                    str_ces.rend(),
                    pattern_ces.rbegin(),
                    at_break,
                    pop_front,
                    pops_on_mismatch);
            }
        };

        template<
            mismatch_dir MismatchDir,
            typename CPIter,
            typename BreakFunc,
            std::size_t N,
            typename PopsFunc>
        text::cp_range<CPIter> search_impl(
            CPIter first,
            CPIter last,
            container::small_vector<collation_element, N> const & pattern_ces,
            BreakFunc break_fn,
            collation_table const & table,
            collation_strength strength,
            case_first case_1st,
            case_level case_lvl,
            variable_weighting weighting,
            PopsFunc pops_on_mismatch)
        {
            if (first == last || pattern_ces.empty())
                return text::cp_range<CPIter>(first, first);

            std::deque<collation_element> str_ces;
            std::deque<int> str_ce_sizes;

            auto it = first;

            auto pop_front = [&str_ces, &str_ce_sizes, &it](int ces) {
#if BOOST_TEXT_COLLATION_SEARCH_INSTRUMENTATION
                auto const old_str_ces_size = str_ces.size();
                auto const old_str_ce_sizes_size = str_ce_sizes.size();
#endif
                str_ces.erase(str_ces.begin(), str_ces.begin() + ces);
                while (0 < ces) {
                    ces -= str_ce_sizes.front();
                    str_ce_sizes.pop_front();
                    ++it;
                }
                str_ces.erase(str_ces.begin(), str_ces.begin() - ces);
#if BOOST_TEXT_COLLATION_SEARCH_INSTRUMENTATION
                std::cout << " === Popped "
                          << (old_str_ces_size - str_ces.size()) << " CEs, "
                          << (old_str_ce_sizes_size - str_ce_sizes.size())
                          << " sizes\n";
#endif
            };

            auto at_break = [first, last, break_fn](CPIter it) {
                return break_fn(first, it, last) == it;
            };

            auto const no_match = text::cp_range<CPIter>(last, last);

            while (it != last) {
                if (at_break(it)) {
                    std::ptrdiff_t const pattern_length = pattern_ces.size();
                    std::ptrdiff_t const str_ces_needed_for_search =
                        pattern_length - (std::ptrdiff_t)str_ces.size();
                    // We need to have sufficient lookahead (at least
                    // pattern_length CEs) for the search below to work.
                    if (0 < str_ces_needed_for_search) {
                        auto const append_it =
                            std::next(it, str_ce_sizes.size());
                        append_search_ces_and_sizes(
                            append_it,
                            next_until(
                                append_it, str_ces_needed_for_search, last),
                            last,
                            str_ces,
                            str_ce_sizes,
                            table,
                            strength,
                            case_1st,
                            case_lvl,
                            weighting);
                    }
                    if ((std::ptrdiff_t)str_ces.size() < pattern_length)
                        return no_match;
                    search_mismatch_t<MismatchDir> search_mismatch;
                    auto const result = search_mismatch(
                        it,
                        pattern_ces,
                        str_ces,
                        str_ce_sizes,
                        at_break,
                        pop_front,
                        pops_on_mismatch);
                    if (!result.empty())
                        return result;
                } else {
                    if (!str_ces.empty())
                        pop_front(str_ce_sizes.front());
                    else
                        ++it;
                }
            }

            return no_match;
        }
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
        text::cp_range<CPIter2> operator()(CPIter2 first, CPIter2 last) const
        {
            using mismatch_t = std::pair<
                std::deque<detail::collation_element>::const_iterator,
                container::small_vector<detail::collation_element, 256>::
                    const_iterator>;
            return detail::search_impl<detail::mismatch_dir::fwd>(
                first,
                last,
                pattern_ces_,
                break_fn_,
                table_,
                strength_,
                case_first_,
                case_level_,
                weighting_,
                [this](
                    mismatch_t, std::deque<detail::collation_element> const &) {
                    return 1;
                });
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
            if (pattern_first == pattern_last)
                return;

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
            for (auto it = pattern_ces_.begin(),
                      end = std::prev(pattern_ces_.end());
                 it != end;
                 ++it, ++i) {
                skips_.insert(*it, pattern_ces_.size() - 1 - i);
            }
        }

        template<typename CPIter2>
        text::cp_range<CPIter2> operator()(CPIter2 first, CPIter2 last) const
        {
            using mismatch_t = std::pair<
                std::deque<detail::collation_element>::const_reverse_iterator,
                ces_t::const_reverse_iterator>;
            return detail::search_impl<detail::mismatch_dir::rev>(
                first,
                last,
                pattern_ces_,
                break_fn_,
                table_,
                strength_,
                case_first_,
                case_level_,
                weighting_,
                [this](
                    mismatch_t,
                    std::deque<detail::collation_element> const & str_ces) {
                    return skips_[str_ces[pattern_ces_.size() - 1]];
                });
        }

    private:
        using ces_t = container::small_vector<detail::collation_element, 256>;

        collation_table table_;
        collation_strength strength_;
        case_first case_first_;
        case_level case_level_;
        variable_weighting weighting_;
        detail::search_skip_table skips_;
        ces_t pattern_ces_;
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

    /** TODO */
    template<typename CPIter, typename BreakFunc>
    struct boyer_moore_collation_searcher
    {
        boyer_moore_collation_searcher(
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

            skips_ = detail::search_skip_table(pattern_ces_.size(), -1);

            std::ptrdiff_t i = 0;
            for (auto ce : pattern_ces_) {
                skips_.insert(ce, i++);
            }

            build_suffix_table();
        }

        template<typename CPIter2>
        text::cp_range<CPIter2> operator()(CPIter2 first, CPIter2 last) const
        {
            using mismatch_t = std::pair<
                std::deque<detail::collation_element>::const_reverse_iterator,
                ces_t::const_reverse_iterator>;
            return detail::search_impl<detail::mismatch_dir::rev>(
                first,
                last,
                pattern_ces_,
                break_fn_,
                table_,
                strength_,
                case_first_,
                case_level_,
                weighting_,
                [this](
                    mismatch_t mismatch,
                    std::deque<detail::collation_element> const & str_ces) {
                    auto const skip_lookup = skips_[*mismatch.first];
                    auto const mismatch_index = str_ces.rend() - mismatch.first;
                    auto const m = mismatch_index - skip_lookup - 1;
                    auto const mismatch_suffix = suffixes_[mismatch_index];
                    if (skip_lookup < mismatch_index && mismatch_suffix < m) {
                        return m;
                    } else {
                        return mismatch_suffix;
                    }
                });
        }

    private:
        using ces_t = container::small_vector<detail::collation_element, 256>;
        using ces_iter = ces_t::iterator;

        template<typename CEIter>
        std::vector<std::ptrdiff_t> compute_prefixes(CEIter first)
        {
            std::vector<std::ptrdiff_t> retval(pattern_ces_.size());

            retval[0] = 0;
            std::size_t k = 0;
            for (std::size_t i = 1, end = retval.size(); i < end; ++i) {
                assert(k < end);
                while (0 < k && first[k] != first[i]) {
                    assert(k < end);
                    k = retval[k - 1];
                }
                if (first[k] == first[i])
                    ++k;
                retval[i] = k;
            }

            return retval;
        }

        void build_suffix_table()
        {
            if (pattern_ces_.empty())
                return;

            std::ptrdiff_t const pattern_size = pattern_ces_.size();
            suffixes_.resize(pattern_size + 1);

            std::vector<std::ptrdiff_t> const prefixes =
                compute_prefixes(pattern_ces_.begin());
            std::vector<std::ptrdiff_t> const prefixes_reversed =
                compute_prefixes(pattern_ces_.rbegin());

            std::fill(
                suffixes_.begin(),
                suffixes_.end(),
                pattern_size - prefixes[pattern_size - 1]);

            for (std::ptrdiff_t i = 0; i < pattern_size; ++i) {
                auto const reversed_i = prefixes_reversed[i];
                auto const j = pattern_size - reversed_i;
                auto const k = i - reversed_i + 1;

                if (k < suffixes_[j])
                    suffixes_[j] = k;
            }
        }

        collation_table table_;
        collation_strength strength_;
        case_first case_first_;
        case_level case_level_;
        variable_weighting weighting_;
        detail::search_skip_table skips_;
        std::vector<std::ptrdiff_t> suffixes_;
        ces_t pattern_ces_;
        BreakFunc break_fn_;
    };

    /** TODO */
    template<typename CPIter>
    boyer_moore_collation_searcher<
        CPIter,
        detail::dummy_prev_break<CPIter, CPIter>>
    make_boyer_moore_collation_searcher(
        CPIter first,
        CPIter last,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
    {
        return boyer_moore_collation_searcher<
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
    boyer_moore_collation_searcher<CPIter, BreakFunc>
    make_boyer_moore_collation_searcher(
        CPIter first,
        CPIter last,
        BreakFunc break_fn,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
    {
        return boyer_moore_collation_searcher<CPIter, BreakFunc>(
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
    auto make_boyer_moore_collation_searcher(
        CPRange r,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
        -> boyer_moore_collation_searcher<
            decltype(std::begin(r)),
            detail::dummy_prev_break<
                decltype(std::begin(r)),
                decltype(std::begin(r))>>
    {
        using std::begin;
        using std::end;
        using r_iter = decltype(std::begin(r));
        return boyer_moore_collation_searcher<
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
    auto make_boyer_moore_collation_searcher(
        CPRange r,
        BreakFunc break_fn,
        collation_table const & table,
        collation_strength strength = collation_strength::tertiary,
        case_first case_1st = case_first::off,
        case_level case_lvl = case_level::off,
        variable_weighting weighting = variable_weighting::non_ignorable)
        -> boyer_moore_collation_searcher<decltype(std::begin(r)), BreakFunc>
    {
        using std::begin;
        using std::end;
        using r_iter = decltype(std::begin(r));
        return boyer_moore_collation_searcher<r_iter, BreakFunc>(
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
    cp_range<CPIter1> collation_search(
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
        -> cp_range<decltype(std::begin(str))>
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
    cp_range<CPIter1> collation_search(
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
        -> cp_range<decltype(std::begin(str))>
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

    // TODO: Add convenience functions for Boyer-Moore and Boyer-Moore-Horspool.

}}

#endif
