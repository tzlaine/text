#ifndef BOOST_TEXT_DETAIL_ITERATOR_HPP
#define BOOST_TEXT_DETAIL_ITERATOR_HPP

#include <boost/text/config.hpp>
#include <boost/stl_interfaces/reverse_iterator.hpp>

#include <iterator>


namespace boost { namespace text { namespace detail {

    using reverse_char_iterator = stl_interfaces::reverse_iterator<char *>;
    using const_reverse_char_iterator =
        stl_interfaces::reverse_iterator<char const *>;


    struct const_repeated_chars_iterator
    {
        using value_type = char;
        using difference_type = std::ptrdiff_t;
        using pointer = char const *;
        using reference = char;
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
            return rhs <= lhs;
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
        stl_interfaces::reverse_iterator<const_repeated_chars_iterator>;

}}}

#endif
