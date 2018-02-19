#ifndef BOOST_TEXT_BIDIRECTIONAL_HPP
#define BOOST_TEXT_BIDIRECTIONAL_HPP

#include <boost/text/paragraph_break.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>

#include <stack>


namespace boost { namespace text {

    /** The bidirectional algorithm character properties outlined in Unicode
        10. */
    enum class bidi_prop_t {
        L,
        R,
        EN,
        ES,
        ET,
        AN,
        CS,
        B,
        S,
        WS,
        ON,
        BN,
        NSM,
        AL,
        LRO,
        RLO,
        LRE,
        RLE,
        PDF,
        LRI,
        RLI,
        FSI,
        PDI
    };

    /** Returns the bidirectional algorithm character property associated with
        code point \a cp. */
    bidi_prop_t bidi_prop(uint32_t cp) noexcept;

    namespace detail {
        inline bool isolate_initiator(bidi_prop_t prop) noexcept
        {
            return prop == bidi_prop_t::LRI || prop == bidi_prop_t::RLI ||
                   prop == bidi_prop_t::FSI;
        }

        template<typename CPIter>
        CPIter matching_pdi(CPIter it, CPIter last) noexcept
        {
            if (it == last)
                return last;

            assert(isolate_initiator(bidi_prop(*it)));

            int iis = 1;
            while (++it != last) {
                auto const prop = bidi_prop(*it);
                if (isolate_initiator(prop)) {
                    ++iis;
                } else if (prop == bidi_prop_t::PDI) {
                    --iis;
                    if (!iis)
                        break;
                }
            }

            return it;
        }

        inline bool embedding_initiator(bidi_prop_t prop) noexcept
        {
            return prop == bidi_prop_t::LRE || prop == bidi_prop_t::RLE ||
                   prop == bidi_prop_t::LRO || prop == bidi_prop_t::RLO;
        }

        // TODO: Needed?
        template<typename CPIter>
        CPIter matching_pdf(CPIter it, CPIter last) noexcept
        {
            if (it == last)
                return last;

            assert(embedding_initiator(bidi_prop(*it)));

            int eis = 1;
            while (++it != last) {
                auto prop = bidi_prop(*it);
                if (isolate_initiator(prop)) {
                    it = matching_pdi(it, last);
                    if (it != last)
                        prop = bidi_prop(*it);
                }
                if (embedding_initiator(prop)) {
                    ++eis;
                } else if (prop == bidi_prop_t::PDF) {
                    --eis;
                    if (!eis)
                        break;
                }
            }

            return it;
        }

        enum class directional_override_t {
            neutral,
            right_to_left,
            left_to_right
        };

        // TODO
        struct bidi_state_t
        {
            int embedding_;
            directional_override_t directional_override_;
            bool directional_isolate_;
        };

        int const bidi_max_depth = 125;

        template<typename CPIter>
        int p2_p3(CPIter para_it, CPIter para_last) noexcept
        {
            // https://unicode.org/reports/tr9/#P2
            auto retval = 0;
            {
                auto it = para_it;
                while (it != para_last) {
                    auto const prop = bidi_prop(*it);
                    if (isolate_initiator(prop)) {
                        it = matching_pdi(it, para_last);
                        if (it == para_last)
                            break;
                        ++it;
                    } else if (
                        prop == bidi_prop_t::L || prop == bidi_prop_t::AL ||
                        prop == bidi_prop_t::R) {
                        // https://unicode.org/reports/tr9/#P3
                        retval = 1;
                    }
                }
            }
            return retval;
        }
    }

    // value_type of out below, TBD.
    template<typename CPIter>
    struct TODO
    {
    };

