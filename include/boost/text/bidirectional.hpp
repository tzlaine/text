#ifndef BOOST_TEXT_BIDIRECTIONAL_HPP
#define BOOST_TEXT_BIDIRECTIONAL_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/bidirectional_fwd.hpp>
#include <boost/text/line_break.hpp>
#include <boost/text/paragraph_break.hpp>
#include <boost/text/detail/bidirectional.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/optional.hpp>

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
            uint32_t cp_;
            int embedding_;
            bidi_prop_t prop_;
            bool unmatched_pdi_;
            bool originally_nsm_;
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

        enum class directional_override_t {
            neutral,
            right_to_left,
            left_to_right
        };

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

        struct props_and_embeddings_cp_iterator
        {
            using value_type = uint32_t;
            using pointer = value_type *;
            using reference = value_type &;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::bidirectional_iterator_tag;

            reference operator*() const noexcept { return it_->cp_; }
            pointer operator->() const noexcept { return &**this; }

            props_and_embeddings_cp_iterator & operator++() noexcept
            {
                ++it_;
                return *this;
            }
            props_and_embeddings_cp_iterator operator++(int)noexcept
            {
                auto const retval = *this;
                ++*this;
                return retval;
            }
            props_and_embeddings_cp_iterator & operator--() noexcept
            {
                --it_;
                return *this;
            }
            props_and_embeddings_cp_iterator operator--(int)noexcept
            {
                auto const retval = *this;
                --*this;
                return retval;
            }

            friend bool operator==(
                props_and_embeddings_cp_iterator lhs,
                props_and_embeddings_cp_iterator rhs) noexcept
            {
                return lhs.it_ == rhs.it_;
            }
            friend bool operator!=(
                props_and_embeddings_cp_iterator lhs,
                props_and_embeddings_cp_iterator rhs) noexcept
            {
                return lhs.it_ != rhs.it_;
            }

            props_and_embeddings_t::iterator it_;
        };

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
            using iterator_category = std::bidirectional_iterator_tag;

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

            run_seq_iter & operator--() noexcept
            {
                if (it_ == runs_it_->first_) {
                    --runs_it_;
                    it_ = runs_it_->last_;
                }
                --it_;
                return *this;
            }

            reference operator*() noexcept { return *it_; }
            pointer operator->() noexcept { return std::addressof(*it_); }

            level_run::iterator base() const { return it_; }

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
            using iterator = run_seq_iter;

            iterator begin() noexcept
            {
                return run_seq_iter{
                    runs_.begin()->first_, runs_.begin(), runs_.end()};
            }
            iterator end() noexcept
            {
                return run_seq_iter{
                    std::prev(runs_.end())->last_, runs_.begin(), runs_.end()};
            }

            run_seq_runs_t runs_;
            int embedding_;
            bidi_prop_t sos_; // L or R
            bidi_prop_t eos_; // L or R
        };

        using all_runs_t = container::small_vector<level_run, 1024>;
        using run_sequences_t = container::small_vector<run_sequence_t, 32>;

        inline all_runs_t find_all_runs(
            props_and_embeddings_t::iterator first,
            props_and_embeddings_t::iterator last)
        {
            container::small_vector<level_run, 1024> retval;
            {
                while (first != last) {
                    auto run = next_level_run(first, last);
                    if (run.empty())
                        break;
                    retval.push_back(run);
                    first = run.last_;
                }
            }
            return retval;
        }

        // https://unicode.org/reports/tr9/#BD13
        inline run_sequences_t
        find_run_sequences(props_and_embeddings_t & pae, all_runs_t & all_runs)
        {
            run_sequences_t retval;
            if (pae.empty())
                return retval;

            auto const end = pae.end();
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
                            // TODO: lower_bound instead?
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
        inline bool even(int x) { return !odd(x); }

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
                    elem.originally_nsm_ = true;
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

        inline bool not_bn(prop_and_embedding_t pae) noexcept
        {
            return pae.prop_ != bidi_prop_t::BN;
        }

        // https://unicode.org/reports/tr9/#W4
        inline void w4(run_sequence_t & seq) noexcept
        {
            auto const end = seq.end();

            // https://unicode.org/reports/tr9/#Retaining_Explicit_Formatting_Characters
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

        struct set_prop_func_t
        {
            prop_and_embedding_t operator()(prop_and_embedding_t pae) noexcept
            {
                pae.prop_ = prop_;
                return pae;
            }
            bidi_prop_t prop_;
        };

        inline set_prop_func_t set_prop(bidi_prop_t prop) noexcept
        {
            return set_prop_func_t{prop};
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
            replace_adjacents_with(seq, et, en, set_prop(bidi_prop_t::EN));
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
            replace_adjacents_with(
                seq, bn, et_es_cs, set_prop(bidi_prop_t::ON));

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
            using iterator = run_sequence_t::iterator;

            iterator begin() const noexcept { return first_; }
            iterator end() const noexcept { return last_; }

            iterator begin() noexcept { return first_; }
            iterator end() noexcept { return last_; }

            friend bool operator<(bracket_pair lhs, bracket_pair rhs)
            {
                return lhs.first_.base() < rhs.first_.base();
            }

            iterator first_;
            iterator last_;
        };

        using bracket_pairs_t = container::small_vector<bracket_pair, 64>;
        struct bracket_stack_element_t
        {
            run_sequence_t::iterator it_;
            uint32_t paired_bracket_;
        };

        // https://unicode.org/reports/tr9/#BD16
        inline bracket_pairs_t find_bracket_pairs(run_sequence_t & seq)
        {
            bracket_pairs_t retval;

            using stack_t =
                container::static_vector<bracket_stack_element_t, 63>;
            stack_t stack;

            for (auto it = seq.begin(), end = seq.end(); it != end; ++it) {
                if (it->prop_ != bidi_prop_t::ON)
                    continue;
                auto const bracket = bidi_bracket(it->cp_);
                if (bracket && bracket.type_ == bidi_bracket_type::open) {
                    if (stack.size() == stack.capacity())
                        break;
                    stack.push_back(
                        bracket_stack_element_t{it, bracket.paired_bracket_});
                } else if (
                    bracket && bracket.type_ == bidi_bracket_type::close) {
                    if (stack.empty())
                        continue;
                    // TODO: Also compare canonical equivalents?
                    auto match_rit = std::find_if(
                        stack.rbegin(),
                        stack.rend(),
                        [&it](bracket_stack_element_t elem) {
                            return it->cp_ == elem.paired_bracket_;
                        });
                    if (match_rit != stack.rend()) {
                        auto const match_it = --match_rit.base();
                        retval.push_back(bracket_pair{match_it->it_, it});
                        stack.erase(match_it, stack.end());
                    }
                }
            }

            std::sort(retval.begin(), retval.end());
            return retval;
        }

        // https://unicode.org/reports/tr9/#N0
        inline void
        n0(run_sequence_t & seq, bracket_pairs_t const & bracket_pairs) noexcept
        {
            auto set_props = [](bracket_pair pair,
                                run_sequence_t::iterator end,
                                bidi_prop_t prop) {
                pair.first_->prop_ = prop;
                auto transform_end = std::find_if(
                    std::next(pair.last_), end, [](prop_and_embedding_t pae) {
                        return !pae.originally_nsm_;
                    });
                std::transform(
                    pair.last_, transform_end, pair.last_, set_prop(prop));
            };

            auto bracket_it = bracket_pairs.begin();
            auto prev_strong_prop = seq.sos_;
            for (auto it = seq.begin(), end = seq.end();
                 it != end && bracket_it != bracket_pairs.end();
                 ++it) {
                if (strong(it->prop_))
                    prev_strong_prop = it->prop_;
                if (it == bracket_it->first_) {
                    auto pair = *bracket_it++;
                    bool strong_found = false;
                    auto same_direction_strong_it = std::find_if(
                        std::next(pair.first_),
                        pair.last_,
                        [&seq, &strong_found](prop_and_embedding_t pae) {
                            bool const strong_ = strong(pae.prop_);
                            if (!strong_)
                                return false;
                            strong_found = true;
                            assert(
                                pae.prop_ == bidi_prop_t::L ||
                                pae.prop_ == bidi_prop_t::R);
                            auto const strong_embedding =
                                pae.prop_ == bidi_prop_t::L ? 0 : 1;
                            return even(seq.embedding_ + strong_embedding);
                        });
                    if (same_direction_strong_it != pair.last_) {
                        set_props(
                            pair, seq.end(), same_direction_strong_it->prop_);
                    } else if (strong_found) {
                        auto const prev_strong_embedding =
                            prev_strong_prop == bidi_prop_t::L ? 0 : 1;
                        if (odd(seq.embedding_ + prev_strong_embedding)) {
                            set_props(pair, seq.end(), prev_strong_prop);
                        } else {
                            auto const seq_embedding_prop =
                                even(seq.embedding_) ? bidi_prop_t::L
                                                     : bidi_prop_t::R;
                            set_props(pair, seq.end(), seq_embedding_prop);
                        }
                    }
                }
            }
        }

        inline bool neutral_or_isolate(prop_and_embedding_t pae) noexcept
        {
            // https://unicode.org/reports/tr9/#Retaining_Explicit_Formatting_Characters
            return pae.prop_ == bidi_prop_t::BN ||
                   pae.prop_ == bidi_prop_t::B || pae.prop_ == bidi_prop_t::S ||
                   pae.prop_ == bidi_prop_t::WS ||
                   pae.prop_ == bidi_prop_t::ON ||
                   pae.prop_ == bidi_prop_t::FSI ||
                   pae.prop_ == bidi_prop_t::LRI ||
                   pae.prop_ == bidi_prop_t::RLI ||
                   pae.prop_ == bidi_prop_t::PDI;
        }

        // https://unicode.org/reports/tr9/#N1
        inline void n1(run_sequence_t & seq) noexcept
        {
            auto num_to_r = [](prop_and_embedding_t pae) {
                if (pae.prop_ == bidi_prop_t::EN ||
                    pae.prop_ == bidi_prop_t::AN)
                    return bidi_prop_t::R;
                return pae.prop_;
            };

            auto const begin = seq.begin();
            auto const end = seq.end();
            auto it = begin;
            while (it != end) {
                auto next_it = std::find_if(it, end, neutral_or_isolate);
                bool only_bns = true;
                auto next_next_it = std::find_if(
                    next_it, end, [&only_bns](prop_and_embedding_t pae) {
                        if (pae.prop_ != bidi_prop_t::BN)
                            only_bns = false;
                        return !neutral_or_isolate(pae);
                    });
                if (next_next_it == it || only_bns) {
                    ++it;
                    continue;
                }

                auto prev_prop = seq.sos_;
                if (next_it != begin || next_it != end)
                    prev_prop = num_to_r(*std::prev(next_it));
                auto next_prop = seq.eos_;
                if (next_next_it != end)
                    next_prop = num_to_r(*next_next_it);

                if (prev_prop == bidi_prop_t::L &&
                    next_prop == bidi_prop_t::L) {
                    std::transform(
                        next_it,
                        next_next_it,
                        next_it,
                        set_prop(bidi_prop_t::L));
                } else if (
                    prev_prop == bidi_prop_t::R &&
                    next_prop == bidi_prop_t::R) {
                    std::transform(
                        next_it,
                        next_next_it,
                        next_it,
                        set_prop(bidi_prop_t::R));
                }

                it = next_next_it;
            }
        }

        // https://unicode.org/reports/tr9/#N2
        inline void n2(run_sequence_t & seq) noexcept
        {
            auto const seq_embedding_prop =
                even(seq.embedding_) ? bidi_prop_t::L : bidi_prop_t::R;
            std::transform(
                seq.begin(),
                seq.end(),
                seq.begin(),
                [seq_embedding_prop](prop_and_embedding_t pae) {
                    if (neutral_or_isolate(pae))
                        pae.prop_ = seq_embedding_prop;
                    return pae;
                });
        }

        // https://unicode.org/reports/tr9/#I1
        // https://unicode.org/reports/tr9/#I2
        inline void i1_i2(run_sequence_t & seq) noexcept
        {
            bool const even_ = even(seq.embedding_);
            for (auto & elem : seq) {
                if (even_) {
                    if (elem.prop_ == bidi_prop_t::R) {
                        elem.embedding_ += 1;
                    } else if (
                        elem.prop_ == bidi_prop_t::EN ||
                        elem.prop_ == bidi_prop_t::AN) {
                        elem.embedding_ += 2;
                    }
                } else if (
                    elem.prop_ == bidi_prop_t::L ||
                    elem.prop_ == bidi_prop_t::EN ||
                    elem.prop_ == bidi_prop_t::AN) {
                    elem.embedding_ += 1;
                }
            }
        }

        // https://unicode.org/reports/tr9/#L1
        inline void
        l1(cp_range<props_and_embeddings_cp_iterator> line,
           int paragraph_embedding_level)
        {
            auto set_paragraph_embedding =
                [paragraph_embedding_level](prop_and_embedding_t pae) {
                    pae.embedding_ = paragraph_embedding_level;
                    return pae;
                };
            auto const end = line.end().it_;
            auto first_contiguous_ws_or_isolate_it = end;
            for (auto it = line.begin().it_; it != end; ++it) {
                auto const original_prop = boost::text::bidi_prop(it->cp_);
                if (original_prop == bidi_prop_t::B ||
                    original_prop == bidi_prop_t::S) {
                    it->embedding_ = paragraph_embedding_level;
                    if (first_contiguous_ws_or_isolate_it != end) {
                        std::transform(
                            first_contiguous_ws_or_isolate_it,
                            it,
                            first_contiguous_ws_or_isolate_it,
                            set_paragraph_embedding);
                        first_contiguous_ws_or_isolate_it = end;
                    }
                } else if (
                    original_prop == bidi_prop_t::WS ||
                    isolate_initiator(original_prop) ||
                    original_prop == bidi_prop_t::PDI) {
                    if (first_contiguous_ws_or_isolate_it == end)
                        first_contiguous_ws_or_isolate_it = it;
                } else {
                    first_contiguous_ws_or_isolate_it = end;
                }
            }
            if (first_contiguous_ws_or_isolate_it != end) {
                std::transform(
                    first_contiguous_ws_or_isolate_it,
                    end,
                    first_contiguous_ws_or_isolate_it,
                    set_paragraph_embedding);
            }
         }

        struct reordered_run
        {
            using iterator = props_and_embeddings_t::iterator;

            bool reversed() const noexcept { return last_ < first_; }
            int embedding() const noexcept
            {
                return reversed() ? last_->embedding_ : first_->embedding_;
            }

            iterator begin() const noexcept { return first_; }
            iterator end() const noexcept { return last_; }

            void reverse() noexcept { std::swap(first_, last_); }

            iterator first_;
            iterator last_;
        };

        using reordered_runs_t = container::small_vector<reordered_run, 1024>;

        inline reordered_runs_t l2(all_runs_t const & all_runs)
        {
            reordered_runs_t retval;
            std::transform(
                all_runs.begin(),
                all_runs.end(),
                std::back_inserter(retval),
                [](level_run run) {
                    return reordered_run{run.first_, run.last_};
                });
            auto min_max_it = std::minmax_element(
                retval.begin(),
                retval.end(),
                [](reordered_run lhs, reordered_run rhs) {
                    return lhs.embedding() < rhs.embedding();
                });
            auto lo = min_max_it.first->embedding();
            auto hi = min_max_it.second->embedding() + 1;
            if (even(lo))
                ++lo;
            for (int i = hi; i-- > lo;) {
                foreach_subrange_if(
                    retval.begin(),
                    retval.end(),
                    [i](reordered_run run) { return i <= run.embedding(); },
                    [](foreach_subrange_range<reordered_runs_t::iterator> r) {
                        std::reverse(r.begin(), r.end());
                        for (auto & elem : r) {
                            elem.reverse();
                        }
                    });
            }
            return retval;
        }

        enum fwd_rev_cp_iter_kind { user_it, rev_user_it, mirror_array_it };

        template<typename CPIter>
        struct fwd_rev_cp_iter
        {
            using value_type = uint32_t;
            using pointer = uint32_t;
            using reference = uint32_t;
            using difference_type =
                typename std::iterator_traits<CPIter>::difference_type;
            using iterator_category = std::forward_iterator_tag;

            using mirrors_array_t =
                typename std::remove_reference<typename std::remove_cv<decltype(
                    bidi_mirroreds())>::type>::type;
            using kind_t = fwd_rev_cp_iter_kind;

            fwd_rev_cp_iter() noexcept : kind_(kind_t::user_it) {}
            fwd_rev_cp_iter(CPIter it) noexcept : kind_(kind_t::user_it)
            {
                new (&it_) CPIter(std::move(it));
            }
            fwd_rev_cp_iter(std::reverse_iterator<CPIter> rit) noexcept :
                kind_(kind_t::rev_user_it)
            {
                new (&rit_) std::reverse_iterator<CPIter>(std::move(rit));
            }
            fwd_rev_cp_iter(
                mirrors_array_t::const_iterator ait,
                fwd_rev_cp_iter_kind k) noexcept :
                kind_(k)
            {
                assert(kind_ == kind_t::mirror_array_it);
                new (&ait_) mirrors_array_t::const_iterator(ait);
            }

            fwd_rev_cp_iter(fwd_rev_cp_iter const & other) noexcept :
                kind_(other.kind_)
            {
                if (kind_ == kind_t::user_it)
                    new (&it_) CPIter(other.it_);
                else if (kind_ == kind_t::rev_user_it)
                    new (&rit_) std::reverse_iterator<CPIter>(other.rit_);
                else
                    new (&ait_) mirrors_array_t::const_iterator(other.ait_);
            }
            fwd_rev_cp_iter(fwd_rev_cp_iter && other) noexcept :
                kind_(other.kind_)
            {
                if (kind_ == kind_t::user_it) {
                    new (&it_) CPIter(std::move(other.it_));
                } else if (kind_ == kind_t::rev_user_it) {
                    new (&rit_)
                        std::reverse_iterator<CPIter>(std::move(other.rit_));
                } else {
                    new (&ait_) mirrors_array_t::const_iterator(other.ait_);
                }
            }
            fwd_rev_cp_iter & operator=(fwd_rev_cp_iter const & other) noexcept
            {
                fwd_rev_cp_iter tmp(other);
                std::swap(*this, tmp);
                return *this;
            }
            fwd_rev_cp_iter & operator=(fwd_rev_cp_iter && other) noexcept
            {
                fwd_rev_cp_iter tmp(std::move(other));
                std::swap(*this, tmp);
                return *this;
            }
            ~fwd_rev_cp_iter()
            {
                using reverse_iterator_t = std::reverse_iterator<CPIter>;
                using array_iterator_t = mirrors_array_t::const_iterator;
                if (kind_ == kind_t::user_it)
                    it_.~CPIter();
                else if (kind_ == kind_t::rev_user_it)
                    rit_.~reverse_iterator_t();
                else
                    ait_.~array_iterator_t();
            }

            fwd_rev_cp_iter & operator++() noexcept
            {
                if (kind_ == kind_t::user_it)
                    ++it_;
                else if (kind_ == kind_t::rev_user_it)
                    ++rit_;
                else
                    ++ait_;
                return *this;
            }
            fwd_rev_cp_iter operator++(int)noexcept
            {
                fwd_rev_cp_iter retval = *this;
                ++*this;
                return retval;
            }

            fwd_rev_cp_iter & operator--() noexcept
            {
                if (kind_ == kind_t::user_it)
                    --rit_;
                else if (kind_ == kind_t::rev_user_it)
                    --it_;
                else
                    --ait_;
                return *this;
            }
            fwd_rev_cp_iter operator--(int)noexcept
            {
                fwd_rev_cp_iter retval = *this;
                --*this;
                return retval;
            }

            reference operator*() noexcept
            {
                if (kind_ == kind_t::user_it)
                    return *it_;
                else if (kind_ == kind_t::rev_user_it)
                    return *rit_;
                else
                    return *ait_;
            }

            friend bool operator==(
                fwd_rev_cp_iter const & lhs,
                fwd_rev_cp_iter const & rhs) noexcept
            {
                assert(lhs.kind_ == rhs.kind_);
                if (lhs.kind_ == kind_t::user_it)
                    return lhs.it_ == rhs.it_;
                else if (lhs.kind_ == kind_t::rev_user_it)
                    return lhs.rit_ == rhs.rit_;
                else
                    return lhs.ait_ == rhs.ait_;
            }
            friend bool operator!=(
                fwd_rev_cp_iter const & lhs,
                fwd_rev_cp_iter const & rhs) noexcept
            {
                return !(lhs == rhs);
            }

        private:
            union
            {
                CPIter it_;
                std::reverse_iterator<CPIter> rit_;
                mirrors_array_t::const_iterator ait_;
            };
            fwd_rev_cp_iter_kind kind_;
        };
    }

    /** TODO */
    struct next_hard_line_break_callable
    {
        template<typename CPIter>
        CPIter operator()(CPIter first, CPIter last) noexcept
        {
            return next_hard_line_break(first, last);
        }
    };

    /** TODO */
    template<typename CPIter>
    struct bidirectional_subrange
    {
        using iterator = detail::fwd_rev_cp_iter<CPIter>;

        bidirectional_subrange() noexcept {}
        bidirectional_subrange(iterator first, iterator last) noexcept :
            first_(first),
            last_(last)
        {}

        bool empty() const noexcept { return first_ == last_; }
        iterator begin() const noexcept { return first_; }
        iterator end() const noexcept { return last_; }

    private:
        iterator first_;
        iterator last_;
    };

    /** TODO
        TODO: Document that CPIter must be bidirectional.
        TODO: Document that NextLineBreakFunc must be polymorphic, taking and
        iterator whose value_type is uint32_t
        TODO: Accept an optional paragraph_embedding_level (or make an
        overload that takes same), in support of H1.  This replaces the call
        to p2_p3() in X1, but not in X5.
    */
    template<
        typename CPIter,
        typename OutIter,
        typename NextLineBreakFunc = next_hard_line_break_callable>
    OutIter bidirectional_order(
        CPIter first,
        CPIter last,
        OutIter out,
        NextLineBreakFunc && next_line_break = NextLineBreakFunc{})
    {
        static_assert(
            std::is_same<
                typename std::iterator_traits<OutIter>::value_type,
                bidirectional_subrange<CPIter>>::value,
            "OutIter::value_type must be bidirectional_subrange<CPIter>");

        // https://unicode.org/reports/tr9/#Basic_Display_Algorithm

        using prop_and_embedding_t = detail::prop_and_embedding_t;
        using props_and_embeddings_t = detail::props_and_embeddings_t;

        using vec_t = container::
            static_vector<detail::bidi_state_t, detail::bidi_max_depth + 2>;
        using stack_t = std::stack<detail::bidi_state_t, vec_t>;

        auto next_odd = [](stack_t const & stack) {
            auto retval = stack.top().embedding_ + 1;
            if (detail::even(retval))
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
        for (auto paragraph : paragraphs(first, last)) {
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
                props_and_embeddings.push_back(prop_and_embedding_t{
                    *it, stack.top().embedding_, prop, false, false});

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
                auto all_runs = detail::find_all_runs(
                    props_and_embeddings.begin(), props_and_embeddings.end());
                auto run_sequences =
                    detail::find_run_sequences(props_and_embeddings, all_runs);

                detail::find_sos_eos(run_sequences, paragraph_embedding_level);
                for (auto & run_sequence : run_sequences) {
                    detail::w1(run_sequence);
                    detail::w2(run_sequence);
                    detail::w3(run_sequence);
                    detail::w4(run_sequence);
                    detail::w5(run_sequence);
                    detail::w6(run_sequence);
                    detail::w7(run_sequence);

                    auto const bracket_pairs = find_bracket_pairs(run_sequence);
                    detail::n0(run_sequence, bracket_pairs);
                    detail::n1(run_sequence);
                    detail::n2(run_sequence);

                    detail::i1_i2(run_sequence);
                }

                // TODO: Need a way to indicate the positions of soft line
                // breaks (but probably not hard ones) interleaved with the
                // subranges produced below.  Probably the way to do this is to
                // output an empty range that indicates a line break.

                lazy_segment_range<
                    detail::props_and_embeddings_cp_iterator,
                    NextLineBreakFunc>
                    lines{{detail::props_and_embeddings_cp_iterator{
                               props_and_embeddings.begin()},
                           detail::props_and_embeddings_cp_iterator{
                               props_and_embeddings.end()}},
                          {detail::props_and_embeddings_cp_iterator{
                               props_and_embeddings.end()},
                           detail::props_and_embeddings_cp_iterator{
                               props_and_embeddings.end()}}};
                auto cp_line_start = paragraph.begin();
                for (auto line : lines) {
                    l1(line, paragraph_embedding_level);

                    // https://unicode.org/reports/tr9/#L2
                    all_runs =
                        detail::find_all_runs(line.begin().it_, line.end().it_);
                    auto reordered_runs = l2(all_runs);

                    // TODO: Document that L3 is the caller's responsibility.

                    // Output the reordered subranges.
                    for (auto run : reordered_runs) {
                        auto out_first = cp_line_start;
                        auto out_last = cp_line_start;
                        std::advance(
                            out_last, line.end().it_ - line.begin().it_);
                        cp_line_start = out_last;

                        auto out_value =
                            bidirectional_subrange<CPIter>{out_first, out_last};

                        if (run.reversed()) {
                            // https://unicode.org/reports/tr9/#L4

                            // If this run's directionality is R (aka odd, aka
                            // reversed), produce 1-code-point ranges for the
                            // mirrored characters in the run, if any.
                            while (out_first != out_last) {
                                int mirror_index = -1;
                                auto it = std::find_if(
                                    out_value.begin(),
                                    out_value.end(),
                                    [&mirror_index](uint32_t cp) {
                                        mirror_index =
                                            detail::bidi_mirroring(cp);
                                        return mirror_index != -1;
                                    });
                                if (it != out_value.end()) {
                                    if (it != out_value.begin()) {
                                        auto prev_subrange =
                                            bidirectional_subrange<CPIter>{
                                                out_value.begin(), it};
                                        *out = prev_subrange;
                                        ++out;
                                    }
                                    *out = bidirectional_subrange<CPIter>{
                                        detail::fwd_rev_cp_iter<CPIter>{
                                            detail::bidi_mirroreds().begin() +
                                                mirror_index,
                                            detail::fwd_rev_cp_iter_kind::
                                                mirror_array_it},
                                        detail::fwd_rev_cp_iter<CPIter>{
                                            detail::bidi_mirroreds().begin() +
                                                mirror_index + 1,
                                            detail::fwd_rev_cp_iter_kind::
                                                mirror_array_it}};
                                    ++out;
                                    out_value = bidirectional_subrange<CPIter>{
                                        ++it, out_last};
                                }
                            }
                            if (!out_value.empty()) {
                                *out = out_value;
                                ++out;
                            }
                        } else {
                            *out = out_value;
                            ++out;
                        }
                    }
                }
            }
        }
        return out;
    }

}}

#endif
