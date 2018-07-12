#ifndef BOOST_TEXT_CASE_MAPPING_HPP
#define BOOST_TEXT_CASE_MAPPING_HPP

#include <boost/text/word_break.hpp>

#include <boost/text/detail/case_constants.hpp>
#include <boost/text/detail/case_mapping_data.hpp>
#include <boost/text/detail/normalization_data.hpp>

#include <numeric>


namespace boost { namespace text {

    // TODO: Consider adding support for Dutch IJ and other cases supported by ICU.

    /** TODO */
    enum class case_language : uint16_t {
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
        inline uint16_t lang_to_condition(case_language lang) noexcept
        {
            switch (lang) {
            case case_language::azerbaijani:
                return (uint16_t)detail::case_condition::az;
            case case_language::lithuanian:
                return (uint16_t)detail::case_condition::lt;
            case case_language::turkish:
                return (uint16_t)detail::case_condition::tr;
            default: return 0;
            }
            return 0;
        }

        inline bool ccc_230_0(uint32_t cp) noexcept
        {
            auto const ccc_ = ccc(cp);
            return ccc_ == 230 || ccc_ == 0;
        }

        enum class title_state_t { none, before, after };

        template<typename CPIter, typename Sentinel, typename OutIter>
        OutIter map_case(
            CPIter first,
            CPIter it,
            Sentinel last,
            OutIter out,
            case_language lang,
            case_map_t const & map,
            bool title) noexcept
        {
            uint16_t const lang_conditions = lang_to_condition(lang);

            title_state_t title_state =
                title ? title_state_t::before : title_state_t::none;

            while (it != last) {
                auto const map_it = title_state == title_state_t::after
                                        ? to_lower_map().find(*it)
                                        : map.find(*it);

                uint16_t conditions = lang_conditions;

                if (map_it != map.end()) {
                    if (title_state == title_state_t::before)
                        title_state = title_state_t::after;

                    case_elements const elements = map_it->second;
                    auto const elem_first =
                        elements.begin(case_mapping_to_ptr());
                    auto const elem_last = elements.end(case_mapping_to_ptr());

                    auto const all_conditions = std::accumulate(
                        elem_first,
                        elem_last,
                        uint16_t(0),
                        [](uint16_t conds, case_mapping_to to) {
                            return conds | to.conditions_;
                        });

                    auto const condition_first = std::next(it);

                    if (all_conditions &
                        (uint16_t)case_condition::Final_Sigma) {
                        bool before = false;
                        auto find_it =
                            find_if_backward(first, it, [](uint32_t cp) {
                                return !case_ignorable(cp);
                            });
                        if (find_it != it) {
                            if (cased(*find_it))
                                before = true;
                        }

                        if (before) {
                            bool after = false;
                            auto find_it =
                                find_if(condition_first, last, [](uint32_t cp) {
                                    return !case_ignorable(cp);
                                });
                            if (find_it != last) {
                                if (cased(*find_it))
                                    after = true;
                            }
                            if (!after) {
                                conditions |=
                                    (uint16_t)case_condition::Final_Sigma;
                            }
                        }
                    }

                    if (all_conditions &
                        (uint16_t)case_condition::After_Soft_Dotted) {
                        auto find_it =
                            find_if_backward(first, it, [](uint32_t cp) {
                                return ccc_230_0(cp);
                            });
                        if (find_it != it) {
                            if (soft_dotted(*find_it)) {
                                conditions |=
                                    (uint16_t)case_condition::After_Soft_Dotted;
                            }
                        }
                    }

                    if (all_conditions & (uint16_t)case_condition::More_Above) {
                        if (condition_first != last &&
                            ccc(*condition_first) == 230) {
                            conditions |= (uint16_t)case_condition::More_Above;
                        } else {
                            auto find_it =
                                find_if(condition_first, last, [](uint32_t cp) {
                                    return ccc_230_0(cp);
                                });
                            if (find_it != last) {
                                if (ccc(*find_it) == 230) {
                                    conditions |=
                                        (uint16_t)case_condition::More_Above;
                                }
                            }
                        }
                    }

                    if (all_conditions &
                        (uint16_t)case_condition::Not_Before_Dot) {
                        bool before = false;
                        auto find_it =
                            find_if(condition_first, last, [](uint32_t cp) {
                                return ccc_230_0(cp);
                            });
                        if (find_it != last) {
                            if (*find_it == 0x0307)
                                before = true;
                        }
                        if (!before) {
                            conditions |=
                                (uint16_t)case_condition::Not_Before_Dot;
                        }
                    }

                    if (all_conditions & (uint16_t)case_condition::After_I) {
                        auto find_it =
                            find_if_backward(first, it, [](uint32_t cp) {
                                return ccc_230_0(cp);
                            });
                        if (find_it != it) {
                            if (*find_it == 0x0049)
                                conditions |= (uint16_t)case_condition::After_I;
                        }
                    }

                    bool met_conditions = false;
                    for (auto elem_it = elem_first; elem_it != elem_last;
                         ++elem_it) {
                        // This condition only works (that is, it correctly
                        // prefers matches with conditions) because the data
                        // generation puts the condition=0 elements at the
                        // end.
                        if ((elem_it->conditions_ & conditions) ==
                            elem_it->conditions_) {
                            auto const cp_first =
                                case_cps_ptr() + elem_it->first_;
                            auto const cp_last =
                                case_cps_ptr() + elem_it->last_;
                            out = std::copy(cp_first, cp_last, out);
                            it = condition_first;
                            met_conditions = true;
                            break;
                        }
                    }
                    if (met_conditions)
                        continue;
                }

                *out = *it;
                ++out;
                ++it;
            }

            return out;
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
        return map_case(
            first, it, last, out, lang, detail::to_lower_map(), false);
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

        for (auto r : words) {
            out = map_case(
                first,
                r.begin(),
                r.end(),
                out,
                lang,
                detail::to_title_map(),
                true);
        }

        return out;
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
        return map_case(
            first, it, last, out, lang, detail::to_upper_map(), false);
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
