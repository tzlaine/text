#ifndef BOOST_TEXT_BIDIRECTIONAL_HPP
#define BOOST_TEXT_BIDIRECTIONAL_HPP

#include <boost/text/algorithm.hpp>
#include <boost/text/bidirectional_fwd.hpp>
#include <boost/text/line_break.hpp>
#include <boost/text/paragraph_break.hpp>
#include <boost/text/utility.hpp>
#include <boost/text/detail/bidirectional.hpp>

#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/optional.hpp>

#include <stack>


namespace boost { namespace text {

    namespace detail {
        struct bidi_prop_interval
        {
            uint32_t lo_;
            uint32_t hi_;
            bidi_property prop_;
        };

        inline bool
        operator<(bidi_prop_interval lhs, bidi_prop_interval rhs) noexcept
        {
            return lhs.hi_ <= rhs.lo_;
        }

        BOOST_TEXT_DECL std::array<bidi_prop_interval, 53> const &
        make_bidi_prop_intervals();
        BOOST_TEXT_DECL std::unordered_map<uint32_t, bidi_property>
        make_bidi_prop_map();
    }

    /** Returns the bidirectional algorithm character property associated with
        code point \a cp. */
    inline bidi_property bidi_prop(uint32_t cp) noexcept
    {
        static auto const map = detail::make_bidi_prop_map();
        static auto const intervals = detail::make_bidi_prop_intervals();

        auto const it = map.find(cp);
        if (it == map.end()) {
            auto const it2 = std::lower_bound(
                intervals.begin(),
                intervals.end(),
                detail::bidi_prop_interval{cp, cp + 1});
            if (it2 == intervals.end() || cp < it2->lo_ || it2->hi_ <= cp)
                return bidi_property::L;
            return it2->prop_;
        }
        return it->second;
    }

    namespace detail {
        inline bool isolate_initiator(bidi_property prop) noexcept
        {
            return prop == bidi_property::LRI || prop == bidi_property::RLI ||
                   prop == bidi_property::FSI;
        }

        template<typename CPIter>
        struct prop_and_embedding_t
        {
            using value_type =
                typename std::iterator_traits<CPIter>::value_type;

            value_type const & cp() const { return *it_; }

            CPIter it_;
            int embedding_;
            bidi_property prop_;
            bool unmatched_pdi_;
            bool originally_nsm_;

#ifdef BOOST_TEXT_TESTING
            friend std::ostream &
            operator<<(std::ostream & os, prop_and_embedding_t pae)
            {
                os << '{' << std::hex << "0x" << pae.cp() << std::dec << " "
                   << pae.embedding_ << " " << pae.prop_ << " "
                   << pae.unmatched_pdi_ << " " << pae.originally_nsm_ << '}';
                return os;
            }
#endif
        };

        template<typename CPIter>
        inline bidi_property
        bidi_prop(prop_and_embedding_t<CPIter> pae) noexcept
        {
            return pae.prop_;
        }

        template<typename CPIter, typename Sentinel>
        CPIter matching_pdi(CPIter it, Sentinel last) noexcept
        {
            if (it == last)
                return it;

            using ::boost::text::bidi_prop;
            assert(isolate_initiator(bidi_prop(*it)));

            int iis = 1;
            while (++it != last) {
                auto const prop = bidi_prop(*it);
                if (isolate_initiator(prop)) {
                    ++iis;
                } else if (prop == bidi_property::PDI) {
                    --iis;
                    if (!iis)
                        break;
                }
            }

            return it;
        }

