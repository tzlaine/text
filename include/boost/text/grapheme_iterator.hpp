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
        using sentinel_type = Sentinel;

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

        constexpr grapheme_iterator() noexcept = default;

        constexpr grapheme_iterator(
            CPIter first, CPIter it, Sentinel last) noexcept :
            first_(detail::unpack_iterator_and_sentinel(first, last).f_),
            grapheme_first_(detail::unpack_iterator_and_sentinel(it, last).f_),
            grapheme_last_(detail::unpack_iterator_and_sentinel(
                               next_grapheme_break(it, last), last)
                               .f_),
            last_(detail::unpack_iterator_and_sentinel(first, last).l_)
        {}

        template<
            typename CPIter2,
            typename Sentinel2,
            typename Enable = std::enable_if_t<
                std::is_convertible<CPIter2, CPIter>::value &&
                std::is_convertible<Sentinel2, Sentinel>::value>>
        constexpr grapheme_iterator(
            grapheme_iterator<CPIter2, Sentinel2> const & other) :
            first_(other.first_),
            grapheme_first_(other.grapheme_first_),
            grapheme_last_(other.grapheme_last_),
            last_(other.last_)
        {}

        constexpr reference operator*() const noexcept
        {
            return value_type(gr_begin(), gr_end());
        }
        constexpr pointer operator->() const noexcept
        {
            return pointer(**this);
        }

        constexpr CPIter base() const noexcept { return gr_begin(); }

        constexpr grapheme_iterator & operator++() noexcept
        {
            CPIter next_break = next_grapheme_break(gr_end(), seq_end());
            grapheme_first_ = grapheme_last_;
            grapheme_last_ =
                detail::unpack_iterator_and_sentinel(next_break, seq_end()).f_;
            return *this;
        }
        constexpr grapheme_iterator operator++(int)noexcept
        {
            grapheme_iterator retval = *this;
            ++*this;
            return retval;
        }

        constexpr grapheme_iterator & operator--() noexcept
        {
            CPIter prev_break = prev_grapheme_break(
                seq_begin(), std::prev(gr_begin()), seq_end());
            grapheme_last_ = grapheme_first_;
            grapheme_first_ =
                detail::unpack_iterator_and_sentinel(prev_break, seq_end()).f_;
            return *this;
        }
        constexpr grapheme_iterator operator--(int)noexcept
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
        using iterator_t =
            decltype(detail::unpack_iterator_and_sentinel(
                         std::declval<CPIter>(), std::declval<Sentinel>())
                         .f_);
        using sentinel_t =
            decltype(detail::unpack_iterator_and_sentinel(
                         std::declval<CPIter>(), std::declval<Sentinel>())
                         .l_);

        constexpr CPIter seq_begin() const noexcept
        {
            return detail::make_iter<CPIter>(first_, first_, last_);
        }
        constexpr CPIter gr_begin() const noexcept
        {
            return detail::make_iter<CPIter>(first_, grapheme_first_, last_);
        }
        constexpr CPIter gr_end() const noexcept
        {
            return detail::make_iter<CPIter>(first_, grapheme_last_, last_);
        }
        constexpr Sentinel seq_end() const noexcept
        {
            return detail::make_iter<Sentinel>(first_, last_, last_);
        }

        iterator_t first_;
        iterator_t grapheme_first_;
        iterator_t grapheme_last_;
        sentinel_t last_;

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
