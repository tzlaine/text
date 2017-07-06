#ifndef BOOST_TEXT_DETAIL_ITERATOR_HPP
#define BOOST_TEXT_DETAIL_ITERATOR_HPP

#include <iterator>


namespace boost { namespace text { namespace detail {

    struct const_reverse_char_iterator;

    struct reverse_char_iterator
    {
        using value_type = char;
        using difference_type = int;
        using pointer = char *;
        using reference = char &;
        using iterator_category = std::random_access_iterator_tag;

        constexpr reverse_char_iterator () noexcept :
            ptr_ (nullptr)
        {}
        explicit constexpr reverse_char_iterator (char * ptr) noexcept :
            ptr_ (ptr)
        {}

        constexpr char * base () const noexcept
        { return ptr_ + 1; }

        constexpr reference operator* () const noexcept
        { return *ptr_; }

        constexpr value_type operator[] (difference_type n) const noexcept
        { return ptr_[-n]; }

        BOOST_TEXT_CXX14_CONSTEXPR reverse_char_iterator & operator++ () noexcept
        {
            --ptr_;
            return *this;
        }
        BOOST_TEXT_CXX14_CONSTEXPR reverse_char_iterator operator++ (int) noexcept
        {
            reverse_char_iterator retval = *this;
            --ptr_;
            return retval;
        }
        BOOST_TEXT_CXX14_CONSTEXPR reverse_char_iterator & operator+= (difference_type n) noexcept
        {
            ptr_ -= n;
            return *this;
        }

        BOOST_TEXT_CXX14_CONSTEXPR reverse_char_iterator & operator-- () noexcept
        {
            ++ptr_;
            return *this;
        }
        BOOST_TEXT_CXX14_CONSTEXPR reverse_char_iterator operator-- (int) noexcept
        {
            reverse_char_iterator retval = *this;
            ++ptr_;
            return retval;
        }
        BOOST_TEXT_CXX14_CONSTEXPR reverse_char_iterator & operator-= (difference_type n) noexcept
        {
            ptr_ += n;
            return *this;
        }

        friend constexpr bool operator== (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ == rhs.ptr_; }
        friend constexpr bool operator!= (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ != rhs.ptr_; }
        friend constexpr bool operator< (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return rhs.ptr_ < lhs.ptr_; }
        friend constexpr bool operator<= (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return rhs.ptr_ <= lhs.ptr_; }
        friend constexpr bool operator> (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ < rhs.ptr_; }
        friend constexpr bool operator>= (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ <= rhs.ptr_; }

        inline constexpr bool operator== (const_reverse_char_iterator rhs) const noexcept;
        inline constexpr bool operator!= (const_reverse_char_iterator rhs) const noexcept;
        inline constexpr bool operator< (const_reverse_char_iterator rhs) const noexcept;
        inline constexpr bool operator<= (const_reverse_char_iterator rhs) const noexcept;
        inline constexpr bool operator> (const_reverse_char_iterator rhs) const noexcept;
        inline constexpr bool operator>= (const_reverse_char_iterator rhs) const noexcept;

        friend BOOST_TEXT_CXX14_CONSTEXPR reverse_char_iterator operator+ (reverse_char_iterator lhs, difference_type rhs) noexcept
        { return lhs += rhs; }
        friend BOOST_TEXT_CXX14_CONSTEXPR reverse_char_iterator operator+ (difference_type lhs, reverse_char_iterator rhs) noexcept
        { return rhs += lhs; }
        friend BOOST_TEXT_CXX14_CONSTEXPR reverse_char_iterator operator- (reverse_char_iterator lhs, difference_type rhs) noexcept
        { return lhs -= rhs; }
        friend BOOST_TEXT_CXX14_CONSTEXPR reverse_char_iterator operator- (difference_type lhs, reverse_char_iterator rhs) noexcept
        { return rhs -= lhs; }
        friend constexpr difference_type operator- (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return rhs.ptr_ - lhs.ptr_; }

    private:
        char * ptr_;

        friend struct const_reverse_char_iterator;
    };

    struct const_reverse_char_iterator
    {
        using value_type = char;
        using difference_type = int;
        using pointer = char const *;
        using reference = char const;
        using iterator_category = std::random_access_iterator_tag;

