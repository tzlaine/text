// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_GRAPHEME_ITERATOR_HPP
#define BOOST_TEXT_GRAPHEME_ITERATOR_HPP

#include <boost/text/config.hpp>
#include <boost/text/grapheme.hpp>
#include <boost/text/grapheme_break.hpp>

#include <boost/assert.hpp>
#include <boost/stl_interfaces/iterator_interface.hpp>

#include <iterator>
#include <type_traits>
#include <stdexcept>


namespace boost { namespace text {

    /** A bidirectional filtering iterator that iterates over the extended
        grapheme clusters in a sequence of code points. */
#if BOOST_TEXT_USE_CONCEPTS
    template<code_point_iter I, std::sentinel_for<I> S = I>
#else
    template<typename I, typename S = I>
#endif
    struct grapheme_iterator
    {
        using value_type = grapheme_ref<I>;
        using difference_type = std::ptrdiff_t;
        using pointer = stl_interfaces::proxy_arrow_result<value_type>;
        using reference = value_type;
        using iterator_category = std::bidirectional_iterator_tag;

        using iterator = I;
        using sentinel = S;

#if !BOOST_TEXT_USE_CONCEPTS
        static_assert(
            detail::is_cp_iter_v<I>, "I must be a code point iterator");
        static_assert(
            std::is_same<
                typename std::iterator_traits<I>::iterator_category,
                std::bidirectional_iterator_tag>::value ||
                std::is_same<
                    typename std::iterator_traits<I>::iterator_category,
                    std::random_access_iterator_tag>::value,
            "grapheme_iterator requires its I parameter to be at least "
            "bidirectional.");
#endif

        constexpr grapheme_iterator() :
            first_(), grapheme_first_(), grapheme_last_(), last_()
        {}

        constexpr grapheme_iterator(
            iterator first, iterator it, sentinel last) :
            first_(detail::unpack_iterator_and_sentinel(first, last).f_),
            grapheme_first_(detail::unpack_iterator_and_sentinel(it, last).f_),
            grapheme_last_(detail::unpack_iterator_and_sentinel(
                               boost::text::next_grapheme_break(it, last), last)
                               .f_),
            last_(detail::unpack_iterator_and_sentinel(first, last).l_)
        {}

#if BOOST_TEXT_USE_CONCEPTS
        template<code_point_iter I2, std::sentinel_for<I2> S2>
        requires std::convertible_to<I2, I> && std::convertible_to<S2, S>
#else
        template<
            typename I2,
            typename S2,
            typename Enable = std::enable_if_t<
                std::is_convertible<I2, iterator>::value &&
                std::is_convertible<S2, sentinel>::value>>
#endif
        constexpr grapheme_iterator(grapheme_iterator<I2, S2> const & other) :
            first_(other.first_),
            grapheme_first_(other.grapheme_first_),
            grapheme_last_(other.grapheme_last_),
            last_(other.last_)
        {}

        constexpr reference operator*() const
        {
            return value_type(gr_begin(), gr_end());
        }
        constexpr pointer operator->() const
        {
            return pointer(**this);
        }

        constexpr iterator base() const { return gr_begin(); }

        constexpr grapheme_iterator & operator++()
        {
            iterator next_break =
                boost::text::next_grapheme_break(gr_end(), seq_end());
            grapheme_first_ = grapheme_last_;
            grapheme_last_ =
                detail::unpack_iterator_and_sentinel(next_break, seq_end()).f_;
            return *this;
        }
        constexpr grapheme_iterator operator++(int)
        {
            grapheme_iterator retval = *this;
            ++*this;
            return retval;
        }

        constexpr grapheme_iterator & operator--()
        {
            iterator prev_break = boost::text::prev_grapheme_break(
                seq_begin(), std::prev(gr_begin()), seq_end());
            grapheme_last_ = grapheme_first_;
            grapheme_first_ =
                detail::unpack_iterator_and_sentinel(prev_break, seq_end()).f_;
            return *this;
        }
        constexpr grapheme_iterator operator--(int)
        {
            grapheme_iterator retval = *this;
            --*this;
            return retval;
        }

        friend constexpr bool
        operator==(grapheme_iterator lhs, grapheme_iterator rhs)
        {
            return lhs.base() == rhs.base();
        }
        friend constexpr bool
        operator!=(grapheme_iterator lhs, grapheme_iterator rhs)
        {
            return !(lhs == rhs);
        }

    private:
        using cu_iterator =
            decltype(detail::unpack_iterator_and_sentinel(
                         std::declval<iterator>(), std::declval<sentinel>())
                         .f_);
        using cu_sentinel =
            decltype(detail::unpack_iterator_and_sentinel(
                         std::declval<iterator>(), std::declval<sentinel>())
                         .l_);

        constexpr iterator seq_begin() const
        {
            return detail::make_iter<iterator>(first_, first_, last_);
        }
        constexpr iterator gr_begin() const
        {
            return detail::make_iter<iterator>(first_, grapheme_first_, last_);
        }
        constexpr iterator gr_end() const
        {
            return detail::make_iter<iterator>(first_, grapheme_last_, last_);
        }
        constexpr sentinel seq_end() const
        {
            return detail::make_iter<sentinel>(first_, last_, last_);
        }

        cu_iterator first_;
        cu_iterator grapheme_first_;
        cu_iterator grapheme_last_;
        cu_sentinel last_;

#if BOOST_TEXT_USE_CONCEPTS
        template<code_point_iter I2, std::sentinel_for<I2> S2>
#else
        template<typename I2, typename S2>
#endif
        friend struct grapheme_iterator;
    };

    template<
        typename Iter1,
        typename Sentinel1,
        typename Iter2,
        typename Sentinel2,
        typename Enable = std::enable_if_t<
            std::is_same<Sentinel1, null_sentinel_t>::value !=
            std::is_same<Sentinel2, null_sentinel_t>::value>>
    constexpr auto operator==(
        grapheme_iterator<Iter1, Sentinel1> const & lhs,
        grapheme_iterator<Iter2, Sentinel2> const & rhs)
        -> decltype(lhs.base() == rhs.base())
    {
        return lhs.base() == rhs.base();
    }

    template<
        typename Iter1,
        typename Sentinel1,
        typename Iter2,
        typename Sentinel2,
        typename Enable = std::enable_if_t<
            std::is_same<Sentinel1, null_sentinel_t>::value !=
            std::is_same<Sentinel2, null_sentinel_t>::value>>
    constexpr auto operator!=(
        grapheme_iterator<Iter1, Sentinel1> const & lhs,
        grapheme_iterator<Iter2, Sentinel2> const & rhs)
        -> decltype(!(lhs == rhs))
    {
        return !(lhs == rhs);
    }

    template<typename CPIter, typename Sentinel>
    constexpr auto
    operator==(grapheme_iterator<CPIter, Sentinel> it, Sentinel s)
        -> decltype(it.base() == s)
    {
        return it.base() == s;
    }

    template<typename CPIter, typename Sentinel>
    constexpr auto
    operator==(Sentinel s, grapheme_iterator<CPIter, Sentinel> it)
        -> decltype(it.base() == s)
    {
        return it.base() == s;
    }

    template<typename CPIter, typename Sentinel>
    constexpr auto
    operator!=(grapheme_iterator<CPIter, Sentinel> it, Sentinel s)
        -> decltype(it.base() != s)
    {
        return it.base() != s;
    }

    template<typename CPIter, typename Sentinel>
    constexpr auto
    operator!=(Sentinel s, grapheme_iterator<CPIter, Sentinel> it)
        -> decltype(it.base() != s)
    {
        return it.base() != s;
    }

}}

#endif
