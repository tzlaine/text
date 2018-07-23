#ifndef BOOST_TEXT_DETAIL_ITERATOR_HPP
#define BOOST_TEXT_DETAIL_ITERATOR_HPP

#include <boost/text/config.hpp>

#include <iterator>


namespace boost { namespace text { namespace detail {

    template<typename Iter>
    constexpr Iter prev(Iter it)
    {
        return --it;
    }

    template<typename Iter>
    struct reverse_iterator
    {
        using iterator_category =
            typename std::iterator_traits<Iter>::iterator_category;
        using value_type = typename std::iterator_traits<Iter>::value_type;
        using difference_type =
            typename std::iterator_traits<Iter>::difference_type;
        using pointer = typename std::iterator_traits<Iter>::pointer;
        using reference = typename std::iterator_traits<Iter>::reference;
        using iterator_type = Iter;

        constexpr reverse_iterator() noexcept : it_() {}
        explicit constexpr reverse_iterator(iterator_type it) noexcept : it_(it)
        {}

        constexpr reverse_iterator(reverse_iterator const & other) noexcept :
            it_(other.it_)
        {}

        template<typename Iter2>
        constexpr reverse_iterator(
            const reverse_iterator<Iter2> & other) noexcept :
            it_(other.base())
        {}

        constexpr iterator_type base() const noexcept { return it_; }

        constexpr reference operator*() const noexcept { return *prev(it_); }

        constexpr pointer operator->() const noexcept { return &**this; }

        constexpr reference operator[](difference_type n) const noexcept
        {
            return *(it_ - n - 1);
        }

        constexpr reverse_iterator operator+(difference_type n) const noexcept
        {
            return reverse_iterator(it_ - n);
        }

        constexpr reverse_iterator operator-(difference_type n) const noexcept
        {
            return reverse_iterator(it_ + n);
        }

        BOOST_TEXT_CXX14_CONSTEXPR reverse_iterator & operator++() noexcept
        {
            --it_;
            return *this;
        }

        BOOST_TEXT_CXX14_CONSTEXPR reverse_iterator operator++(int)noexcept
        {
            reverse_iterator retval = *this;
            --it_;
            return retval;
        }

        BOOST_TEXT_CXX14_CONSTEXPR reverse_iterator & operator--() noexcept
        {
            ++it_;
            return *this;
        }

        BOOST_TEXT_CXX14_CONSTEXPR reverse_iterator operator--(int)noexcept
        {
            reverse_iterator retval = *this;
            ++it_;
            return retval;
        }

        BOOST_TEXT_CXX14_CONSTEXPR reverse_iterator &
        operator+=(difference_type n) noexcept
        {
            it_ -= n;
            return *this;
        }

        BOOST_TEXT_CXX14_CONSTEXPR reverse_iterator &
        operator-=(difference_type n) noexcept
        {
            it_ += n;
            return *this;
        }

    private:
        Iter it_;
    };

    template<typename Iter>
    constexpr bool operator==(
        reverse_iterator<Iter> const & lhs,
        reverse_iterator<Iter> const & rhs) noexcept
    {
        return lhs.base() == rhs.base();
    }

    template<typename Iter>
    constexpr bool operator<(
        reverse_iterator<Iter> const & lhs,
        reverse_iterator<Iter> const & rhs) noexcept
    {
        return rhs.base() < lhs.base();
    }

    template<typename Iter>
    constexpr bool operator!=(
        reverse_iterator<Iter> const & lhs,
        reverse_iterator<Iter> const & rhs) noexcept
    {
        return !(lhs == rhs);
    }

    template<typename Iter>
    constexpr bool operator>(
        reverse_iterator<Iter> const & lhs,
        reverse_iterator<Iter> const & rhs) noexcept
    {
        return rhs < lhs;
    }

    template<typename Iter>
    constexpr bool operator<=(
        reverse_iterator<Iter> const & lhs,
        reverse_iterator<Iter> const & rhs) noexcept
    {
        return !(rhs < lhs);
    }

    template<typename Iter>
    constexpr bool operator>=(
        reverse_iterator<Iter> const & lhs,
        reverse_iterator<Iter> const & rhs) noexcept
    {
        return !(lhs < rhs);
    }

    template<typename Iter1, typename Iter2>
    constexpr bool operator==(
        reverse_iterator<Iter1> const & lhs,
        reverse_iterator<Iter2> const & rhs) noexcept
    {
        return lhs.base() == rhs.base();
    }

    template<typename Iter1, typename Iter2>
    constexpr bool operator<(
        reverse_iterator<Iter1> const & lhs,
        reverse_iterator<Iter2> const & rhs) noexcept
    {
        return rhs.base() < lhs.base();
    }

    template<typename Iter1, typename Iter2>
    constexpr bool operator!=(
        reverse_iterator<Iter1> const & lhs,
        reverse_iterator<Iter2> const & rhs) noexcept
    {
        return !(lhs == rhs);
    }

