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
    };

    /** TODO */
    template<typename Iter, typename Sentinel = Iter>
    struct forward_grapheme_iterator
    {
        using value_type = grapheme<Iter>;
        using difference_type = int;
        using pointer = value_type *;
        using reference = value_type;
        using iterator_category = std::forward_iterator_tag; // TODO

#if 0 // TODO: Turn this back on one we have bidirectionality.
        static_assert(
            std::is_same<
                typename std::iterator_traits<Iter>::iterator_category,
                std::bidirectional_iterator_tag>::value ||
                std::is_same<
                    typename std::iterator_traits<Iter>::iterator_category,
                    std::random_access_iterator_tag>::value,
            "forward_grapheme_boundary_iterator requires its Iter parameter "
            "to be at least bidirectional.");
#endif
        static_assert(
            sizeof(typename std::iterator_traits<Iter>::value_type) == 4,
            "forward_grapheme_boundary_iterator requires its Iter parameter to "
            "produce a 4-byte value_type.");

        constexpr forward_grapheme_iterator() noexcept : it_(), next_it_() {}

        // TODO: Should take it, first, last for bidi.
        constexpr forward_grapheme_iterator(Iter first, Sentinel last) noexcept :
            it_(first),
            next_it_(first),
            first_(first),
            last_(last)
        {
            if (first_ != last_) {
                break_ = grapheme_break(break_.fsm_, break_.prop_, *it_);
                find_next_break();
            }
        }

        reference operator*() const noexcept
        {
            return reference{it_, next_it_};
        }

        constexpr Iter base() const noexcept { return it_; }

        forward_grapheme_iterator & operator++() noexcept
        {
            it_ = next_it_;
            find_next_break();
            return *this;
        }

        forward_grapheme_iterator operator++(int) noexcept
        {
            forward_grapheme_iterator retval = *this;
            ++*this;
            return retval;
        }

        friend bool operator==(
            forward_grapheme_iterator<Iter> lhs,
            forward_grapheme_iterator<Iter> rhs) noexcept
        {
            return lhs.it_ == rhs.it_ && rhs.next_it_ == lhs.next_it_;
        }

        friend bool operator!=(
            forward_grapheme_iterator<Iter> lhs,
            forward_grapheme_iterator<Iter> rhs) noexcept
        {
            return !(lhs == rhs);
        }

    private:
        void find_next_break() noexcept
        {
            bool new_break = false;
            while (!new_break && next_it_ != last_) {
                ++next_it_;
                break_ = grapheme_break(break_.fsm_, break_.prop_, *next_it_);
                new_break = break_;
            }
        }

        Iter it_;
        Iter next_it_;
        Iter first_;
        Sentinel last_;
        grapheme_break_t break_;
    };

}}

#endif
