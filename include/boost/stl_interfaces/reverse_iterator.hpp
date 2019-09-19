// Copyright (C) 2019 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_STL_INTERFACES_REVERSE_ITERATOR_HPP
#define BOOST_STL_INTERFACES_REVERSE_ITERATOR_HPP

#include <boost/stl_interfaces/iterator_interface.hpp>


namespace boost { namespace stl_interfaces { inline namespace v1 {

    /** This type is very similar to the C++20 version of
        `std::reverse_iterator`; it is `constexpr`-, `noexcept`-, and
        proxy-friendly. */
    template<typename BidiIter>
    struct reverse_iterator
        : iterator_interface<
              reverse_iterator<BidiIter>,
#if 201703L < __cplusplus && defined(__cpp_lib_ranges)
              typename std::iterator_traits<BidiIter>::iterator_concept,
#else
              typename std::iterator_traits<BidiIter>::iterator_category,
#endif
              typename std::iterator_traits<BidiIter>::value_type,
              typename std::iterator_traits<BidiIter>::reference,
              typename std::iterator_traits<BidiIter>::pointer,
              typename std::iterator_traits<BidiIter>::difference_type>
    {
        constexpr reverse_iterator() noexcept(noexcept(BidiIter())) : it_() {}
        constexpr reverse_iterator(BidiIter it) noexcept(
            noexcept(BidiIter(it))) :
            it_(it)
        {}
        template<
            typename BidiIter2,
            typename E = std::enable_if_t<
                std::is_convertible<BidiIter2, BidiIter>::value>>
        reverse_iterator(reverse_iterator<BidiIter2> it) : it_(it.it_)
        {}

        friend BOOST_STL_INTERFACES_HIDDEN_FRIEND_CONSTEXPR bool operator==(
            reverse_iterator lhs,
            reverse_iterator rhs) noexcept(noexcept(lhs.it_ == rhs.it_))
        {
            return lhs.it_ == rhs.it_;
        }

        friend BOOST_STL_INTERFACES_HIDDEN_FRIEND_CONSTEXPR auto
        operator-(reverse_iterator lhs, reverse_iterator rhs) noexcept(
            noexcept(std::distance(lhs.it_, rhs.it_)))
        {
            return -std::distance(rhs.it_, lhs.it_);
        }

        constexpr typename std::iterator_traits<BidiIter>::reference
        operator*() const
            noexcept(noexcept(std::prev(std::declval<BidiIter &>())))
        {
            return *std::prev(it_);
        }

        constexpr reverse_iterator & operator+=(
            typename std::iterator_traits<BidiIter>::difference_type
                n) noexcept(noexcept(std::
                                         advance(
                                             std::declval<BidiIter &>(), -n)))
        {
            std::advance(it_, -n);
            return *this;
        }

        constexpr BidiIter base() const noexcept { return it_; }

    private:
        friend access;
        constexpr BidiIter & base_reference() noexcept { return it_; }
        constexpr BidiIter const & base_reference() const noexcept
        {
            return it_;
        }

        template<typename BidiIter2>
        friend struct reverse_iterator;

        BidiIter it_;
    };

    /** Makes a `reverse_iterator<BidiIter>` from an iterator of type
        `Bidiiter`. */
    template<typename BidiIter>
    auto make_reverse_iterator(BidiIter it)
    {
        return reverse_iterator<BidiIter>(it);
    }

}}}


#if 201703L < __cplusplus && defined(__cpp_lib_concepts) || BOOST_STL_INTERFACES_DOXYGEN

namespace boost { namespace stl_interfaces { namespace v2 {

    /** A template alias for `std::reverse_iterator`.  This only exists to
        make migration from Boost.STLInterfaces to C++20 easier; switch to the
        one in `std` as soon as you can. */
    template<typename BidiIter>
    using reverse_iterator = std::reverse_iterator<BidiIter>;


    /** Makes a `reverse_iterator<BidiIter>` from an iterator of type
        `Bidiiter`.  This only exists to make migration from
        Boost.STLInterfaces to C++20 easier; switch to the one in `std` as
        soon as you can. */
    template<typename BidiIter>
    auto make_reverse_iterator(BidiIter it)
    {
        return reverse_iterator<BidiIter>(it);
    }

}}}

#endif

#endif
