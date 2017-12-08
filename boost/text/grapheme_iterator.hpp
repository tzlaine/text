#ifndef BOOST_TEXT_GRAPHEME_ITERATOR_HPP
#define BOOST_TEXT_GRAPHEME_ITERATOR_HPP

#include <boost/text/config.hpp>
#include <boost/text/grapheme_break.hpp>

#include <iterator>
#include <type_traits>
#include <stdexcept>

#include <cassert>


namespace boost { namespace text {

    template<typename Iter>
    struct grapheme
    {
        Iter begin () const { return first_; }
        Iter end () const { return last_; }
        Iter cbegin () const { return first_; }
        Iter cend () const { return last_; }

        Iter first_;
        Iter last_;

        friend bool operator==(grapheme<Iter> lhs, grapheme<Iter> rhs)
        {
            return lhs.first_ == rhs.first_ && lhs.last_ == rhs.last_;
        }

        friend bool operator!=(grapheme<Iter> lhs, grapheme<Iter> rhs)
        {
            return !(lhs == rhs);
        }
    };

    /** TODO */
    template<typename Iter, typename Sentinel = Iter>
    struct grapheme_iterator
    {
        using value_type = grapheme<Iter>;
        using difference_type = int;
        using pointer = value_type const *;
        using reference = value_type;
        using iterator_category = std::bidirectional_iterator_tag;

        static_assert(
            std::is_same<
                typename std::iterator_traits<Iter>::iterator_category,
                std::bidirectional_iterator_tag>::value ||
                std::is_same<
                    typename std::iterator_traits<Iter>::iterator_category,
                    std::random_access_iterator_tag>::value,
            "grapheme_iterator requires its Iter parameter to be at least "
            "bidirectional.");
        static_assert(
            sizeof(typename std::iterator_traits<Iter>::value_type) == 4,
            "grapheme_iterator requires its Iter parameter to produce a 4-byte "
            "value_type.");

        grapheme_iterator() noexcept : grapheme_{} {}

        grapheme_iterator(
            Iter first, Iter it, Sentinel last) noexcept :
            grapheme_{it, it},
            first_(first),
            last_(last)
        {
            if (first_ != last_) {
                break_ = grapheme_break(break_.fsm_, break_.prop_, *grapheme_.first_);
                find_next_break();
            }
        }

        reference operator*() const noexcept
        {
            return grapheme_;
        }

        pointer operator->() const noexcept
        {
            return &grapheme_;
        }

        Iter base() const noexcept { return grapheme_.first_; }

        grapheme_iterator & operator++() noexcept
        {
            grapheme_.first_ = grapheme_.last_;
            find_next_break();
            return *this;
        }

        grapheme_iterator operator++(int) noexcept
        {
            grapheme_iterator retval = *this;
            ++*this;
            return retval;
        }

        grapheme_iterator & operator--() noexcept
        {
            grapheme_.last_ = grapheme_.first_;
            find_prev_break();
            return *this;
        }

       grapheme_iterator operator--(int) noexcept
        {
            grapheme_iterator retval = *this;
            --*this;
            return retval;
        }

        friend bool operator==(
            grapheme_iterator<Iter> lhs, grapheme_iterator<Iter> rhs) noexcept
        {
            return lhs.grapheme_ == lhs.grapheme_;
        }

        friend bool operator!=(
            grapheme_iterator<Iter> lhs, grapheme_iterator<Iter> rhs) noexcept
        {
            return !(lhs == rhs);
        }

    private:
        void find_next_break() noexcept
        {
            bool new_break = false;
            while (!new_break && grapheme_.last_ != last_) {
                ++grapheme_.last_;
                break_ = grapheme_break(break_.fsm_, break_.prop_, *grapheme_.last_);
                new_break = break_;
            }
        }

        void find_prev_break() noexcept
        {
            if (grapheme_.first_ == first_)
                return;

            // See http://www.unicode.org/reports/tr15/#Stream_Safe_Text_Format
            int const max_steps = 31;

            // GB10
            if (break_.prop_ == grapheme_prop_t::E_Modifier) {
                auto it = grapheme_.first_;
                for (int i = 0; i < max_steps; ++i) {
                    if (it == first_)
                        break;
                    auto const prop = grapheme_prop(*--it);
                    if (prop == grapheme_prop_t::E_Base ||
                        prop == grapheme_prop_t::E_Base_GAZ) {
                        break;
                    } else if (prop != grapheme_prop_t::Extend) {
                        it = grapheme_.first_;
                        break;
                    }
                }
                if (it == grapheme_.first_)
                    it = --grapheme_.first_;
                auto const prop = grapheme_prop(*it);
                break_ = grapheme_break_t(true, prop, grapheme_break_fsm());
            } else if (break_.prop_ == grapheme_prop_t::Regional_Indicator) {
                auto num_ris = 1;
                auto it = grapheme_.first_;
                for (int i = 0; i < max_steps; ++i) {
                    if (it == first_)
                        break;
                    auto const prop = grapheme_prop(*--it);
                    if (prop == grapheme_prop_t::Regional_Indicator) {
                        ++num_ris;
                    } else {
                        break;
                    }
                }
                it = grapheme_.first_;
                --it;
                if ((num_ris & 1) == 0)
                    --it;
                auto const prop = grapheme_prop(*it);
                break_ = grapheme_break_t(true, prop, grapheme_break_fsm());
            } else {
                bool new_break = false;
                while (!new_break && grapheme_.first_ != first_) {
                    auto const prop = grapheme_prop(*--grapheme_.first_);
                    break_ = grapheme_break_t(
                        grapheme_table_break(prop, break_.prop_),
                        prop,
                        break_.fsm_);
                    new_break = break_;
                }
            }
        }

        value_type grapheme_;
        Iter first_;
        Sentinel last_;
        grapheme_break_t break_;
    };

}}

#endif
