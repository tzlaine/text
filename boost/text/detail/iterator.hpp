#ifndef BOOST_TEXT_DETAIL_ITERATOR_HPP
#define BOOST_TEXT_DETAIL_ITERATOR_HPP

#include <iterator>


namespace boost { namespace text { namespace detail {

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
        constexpr pointer operator->() const noexcept
        { return ptr_; }

        constexpr value_type operator[] (difference_type n) const noexcept
        { return ptr_[n]; }

        constexpr reverse_char_iterator operator++ () noexcept
        {
            ++ptr_;
            return *this;
        }
        constexpr reverse_char_iterator operator++ (int) noexcept
        {
            reverse_char_iterator retval = *this;
            ++ptr_;
            return retval;
        }
        constexpr reverse_char_iterator operator+= (int n) noexcept
        {
            ptr_ += n;
            return *this;
        }

        constexpr reverse_char_iterator operator-- () noexcept
        {
            --ptr_;
            return *this;
        }
        constexpr reverse_char_iterator operator-- (int) noexcept
        {
            reverse_char_iterator retval = *this;
            --ptr_;
            return retval;
        }
        constexpr reverse_char_iterator operator-= (int n) noexcept
        {
            ptr_ += n;
            return *this;
        }

        // TODO: operator<=> () const
        friend constexpr bool operator== (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ == rhs.ptr_; }
        friend constexpr bool operator!= (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ != rhs.ptr_; }
        friend constexpr bool operator< (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ < rhs.ptr_; }
        friend constexpr bool operator<= (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ <= rhs.ptr_; }
        friend constexpr bool operator> (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ > rhs.ptr_; }
        friend constexpr bool operator>= (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ >= rhs.ptr_; }

        friend constexpr reverse_char_iterator operator+ (reverse_char_iterator lhs, int rhs) noexcept
        { return lhs += rhs; }
        friend constexpr reverse_char_iterator operator+ (int lhs, reverse_char_iterator rhs) noexcept
        { return rhs += lhs; }
        friend constexpr reverse_char_iterator operator- (reverse_char_iterator lhs, int rhs) noexcept
        { return lhs -= rhs; }
        friend constexpr reverse_char_iterator operator- (int lhs, reverse_char_iterator rhs) noexcept
        { return rhs -= lhs; }
        friend constexpr int operator- (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ - rhs.ptr_; }

    private:
        char * ptr_;
    };

    struct const_reverse_char_iterator
    {
        using value_type = char const;
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

        constexpr char const * base () const noexcept
        { return ptr_ + 1; }

        constexpr reference operator* () const noexcept
        { return *ptr_; }
        constexpr pointer operator->() const noexcept
        { return ptr_; }

        constexpr value_type operator[] (difference_type n) const noexcept
        { return ptr_[n]; }

        constexpr const_reverse_char_iterator operator++ () noexcept
        {
            ++ptr_;
            return *this;
        }
        constexpr const_reverse_char_iterator operator++ (int) noexcept
        {
            const_reverse_char_iterator retval = *this;
            ++ptr_;
            return retval;
        }
        constexpr const_reverse_char_iterator operator+= (int n) noexcept
        {
            ptr_ += n;
            return *this;
        }

        constexpr const_reverse_char_iterator operator-- () noexcept
        {
            --ptr_;
            return *this;
        }
        constexpr const_reverse_char_iterator operator-- (int) noexcept
        {
            const_reverse_char_iterator retval = *this;
            --ptr_;
            return retval;
        }
        constexpr const_reverse_char_iterator operator-= (int n) noexcept
        {
            ptr_ += n;
            return *this;
        }

        // TODO: operator<=> () const
        friend constexpr bool operator== (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ == rhs.ptr_; }
        friend constexpr bool operator!= (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ != rhs.ptr_; }
        friend constexpr bool operator< (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ < rhs.ptr_; }
        friend constexpr bool operator<= (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ <= rhs.ptr_; }
        friend constexpr bool operator> (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ > rhs.ptr_; }
        friend constexpr bool operator>= (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ >= rhs.ptr_; }

        friend constexpr const_reverse_char_iterator operator+ (const_reverse_char_iterator lhs, int rhs) noexcept
        { return lhs += rhs; }
        friend constexpr const_reverse_char_iterator operator+ (int lhs, const_reverse_char_iterator rhs) noexcept
        { return rhs += lhs; }
        friend constexpr const_reverse_char_iterator operator- (const_reverse_char_iterator lhs, int rhs) noexcept
        { return lhs -= rhs; }
        friend constexpr const_reverse_char_iterator operator- (int lhs, const_reverse_char_iterator rhs) noexcept
        { return rhs -= lhs; }
        friend constexpr int operator- (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ - rhs.ptr_; }

    private:
        char const * ptr_;
    };

} } }

#endif