    template<typename Iter1, typename Iter2>
    constexpr bool operator>(
        reverse_iterator<Iter1> const & lhs,
        reverse_iterator<Iter2> const & rhs) noexcept
    {
        return rhs < lhs;
    }

    template<typename Iter1, typename Iter2>
    constexpr bool operator<=(
        reverse_iterator<Iter1> const & lhs,
        reverse_iterator<Iter2> const & rhs) noexcept
    {
        return !(rhs < lhs);
    }

    template<typename Iter1, typename Iter2>
    constexpr bool operator>=(
        reverse_iterator<Iter1> const & lhs,
        reverse_iterator<Iter2> const & rhs) noexcept
    {
        return !(lhs < rhs);
    }

    template<typename Iter1, typename Iter2>
    constexpr auto operator-(
        reverse_iterator<Iter1> const & lhs,
        reverse_iterator<Iter2> const & rhs) noexcept
        -> decltype(rhs.base() - lhs.base())
    {
        return rhs.base() - lhs.base();
    }

    using reverse_char_iterator = reverse_iterator<char *>;
    using const_reverse_char_iterator = reverse_iterator<char const *>;


    struct const_repeated_chars_iterator
    {
        using value_type = char;
        using difference_type = std::ptrdiff_t;
        using pointer = char const *;
        using reference = char const;
        using iterator_category = std::random_access_iterator_tag;

        constexpr const_repeated_chars_iterator() noexcept :
            first_(nullptr),
            size_(0),
            n_(0)
        {}
        constexpr const_repeated_chars_iterator(
            char const * first,
            difference_type size,
            difference_type n) noexcept :
            first_(first),
            size_(size),
            n_(n)
        {}

        constexpr reference operator*() const noexcept
        {
            return first_[n_ % size_];
        }

        constexpr value_type operator[](difference_type n) const noexcept
        {
            return first_[(n_ + n) % size_];
        }

        BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator &
        operator++() noexcept
        {
            ++n_;
            return *this;
        }
        BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator
        operator++(int)noexcept
        {
            const_repeated_chars_iterator retval = *this;
            ++*this;
            return retval;
        }
        BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator &
        operator+=(difference_type n) noexcept
        {
            n_ += n;
            return *this;
        }

        BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator &
        operator--() noexcept
        {
            --n_;
            return *this;
        }
        BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator
        operator--(int)noexcept
        {
            const_repeated_chars_iterator retval = *this;
            --*this;
            return retval;
        }
        BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator &
        operator-=(difference_type n) noexcept
        {
            n_ -= n;
            return *this;
        }

        friend constexpr bool operator==(
            const_repeated_chars_iterator lhs,
            const_repeated_chars_iterator rhs) noexcept
        {
            return lhs.first_ == rhs.first_ && lhs.n_ == rhs.n_;
        }
        friend constexpr bool operator!=(
            const_repeated_chars_iterator lhs,
            const_repeated_chars_iterator rhs) noexcept
        {
            return !(lhs == rhs);
        }
        friend constexpr bool operator<(
            const_repeated_chars_iterator lhs,
            const_repeated_chars_iterator rhs) noexcept
        {
            return lhs.first_ == rhs.first_ && lhs.n_ < rhs.n_;
        }
        friend constexpr bool operator<=(
            const_repeated_chars_iterator lhs,
            const_repeated_chars_iterator rhs) noexcept
        {
            return lhs == rhs || lhs < rhs;
        }
        friend constexpr bool operator>(
            const_repeated_chars_iterator lhs,
            const_repeated_chars_iterator rhs) noexcept
        {
            return rhs < lhs;
        }
        friend constexpr bool operator>=(
            const_repeated_chars_iterator lhs,
            const_repeated_chars_iterator rhs) noexcept
        {
            return lhs <= rhs;
        }

        friend BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator
        operator+(
            const_repeated_chars_iterator lhs, difference_type rhs) noexcept
        {
            return lhs += rhs;
        }
        friend BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator
        operator+(
            difference_type lhs, const_repeated_chars_iterator rhs) noexcept
        {
            return rhs += lhs;
        }
        friend BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator
        operator-(
            const_repeated_chars_iterator lhs, difference_type rhs) noexcept
        {
            return lhs -= rhs;
        }
        friend BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator
        operator-(
            difference_type lhs, const_repeated_chars_iterator rhs) noexcept
        {
            return rhs -= lhs;
        }
        friend constexpr difference_type operator-(
            const_repeated_chars_iterator lhs,
            const_repeated_chars_iterator rhs) noexcept
        {
            return lhs.n_ - rhs.n_;
        }

    private:
        char const * first_;
        difference_type size_;
        difference_type n_;
    };

    using const_reverse_repeated_chars_iterator =
        reverse_iterator<const_repeated_chars_iterator>;

}}}

#endif
