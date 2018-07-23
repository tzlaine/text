#ifndef BOOST_TEXT_CASE_MAPPING_HPP
#define BOOST_TEXT_CASE_MAPPING_HPP

#include <boost/text/word_break.hpp>

#include <boost/text/detail/case_constants.hpp>
#include <boost/text/detail/case_mapping_data.hpp>
#include <boost/text/detail/normalization_data.hpp>

#include <numeric>


namespace boost { namespace text {

    /** The list of languages handled specially in the case mapping
        functions. */
    enum class case_language : uint16_t {
        other,
        azerbaijani,
        lithuanian,
        turkish
    };

    /** A callable type that returns the next possible word break in [first,
        last).  This is the default line break callable type used with the
        case mapping functions. */
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

    /** Returns true if no code point in [first, last) would change in a call
        to to_lower(), and false otherwise. */
    template<typename CPIter, typename Sentinel>
    bool is_lower(CPIter first, Sentinel last) noexcept
    {
        return all_of(first, last, [](uint32_t cp) {
            return !detail::changes_when_lowered(cp);
        });
    }

    /** Returns true if no code point in range would change in a call to
        to_lower(), and false otherwise. */
    template<typename CPRange>
    bool is_lower(CPRange & range) noexcept
    {
        return is_lower(std::begin(range), std::end(range));
    }

    /** Writes the lower-case form of [first, last) to output iterator out,
        using language-specific handling as indicated by lang. */
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

    /** Writes the lower-case form of range to output iterator out, using
        language-specific handling as indicated by lang. */
    template<typename CPRange, typename OutIter>
    OutIter to_lower(
        CPRange & range,
        OutIter out,
        case_language lang = case_language::other) noexcept
    {
        return to_lower(
            std::begin(range), std::begin(range), std::end(range), out, lang);
    }

    /** Returns true if no code point in [first, last) would change in a call
        to to_title(), and false otherwise. */
    template<
        typename CPIter,
        typename Sentinel,
        typename NextWordBreakFunc = next_word_break_callable>
    bool is_title(
        CPIter first,
        Sentinel last,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{}) noexcept
    {
        lazy_segment_range<CPIter, Sentinel, NextWordBreakFunc> words{
            {first, last}, {last}};
        for (auto r : words) {
            auto it = r.begin();
            if (detail::changes_when_titled(*it))
                return false;
            if (!is_lower(++it, r.end()))
                return false;
        }
        return true;
    }

    /** Returns true if no code point in range would change in a call to
        to_title(), and false otherwise. */
    template<
        typename CPRange,
        typename NextWordBreakFunc = next_word_break_callable>
    bool is_title(
        CPRange & range,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{}) noexcept
    {
        return is_title(std::begin(range), std::end(range), next_word_break);
    }

    /** Writes the title-case form of [first, last) to output iterator out,
        using language-specific handling as indicated by lang. */
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
        NextWordBreakFunc next_word_break = NextWordBreakFunc{}) noexcept
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

    /** Writes the title-case form of range to output iterator out, using
        language-specific handling as indicated by lang. */
    template<
        typename CPRange,
        typename OutIter,
        typename NextWordBreakFunc = next_word_break_callable>
    OutIter to_title(
        CPRange & range,
        OutIter out,
        case_language lang = case_language::other,
        NextWordBreakFunc next_word_break = NextWordBreakFunc{}) noexcept
    {
        return to_title(
            std::begin(range),
            std::begin(range),
            std::end(range),
            out,
            lang,
            next_word_break);
    }

    /** Returns true if no code point in [first, last) would change in a call
        to to_upper(), and false otherwise. */
    template<typename CPIter, typename Sentinel>
    bool is_upper(CPIter first, Sentinel last) noexcept
    {
        return all_of(first, last, [](uint32_t cp) {
            return !detail::changes_when_uppered(cp);
        });
    }

    /** Returns true if no code point in range would change in a call to
        to_upper(), and false otherwise. */
    template<typename CPRange>
    bool is_upper(CPRange & range) noexcept
    {
        return is_upper(std::begin(range), std::end(range));
    }

    /** Writes the upper-case form of [first, last) to output iterator out,
        using language-specific handling as indicated by lang. */
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

    /** Writes the upper-case form of range to output iterator out, using
        language-specific handling as indicated by lang. */
    template<typename CPRange, typename OutIter>
    OutIter to_upper(
        CPRange & range,
        OutIter out,
        case_language lang = case_language::other) noexcept
    {
        return to_upper(
            std::begin(range), std::begin(range), std::end(range), out, lang);
    }

}}

#endif
