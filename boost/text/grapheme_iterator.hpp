#ifndef BOOST_TEXT_GRAPHEME_ITERATOR_HPP
#define BOOST_TEXT_GRAPHEME_ITERATOR_HPP

#include <boost/text/config.hpp>
#include <boost/text/grapheme_break.hpp>

#include <iterator>
#include <type_traits>
#include <stdexcept>

#include <cassert>


namespace boost { namespace text {

    template<typename Iter, typename Sentinel = Iter>
    struct grapheme_iterator;

    /** An immutable range of code points that form an extended grapheme
        cluster. */
    template<typename Iter>
    struct grapheme
    {
        grapheme() noexcept {}
        grapheme(Iter f, Iter l) noexcept : first_(f), last_(l) {}

        Iter begin() const noexcept { return first_; }
        Iter end() const noexcept { return last_; }

    private:
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

        template<typename I, typename S>
        friend struct grapheme_iterator;
    };

    /** A bidirectional filtering iterator that finds the extended grapheme
        clusters in a sequence of code points. */
    template<typename Iter, typename Sentinel>
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

        grapheme_iterator(Iter first, Iter it, Sentinel last) noexcept :
            grapheme_{it, it},
            first_(first),
            last_(last)
        {
            if (grapheme_.first_ != last_)
                find_next_break();
        }

        reference operator*() const noexcept { return grapheme_; }

        pointer operator->() const noexcept { return &grapheme_; }

        Iter base() const noexcept { return grapheme_.first_; }

        grapheme_iterator & operator++() noexcept
        {
            grapheme_.first_ = grapheme_.last_;
            find_next_break();
            return *this;
        }

        grapheme_iterator operator++(int)noexcept
        {
            grapheme_iterator retval = *this;
            ++*this;
            return retval;
        }

        grapheme_iterator & operator--() noexcept
        {
            grapheme_.last_ = grapheme_.first_;
            grapheme_.first_ =
                find_grapheme_start(first_, --grapheme_.first_, last_);
            break_.prop_ = grapheme_prop(*grapheme_.first_);
            break_.fsm_ = grapheme_break_fsm{};
            return *this;
        }

        grapheme_iterator operator--(int)noexcept
        {
            grapheme_iterator retval = *this;
            --*this;
            return retval;
        }

        friend bool operator==(
            grapheme_iterator<Iter, Sentinel> lhs,
            grapheme_iterator<Iter, Sentinel> rhs) noexcept
        {
            return lhs.grapheme_ == lhs.grapheme_;
        }

        friend bool operator!=(
            grapheme_iterator<Iter, Sentinel> lhs,
            grapheme_iterator<Iter, Sentinel> rhs) noexcept
        {
            return !(lhs == rhs);
        }

    private:
        void find_next_break() noexcept
        {
            if (grapheme_.last_ != last_) {
                break_ =
                    grapheme_break(break_.fsm_, break_.prop_, *grapheme_.last_);
                ++grapheme_.last_;
            }
            while (grapheme_.last_ != last_) {
                auto const new_break =
                    grapheme_break(break_.fsm_, break_.prop_, *grapheme_.last_);
                if (new_break)
                    break;
                break_ = new_break;
                ++grapheme_.last_;
            }
        }

        value_type grapheme_;
        Iter first_;
        Sentinel last_;
        grapheme_break_t break_;
    };

}}

#endif
