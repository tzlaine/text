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


namespace boost { namespace text { inline namespace v1 {

    /** A bidirectional filtering iterator that iterates over the extended
        grapheme clusters in a sequence of code points. */
    template<typename CPIter, typename Sentinel = CPIter>
    struct grapheme_iterator
    {
        using value_type = grapheme_ref<CPIter>;
        using difference_type = std::ptrdiff_t;
        using pointer = stl_interfaces::proxy_arrow_result<value_type>;
        using reference = value_type;
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_type = CPIter;

        static_assert(
            detail::is_cp_iter<CPIter>::value,
            "CPIter must be a code point iterator");
        static_assert(
            std::is_same<
                typename std::iterator_traits<CPIter>::iterator_category,
                std::bidirectional_iterator_tag>::value ||
                std::is_same<
                    typename std::iterator_traits<CPIter>::iterator_category,
                    std::random_access_iterator_tag>::value,
            "grapheme_iterator requires its CPIter parameter to be at least "
            "bidirectional.");

        grapheme_iterator() noexcept : grapheme_{} {}

        grapheme_iterator(CPIter first, CPIter it, Sentinel last) noexcept :
            grapheme_{it, next_grapheme_break(it, last)},
            first_(first),
            last_(last)
        {}

        template<
            typename CPIter2,
            typename Sentinel2,
            typename Enable = std::enable_if_t<
                std::is_convertible<CPIter2, CPIter>::value &&
                std::is_convertible<Sentinel2, Sentinel>::value>>
        grapheme_iterator(grapheme_iterator<CPIter2, Sentinel2> const & other) :
            grapheme_(other.grapheme_.begin(), other.grapheme_.end()),
            first_(other.first_),
            last_(other.last_)
        {}

        reference operator*() const noexcept { return grapheme_; }
        pointer operator->() const noexcept { return pointer(grapheme_); }

        CPIter base() const noexcept { return grapheme_.begin(); }

        grapheme_iterator & operator++() noexcept
        {
            auto const first = grapheme_.end();
            grapheme_ = value_type(first, next_grapheme_break(first, last_));
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
            auto const last = grapheme_.begin();
            grapheme_ = value_type(
                prev_grapheme_break(first_, std::prev(last), last_), last);
            return *this;
        }
        grapheme_iterator operator--(int)noexcept
        {
            grapheme_iterator retval = *this;
            --*this;
            return retval;
        }

        friend constexpr bool
        operator==(grapheme_iterator lhs, grapheme_iterator rhs) noexcept
        {
            return lhs.base() == rhs.base();
        }
        friend constexpr bool
        operator!=(grapheme_iterator lhs, grapheme_iterator rhs) noexcept
        {
            return !(lhs == rhs);
        }

    private:
        value_type grapheme_;
        CPIter first_;
        Sentinel last_;

        template<typename CPIter2, typename Sentinel2>
        friend struct grapheme_iterator;
    };

    /** This function is constexpr in C++14 and later. */
    template<
        typename Iter1,
        typename Sentinel1,
        typename Iter2,
        typename Sentinel2,
        typename Enable = std::enable_if_t<
            std::is_same<Sentinel1, null_sentinel>::value !=
            std::is_same<Sentinel2, null_sentinel>::value>>
    BOOST_TEXT_CXX14_CONSTEXPR auto operator==(
        grapheme_iterator<Iter1, Sentinel1> const & lhs,
        grapheme_iterator<Iter2, Sentinel2> const & rhs) noexcept
        -> decltype(lhs.base() == rhs.base())
    {
        return lhs.base() == rhs.base();
    }

    /** This function is constexpr in C++14 and later. */
    template<
        typename Iter1,
        typename Sentinel1,
        typename Iter2,
        typename Sentinel2,
        typename Enable = std::enable_if_t<
            std::is_same<Sentinel1, null_sentinel>::value !=
            std::is_same<Sentinel2, null_sentinel>::value>>
    BOOST_TEXT_CXX14_CONSTEXPR auto operator!=(
        grapheme_iterator<Iter1, Sentinel1> const & lhs,
        grapheme_iterator<Iter2, Sentinel2> const & rhs) noexcept
        -> decltype(!(lhs == rhs))
    {
        return !(lhs == rhs);
    }

    /** This function is constexpr in C++14 and later. */
    template<typename CPIter, typename Sentinel>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    operator==(grapheme_iterator<CPIter, Sentinel> it, Sentinel s) noexcept
        -> decltype(it.base() == s)
    {
        return it.base() == s;
    }

    /** This function is constexpr in C++14 and later. */
    template<typename CPIter, typename Sentinel>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    operator==(Sentinel s, grapheme_iterator<CPIter, Sentinel> it) noexcept
        -> decltype(it.base() == s)
    {
        return it.base() == s;
    }

    /** This function is constexpr in C++14 and later. */
    template<typename CPIter, typename Sentinel>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    operator!=(grapheme_iterator<CPIter, Sentinel> it, Sentinel s) noexcept
        -> decltype(it.base() != s)
    {
        return it.base() != s;
    }

    /** This function is constexpr in C++14 and later. */
    template<typename CPIter, typename Sentinel>
    BOOST_TEXT_CXX14_CONSTEXPR auto
    operator!=(Sentinel s, grapheme_iterator<CPIter, Sentinel> it) noexcept
        -> decltype(it.base() != s)
    {
        return it.base() != s;
    }

}}}

#endif
