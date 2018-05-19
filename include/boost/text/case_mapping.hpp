#ifndef BOOST_TEXT_CASE_MAPPING_HPP
#define BOOST_TEXT_CASE_MAPPING_HPP

#include <boost/text/word_break.hpp>

#include <boost/text/detail/case_constants.hpp>
#include <boost/text/detail/case_mapping_data.hpp>
#include <boost/text/detail/normalization_data.hpp>


namespace boost { namespace text {

    /** TODO */
    enum class case_language : uint8_t {
        other,
        azerbaijani,
        lithuanian,
        turkish
    };

    /** TODO */
    struct next_word_break_callable
    {
        template<typename CPIter, typename Sentinel>
        auto operator()(CPIter it, Sentinel last) noexcept
            -> detail::cp_iter_ret_t<CPIter, CPIter>
        {
            return next_word_break(it, last);
        }
    };

    namespace detail {
        inline uint8_t lang_to_condition(case_language lang) noexcept
        {
            switch (lang) {
            case case_language::azerbaijani:
                return (uint8_t)detail::case_condition::az;
            case case_language::lithuanian:
                return (uint8_t)detail::case_condition::lt;
            case case_language::turkish:
                return (uint8_t)detail::case_condition::tr;
            default: return 0;
            }
            return 0;
        }

        inline bool ccc_230_0(uint32_t cp) noexcept
        {
            auto const ccc_ = ccc(cp);
            return ccc_ == 230 || ccc_ == 0;
        }

        template<typename OutIter>
        struct map_case_state_t
        {
            bool final_sigma_before;
            uint8_t after_soft_dotted;
            uint8_t after_i;
            OutIter out;
        };

        // Look backward as necessary to determine starting conditions.
        template<typename CPIter, typename OutIter>
        map_case_state_t<OutIter>
        map_case_state(CPIter first, CPIter it, OutIter out)
        {
            map_case_state_t<OutIter> retval{false, 0, 0, out};

            auto find_it = find_if_backward(first, it, [](uint32_t cp) {
                return cased(cp) || !case_ignorable(cp);
            });
            if (find_it != first) {
                --find_it;
                if (cased(*find_it))
                    retval.final_sigma_before = true;
            }

            find_it = find_if_backward(first, it, [](uint32_t cp) {
                return soft_dotted(cp) || ccc_230_0(cp);
            });
            if (find_it != first) {
                --find_it;
                if (soft_dotted(*find_it)) {
                    retval.after_soft_dotted =
                        (uint8_t)case_condition::After_Soft_Dotted;
                }
            }

            find_it = find_if_backward(first, it, [](uint32_t cp) {
                return cp == 0x0049 || ccc_230_0(cp);
            });
            if (find_it != first) {
                --find_it;
                if (*find_it == 0x0049)
                    retval.after_i = (uint8_t)case_condition::After_I;
            }

            return retval;
        }