        constexpr const_reverse_char_iterator () noexcept :
            ptr_ (nullptr)
        {}
        explicit constexpr const_reverse_char_iterator (char const * ptr) noexcept :
            ptr_ (ptr)
        {}
        constexpr const_reverse_char_iterator (reverse_char_iterator rhs) noexcept :
            ptr_ (rhs.ptr_ + 1)
        {}

        constexpr char const * base () const noexcept
        { return ptr_; }

        constexpr reference operator* () const noexcept
        { return *(ptr_ - 1); }

        constexpr value_type operator[] (difference_type n) const noexcept
        { return ptr_[-n - 1]; }

        BOOST_TEXT_CXX14_CONSTEXPR const_reverse_char_iterator & operator++ () noexcept
        {
            --ptr_;
            return *this;
        }
        BOOST_TEXT_CXX14_CONSTEXPR const_reverse_char_iterator operator++ (int) noexcept
        {
            const_reverse_char_iterator retval = *this;
            --ptr_;
            return retval;
        }
        BOOST_TEXT_CXX14_CONSTEXPR const_reverse_char_iterator & operator+= (difference_type n) noexcept
        {
            ptr_ -= n;
            return *this;
        }

        BOOST_TEXT_CXX14_CONSTEXPR const_reverse_char_iterator & operator-- () noexcept
        {
            ++ptr_;
            return *this;
        }
        BOOST_TEXT_CXX14_CONSTEXPR const_reverse_char_iterator operator-- (int) noexcept
        {
            const_reverse_char_iterator retval = *this;
            ++ptr_;
            return retval;
        }
        BOOST_TEXT_CXX14_CONSTEXPR const_reverse_char_iterator & operator-= (difference_type n) noexcept
        {
            ptr_ += n;
            return *this;
        }

        friend constexpr bool operator== (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ == rhs.ptr_; }
        friend constexpr bool operator!= (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ != rhs.ptr_; }
        friend constexpr bool operator< (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return rhs.ptr_ < lhs.ptr_; }
        friend constexpr bool operator<= (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return rhs.ptr_ <= lhs.ptr_; }
        friend constexpr bool operator> (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ < rhs.ptr_; }
        friend constexpr bool operator>= (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ <= rhs.ptr_; }

        inline constexpr bool operator== (reverse_char_iterator rhs) const noexcept;
        inline constexpr bool operator!= (reverse_char_iterator rhs) const noexcept;
        inline constexpr bool operator< (reverse_char_iterator rhs) const noexcept;
        inline constexpr bool operator<= (reverse_char_iterator rhs) const noexcept;
        inline constexpr bool operator> (reverse_char_iterator rhs) const noexcept;
        inline constexpr bool operator>= (reverse_char_iterator rhs) const noexcept;

        friend BOOST_TEXT_CXX14_CONSTEXPR const_reverse_char_iterator operator+ (const_reverse_char_iterator lhs, difference_type rhs) noexcept
        { return lhs += rhs; }
        friend BOOST_TEXT_CXX14_CONSTEXPR const_reverse_char_iterator operator+ (difference_type lhs, const_reverse_char_iterator rhs) noexcept
        { return rhs += lhs; }
        friend BOOST_TEXT_CXX14_CONSTEXPR const_reverse_char_iterator operator- (const_reverse_char_iterator lhs, difference_type rhs) noexcept
        { return lhs -= rhs; }
        friend BOOST_TEXT_CXX14_CONSTEXPR const_reverse_char_iterator operator- (difference_type lhs, const_reverse_char_iterator rhs) noexcept
        { return rhs -= lhs; }
        friend constexpr difference_type operator- (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return rhs.ptr_ - lhs.ptr_; }

    private:
        char const * ptr_;

        friend struct reverse_char_iterator;
    };

    inline constexpr bool reverse_char_iterator::operator== (const_reverse_char_iterator rhs) const noexcept
    { return ptr_ == rhs.ptr_ - 1; }
    inline constexpr bool reverse_char_iterator::operator!= (const_reverse_char_iterator rhs) const noexcept
    { return ptr_ != rhs.ptr_ - 1; }
    inline constexpr bool reverse_char_iterator::operator< (const_reverse_char_iterator rhs) const noexcept
    { return rhs.ptr_ - 1 < ptr_; }
    inline constexpr bool reverse_char_iterator::operator<= (const_reverse_char_iterator rhs) const noexcept
    { return rhs.ptr_ - 1 <= ptr_; }
    inline constexpr bool reverse_char_iterator::operator> (const_reverse_char_iterator rhs) const noexcept
    { return ptr_ < rhs.ptr_ - 1; }
    inline constexpr bool reverse_char_iterator::operator>= (const_reverse_char_iterator rhs) const noexcept
    { return ptr_ <= rhs.ptr_ - 1; }