        inline bool embedding_initiator(bidi_property prop) noexcept
        {
            return prop == bidi_property::LRE || prop == bidi_property::RLE ||
                   prop == bidi_property::LRO || prop == bidi_property::RLO;
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

        template<typename CPIter, typename Sentinel>
        int p2_p3(CPIter first, Sentinel last) noexcept
        {
            // https://unicode.org/reports/tr9/#P2
            using ::boost::text::bidi_prop;

            auto retval = 0;

            while (first != last) {
                auto const prop = bidi_prop(*first);
                if (prop == bidi_property::L || prop == bidi_property::AL ||
                    prop == bidi_property::R) {
                    break;
                }
                if (isolate_initiator(prop))
                    first = matching_pdi(first, last);
                else
                    ++first;
            }

            // https://unicode.org/reports/tr9/#P3
            if (first == last)
                return retval;

            auto const prop = bidi_prop(*first);
            if (prop == bidi_property::AL || prop == bidi_property::R)
                retval = 1;

            return retval;
        }

        template<typename CPIter>
        using props_and_embeddings_t =
            container::small_vector<prop_and_embedding_t<CPIter>, 1024>;

        template<typename CPIter>
        struct props_and_embeddings_cp_iterator
        {
            using value_type = uint32_t const;
            using pointer = value_type *;
            using reference = value_type &;
            using difference_type = std::ptrdiff_t;
            using iterator_category = std::bidirectional_iterator_tag;

            reference operator*() const noexcept { return it_->cp(); }
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

            typename props_and_embeddings_t<CPIter>::iterator it_;
        };

        template<typename CPIter>
        struct level_run
        {
            using iterator = typename props_and_embeddings_t<CPIter>::iterator;
            using const_iterator =
                typename props_and_embeddings_t<CPIter>::const_iterator;

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

        template<typename CPIter>
        using run_seq_runs_t = container::small_vector<level_run<CPIter>, 32>;

        template<typename CPIter>
        struct run_seq_iter
        {
            using value_type = prop_and_embedding_t<CPIter>;
            using pointer = prop_and_embedding_t<CPIter> *;
            using reference = prop_and_embedding_t<CPIter> &;
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

            reference operator*() const noexcept { return *it_; }
            pointer operator->() const noexcept { return std::addressof(*it_); }

            typename level_run<CPIter>::iterator base() const { return it_; }

            friend bool operator==(run_seq_iter lhs, run_seq_iter rhs)
            {
                return lhs.it_ == rhs.it_;
            }
            friend bool operator!=(run_seq_iter lhs, run_seq_iter rhs)
            {
                return lhs.it_ != rhs.it_;
            }

            typename level_run<CPIter>::iterator it_;
            typename run_seq_runs_t<CPIter>::iterator runs_it_;
            typename run_seq_runs_t<CPIter>::iterator runs_end_;
        };

        template<typename CPIter>
        struct run_sequence_t
        {
            using iterator = run_seq_iter<CPIter>;

            bool empty() noexcept { return begin() == end(); }

            iterator begin() noexcept
            {
                return run_seq_iter<CPIter>{
                    runs_.begin()->first_, runs_.begin(), runs_.end()};
            }
            iterator end() noexcept
            {
                return run_seq_iter<CPIter>{
                    std::prev(runs_.end())->last_, runs_.begin(), runs_.end()};
            }

            run_seq_runs_t<CPIter> runs_;
            int embedding_;
            bidi_property sos_; // L or R
            bidi_property eos_; // L or R
        };

        template<typename CPIter>
        using all_runs_t = container::small_vector<level_run<CPIter>, 1024>;

        template<typename CPIter>
        using run_sequences_t =
            container::small_vector<run_sequence_t<CPIter>, 32>;

        template<typename CPIter>
        inline all_runs_t<CPIter> find_all_runs(
            typename props_and_embeddings_t<CPIter>::iterator first,
            typename props_and_embeddings_t<CPIter>::iterator last)
        {
            all_runs_t<CPIter> retval;
            using iter_t = decltype(first);
            foreach_subrange(
                first,
                last,
                [&retval](foreach_subrange_range<iter_t> r) {
                    retval.push_back(
                        level_run<CPIter>{r.begin(), r.end(), false});
                },
                [](prop_and_embedding_t<CPIter> pae) {
                    return pae.embedding_;
                });
            return retval;
        }

        // https://unicode.org/reports/tr9/#BD13
        template<typename CPIter>
        inline run_sequences_t<CPIter> find_run_sequences(
            props_and_embeddings_t<CPIter> & pae, all_runs_t<CPIter> & all_runs)
        {
            run_sequences_t<CPIter> retval;
            if (pae.empty())
                return retval;

            auto const end = pae.end();
            for (auto & run : all_runs) {
                if (!run.used_ && (run.first_->prop_ != bidi_property::PDI ||
                                   run.first_->unmatched_pdi_)) {
                    retval.resize(retval.size() + 1);
                    run_sequence_t<CPIter> & sequence = retval.back();
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
                                &run,
                                all_runs_end,
                                [pdi_it](level_run<CPIter> r) {
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

        template<typename CPIter>
        inline void find_sos_eos(
            run_sequences_t<CPIter> & run_sequences,
            int paragraph_embedding_level)
        {
#if 0
            std::cout << "find_sos_eos:\n";
            for (auto rs : run_sequences) {
                std::cout << "run seq (embedding=" << rs.embedding_ << "):\n";
                for (auto pae : rs) {
                    std::cout << pae << "\n";
                }
            }
            std::cout << std::endl;
#endif
            if (run_sequences.empty())
                return;

            auto prev_embedding = paragraph_embedding_level;
            for (auto it = run_sequences.begin(), end = run_sequences.end();
                 it != end;
                 ++it) {
                auto embedding = it->embedding_;
                auto next_embedding = embedding;
                auto next_it = std::next(it);
                if (next_it != end)
                    next_embedding = next_it->embedding_;
                it->sos_ = odd((std::max)(prev_embedding, embedding))
                               ? bidi_property::R
                               : bidi_property::L;
                it->eos_ = odd((std::max)(embedding, next_embedding))
                               ? bidi_property::R
                               : bidi_property::L;
                prev_embedding = embedding;
            }
        }

        // https://unicode.org/reports/tr9/#W1
        template<typename CPIter>
        inline void w1(run_sequence_t<CPIter> & seq) noexcept
        {
            auto prev_prop = seq.sos_;
            for (auto & elem : seq) {
                auto prop = elem.prop_;
                if (prop == bidi_property::NSM) {
                    elem.prop_ = prev_prop == bidi_property::PDI ||
                                         isolate_initiator(prev_prop)
                                     ? bidi_property::ON
                                     : prev_prop;
                    elem.originally_nsm_ = true;
                    prop = elem.prop_;
                }
                prev_prop = prop;
            }
        }

        // This works for W7 because all ALs are removed in W3.
        template<typename CPIter, typename Pred>
        inline void w2_w7_impl(
            run_sequence_t<CPIter> & seq,
            Pred strong,
            bidi_property trigger,
            bidi_property replacement) noexcept
        {
#if 0
            std::cout << "line:" << __LINE__ << "\n";
            std::cout << "sos=" << seq.sos_ << "\n";
            for (auto it = seq.begin(), end = seq.end(); it != end; ++it) {
                std::cout << *it << "\n";
            }
            std::cout << std::endl;
#endif
            auto en = [](prop_and_embedding_t<CPIter> pae) {
                return pae.prop_ == bidi_property::EN;
            };
            auto it = seq.end();
            auto const first = seq.begin();
            while (it != first) {
                auto const from_it = find_if_backward(first, it, en);
                if (from_it == it)
                    break;
                auto const pred_it = find_if_backward(first, from_it, strong);
                if ((pred_it == from_it && seq.sos_ == trigger) ||
                    pred_it->prop_ == trigger) {
                    from_it->prop_ = replacement;
#if 0
                    std::cout << "replacing!\n";
#endif
                }
                --it;
            }
        }

        // https://unicode.org/reports/tr9/#W2
        template<typename CPIter>
        inline void w2(run_sequence_t<CPIter> & seq) noexcept
        {
            w2_w7_impl(
                seq,
                [](prop_and_embedding_t<CPIter> pae) {
                    return pae.prop_ == bidi_property::R ||
                           pae.prop_ == bidi_property::L ||
                           pae.prop_ == bidi_property::AL;
                },
                bidi_property::AL,
                bidi_property::AN);
        }

        // https://unicode.org/reports/tr9/#W3
        template<typename CPIter>
        inline void w3(run_sequence_t<CPIter> & seq) noexcept
        {
            for (auto & elem : seq) {
                if (elem.prop_ == bidi_property::AL)
                    elem.prop_ = bidi_property::R;
            }
        }

        template<typename CPIter>
        inline bool not_bn(prop_and_embedding_t<CPIter> pae) noexcept
        {
            return pae.prop_ != bidi_property::BN;
        }

        // https://unicode.org/reports/tr9/#W4
        template<typename CPIter>
        inline void w4(run_sequence_t<CPIter> & seq) noexcept
        {
            if (seq.empty())
                return;
            {
                auto it = std::next(seq.begin());
                if (it == seq.end())
                    return;
                if (++it == seq.end())
                    return;
            }

            for (auto prev_it = seq.begin(),
                      it = std::next(prev_it),
                      next_it = std::next(it),
                      end = seq.end();
                 next_it != end;
                 ++prev_it, ++it, ++next_it) {
                if (prev_it->prop_ == bidi_property::EN &&
                    it->prop_ == bidi_property::ES &&
                    next_it->prop_ == bidi_property::EN) {
                    it->prop_ = bidi_property::EN;
                } else if (
                    it->prop_ == bidi_property::CS &&
                    prev_it->prop_ == next_it->prop_ &&
                    (prev_it->prop_ == bidi_property::EN ||
                     prev_it->prop_ == bidi_property::AN)) {
                    it->prop_ = prev_it->prop_;
                }
            }
        }

        template<typename CPIter>
        struct set_prop_func_t
        {
            prop_and_embedding_t<CPIter>
            operator()(prop_and_embedding_t<CPIter> pae) noexcept
            {
                pae.prop_ = prop_;
                return pae;
            }
            bidi_property prop_;
        };

        template<typename CPIter>
        inline set_prop_func_t<CPIter> set_prop(bidi_property prop) noexcept
        {
            return set_prop_func_t<CPIter>{prop};
        }

        // https://unicode.org/reports/tr9/#W5
        template<typename CPIter>
        inline void w5(run_sequence_t<CPIter> & seq) noexcept
        {
            using iter_t = decltype(seq.begin());
            foreach_subrange_if(
                seq.begin(),
                seq.end(),
                [](prop_and_embedding_t<CPIter> pae) {
                    return pae.prop_ == bidi_property::ET;
                },
                [&seq](foreach_subrange_range<iter_t> r) {
                    if ((r.begin() != seq.begin() &&
                         std::prev(r.begin())->prop_ == bidi_property::EN) ||
                        (r.end() != seq.end() &&
                         r.end()->prop_ == bidi_property::EN)) {
                        std::transform(
                            r.begin(),
                            r.end(),
                            r.begin(),
                            set_prop<CPIter>(bidi_property::EN));
                    }
                });
        }

        // https://unicode.org/reports/tr9/#W6
        template<typename CPIter>
        inline void w6(run_sequence_t<CPIter> & seq) noexcept
        {
            std::transform(
                seq.begin(),
                seq.end(),
                seq.begin(),
                [](prop_and_embedding_t<CPIter> pae) {
                    if (pae.prop_ == bidi_property::ES ||
                        pae.prop_ == bidi_property::CS ||
                        pae.prop_ == bidi_property::ET) {
                        pae.prop_ = bidi_property::ON;
                    }
                    return pae;
                });
        }

        // https://unicode.org/reports/tr9/#W7
        template<typename CPIter>
        inline void w7(run_sequence_t<CPIter> & seq) noexcept
        {
            w2_w7_impl(
                seq,
                [](prop_and_embedding_t<CPIter> pae) {
                    return pae.prop_ == bidi_property::R ||
                           pae.prop_ == bidi_property::L;
                },
                bidi_property::L,
                bidi_property::L);
        }

        template<typename CPIter>
        struct bracket_pair
        {
            using iterator = typename run_sequence_t<CPIter>::iterator;

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

        template<typename CPIter>
        using bracket_pairs_t =
            container::small_vector<bracket_pair<CPIter>, 64>;

        template<typename CPIter>
        struct bracket_stack_element_t
        {
            typename run_sequence_t<CPIter>::iterator it_;
            uint32_t paired_bracket_;
        };

        // https://unicode.org/reports/tr9/#BD16
        template<typename CPIter>
        inline bracket_pairs_t<CPIter>
        find_bracket_pairs(run_sequence_t<CPIter> & seq)
        {
            bracket_pairs_t<CPIter> retval;

            using stack_t =
                container::static_vector<bracket_stack_element_t<CPIter>, 63>;
            stack_t stack;

            for (auto it = seq.begin(), end = seq.end(); it != end; ++it) {
                if (it->prop_ != bidi_property::ON)
                    continue;
                auto const bracket = bidi_bracket(it->cp());
                if (bracket && bracket.type_ == bidi_bracket_type::open) {
                    if (stack.size() == stack.capacity())
                        break;
                    stack.push_back(bracket_stack_element_t<CPIter>{
                        it, bracket.paired_bracket_});
                } else if (
                    bracket && bracket.type_ == bidi_bracket_type::close) {
                    if (stack.empty())
                        continue;
                    // TODO: Also compare canonical equivalents?
                    auto match_rit = std::find_if(
                        stack.rbegin(),
                        stack.rend(),
                        [&it](bracket_stack_element_t<CPIter> elem) {
                            return it->cp() == elem.paired_bracket_;
                        });
                    if (match_rit != stack.rend()) {
                        auto const match_it = --match_rit.base();
                        retval.push_back(
                            bracket_pair<CPIter>{match_it->it_, it});
                        stack.erase(match_it, stack.end());
                    }
                }
            }

            std::sort(retval.begin(), retval.end());
            return retval;
        }

        // https://unicode.org/reports/tr9/#N0
        template<typename CPIter>
        inline void
        n0(run_sequence_t<CPIter> & seq,
           bracket_pairs_t<CPIter> const & bracket_pairs) noexcept
        {
            auto set_props = [](bracket_pair<CPIter> pair,
                                typename run_sequence_t<CPIter>::iterator end,
                                bidi_property prop) {
                pair.first_->prop_ = prop;
                auto transform_end = std::find_if(
                    std::next(pair.last_),
                    end,
                    [](prop_and_embedding_t<CPIter> pae) {
                        return !pae.originally_nsm_;
                    });
                std::transform(
                    pair.last_,
                    transform_end,
                    pair.last_,
                    set_prop<CPIter>(prop));
            };

            auto strong = [](bidi_property prop) {
                return prop == bidi_property::L || prop == bidi_property::R;
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
                        [&seq, &strong_found, strong](
                            prop_and_embedding_t<CPIter> pae) {
                            bool const strong_ = strong(pae.prop_);
                            if (!strong_)
                                return false;
                            strong_found = true;
                            assert(
                                pae.prop_ == bidi_property::L ||
                                pae.prop_ == bidi_property::R);
                            auto const strong_embedding =
                                pae.prop_ == bidi_property::L ? 0 : 1;
                            return even(seq.embedding_ + strong_embedding);
                        });
                    if (same_direction_strong_it != pair.last_) {
                        set_props(
                            pair, seq.end(), same_direction_strong_it->prop_);
                    } else if (strong_found) {
                        auto const prev_strong_embedding =
                            prev_strong_prop == bidi_property::L ? 0 : 1;
                        if (odd(seq.embedding_ + prev_strong_embedding)) {
                            set_props(pair, seq.end(), prev_strong_prop);
                        } else {
                            auto const seq_embedding_prop =
                                even(seq.embedding_) ? bidi_property::L
                                                     : bidi_property::R;
                            set_props(pair, seq.end(), seq_embedding_prop);
                        }
                    }
                }
            }
        }

        template<typename CPIter>
        inline bool
        neutral_or_isolate(prop_and_embedding_t<CPIter> pae) noexcept
        {
            return pae.prop_ == bidi_property::B ||
                   pae.prop_ == bidi_property::S ||
                   pae.prop_ == bidi_property::WS ||
                   pae.prop_ == bidi_property::ON ||
                   pae.prop_ == bidi_property::FSI ||
                   pae.prop_ == bidi_property::LRI ||
                   pae.prop_ == bidi_property::RLI ||
                   pae.prop_ == bidi_property::PDI;
        }

        // https://unicode.org/reports/tr9/#N1
        template<typename CPIter>
        inline void n1(run_sequence_t<CPIter> & seq) noexcept
        {
            auto num_to_r = [](prop_and_embedding_t<CPIter> pae) {
                if (pae.prop_ == bidi_property::EN ||
                    pae.prop_ == bidi_property::AN)
                    return bidi_property::R;
                return pae.prop_;
            };

#if 0
            std::cout << "seq size=" << std::distance(seq.begin(), seq.end())
                      << "\n";
            std::cout << std::hex;
            for (auto x : seq) {
                std::cout << x << "\n";
            }
            std::cout << std::dec;
#endif

            using iter_t = decltype(seq.begin());
            foreach_subrange_if(
                seq.begin(),
                seq.end(),
                neutral_or_isolate<CPIter>,
                [&seq, &num_to_r](foreach_subrange_range<iter_t> r) {
                    auto prev_prop = seq.sos_;
                    if (r.begin() != seq.begin())
                        prev_prop = num_to_r(*std::prev(r.begin()));
                    auto next_prop = seq.eos_;
                    if (r.end() != seq.end())
                        next_prop = num_to_r(*r.end());

#if 0
                    std::cout << "r size=" << std::distance(r.begin(), r.end())
                              << "\n";
                    std::cout << std::hex;
                    for (auto x : r) {
                        std::cout << x << "\n";
                    }
                    std::cout << std::dec;
#endif

                    if (prev_prop == bidi_property::L &&
                        next_prop == bidi_property::L) {
#if 0
                        std::cout << "n1 changing NIs to bidi_property::L\n";
#endif
                        std::transform(
                            r.begin(),
                            r.end(),
                            r.begin(),
                            set_prop<CPIter>(bidi_property::L));
                    } else if (
                        prev_prop == bidi_property::R &&
                        next_prop == bidi_property::R) {
#if 0
                        std::cout << "n1 changing NIs to bidi_property::R\n";
#endif
                        std::transform(
                            r.begin(),
                            r.end(),
                            r.begin(),
                            set_prop<CPIter>(bidi_property::R));
                    }
                });
        }

        // https://unicode.org/reports/tr9/#N2
        template<typename CPIter>
        inline void n2(run_sequence_t<CPIter> & seq) noexcept
        {
            auto const seq_embedding_prop =
                even(seq.embedding_) ? bidi_property::L : bidi_property::R;
            std::transform(
                seq.begin(),
                seq.end(),
                seq.begin(),
                [seq_embedding_prop](prop_and_embedding_t<CPIter> pae) {
                    if (neutral_or_isolate(pae))
                    {
#if 0
                        std::cout << "n2 changing " << pae.prop_ << " to "
                                  << seq_embedding_prop << "\n";
#endif
                        pae.prop_ = seq_embedding_prop;
                    }
                    return pae;
                });
        }

        // https://unicode.org/reports/tr9/#I1
        // https://unicode.org/reports/tr9/#I2
        template<typename CPIter>
        inline void i1_i2(run_sequence_t<CPIter> & seq) noexcept
        {
            bool const even_ = even(seq.embedding_);
            for (auto & elem : seq) {
                if (even_) {
                    if (elem.prop_ == bidi_property::R) {
                        elem.embedding_ += 1;
                    } else if (
                        elem.prop_ == bidi_property::EN ||
                        elem.prop_ == bidi_property::AN) {
                        elem.embedding_ += 2;
                    }
                } else if (
                    elem.prop_ == bidi_property::L ||
                    elem.prop_ == bidi_property::EN ||
                    elem.prop_ == bidi_property::AN) {
                    elem.embedding_ += 1;
                }
            }
        }

        // TODO: Remove FSI, LRI, RLI, and PDI after L1.

        // TODO: L2 should only produce contiguous subranges of CPs, natch.

        // https://unicode.org/reports/tr9/#L1
        template<typename CPIter>
        inline void
        l1(cp_range<props_and_embeddings_cp_iterator<CPIter>> line,
           int paragraph_embedding_level)
        {
            // L1.1, L1.2
            for (auto it = line.begin(), end = line.end(); it != end; ++it) {
                auto const original_prop = boost::text::bidi_prop(it.it_->cp());
                if (original_prop == bidi_property::B ||
                    original_prop == bidi_property::S) {
                    it.it_->embedding_ = paragraph_embedding_level;
                }
            }

#if 0
            std::cout << "line:\n";
            for (auto it = line.begin(), end = line.end(); it != end; ++it) {
                std::cout << it.it_->prop_ << "\n";
            }
            std::cout << std::endl;
#endif

            // L1.3, L1.4
            using iter_t = decltype(line.begin().it_);
            auto const last = line.end().it_;
            foreach_subrange_if(
                line.begin().it_,
                line.end().it_,
                [](prop_and_embedding_t<CPIter> pae) {
                    auto const original_prop = boost::text::bidi_prop(pae.cp());
                    return original_prop == bidi_property::WS ||
                           original_prop == bidi_property::FSI ||
                           original_prop == bidi_property::LRI ||
                           original_prop == bidi_property::RLI ||
                           original_prop == bidi_property::PDI;
                },
                [paragraph_embedding_level,
                 last](foreach_subrange_range<iter_t> r) {
#if 0
                    std::cout << "subrange:\n";
                    for (auto pae : r) {
                        std::cout << pae << "\n";
                    }
                    std::cout << std::endl;
#endif


                    bool reset = r.end() == last;
                    if (!reset) {
                        auto const original_prop =
                            boost::text::bidi_prop(r.end()->cp());
                        reset = original_prop == bidi_property::S ||
                                original_prop == bidi_property::B;
#if 0
                        std::cout << "original_prop=" << original_prop << "\n";
#endif
                    }
#if 0
                    else
                        std::cout << "subrange at end\n";
#endif

                    if (reset) {
                        std::transform(
                            r.begin(),
                            r.end(),
                            r.begin(),
                            [paragraph_embedding_level](
                                prop_and_embedding_t<CPIter> pae) {
#if 0
                                std::cout << "resetting to pel\n";
#endif
                                pae.embedding_ = paragraph_embedding_level;
                                return pae;
                            });
                    }
                });
        }

        template<typename CPIter>
        struct reordered_run
        {
            using iterator = typename props_and_embeddings_t<CPIter>::iterator;
            using reverse_iterator = detail::reverse_iterator<
                typename props_and_embeddings_t<CPIter>::iterator>;

            bool reversed() const noexcept { return reversed_; }
            int embedding() const noexcept { return first_->embedding_; }
            void reverse() noexcept { reversed_ = !reversed_; }

            iterator begin() const noexcept { return first_; }
            iterator end() const noexcept { return last_; }
            reverse_iterator rbegin() const noexcept
            {
                return make_reverse_iterator(last_);
            }
            reverse_iterator rend() const noexcept
            {
                return make_reverse_iterator(first_);
            }

            iterator first_;
            iterator last_;
            bool reversed_;
        };

        template<typename CPIter>
        using reordered_runs_t =
            container::small_vector<reordered_run<CPIter>, 1024>;

        template<typename CPIter>
        inline reordered_runs_t<CPIter> l2(all_runs_t<CPIter> const & all_runs)
        {
            reordered_runs_t<CPIter> retval;
            std::transform(
                all_runs.begin(),
                all_runs.end(),
                std::back_inserter(retval),
                [](level_run<CPIter> run) {
                    return reordered_run<CPIter>{run.first_, run.last_, false};
                });
            auto min_max_it = std::minmax_element(
                retval.begin(),
                retval.end(),
                [](reordered_run<CPIter> lhs, reordered_run<CPIter> rhs) {
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
                    [i](reordered_run<CPIter> run) {
                        return i <= run.embedding();
                    },
                    [](foreach_subrange_range<
                        typename reordered_runs_t<CPIter>::iterator> r) {
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

            using mirrors_array_t = remove_cv_ref_t<decltype(bidi_mirroreds())>;
            using kind_t = fwd_rev_cp_iter_kind;

            fwd_rev_cp_iter() noexcept : kind_(kind_t::user_it) {}
            fwd_rev_cp_iter(CPIter it) noexcept : kind_(kind_t::user_it)
            {
                new (&it_) CPIter(std::move(it));
            }
            fwd_rev_cp_iter(detail::reverse_iterator<CPIter> rit) noexcept :
                kind_(kind_t::rev_user_it)
            {
                new (&rit_) detail::reverse_iterator<CPIter>(std::move(rit));
            }
            fwd_rev_cp_iter(
                mirrors_array_t::const_iterator ait,
                fwd_rev_cp_iter_kind k) noexcept :
                kind_(k)
            {
                assert(kind_ == kind_t::mirror_array_it);
                new (&ait_) mirrors_array_t::const_iterator(ait);
            }

            fwd_rev_cp_iter(fwd_rev_cp_iter const & other) noexcept
            {
                construct(other);
            }
            fwd_rev_cp_iter(fwd_rev_cp_iter && other) noexcept
            {
                construct(std::move(other));
            }
            fwd_rev_cp_iter & operator=(fwd_rev_cp_iter const & other) noexcept
            {
                destroy();
                construct(other);
                return *this;
            }
            fwd_rev_cp_iter & operator=(fwd_rev_cp_iter && other) noexcept
            {
                destroy();
                construct(std::move(other));
                return *this;
            }
            ~fwd_rev_cp_iter() { destroy(); }

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

            reference operator*() const noexcept
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
            void construct(fwd_rev_cp_iter const & other)
            {
                kind_ = other.kind_;
                if (kind_ == kind_t::user_it)
                    new (&it_) CPIter(other.it_);
                else if (kind_ == kind_t::rev_user_it)
                    new (&rit_) detail::reverse_iterator<CPIter>(other.rit_);
                else
                    new (&ait_) mirrors_array_t::const_iterator(other.ait_);
            }
            void construct(fwd_rev_cp_iter && other)
            {
                kind_ = other.kind_;
                if (kind_ == kind_t::user_it) {
                    new (&it_) CPIter(std::move(other.it_));
                } else if (kind_ == kind_t::rev_user_it) {
                    new (&rit_)
                        detail::reverse_iterator<CPIter>(std::move(other.rit_));
                } else {
                    new (&ait_) mirrors_array_t::const_iterator(other.ait_);
                }
            }
            void destroy()
            {
                using reverse_iterator_t = detail::reverse_iterator<CPIter>;
                using array_iterator_t = mirrors_array_t::const_iterator;
                if (kind_ == kind_t::user_it)
                    it_.~CPIter();
                else if (kind_ == kind_t::rev_user_it)
                    rit_.~reverse_iterator_t();
                else
                    ait_.~array_iterator_t();
            }

            union
            {
                CPIter it_;
                detail::reverse_iterator<CPIter> rit_;
                mirrors_array_t::const_iterator ait_;
            };
            fwd_rev_cp_iter_kind kind_;
        };

        enum class bidi_line_break_kind { none, hard, possible };
    }

    /** A callable type that returns the next hard line break in [first,
        last).  This is the default line break callable type used with
        bidirectional_order(). */
    struct next_hard_line_break_callable
    {
        template<typename BreakResult, typename Sentinel>
        BreakResult operator()(BreakResult result, Sentinel last) noexcept
        {
            return BreakResult{next_hard_line_break(result.iter, last), true};
        }
    };

    /** Represents either a subrange of code points ordered by the Unicode
        bidirectional algorithm, or a line break; the line break may be a hard
        line break, or a possible line break, according to the Unicode line
        break algorithm.  This is the output type for
        bidirectional_order(). */
    template<typename CPIter>
    struct bidirectional_subrange
    {
        using iterator = detail::fwd_rev_cp_iter<CPIter>;

        static_assert(
            detail::is_cp_iter<CPIter>::value,
            "CPIter must be a code point iterator");

        bidirectional_subrange() noexcept :
            break_(detail::bidi_line_break_kind::none)
        {}
        bidirectional_subrange(iterator first, iterator last) noexcept :
            first_(first),
            last_(last),
            break_(detail::bidi_line_break_kind::none)
        {}
#ifndef BOOST_TEXT_DOXYGEN
        bidirectional_subrange(detail::bidi_line_break_kind kind) noexcept :
            break_(kind)
        {}
#endif

        bool line_break() const noexcept
        {
            return hard_line_break() || posible_line_break();
        }
        bool hard_line_break() const noexcept
        {
            return break_ == detail::bidi_line_break_kind::hard;
        }
        bool posible_line_break() const noexcept
        {
            return break_ == detail::bidi_line_break_kind::possible;
        }
        bool empty() const noexcept { return first_ == last_; }
        iterator begin() const noexcept { return first_; }
        iterator end() const noexcept { return last_; }

    private:
        iterator first_;
        iterator last_;
        detail::bidi_line_break_kind break_;
    };

    namespace detail {
        template<typename Impl, typename BreakResult, typename Sentinel>
        struct next_line_break_t
        {
            BreakResult operator()(BreakResult result, Sentinel last) noexcept
            {
                return impl_(result, last);
            }

            Impl impl_;
        };

        template<typename CPIter, typename OutIter, typename NextLineBreakFunc>
        OutIter emit_bidi_subranges(
            props_and_embeddings_t<CPIter> & props_and_embeddings,
            int paragraph_embedding_level,
            NextLineBreakFunc & next_line_break,
            OutIter out)
        {
            using pae_cp_iterator = props_and_embeddings_cp_iterator<CPIter>;

            all_runs_t<CPIter> all_runs;

            lazy_segment_range<
                line_break_result<pae_cp_iterator>,
                pae_cp_iterator,
                next_line_break_t<
                    NextLineBreakFunc,
                    line_break_result<pae_cp_iterator>,
                    pae_cp_iterator>,
                line_break_cp_range<pae_cp_iterator>>
                lines{
                    {line_break_result<pae_cp_iterator>{
                         pae_cp_iterator{props_and_embeddings.begin()}, false},
                     pae_cp_iterator{props_and_embeddings.end()}},
                    {pae_cp_iterator{props_and_embeddings.end()}}};

            for (auto line : lines) {
                l1(line, paragraph_embedding_level);

                // https://unicode.org/reports/tr9/#L2
                all_runs =
                    find_all_runs<CPIter>(line.begin().it_, line.end().it_);
                auto reordered_runs = l2(all_runs);

                // TODO: Document that L3 is the caller's responsibility.

                // Output the reordered subranges.
                for (auto run : reordered_runs) {
                    if (run.reversed()) {
                        // https://unicode.org/reports/tr9/#L4

                        auto out_value = bidirectional_subrange<CPIter>{
                            boost::text::detail::make_reverse_iterator(run.end()->it_),
                            boost::text::detail::make_reverse_iterator(run.begin()->it_)};
                        auto out_first = out_value.begin();
                        auto out_last = out_value.end();

                        // If this run's directionality is R (aka odd, aka
                        // reversed), produce 1-code-point ranges for the
                        // mirrored characters in the run, if any.
                        while (out_first != out_last) {
                            int mirror_index = -1;
                            auto it = std::find_if(
                                out_first,
                                out_last,
                                [&mirror_index](uint32_t cp) {
                                    mirror_index = bidi_mirroring(cp);
                                    return mirror_index != -1;
                                });
                            if (it == out_last)
                                break;

                            // If we found a reversible CP, emit any
                            // preceding CPs first.
                            if (it != out_first) {
                                auto prev_subrange =
                                    bidirectional_subrange<CPIter>{out_first,
                                                                   it};
                                *out = prev_subrange;
                                ++out;
                            }

                            // Emit the reversed CP.
                            *out = bidirectional_subrange<CPIter>{
                                fwd_rev_cp_iter<CPIter>{
                                    bidi_mirroreds().begin() + mirror_index,
                                    fwd_rev_cp_iter_kind::mirror_array_it},
                                fwd_rev_cp_iter<CPIter>{
                                    bidi_mirroreds().begin() + mirror_index + 1,
                                    fwd_rev_cp_iter_kind::mirror_array_it}};
                            ++out;

                            // Increment for the next iteration.
                            out_value =
                                bidirectional_subrange<CPIter>{++it, out_last};
                            out_first = out_value.begin();
                        }

                        if (!out_value.empty()) {
                            *out = out_value;
                            ++out;
                        }
                    } else {
                        auto out_value = bidirectional_subrange<CPIter>{
                            run.begin()->it_, run.end()->it_};
                        *out = out_value;
                        ++out;
                    }
                }

                bidirectional_subrange<CPIter> const line_break_range{
                    line.hard_break() ? bidi_line_break_kind::hard
                                      : bidi_line_break_kind::possible};
                *out = line_break_range;
                ++out;
            }

            return out;
        }

        template<
            typename CPIter,
            typename Sentinel,
            typename OutIter,
            typename NextLineBreakFunc,
            typename EmitResultsFunc>
        OutIter bidirectional_order_impl(
            CPIter first,
            Sentinel last,
            OutIter out,
            NextLineBreakFunc && next_line_break,
            EmitResultsFunc emit)
        {
            // https://unicode.org/reports/tr9/#Basic_Display_Algorithm

            using prop_and_embedding_t = prop_and_embedding_t<CPIter>;
            using props_and_embeddings_t = props_and_embeddings_t<CPIter>;

            using vec_t =
                container::static_vector<bidi_state_t, bidi_max_depth + 2>;
            using stack_t = std::stack<bidi_state_t, vec_t>;

            auto next_odd = [](stack_t const & stack) {
                auto retval = stack.top().embedding_ + 1;
                if (even(retval))
                    ++retval;
                return retval;
            };
            auto next_even = [](stack_t const & stack) {
                auto retval = stack.top().embedding_ + 1;
                if (odd(retval))
                    ++retval;
                return retval;
            };

            auto prop_from_top =
                [](stack_t const & stack,
                   props_and_embeddings_t & props_and_embeddings) {
                    if (stack.top().directional_override_ ==
                        directional_override_t::left_to_right) {
                        props_and_embeddings.back().prop_ = bidi_property::L;
                    } else if (
                        stack.top().directional_override_ ==
                        directional_override_t::right_to_left) {
                        props_and_embeddings.back().prop_ = bidi_property::R;
                    }
                };

            // TODO: is_upper() et al are probably broken if they are not
            // language-aware; they need to pump the state machine in the same
            // way as to_upper(), etc.

            // https://unicode.org/reports/tr9/#X5a
            auto x5a = [&](stack_t & stack,
                           props_and_embeddings_t & props_and_embeddings,
                           int & overflow_isolates,
                           int overflow_embedding,
                           int & valid_isolates) {
                prop_from_top(stack, props_and_embeddings);
                auto const next_odd_embedding_level = next_odd(stack);
                if (next_odd_embedding_level <= bidi_max_depth &&
                    !overflow_isolates && !overflow_embedding) {
                    ++valid_isolates;
                    stack.push(bidi_state_t{next_odd_embedding_level,
                                            directional_override_t::neutral,
                                            true});
                } else {
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
                if (next_even_embedding_level <= bidi_max_depth &&
                    !overflow_isolates && !overflow_embedding) {
                    ++valid_isolates;
                    stack.push(bidi_state_t{next_even_embedding_level,
                                            directional_override_t::neutral,
                                            true});
                } else {
                    ++overflow_isolates;
                }
            };

            // TODO: I think we can dispense with all the empty stack
            // checks.

            // https://unicode.org/reports/tr9/#P1
            for (auto paragraph : paragraphs(first, last)) {
                auto const para_first = paragraph.begin();
                auto const para_last = paragraph.end();

                auto const paragraph_embedding_level =
                    p2_p3(para_first, para_last);

#if 0
                std::cout << "initial pel=" << paragraph_embedding_level << "\n";
#endif

                // https://unicode.org/reports/tr9/#X1
                stack_t stack;
                stack.push(bidi_state_t{paragraph_embedding_level,
                                        directional_override_t::neutral,
                                        false});
                int overflow_isolates = 0;
                int overflow_embedding = 0;
                int valid_isolates = 0;

                props_and_embeddings_t props_and_embeddings;

                // https://unicode.org/reports/tr9/#X2 through #X5
                auto explicits =
                    [&](int next_embedding_level,
                        directional_override_t directional_override) {
                        if (next_embedding_level <= bidi_max_depth &&
                            !overflow_isolates && !overflow_embedding) {
                            stack.push(bidi_state_t{next_embedding_level,
                                                    directional_override,
                                                    false});
                        } else if (!overflow_isolates) {
                            ++overflow_embedding;
                        }
                    };

                for (auto it = para_first; it != para_last; ++it) {
                    auto const prop = boost::text::bidi_prop(*it);
                    props_and_embeddings.push_back(prop_and_embedding_t{
                        it, stack.top().embedding_, prop, false, false});

                    switch (prop) {
                    case bidi_property::RLE:
                        // https://unicode.org/reports/tr9/#X2
                        explicits(
                            next_odd(stack), directional_override_t::neutral);
                        break;

                    case bidi_property::LRE:
                        // https://unicode.org/reports/tr9/#X3
                        explicits(
                            next_even(stack), directional_override_t::neutral);
                        break;

                    case bidi_property::RLO:
                        // https://unicode.org/reports/tr9/#X4
                        explicits(
                            next_odd(stack),
                            directional_override_t::right_to_left);
                        break;

                    case bidi_property::LRO:
                        // https://unicode.org/reports/tr9/#X5
                        explicits(
                            next_even(stack),
                            directional_override_t::left_to_right);
                        break;

                    case bidi_property::RLI:
                        // https://unicode.org/reports/tr9/#X5a
                        x5a(stack,
                            props_and_embeddings,
                            overflow_isolates,
                            overflow_embedding,
                            valid_isolates);
                        break;
                    case bidi_property::LRI:
                        // https://unicode.org/reports/tr9/#X5b
                        x5b(stack,
                            props_and_embeddings,
                            overflow_isolates,
                            overflow_embedding,
                            valid_isolates);
                        break;
                    case bidi_property::FSI:
                        // https://unicode.org/reports/tr9/#X5c
                        if (p2_p3(std::next(it), matching_pdi(it, para_last)) ==
                            1) {
#if 0
                            std::cout << "fsi pel=" << p2_p3(it, para_last)
                                      << " (x5a)\n";
#endif
                            x5a(stack,
                                props_and_embeddings,
                                overflow_isolates,
                                overflow_embedding,
                                valid_isolates);
                        } else {
#if 0
                            std::cout << "fsi pel=" << p2_p3(it, para_last)
                                      << " (x5b)\n";
#endif
                            x5b(stack,
                                props_and_embeddings,
                                overflow_isolates,
                                overflow_embedding,
                                valid_isolates);
                        }
                        break;

                    case bidi_property::PDI:
                        // https://unicode.org/reports/tr9/#X6a
                        if (0 < overflow_isolates) {
                            --overflow_isolates;
                        } else if (!valid_isolates) {
                            props_and_embeddings.back().unmatched_pdi_ = true;
                        } else {
                            overflow_embedding = 0;
                            while (!stack.top().directional_isolate_) {
                                stack.pop();
                            }
                            assert(!stack.empty());
                            stack.pop();
                            --valid_isolates;
                        }
                        assert(!stack.empty());
                        props_and_embeddings.back().embedding_ =
                            stack.top().embedding_;
                        prop_from_top(stack, props_and_embeddings);
                        break;

                    case bidi_property::PDF:
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
                        break;

                    case bidi_property::B: break;
                    case bidi_property::BN: break;

                    default:
                        // https://unicode.org/reports/tr9/#X6
                        prop_from_top(stack, props_and_embeddings);
                        break;
                    }

#if 0
                    for (auto pae : props_and_embeddings) {
                        std::cout << pae << "\n";
                    }
                    std::cout << std::endl;
#endif
                }

                // https://unicode.org/reports/tr9/#X9
                props_and_embeddings.erase(
                    std::remove_if(
                        props_and_embeddings.begin(),
                        props_and_embeddings.end(),
                        [](prop_and_embedding_t pae) {
                            return pae.prop_ == bidi_property::RLE ||
                                   pae.prop_ == bidi_property::LRE ||
                                   pae.prop_ == bidi_property::RLO ||
                                   pae.prop_ == bidi_property::LRO ||
                                   pae.prop_ == bidi_property::PDF ||
                                   pae.prop_ == bidi_property::BN;
                        }),
                    props_and_embeddings.end());

#if 0
                std::cout << "line:" << __LINE__ << "\n";
                for (auto pae : props_and_embeddings) {
                    std::cout << pae << "\n";
                }
                std::cout << std::endl;
#endif

                // https://unicode.org/reports/tr9/#X10
                auto all_runs = find_all_runs<CPIter>(
                    props_and_embeddings.begin(), props_and_embeddings.end());
                auto run_sequences =
                    find_run_sequences(props_and_embeddings, all_runs);

#if 0
                for (auto pae : props_and_embeddings) {
                    std::cout << pae << "\n";
                }
                std::cout << std::endl;
#endif

                find_sos_eos(run_sequences, paragraph_embedding_level);
                for (auto & run_sequence : run_sequences) {
                    w1(run_sequence);
                    w2(run_sequence);
                    w3(run_sequence);
                    w4(run_sequence);
                    w5(run_sequence);
                    w6(run_sequence);
                    w7(run_sequence);

                    auto const bracket_pairs = find_bracket_pairs(run_sequence);
                    n0(run_sequence, bracket_pairs);
#if 0
                    std::cout << "line:" << __LINE__ << "\n";
                    for (auto pae : props_and_embeddings) {
                        std::cout << pae << "\n";
                    }
                    std::cout << std::endl;
#endif
                    n1(run_sequence);
#if 0
                    std::cout << "line:" << __LINE__ << "\n";
                    for (auto pae : props_and_embeddings) {
                        std::cout << pae << "\n";
                    }
                    std::cout << std::endl;
#endif
                    n2(run_sequence);

                    i1_i2(run_sequence);
                }

#if 0
                std::cout << "line:" << __LINE__ << "\n";
                for (auto pae : props_and_embeddings) {
                    std::cout << pae << "\n";
                }
                std::cout << std::endl;
#endif

                out = emit(
                    props_and_embeddings,
                    paragraph_embedding_level,
                    next_line_break,
                    out);
            }

            return out;
        }
    }

    /** TODO
        TODO: Document that NextLineBreakFunc must be polymorphic, taking an
        iterator whose value_type is uint32_t
        TODO: Accept an optional paragraph_embedding_level (or make an
        overload that takes same), in support of H1.  This replaces the call
        to p2_p3() in X1, but not in X5.
    */
    template<
        typename CPIter,
        typename Sentinel,
        typename OutIter,
        typename NextLineBreakFunc = next_hard_line_break_callable>
    auto bidirectional_order(
        CPIter first,
        Sentinel last,
        OutIter out,
        NextLineBreakFunc && next_line_break = NextLineBreakFunc{})
        -> detail::cp_iter_ret_t<OutIter, CPIter>
    {
        static_assert(
            std::is_same<
                typename std::iterator_traits<OutIter>::value_type,
                bidirectional_subrange<CPIter>>::value,
            "OutIter::value_type must be bidirectional_subrange<CPIter>");

        return detail::bidirectional_order_impl(
            first,
            last,
            out,
            std::forward<NextLineBreakFunc>(next_line_break),
            [](detail::props_and_embeddings_t<CPIter> & props_and_embeddings,
               int paragraph_embedding_level,
               NextLineBreakFunc & next_line_break,
               OutIter out) {
                return emit_bidi_subranges(
                    props_and_embeddings,
                    paragraph_embedding_level,
                    next_line_break,
                    out);
            });
    }

}}

#endif