        template<typename CPIter, typename Sentinel, typename OutIter>
        map_case_state_t<OutIter> map_case(
            CPIter first,
            CPIter it,
            Sentinel last,
            map_case_state_t<OutIter> state,
            case_language lang,
            case_trie_t const & trie,
            bool title) noexcept
        {
            uint8_t const lang_conditions = lang_to_condition(lang);

            auto update_before_states = [&](uint32_t cp) {
                if (cased(cp) ||
                    (state.final_sigma_before && case_ignorable(cp))) {
                    state.final_sigma_before = true;
                } else {
                    state.final_sigma_before = false;
                }
                if (soft_dotted(cp) ||
                    (state.after_soft_dotted && !ccc_230_0(cp))) {
                    state.after_soft_dotted =
                        (uint8_t)case_condition::After_Soft_Dotted;
                } else {
                    state.after_soft_dotted = 0;
                }
                if (cp == 0x0049 || (state.after_i && !ccc_230_0(cp))) {
                    state.after_i = (uint8_t)case_condition::After_I;
                } else {
                    state.after_i = 0;
                }
            };

            for (; it != last; ++it) {
                case_trie_match_t const match = trie.longest_match(it, last);

                uint8_t conditions =
                    lang_conditions | state.after_soft_dotted | state.after_i;

                bool used_match = false;
                if (match.match) {
                    case_elements const elements = trie[match];
                    auto const elem_first = elements.begin(g_case_cps_first);
                    auto const elem_last = elements.end(g_case_cps_first);
                    auto const condition_first =
                        std::next(it, elem_last - elem_first);

                    if (elements.conditions_ &
                        (uint8_t)case_condition::Final_Sigma) {
                        // TODO: Cache the found boundary for later, for this
                        // and the other two after-conditions below.
                        bool after = false;
                        auto find_it =
                            find_if(condition_first, last, [](uint32_t cp) {
                                return cased(cp) || case_ignorable(cp);
                            });
                        if (find_it != last) {
                            ++find_it;
                            if (cased(*find_it))
                                after = true;
                        }
                        if (!after)
                            conditions |= (uint8_t)case_condition::Final_Sigma;
                    }

                    if (elements.conditions_ &
                        (uint8_t)case_condition::More_Above) {
                        if (condition_first != last &&
                            ccc(*condition_first) == 230) {
                            conditions |= (uint8_t)case_condition::More_Above;
                        } else {
                            auto find_it =
                                find_if(condition_first, last, [](uint32_t cp) {
                                    return ccc_230_0(cp);
                                });
                            if (find_it != last) {
                                ++find_it;
                                if (ccc(*find_it) == 230) {
                                    conditions |=
                                        (uint8_t)case_condition::More_Above;
                                }
                            }
                        }
                    }

                    if (elements.conditions_ &
                        (uint8_t)case_condition::Not_Before_Dot) {
                        auto find_it =
                            find_if(condition_first, last, [](uint32_t cp) {
                                return cp == 0x0307 || ccc_230_0(cp);
                            });
                        if (find_it != last) {
                            ++find_it;
                            if (*find_it == 0x0307) {
                                conditions |=
                                    (uint8_t)case_condition::Not_Before_Dot;
                            }
                        }
                    }

                    if ((elements.conditions_ & conditions) ==
                        elements.conditions_) {
                        used_match = true;
                        for (auto elem_it = elem_first; elem_it != elem_last;
                             ++elem_it) {
                            update_before_states(*elem_it);
                        }
                        state.out = std::copy(elem_first, elem_last, state.out);
                    }
                }

                if (!used_match) {
                    auto const cp = *it;
                    update_before_states(cp);
                    *state.out = *it;
                    ++state.out;
                }
            }

            return state;
        }
    }

    /** TODO */
    template<typename CPIter, typename Sentinel, typename OutIter>
    OutIter to_lower(
        CPIter first,
        CPIter it,
        Sentinel last,
        OutIter out,
        case_language lang = case_language::other) noexcept
    {
        auto state = detail::map_case_state(first, it, out);
        state = map_case(first, it, last, state, lang, detail::to_lower_trie());
        return state.out;
    }

    /** TODO */
    template<typename CPRange, typename OutIter>
    OutIter to_lower(
        CPRange & range,
        OutIter out,
        case_language lang = case_language::other) noexcept
    {
        using std::begin;
        using std::end;
        return to_lower(begin(range), begin(range), end(range), out, lang);
    }

    /** TODO */
    template<
        typename CPIter,
        typename Sentinel,
        typename OutIter,
        typename NextWordBreakFunc = next_word_break_callable>
    OutIter to_title(
        CPIter first,
        CPIter it,
        Sentinel last,
        OutIter out,
        case_language lang = case_language::other,
        NextWordBreakFunc && next_word_break = NextWordBreakFunc{}) noexcept
    {
        lazy_segment_range<CPIter, Sentinel, NextWordBreakFunc> words{
            {it, last}, {last}};

        auto state = detail::map_case_state(first, it, out);
        for (auto r : words) {
            state = map_case(
                first,
                r.begin(),
                r.end(),
                state,
                lang,
                detail::to_title_trie(),
                true);
        }

        return state.out;
    }

    /** TODO */
    template<
        typename CPRange,
        typename OutIter,
        typename NextWordBreakFunc = next_word_break_callable>
    OutIter to_title(
        CPRange & range,
        OutIter out,
        case_language lang = case_language::other,
        NextWordBreakFunc && next_word_break = NextWordBreakFunc{}) noexcept
    {
        using std::begin;
        using std::end;
        return to_title(
            begin(range),
            begin(range),
            end(range),
            out,
            lang,
            static_cast<NextWordBreakFunc &&>(next_word_break));
    }

    /** TODO */
    template<typename CPIter, typename Sentinel, typename OutIter>
    OutIter to_upper(
        CPIter first,
        CPIter it,
        Sentinel last,
        OutIter out,
        case_language lang = case_language::other) noexcept
    {
        auto state = detail::map_case_state(first, it, out);
        state = map_case(first, it, last, state, lang, detail::to_upper_trie());
        return state.out;
    }

    /** TODO */
    template<typename CPRange, typename OutIter>
    OutIter to_upper(
        CPRange & range,
        OutIter out,
        case_language lang = case_language::other) noexcept
    {
        using std::begin;
        using std::end;
        return to_upper(begin(range), begin(range), end(range), out, lang);
    }

}}

#endif
