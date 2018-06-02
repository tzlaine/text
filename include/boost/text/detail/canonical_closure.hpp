#ifndef BOOST_TEXT_DETAIL_CANONICAL_CLOSURE_HPP
#define BOOST_TEXT_DETAIL_CANONICAL_CLOSURE_HPP

#include <boost/text/normalize.hpp>

#include <boost/container/small_vector.hpp>

#include <algorithm>


namespace boost { namespace text {

    namespace detail {

        inline bool
        appears_at_noninitial_position_of_decomp(uint32_t cp) noexcept
        {
            return appears_at_noninitial_position_of_decomp_set().count(cp);
        }

        using canonical_closure_string_t =
            boost::container::small_vector<uint32_t, 32>;
        using canonical_closure_buffer_t =
            boost::container::small_vector<uint32_t, 64>;

        using canonical_closure_subsegments_elem_t =
            boost::container::small_vector<canonical_closure_string_t, 8>;
        using canonical_closure_subsegments_t = boost::container::
            small_vector<canonical_closure_subsegments_elem_t, 32>;

#if 0
        template<typename Seq>
        void dump_(Seq s)
        {
            for (auto cp : s) {
                std::cout << std::hex << std::setw(4) << std::setfill('0') << cp
                          << " ";
            }
        }
#endif

        inline bool canonical_closure_starter(uint32_t cp) noexcept
        {
            return !ccc(cp) && !appears_at_noninitial_position_of_decomp(cp);
        }

        template<typename OutIter>
        OutIter canonical_closure_combinations(
            canonical_closure_subsegments_t const & subsegments,
            int total,
            OutIter out)
        {
            // First is the total count, second is the current index.
            boost::container::small_vector<std::pair<int, int>, 32> counters(
                subsegments.size());
            for (auto i = 0, end = (int)counters.size(); i < end; ++i) {
                counters[i].first = (int)subsegments[i].size();
            }

            for (auto i = 0; i < total; ++i) {
                canonical_closure_string_t string;
                for (auto i = 0, end = (int)subsegments.size(); i < end; ++i) {
                    auto const & seg = subsegments[i][counters[i].second];
                    string.insert(string.end(), seg.begin(), seg.end());
                }

                *out = string;
                ++out;

                for (auto counters_it = counters.end();
                     counters_it-- != counters.begin();) {
                    if (++counters_it->second == counters_it->first)
                        counters_it->second = 0;
                    else
                        break;
                }
            }

            return out;
        }

