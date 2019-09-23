#ifndef BOOST_TEXT_DETAIL_ITERATOR_HPP
#define BOOST_TEXT_DETAIL_ITERATOR_HPP

#include <boost/text/config.hpp>
#include <boost/stl_interfaces/reverse_iterator.hpp>

#include <iterator>


namespace boost { namespace text { namespace detail {

    using reverse_char_iterator = stl_interfaces::reverse_iterator<char *>;
    using const_reverse_char_iterator =
        stl_interfaces::reverse_iterator<char const *>;


    struct const_repeated_chars_iterator : stl_interfaces::iterator_interface<
                                               const_repeated_chars_iterator,
                                               std::random_access_iterator_tag,
                                               char,
                                               char>
    {
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

        constexpr char operator*() const noexcept { return first_[n_ % size_]; }

        BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator &
        operator+=(std::ptrdiff_t n) noexcept
        {
            n_ += n;
            return *this;
        }

        friend constexpr std::ptrdiff_t operator-(
            const_repeated_chars_iterator lhs,
            const_repeated_chars_iterator rhs) noexcept
        {
            return lhs.n_ - rhs.n_;
        }

    private:
        char const * first_;
        std::ptrdiff_t size_;
        std::ptrdiff_t n_;
    };

    using const_reverse_repeated_chars_iterator =
        stl_interfaces::reverse_iterator<const_repeated_chars_iterator>;

}}}

#endif