    inline constexpr bool const_reverse_char_iterator::operator== (reverse_char_iterator rhs) const noexcept
    { return ptr_ == rhs.ptr_ - 1; }
    inline constexpr bool const_reverse_char_iterator::operator!= (reverse_char_iterator rhs) const noexcept
    { return ptr_ != rhs.ptr_ - 1; }
    inline constexpr bool const_reverse_char_iterator::operator< (reverse_char_iterator rhs) const noexcept
    { return rhs.ptr_ - 1 < ptr_; }
    inline constexpr bool const_reverse_char_iterator::operator<= (reverse_char_iterator rhs) const noexcept
    { return rhs.ptr_ - 1 <= ptr_; }
    inline constexpr bool const_reverse_char_iterator::operator> (reverse_char_iterator rhs) const noexcept
    { return ptr_ < rhs.ptr_ - 1; }
    inline constexpr bool const_reverse_char_iterator::operator>= (reverse_char_iterator rhs) const noexcept
    { return ptr_ <= rhs.ptr_ - 1; }

    struct const_repeated_chars_iterator
    {
        using value_type = char;
        using difference_type = std::ptrdiff_t;
        using pointer = char const *;
        using reference = char const;
        using iterator_category = std::random_access_iterator_tag;

        constexpr const_repeated_chars_iterator () noexcept :
            first_ (nullptr),
            size_ (0),
            n_ (0)
        {}
        constexpr const_repeated_chars_iterator (
            char const * first,
            difference_type size,
            difference_type n
        ) noexcept :
            first_ (first),
            size_ (size),
            n_ (n)
        {}

        constexpr reference operator* () const noexcept
        { return first_[n_ % size_]; }

        constexpr value_type operator[] (difference_type n) const noexcept
        { return first_[(n_ + n) % size_]; }

        BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator & operator++ () noexcept
        {
            ++n_;
            return *this;
        }
        BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator operator++ (int) noexcept
        {
            const_repeated_chars_iterator retval = *this;
            ++*this;
            return retval;
        }
        BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator & operator+= (difference_type n) noexcept
        {
            n_ += n;
            return *this;
        }

        BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator & operator-- () noexcept
        {
            --n_;
            return *this;
        }
        BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator operator-- (int) noexcept
        {
            const_repeated_chars_iterator retval = *this;
            --*this;
            return retval;
        }
        BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator & operator-= (difference_type n) noexcept
        {
            n_ -= n;
            return *this;
        }

        friend constexpr bool operator== (const_repeated_chars_iterator lhs, const_repeated_chars_iterator rhs) noexcept
        { return lhs.first_ == rhs.first_ && lhs.n_ == rhs.n_; }
        friend constexpr bool operator!= (const_repeated_chars_iterator lhs, const_repeated_chars_iterator rhs) noexcept
        { return !(lhs == rhs); }
        friend constexpr bool operator< (const_repeated_chars_iterator lhs, const_repeated_chars_iterator rhs) noexcept
        { return lhs.first_ == rhs.first_ && lhs.n_ < rhs.n_; }
        friend constexpr bool operator<= (const_repeated_chars_iterator lhs, const_repeated_chars_iterator rhs) noexcept
        { return lhs == rhs || lhs < rhs; }
        friend constexpr bool operator> (const_repeated_chars_iterator lhs, const_repeated_chars_iterator rhs) noexcept
        { return rhs < lhs; }
        friend constexpr bool operator>= (const_repeated_chars_iterator lhs, const_repeated_chars_iterator rhs) noexcept
        { return lhs <= rhs; }

