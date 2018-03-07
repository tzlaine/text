#ifndef BOOST_TEXT_BIDIRECTIONAL_HPP
#define BOOST_TEXT_BIDIRECTIONAL_HPP

#include <boost/text/bidirectional_fwd.hpp>
#include <boost/text/paragraph_break.hpp>
#include <boost/text/detail/bidi_brackets.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>

#include <stack>


namespace boost { namespace text {

    /** Returns the bidirectional algorithm character property associated with
        code point \a cp. */
    bidi_prop_t bidi_prop(uint32_t cp) noexcept;

    namespace detail {
        inline bool isolate_initiator(bidi_prop_t prop) noexcept
        {
            return prop == bidi_prop_t::LRI || prop == bidi_prop_t::RLI ||
                   prop == bidi_prop_t::FSI;
        }

        struct prop_and_embedding_t
        {
            bidi_prop_t prop_;
            bool unmatched_pdi_;
            int embedding_;
        };

        inline bidi_prop_t bidi_prop(prop_and_embedding_t pae) noexcept
        {
            return pae.prop_;
        }

        template<typename CPIter>
        CPIter matching_pdi(CPIter it, CPIter last) noexcept
        {
            if (it == last)
                return last;

            using ::boost::text::bidi_prop;
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
                    using ::boost::text::bidi_prop;
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

        using props_and_embeddings_t =
            container::small_vector<prop_and_embedding_t, 1024>;

        struct level_run
        {
            using iterator = props_and_embeddings_t::iterator;
            using const_iterator = props_and_embeddings_t::const_iterator;

            bool empty() const noexcept { return first_ == last_; }

            const_iterator begin() const noexcept { return first_; }
            const_iterator end() const noexcept { return last_; }

            iterator begin() noexcept { return first_; }
            iterator end() noexcept { return last_; }

            int embedding() const noexcept { return first_->embedding_; }

            iterator first_;
            iterator last_;
            bool used_;
        };

        inline level_run next_level_run(
            props_and_embeddings_t::iterator first,
            props_and_embeddings_t::iterator last) noexcept
        {
            if (first == last)
                return level_run{last, last, false};

            auto skippable = [](bidi_prop_t prop) {
                return prop == bidi_prop_t::RLE || prop == bidi_prop_t::LRE ||
                       prop == bidi_prop_t::RLO || prop == bidi_prop_t::LRO ||
                       prop == bidi_prop_t::PDF || prop == bidi_prop_t::BN;
            };

            auto const initial_level = first->embedding_;
            return level_run{
                first,
                std::find_if(
                    first,
                    last,
                    [initial_level, skippable](prop_and_embedding_t pae) {
                        return pae.embedding_ != initial_level;
                    }),
                false};
        }

        using run_seq_runs_t = container::small_vector<level_run, 32>;

        struct run_seq_iter
        {
            using value_type = prop_and_embedding_t;
            using pointer = prop_and_embedding_t *;
            using reference = prop_and_embedding_t &;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::forward_iterator_tag;

            run_seq_iter & operator++() noexcept
            {
                if (++it_ == runs_it_->last_) {
                    auto const next_runs_it = std::next(runs_it_);
                    if (next_runs_it == runs_end_) {
                        it_ = runs_it_->last_;
                    } else {
                        runs_it_ = next_runs_it;
                        it_ = runs_it_->first_;
                    }
                }
                return *this;
            }

            reference operator*() noexcept { return *it_; }
            pointer operator->() noexcept { return &*it_; }

            friend bool operator==(run_seq_iter lhs, run_seq_iter rhs)
            {
                return lhs.it_ == rhs.it_;
            }
            friend bool operator!=(run_seq_iter lhs, run_seq_iter rhs)
            {
                return lhs.it_ != rhs.it_;
            }

            level_run::iterator it_;
            run_seq_runs_t::iterator runs_it_;
            run_seq_runs_t::iterator runs_end_;
        };

        struct run_sequence_t
        {
            run_seq_iter begin() noexcept
            {
                return run_seq_iter{
                    runs_.begin()->first_, runs_.begin(), runs_.end()};
            }
            run_seq_iter end() noexcept
            {
                return run_seq_iter{
                    std::prev(runs_.end())->last_, runs_.begin(), runs_.end()};
            }

            run_seq_runs_t runs_;
            int embedding_;
            bidi_prop_t sos_; // L or R
            bidi_prop_t eos_; // L or R
        };

        using run_sequences_t = container::small_vector<run_sequence_t, 32>;

        inline container::small_vector<level_run, 1024>
        find_all_runs(props_and_embeddings_t & pae)
        {
            container::small_vector<level_run, 1024> retval;
            {
                auto it = pae.begin();
                auto const end = pae.end();
                while (it != end) {
                    auto run = next_level_run(it, end);
                    if (run.empty())
                        break;
                    retval.push_back(run);
                    it = run.last_;
                }
            }
            return retval;
        }

        // https://unicode.org/reports/tr9/#BD13
        inline run_sequences_t find_run_sequences(props_and_embeddings_t & pae)
        {
            run_sequences_t retval;
            if (pae.empty())
                return retval;

            auto const end = pae.end();
            auto all_runs = find_all_runs(pae);

            for (auto & run : all_runs) {
                if (!run.used_ && (run.first_->prop_ != bidi_prop_t::PDI ||
                                   run.first_->unmatched_pdi_)) {
                    retval.resize(retval.size() + 1);
                    run_sequence_t & sequence = retval.back();
                    sequence.runs_.push_back(run);
                    sequence.embedding_ = run.first_->embedding_;
                    run.used_ = true;
                    while (isolate_initiator(
                        std::prev(sequence.runs_.back().last_)->prop_)) {
                        auto const pdi_it = matching_pdi(
                            std::prev(sequence.runs_.back().last_), end);
                        if (pdi_it != end) {
                            auto const all_runs_end =
                                &all_runs[0] + all_runs.size();
                            auto const run_it = std::find_if(
                                &run, all_runs_end, [pdi_it](level_run r) {
                                    return pdi_it < r.last_;
                                });
                            if (run_it != all_runs_end &&
                                run_it->first_ == pdi_it) {
                                run_it->used_ = true;
                                sequence.runs_.push_back(*run_it);
                                continue;
                            }
                        }
                        break;
                    }
                }
            }

            return retval;
        }

        inline bool odd(int x) { return x & 0x1; }

        inline void find_sos_eos(
            run_sequences_t & run_sequences, int paragraph_embedding_level)
        {
            auto prev_embedding = paragraph_embedding_level;
            auto embedding = run_sequences[0].embedding_;
            auto next_embedding = run_sequences.size() < 2
                                      ? paragraph_embedding_level
                                      : run_sequences[1].embedding_;
            for (int i = 0, end = (int)run_sequences.size(); i != end; ++i) {
                run_sequences[i].sos_ =
                    odd((std::max)(prev_embedding, embedding)) ? bidi_prop_t::R
                                                               : bidi_prop_t::L;
                run_sequences[i].eos_ =
                    odd((std::max)(embedding, next_embedding)) ? bidi_prop_t::R
                                                               : bidi_prop_t::L;
                prev_embedding = embedding;
                embedding = next_embedding;
                if (i < (int)run_sequences.size() - 1)
                    next_embedding = run_sequences[i + 1].embedding_;
            }
        }

        // https://unicode.org/reports/tr9/#W1
        inline void w1(run_sequence_t & seq) noexcept
        {
            auto prev_prop = seq.sos_;
            for (auto & elem : seq) {
                auto prop = elem.prop_;
                // https://unicode.org/reports/tr9/#Retaining_Explicit_Formatting_Characters
                if (prop == bidi_prop_t::BN)
                    continue;
                if (prop == bidi_prop_t::NSM) {
                    elem.prop_ = prev_prop == bidi_prop_t::PDI ||
                                         isolate_initiator(prev_prop)
                                     ? bidi_prop_t::ON
                                     : prev_prop;
                    prop = elem.prop_;
                }
                prev_prop = prop;
            }
        }

        inline bool strong(bidi_prop_t prop) noexcept
        {
            return prop == bidi_prop_t::R || prop == bidi_prop_t::L ||
                   prop == bidi_prop_t::AL;
        }

        // This works for W7 because all ALs are removed in W3.
        inline void w2_w7_impl(
            run_sequence_t & seq,
            bidi_prop_t trigger,
            bidi_prop_t replacement) noexcept
        {
            auto curr_strong_prop = seq.sos_;
            for (auto & elem : seq) {
                if (strong(elem.prop_)) {
                    curr_strong_prop = elem.prop_;
                } else if (
                    elem.prop_ == bidi_prop_t::EN &&
                    curr_strong_prop == trigger) {
                    elem.prop_ = replacement;
                }
            }
        }

        // https://unicode.org/reports/tr9/#W2
        inline void w2(run_sequence_t & seq) noexcept
        {
            w2_w7_impl(seq, bidi_prop_t::AL, bidi_prop_t::AN);
        }

        // https://unicode.org/reports/tr9/#W3
        inline void w3(run_sequence_t & seq) noexcept
        {
            for (auto & elem : seq) {
                if (elem.prop_ == bidi_prop_t::AL)
                    elem.prop_ = bidi_prop_t::R;
            }
        }

        // https://unicode.org/reports/tr9/#W4
        inline void w4(run_sequence_t & seq) noexcept
        {
            auto const end = seq.end();

            // https://unicode.org/reports/tr9/#Retaining_Explicit_Formatting_Characters
            auto not_bn = [](prop_and_embedding_t pae) {
                return pae.prop_ != bidi_prop_t::BN;
            };

            auto prev_it = std::find_if(seq.begin(), end, not_bn);
            if (prev_it == seq.end())
                return;
            auto it = std::find_if(std::next(prev_it), end, not_bn);
            if (it == seq.end())
                return;
            auto next_it = std::find_if(std::next(it), end, not_bn);

            for (; next_it != end;
                 prev_it = it,
                 it = next_it,
                 next_it = std::find_if(std::next(next_it), end, not_bn)) {
                if (prev_it->prop_ == bidi_prop_t::EN &&
                    it->prop_ == bidi_prop_t::ES &&
                    next_it->prop_ == bidi_prop_t::EN) {
                    it->prop_ = bidi_prop_t::EN;
                } else if (
                    it->prop_ == bidi_prop_t::CS &&
                    prev_it->prop_ == next_it->prop_ &&
                    (prev_it->prop_ == bidi_prop_t::EN ||
                     prev_it->prop_ == bidi_prop_t::AN)) {
                    it->prop_ = prev_it->prop_;
                }
            }
        }

        // Find props matching \a changeable_prop that are adjacent to props
        // matching \a adjacent_prop, and replace them using \a replace.
        template<
            typename ChangeablePropPred,
            typename AdjacentPropPred,
            typename ReplaceFn>
        void replace_adjacents_with(
            run_sequence_t & seq,
            ChangeablePropPred changeable_prop,
            AdjacentPropPred adjacent_prop,
            ReplaceFn replace) noexcept
        {
            auto changeable = [changeable_prop](prop_and_embedding_t pae) {
                // https://unicode.org/reports/tr9/#Retaining_Explicit_Formatting_Characters
                return changeable_prop(pae) || pae.prop_ == bidi_prop_t::BN;
            };

            auto it = seq.begin();
            auto const end = seq.end();
            while (it != end) {
                it = std::find_if(
                    it,
                    end,
                    [changeable, adjacent_prop](prop_and_embedding_t pae) {
                        return changeable(pae) || adjacent_prop(pae);
                    });
                if (it == end)
                    break;
                if (changeable(*it)) {
                    auto next_it = std::find_if_not(it, end, changeable);
                    if (next_it != end && adjacent_prop(*next_it)) {
                        std::transform(it, next_it, it, replace);
                    }
                    it = next_it;
                } else {
                    it = std::find_if_not(it, end, adjacent_prop);
                    auto next_it = std::find_if_not(it, end, changeable);
                    std::transform(it, next_it, it, replace);
                    it = next_it;
                }
            }
        }

        // https://unicode.org/reports/tr9/#W5
        inline void w5(run_sequence_t & seq) noexcept
        {
            auto et = [](prop_and_embedding_t pae) {
                return pae.prop_ == bidi_prop_t::ET;
            };
            auto en = [](prop_and_embedding_t pae) {
                return pae.prop_ == bidi_prop_t::EN;
            };
            auto to_en = [](prop_and_embedding_t pae) {
                pae.prop_ = bidi_prop_t::EN;
                return pae;
            };
            replace_adjacents_with(seq, et, en, to_en);
        }

        // https://unicode.org/reports/tr9/#W6
        inline void w6(run_sequence_t & seq) noexcept
        {
            // https://unicode.org/reports/tr9/#Retaining_Explicit_Formatting_Characters
            auto bn = [](prop_and_embedding_t pae) {
                return false; // BN is covered in replace_adjacents_with().
            };
            auto et_es_cs = [](prop_and_embedding_t pae) {
                return pae.prop_ == bidi_prop_t::ET ||
                       pae.prop_ == bidi_prop_t::ES ||
                       pae.prop_ == bidi_prop_t::CS;
            };
            auto to_on = [](prop_and_embedding_t pae) {
                pae.prop_ = bidi_prop_t::ON;
                return pae;
            };
            replace_adjacents_with(seq, bn, et_es_cs, to_on);

            std::transform(
                seq.begin(),
                seq.end(),
                seq.begin(),
                [](prop_and_embedding_t pae) {
                    if (pae.prop_ == bidi_prop_t::ES ||
                        pae.prop_ == bidi_prop_t::CS ||
                        pae.prop_ == bidi_prop_t::ET) {
                        pae.prop_ = bidi_prop_t::ON;
                    }
                    return pae;
                });
        }

        // https://unicode.org/reports/tr9/#W7
        inline void w7(run_sequence_t & seq) noexcept
        {
            w2_w7_impl(seq, bidi_prop_t::L, bidi_prop_t::L);
        }

        struct bracket_pair
        {
            using iterator = props_and_embeddings_t::iterator;
            using const_iterator = props_and_embeddings_t::const_iterator;

            bool empty() const noexcept { return first_ == last_; }

            const_iterator begin() const noexcept { return first_; }
            const_iterator end() const noexcept { return last_; }

            iterator begin() noexcept { return first_; }
            iterator end() noexcept { return last_; }

            int embedding() const noexcept { return first_->embedding_; }

            friend bool operator<(bracket_pair lhs, bracket_pair rhs)
            {
                return lhs.first_ < rhs.first_;
            }

            iterator first_;
            iterator last_;
        };

        using bracket_pairs_t = container::small_vector<bracket_pair, 64>;
        struct bracket_stack_element_t
        {
            props_and_embeddings_t::iterator it_;
            uint32_t cp_;
        };

        // https://unicode.org/reports/tr9/#BD16
        inline bracket_pairs_t find_bracket_pairs(run_sequence_t & seq)
        {
            bracket_pairs_t retval;

            using vec_t = container::static_vector<bracket_stack_element_t, 63>;
            using stack_t = std::stack<bracket_stack_element_t, vec_t>;
            stack_t stack;

            // TODO

            std::sort(retval.begin(), retval.end());
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

        using prop_and_embedding_t = detail::prop_and_embedding_t;
        using props_and_embeddings_t = detail::props_and_embeddings_t;

        using vec_t = container::
            static_vector<detail::bidi_state_t, detail::bidi_max_depth + 2>;
        using stack_t = std::stack<detail::bidi_state_t, vec_t>;

        auto next_odd = [](stack_t const & stack) {
            auto retval = stack.top().embedding_ + 1;
            if (!detail::odd(retval))
                ++retval;
            return retval;
        };
        auto next_even = [](stack_t const & stack) {
            auto retval = stack.top().embedding_ + 1;
            if (detail::odd(retval))
                ++retval;
            return retval;
        };

        auto prop_from_top = [](stack_t const & stack,
                                props_and_embeddings_t & props_and_embeddings) {
            if (stack.top().directional_override_ ==
                detail::directional_override_t::left_to_right) {
                props_and_embeddings.back().prop_ = bidi_prop_t::L;
            } else if (
                stack.top().directional_override_ ==
                detail::directional_override_t::right_to_left) {
                props_and_embeddings.back().prop_ = bidi_prop_t::R;
            }
        };

        // https://unicode.org/reports/tr9/#X5a
        auto x5a = [&](stack_t & stack,
                       props_and_embeddings_t & props_and_embeddings,
                       int & overflow_isolates,
                       int overflow_embedding,
                       int & valid_isolates) {
            prop_from_top(stack, props_and_embeddings);
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
                       props_and_embeddings_t & props_and_embeddings,
                       int & overflow_isolates,
                       int overflow_embedding,
                       int & valid_isolates) {
            prop_from_top(stack, props_and_embeddings);
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

            props_and_embeddings_t props_and_embeddings;

            for (auto it = para_it; it != para_last; ++it) {
                auto const prop = bidi_prop(*it);
                // https://unicode.org/reports/tr9/#Retaining_Explicit_Formatting_Characters
                // indicates that the embedding level should always be
                // whatever the top of stack's embedding level is.
                props_and_embeddings.push_back(
                    prop_and_embedding_t{prop, false, stack.top().embedding_});

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
                        props_and_embeddings,
                        overflow_isolates,
                        overflow_embedding,
                        valid_isolates);
                    break;
                case bidi_prop_t::LRI:
                    // https://unicode.org/reports/tr9/#X5b
                    x5b(stack,
                        props_and_embeddings,
                        overflow_isolates,
                        overflow_embedding,
                        valid_isolates);
                    break;
                case bidi_prop_t::FSI:
                    // https://unicode.org/reports/tr9/#X5c
                    if (detail::p2_p3(it, para_it) == 1) {
                        x5a(stack,
                            props_and_embeddings,
                            overflow_isolates,
                            overflow_embedding,
                            valid_isolates);
                    } else {
                        x5b(stack,
                            props_and_embeddings,
                            overflow_isolates,
                            overflow_embedding,
                            valid_isolates);
                    }
                    break;

                default:
                    // https://unicode.org/reports/tr9/#X6
                    prop_from_top(stack, props_and_embeddings);
                    break;

                case bidi_prop_t::PDI:
                    // https://unicode.org/reports/tr9/#X6a
                    if (0 < overflow_isolates) {
                        --overflow_isolates;
                    } else if (valid_isolates) {
                        props_and_embeddings.back().unmatched_pdi_ = true;
                    } else {
                        overflow_embedding = 0;
                        while (!stack.top().directional_isolate_) {
                            stack.pop();
                        }
                        stack.pop();
                        --valid_isolates;
                    }
                    props_and_embeddings.back().embedding_ =
                        stack.top().embedding_;
                    if (stack.top().directional_override_ ==
                        detail::directional_override_t::left_to_right) {
                        props_and_embeddings.back().prop_ = bidi_prop_t::L;
                    } else if (
                        stack.top().directional_override_ ==
                        detail::directional_override_t::right_to_left) {
                        props_and_embeddings.back().prop_ = bidi_prop_t::R;
                    }
                    break;

                case bidi_prop_t::PDF:
                    // https://unicode.org/reports/tr9/#X7
                    if (!overflow_isolates) {
                        if (0 < overflow_embedding) {
                            --overflow_embedding;
                        } else if (
                            !stack.top().directional_isolate_ &&
                            2u <= stack.size()) {
                            stack.pop();
                        }
                    }
                    // https://unicode.org/reports/tr9/#Retaining_Explicit_Formatting_Characters
                    props_and_embeddings.back().embedding_ =
                        stack.top().embedding_;
                    break;

                case bidi_prop_t::B:
                    break;

                    // https://unicode.org/reports/tr9/#Retaining_Explicit_Formatting_Characters
                    // case bidi_prop_t::BN: break;
                }

                // https://unicode.org/reports/tr9/#Retaining_Explicit_Formatting_Characters
                // https://unicode.org/reports/tr9/#X9
                std::transform(
                    props_and_embeddings.begin(),
                    props_and_embeddings.end(),
                    props_and_embeddings.begin(),
                    [](prop_and_embedding_t pae) {
                        if (pae.prop_ == bidi_prop_t::RLE ||
                            pae.prop_ == bidi_prop_t::LRE ||
                            pae.prop_ == bidi_prop_t::RLO ||
                            pae.prop_ == bidi_prop_t::LRO ||
                            pae.prop_ == bidi_prop_t::PDF) {
                            pae.prop_ = bidi_prop_t::BN;
                        }
                        return pae;
                    });

                // https://unicode.org/reports/tr9/#X10
                auto run_sequences =
                    detail::find_run_sequences(props_and_embeddings);

                detail::find_sos_eos(run_sequences, paragraph_embedding_level);
                for (auto & run_sequence : run_sequences) {
                    w1(run_sequence);
                    w2(run_sequence);
                    w3(run_sequence);
                    w4(run_sequence);
                    w5(run_sequence);
                    w6(run_sequence);
                    w7(run_sequence);
                }

                // TODO
            }
        }
        return out;
    }

}}

#endif