    /** TODO */
    template<typename CPIter, typename OutIter>
    OutIter bidirectional_order(CPIter first, CPIter last, OutIter out)
    {
        // https://unicode.org/reports/tr9/#Basic_Display_Algorithm

        using props_t = container::small_vector<bidi_prop_t, 1024>;
        using vec_t = container::
            static_vector<detail::bidi_state_t, detail::bidi_max_depth + 2>;
        using stack_t = std::stack<detail::bidi_state_t, vec_t>;

        auto next_odd = [](stack_t const & stack) {
            auto retval = stack.top().embedding_ + 1;
            if ((retval & 0x1) == 0)
                ++retval;
            return retval;
        };
        auto next_even = [](stack_t const & stack) {
            auto retval = stack.top().embedding_ + 1;
            if ((retval & 0x1) == 1)
                ++retval;
            return retval;
        };

        auto prop_from_top = [](stack_t const & stack, props_t & props) {
            if (stack.top().directional_override_ ==
                detail::directional_override_t::left_to_right) {
                props.back() = bidi_prop_t::L;
            } else if (
                stack.top().directional_override_ ==
                detail::directional_override_t::right_to_left) {
                props.back() = bidi_prop_t::R;
            }
        };

        // https://unicode.org/reports/tr9/#X5a
        auto x5a = [&](stack_t & stack,
                       props_t & props,
                       int & overflow_isolates,
                       int overflow_embedding,
                       int & valid_isolates) {
            prop_from_top(stack, props);
            auto const next_odd_embedding_level = next_odd(stack);
            if (next_odd_embedding_level <= detail::bidi_max_depth &&
                !overflow_isolates && !overflow_embedding) {
                ++valid_isolates;
                stack.push(detail::bidi_state_t{
                    next_odd_embedding_level,
                    detail::directional_override_t::neutral,
                    true});
            } else if (!overflow_isolates) {
                ++overflow_isolates;
            }
        };

        // https://unicode.org/reports/tr9/#X5b
        auto x5b = [&](stack_t & stack,
                       props_t & props,
                       int & overflow_isolates,
                       int overflow_embedding,
                       int & valid_isolates) {
            prop_from_top(stack, props);
            auto const next_even_embedding_level = next_even(stack);
            if (next_even_embedding_level <= detail::bidi_max_depth &&
                !overflow_isolates && !overflow_embedding) {
                ++valid_isolates;
                stack.push(detail::bidi_state_t{
                    next_even_embedding_level,
                    detail::directional_override_t::neutral,
                    true});
            } else if (!overflow_isolates) {
                ++overflow_isolates;
            }
        };

        // https://unicode.org/reports/tr9/#P1
        for (auto paragraph : paragraphs(first, first, last)) {
            auto const para_it = paragraph.begin();
            auto const para_last = paragraph.end();

            auto const paragraph_embedding_level =
                detail::p2_p3(para_it, para_last);

            // https://unicode.org/reports/tr9/#X1
            stack_t stack;
            stack.push(
                detail::bidi_state_t{paragraph_embedding_level,
                                     detail::directional_override_t::neutral,
                                     false});
            int overflow_isolates = 0;
            int overflow_embedding = 0;
            int valid_isolates = 0;

            props_t props;
            container::small_vector<uint8_t, 1024> embedding_levels;

            for (auto it = para_it; it != para_last; ++it) {
                auto const prop = bidi_prop(*it);
                props.push_back(prop);
                embedding_levels.push_back(stack.top().embedding_);

                // https://unicode.org/reports/tr9/#X2
                switch (prop) {
                case bidi_prop_t::RLE: {
                    // https://unicode.org/reports/tr9/#X2
                    auto const next_odd_embedding_level = next_odd(stack);
                    if (next_odd_embedding_level <= detail::bidi_max_depth &&
                        !overflow_isolates && !overflow_embedding) {
                        stack.push(detail::bidi_state_t{
                            next_odd_embedding_level,
                            detail::directional_override_t::neutral,
                            false});
                    } else if (!overflow_isolates) {
                        ++overflow_embedding;
                    }
                    break;
                }

                case bidi_prop_t::LRE: {
                    // https://unicode.org/reports/tr9/#X3
                    auto const next_even_embedding_level = next_even(stack);
                    if (next_even_embedding_level <= detail::bidi_max_depth &&
                        !overflow_isolates && !overflow_embedding) {
                        stack.push(detail::bidi_state_t{
                            next_even_embedding_level,
                            detail::directional_override_t::neutral,
                            false});
                    } else if (!overflow_isolates) {
                        ++overflow_embedding;
                    }
                    break;
                }

                case bidi_prop_t::RLO: {
                    // https://unicode.org/reports/tr9/#X4
                    auto const next_odd_embedding_level = next_odd(stack);
                    if (next_odd_embedding_level <= detail::bidi_max_depth &&
                        !overflow_isolates && !overflow_embedding) {
                        stack.push(detail::bidi_state_t{
                            next_odd_embedding_level,
                            detail::directional_override_t::right_to_left,
                            false});
                    } else if (!overflow_isolates) {
                        ++overflow_embedding;
                    }
                    break;
                }

                case bidi_prop_t::LRO: {
                    // https://unicode.org/reports/tr9/#X5
                    auto const next_even_embedding_level = next_even(stack);
                    if (next_even_embedding_level <= detail::bidi_max_depth &&
                        !overflow_isolates && !overflow_embedding) {
                        stack.push(detail::bidi_state_t{
                            next_even_embedding_level,
                            detail::directional_override_t::left_to_right,
                            false});
                    } else if (!overflow_isolates) {
                        ++overflow_embedding;
                    }
                    break;
                }

                case bidi_prop_t::RLI:
                    // https://unicode.org/reports/tr9/#X5a
                    x5a(stack,
                        props,
                        overflow_isolates,
                        overflow_embedding,
                        valid_isolates);
                    break;
                case bidi_prop_t::LRI:
                    // https://unicode.org/reports/tr9/#X5b
                    x5b(stack,
                        props,
                        props,
                        overflow_isolates,
                        overflow_embedding,
                        valid_isolates);
                    break;
                case bidi_prop_t::FSI:
                    // https://unicode.org/reports/tr9/#X5c
                    if (detail::p2_p3(it, para_it) == 1) {
                        x5a(stack,
                            props,
                            props,
                            overflow_isolates,
                            overflow_embedding,
                            valid_isolates);
                    } else {
                        x5b(stack,
                            props,
                            props,
                            overflow_isolates,
                            overflow_embedding,
                            valid_isolates);
                    }
                    break;

                default:
                    // https://unicode.org/reports/tr9/#X6
                    prop_from_top(stack, props);
                    break;

                case bidi_prop_t::PDI:
                    // https://unicode.org/reports/tr9/#X6a
                    break;

                case bidi_prop_t::PDF:
                    // https://unicode.org/reports/tr9/#X7
                    break;

                case bidi_prop_t::B: break;
                case bidi_prop_t::BN: break;
                }

                // TODO: X9, X10
            }
        }
        return out;
    }

}}

#endif