        friend BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator operator+ (const_repeated_chars_iterator lhs, difference_type rhs) noexcept
        { return lhs += rhs; }
        friend BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator operator+ (difference_type lhs, const_repeated_chars_iterator rhs) noexcept
        { return rhs += lhs; }
        friend BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator operator- (const_repeated_chars_iterator lhs, difference_type rhs) noexcept
        { return lhs -= rhs; }
        friend BOOST_TEXT_CXX14_CONSTEXPR const_repeated_chars_iterator operator- (difference_type lhs, const_repeated_chars_iterator rhs) noexcept
        { return rhs -= lhs; }
        friend constexpr difference_type operator- (const_repeated_chars_iterator lhs, const_repeated_chars_iterator rhs) noexcept
        { return lhs.n_ - rhs.n_; }

    private:
        char const * first_;
        difference_type size_;
        difference_type n_;
    };

    struct const_reverse_repeated_chars_iterator
    {
        using value_type = char;
        using difference_type = std::ptrdiff_t;
        using pointer = char const *;
        using reference = char const;
        using iterator_category = std::random_access_iterator_tag;

        constexpr const_reverse_repeated_chars_iterator () noexcept :
            base_ ()
        {}
        explicit constexpr const_reverse_repeated_chars_iterator (const_repeated_chars_iterator it) noexcept :
            base_ (it)
        {}

        constexpr const_repeated_chars_iterator base () const
        { return base_; }

        BOOST_TEXT_CXX14_CONSTEXPR reference operator* () const noexcept
        { return *(base_ - 1); }
        constexpr value_type operator[] (difference_type n) const noexcept
        { return base_[-n - 1]; }

        BOOST_TEXT_CXX14_CONSTEXPR const_reverse_repeated_chars_iterator & operator++ () noexcept
        { --base_; return *this; }
        BOOST_TEXT_CXX14_CONSTEXPR const_reverse_repeated_chars_iterator operator++ (int) noexcept
        {
            const_reverse_repeated_chars_iterator retval = *this;
            --base_;
            return retval;
        }
        BOOST_TEXT_CXX14_CONSTEXPR const_reverse_repeated_chars_iterator & operator+= (difference_type n) noexcept
        { base_ -= n; return *this; }

        BOOST_TEXT_CXX14_CONSTEXPR const_reverse_repeated_chars_iterator & operator-- () noexcept
        { ++base_; return *this; }
        BOOST_TEXT_CXX14_CONSTEXPR const_reverse_repeated_chars_iterator operator-- (int) noexcept
        {
            const_reverse_repeated_chars_iterator retval = *this;
            ++base_;
            return retval;
        }
        BOOST_TEXT_CXX14_CONSTEXPR const_reverse_repeated_chars_iterator & operator-= (difference_type n) noexcept
        { base_ += n; return *this; }

        friend constexpr bool operator== (const_reverse_repeated_chars_iterator lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return lhs.base_ == rhs.base_; }
        friend constexpr bool operator!= (const_reverse_repeated_chars_iterator lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return !(lhs == rhs); }
        friend constexpr bool operator< (const_reverse_repeated_chars_iterator lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return rhs.base_ < lhs.base_; }
        friend constexpr bool operator<= (const_reverse_repeated_chars_iterator lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return rhs.base_ <= lhs.base_; }
        friend constexpr bool operator> (const_reverse_repeated_chars_iterator lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return rhs.base_ > lhs.base_; }
        friend constexpr bool operator>= (const_reverse_repeated_chars_iterator lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return rhs.base_ >= lhs.base_; }

        friend BOOST_TEXT_CXX14_CONSTEXPR const_reverse_repeated_chars_iterator operator+ (const_reverse_repeated_chars_iterator lhs, difference_type rhs) noexcept
        { return lhs += rhs; }
        friend BOOST_TEXT_CXX14_CONSTEXPR const_reverse_repeated_chars_iterator operator+ (difference_type lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return rhs += lhs; }
        friend BOOST_TEXT_CXX14_CONSTEXPR const_reverse_repeated_chars_iterator operator- (const_reverse_repeated_chars_iterator lhs, difference_type rhs) noexcept
        { return lhs -= rhs; }
        friend BOOST_TEXT_CXX14_CONSTEXPR const_reverse_repeated_chars_iterator operator- (difference_type lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return rhs -= lhs; }
        friend constexpr difference_type operator- (const_reverse_repeated_chars_iterator lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return rhs.base_ - lhs.base_; }

    private:
        const_repeated_chars_iterator base_;
    };

} } }

#endif