        template<typename CPIter, typename OutIter>
        OutIter
        segment_canonical_closure(CPIter first, CPIter last, OutIter out) noexcept
        {
            assert(first != last);

            // 3 For each segment enumerate canonically equivalent forms, as
            // follows:

            if (!canonical_closure_starter(*first))
                return out;

            // 3a Use the set of [composed] characters whose decomposition
            // begins with the segment's starter.
            canonical_closure_buffer_t comps;
            {
                auto const equal_range =
                    compositions_whose_decompositions_start_with_cp_map()
                        .equal_range(*first);
                std::transform(
                    equal_range.first,
                    equal_range.second,
                    std::back_inserter(comps),
                    [](std::pair<uint32_t, uint32_t> pair) {
                        return pair.second;
                    });
            }

            bool did_output = false;

            // 3b For each character in this set:
            for (auto comp : comps) {
                // 3b I Get the character's decomposition.
                canonical_decomposition decomp = canonical_decompose(comp);

                bool skip = false;
                for (auto decomp_cp : decomp) {
                    // 3b II If the decomposition contains characters that are
                    // not in the segment, then skip this character.
                    auto same_cp_in_seg_it = std::find(first, last, decomp_cp);
                    if (same_cp_in_seg_it == last){
                        skip = true;
                        break;
                    }

                    // 3b III If the decomposition contains a character that
                    // is blocked in the segment (preceded by a combining mark
                    // with the same combining class), then also skip this
                    // character.
                    if (same_cp_in_seg_it != first) {
                        auto const decomp_cp_ccc = ccc(decomp_cp);
                        if (ccc(*std::prev(same_cp_in_seg_it)) ==
                            decomp_cp_ccc) {
                            skip = true;
                            break;
                        }
                    }
                }
                if (skip)
                    continue;

                // 3b IV Otherwise, start building a new string with this
                // character.

                // Note that here we don't actually create a string per se,
                // but instead make an initial subsegment from the character.
                // This is because we collapse the string creating and
                // permutation into a single recursive approach below.
                canonical_closure_subsegments_t subsegments(1);
                subsegments.back().push_back(
                    canonical_closure_string_t(1, comp));
                subsegments.back().push_back(
                    canonical_closure_string_t(decomp.begin(), decomp.end()));

                // 3b V Append all characters from the input segment that are
                // not in this character's decomposition in canonical order.
                for (auto input_it = std::next(first), end = last;
                     input_it != end;
                     ++input_it) {
                    if (std::find(decomp.begin(), decomp.end(), *input_it) ==
                        decomp.end()) {
                        subsegments.resize(subsegments.size() + 1);
                        subsegments.back().push_back(
                            canonical_closure_string_t(1, *input_it));
                    }
                }

                // 3b VI Add this string to the set of canonical equivalents
                // for the current segment.

                // See below

                // 3b VII Recurse: Treat all but the initial character of this
                // new string as a segment and add to the set for the current
                // segment all combinations of the initial character and the
                // equivalent strings of the rest.

                // Even though 3b VI and 3b VII respectively indicate that we
                // should add the original string and its recursive
                // permutations, we can treat them uniformly here, and this is
                // simpler.

                auto total = 1;
                for (auto i = 0, end = (int)subsegments.size(); i != end; ++i) {
                    if (!i) {
                        total *= subsegments[0].size();
                        continue;
                    }

                    // We might have multiple alternatives here, if they were
                    // collapsed together above for being ccc-equal.
                    for (auto j = 0, j_end = (int)subsegments[i].size();
                         j != j_end;
                         ++j) {
                        // Find other equivalents.
                        uint32_t new_c[1] = {subsegments[i][j][0]};
                        canonical_closure_string_t nfd;
                        normalize_to_nfd(
                            new_c, new_c + 1, std::back_inserter(nfd));
                        segment_canonical_closure(
                            nfd.begin(),
                            nfd.end(),
                            std::back_inserter(subsegments[i]));
                    }
                    total *= subsegments[i].size();
                }
                assert(total);

                did_output = true;
                out = canonical_closure_combinations(subsegments, total, out);
            }

            if (!did_output) {
                canonical_closure_string_t str(first, last);

                std::sort(std::next(str.begin()), str.end());
                do {
                    canonical_closure_subsegments_t subsegments;
                    std::transform(
                        str.begin(),
                        str.end(),
                        std::back_inserter(subsegments),
                        [](uint32_t cp) {
                            return canonical_closure_subsegments_elem_t(
                                1, canonical_closure_string_t(1, cp));
                        });

                    auto total = 1;
                    for (auto i = 0, end = (int)subsegments.size(); i != end;
                         ++i) {
                        if (!i) {
                            total *= subsegments[0].size();
                            continue;
                        }

                        // We might have multiple alternatives here, if they
                        // were collapsed together above for being ccc-equal.
                        for (auto j = 0, j_end = (int)subsegments[i].size();
                             j != j_end;
                             ++j) {
                            // Find other equivalents.
                            uint32_t new_c[1] = {subsegments[i][j][0]};
                            canonical_closure_string_t nfd;
                            normalize_to_nfd(
                                new_c, new_c + 1, std::back_inserter(nfd));
                            segment_canonical_closure(
                                nfd.begin(),
                                nfd.end(),
                                std::back_inserter(subsegments[i]));
                        }
                        total *= subsegments[i].size();
                    }
                    assert(total);

                    out =
                        canonical_closure_combinations(subsegments, total, out);
                } while (
                    std::next_permutation(std::next(str.begin()), str.end()));
            }

            return out;
        }

        // Note that this always produces results in NFD.
        template<typename CPIter, typename OutIter>
        OutIter
        canonical_closure(CPIter first, CPIter last, OutIter out) noexcept
        {
            // https://www.unicode.org/notes/tn5/#Enumerating_Equivalent_Strings

            assert(first != last);

            // 1 Transform the input string into its NFD form.
            canonical_closure_buffer_t nfd;
            normalize_to_nfd(first, last, std::back_inserter(nfd));

            assert(canonical_closure_starter(nfd[0]));

            // 2 Partition the string into segments, with each starter
            // character in the string at the beginning of a segment.  (A
            // starter in this sense has combining class 0 and does not appear
            // in non-initial position of any other character's
            // decomposition.)

            canonical_closure_subsegments_t segment_results;
            auto it = nfd.begin();
            while (it != nfd.end()) {
                auto next = std::find_if(
                    std::next(it), nfd.end(), canonical_closure_starter);
                segment_results.resize(segment_results.size() + 1);
                segment_canonical_closure(
                    it, next, std::back_inserter(segment_results.back()));
                it = next;
            }

            // 4 Enumerate the combinations of all forms of all segments.
            auto total = 1;
            for (auto & seg : segment_results) {
                std::sort(seg.begin(), seg.end());
                seg.erase(std::unique(seg.begin(), seg.end()), seg.end());
                total *= seg.size();
            }
            assert(total);

            return canonical_closure_combinations(segment_results, total, out);
        }

        // TODO: Needed (converts to fcc form + sort + unique)? fcc_canonical_closure()

}}}

#endif
