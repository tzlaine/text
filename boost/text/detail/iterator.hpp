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

        constexpr reverse_char_iterator & operator++ () noexcept
        {
            --ptr_;
            return *this;
        }
        constexpr reverse_char_iterator operator++ (int) noexcept
        {
            reverse_char_iterator retval = *this;
            --ptr_;
            return retval;
        }
        constexpr reverse_char_iterator & operator+= (difference_type n) noexcept
        {
            ptr_ -= n;
            return *this;
        }

        constexpr reverse_char_iterator & operator-- () noexcept
        {
            ++ptr_;
            return *this;
        }
        constexpr reverse_char_iterator operator-- (int) noexcept
        {
            reverse_char_iterator retval = *this;
            ++ptr_;
            return retval;
        }
        constexpr reverse_char_iterator & operator-= (difference_type n) noexcept
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

        friend constexpr reverse_char_iterator operator+ (reverse_char_iterator lhs, difference_type rhs) noexcept
        { return lhs += rhs; }
        friend constexpr reverse_char_iterator operator+ (difference_type lhs, reverse_char_iterator rhs) noexcept
        { return rhs += lhs; }
        friend constexpr reverse_char_iterator operator- (reverse_char_iterator lhs, difference_type rhs) noexcept
        { return lhs -= rhs; }
        friend constexpr reverse_char_iterator operator- (difference_type lhs, reverse_char_iterator rhs) noexcept
        { return rhs -= lhs; }
        friend constexpr difference_type operator- (reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return rhs.ptr_ - lhs.ptr_; }

        friend struct const_reverse_char_iterator;

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
        constexpr const_reverse_char_iterator (reverse_char_iterator rhs) noexcept :
            ptr_ (rhs.ptr_ + 1)
        {}

        constexpr char const * base () const noexcept
        { return ptr_; }

        constexpr reference operator* () const noexcept
        { return *(ptr_ - 1); }

        constexpr value_type operator[] (difference_type n) const noexcept
        { return ptr_[-n - 1]; }

        constexpr const_reverse_char_iterator & operator++ () noexcept
        {
            --ptr_;
            return *this;
        }
        constexpr const_reverse_char_iterator operator++ (int) noexcept
        {
            const_reverse_char_iterator retval = *this;
            --ptr_;
            return retval;
        }
        constexpr const_reverse_char_iterator & operator+= (difference_type n) noexcept
        {
            ptr_ -= n;
            return *this;
        }

        constexpr const_reverse_char_iterator & operator-- () noexcept
        {
            ++ptr_;
            return *this;
        }
        constexpr const_reverse_char_iterator operator-- (int) noexcept
        {
            const_reverse_char_iterator retval = *this;
            ++ptr_;
            return retval;
        }
        constexpr const_reverse_char_iterator & operator-= (difference_type n) noexcept
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

        friend constexpr bool operator== (reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ == rhs.ptr_ - 1; }
        friend constexpr bool operator!= (reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ != rhs.ptr_ - 1; }
        friend constexpr bool operator< (reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return rhs.ptr_ - 1 < lhs.ptr_; }
        friend constexpr bool operator<= (reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return rhs.ptr_ - 1 <= lhs.ptr_; }
        friend constexpr bool operator> (reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ < rhs.ptr_ - 1; }
        friend constexpr bool operator>= (reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ <= rhs.ptr_ - 1; }

        friend constexpr bool operator== (const_reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ == rhs.ptr_ - 1; }
        friend constexpr bool operator!= (const_reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ != rhs.ptr_ - 1; }
        friend constexpr bool operator< (const_reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return rhs.ptr_ - 1 < lhs.ptr_; }
        friend constexpr bool operator<= (const_reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return rhs.ptr_ - 1 <= lhs.ptr_; }
        friend constexpr bool operator> (const_reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ < rhs.ptr_ - 1; }
        friend constexpr bool operator>= (const_reverse_char_iterator lhs, reverse_char_iterator rhs) noexcept
        { return lhs.ptr_ <= rhs.ptr_ - 1; }

        friend constexpr const_reverse_char_iterator operator+ (const_reverse_char_iterator lhs, difference_type rhs) noexcept
        { return lhs += rhs; }
        friend constexpr const_reverse_char_iterator operator+ (difference_type lhs, const_reverse_char_iterator rhs) noexcept
        { return rhs += lhs; }
        friend constexpr const_reverse_char_iterator operator- (const_reverse_char_iterator lhs, difference_type rhs) noexcept
        { return lhs -= rhs; }
        friend constexpr const_reverse_char_iterator operator- (difference_type lhs, const_reverse_char_iterator rhs) noexcept
        { return rhs -= lhs; }
        friend constexpr difference_type operator- (const_reverse_char_iterator lhs, const_reverse_char_iterator rhs) noexcept
        { return rhs.ptr_ - lhs.ptr_; }

    private:
        char const * ptr_;
    };

    struct const_repeated_chars_iterator
    {
        using value_type = char const;
        using difference_type = std::ptrdiff_t;
        using pointer = char const *;
        using reference = char const;
        using iterator_category = std::random_access_iterator_tag;

        constexpr const_repeated_chars_iterator () noexcept :
            first_ (nullptr),
            curr_ (nullptr),
            last_ (nullptr),
            count_ (0)
        {}
        // ctor for begin iterators
        constexpr const_repeated_chars_iterator (
            char const * first, char const * last,
            difference_type count
        ) noexcept :
            first_ (first),
            curr_ (first),
            last_ (last),
            count_ (-count)
        {}
        // ctor for end iterators
        constexpr const_repeated_chars_iterator (
            char const * first, char const * last
        ) noexcept :
            first_ (first),
            curr_ (last),
            last_ (last),
            count_ (0)
        {}

        constexpr reference operator* () const noexcept
        { return *curr_; }

        constexpr value_type operator[] (difference_type n) const noexcept
        {
            auto it = *this;
            if (0 <= n)
                it += n;
            else
                it -= -n;
            return *it;
        }

        constexpr const_repeated_chars_iterator & operator++ () noexcept
        {
            if (++curr_ == last_) {
                curr_ = first_;
                if (!++count_)
                    curr_ = last_;
            }
            return *this;
        }
        constexpr const_repeated_chars_iterator operator++ (int) noexcept
        {
            const_repeated_chars_iterator retval = *this;
            ++*this;
            return retval;
        }
        constexpr const_repeated_chars_iterator & operator+= (difference_type n) noexcept
        {
            difference_type const size = last_ - first_;
            difference_type const repetitions = n / size;
            count_ += repetitions;
            n = n % size;
            difference_type const remaining = last_ - curr_;
            if (n < remaining) {
                curr_ += n;
            } else {
                curr_ = first_ + n - remaining;
                ++count_;
            }
            if (!count_ && curr_ == first_)
                curr_ = last_;
            return *this;
        }

        constexpr const_repeated_chars_iterator & operator-- () noexcept
        {
            if (curr_ == first_) {
                curr_ = last_;
                --count_;
            }
            --curr_;
            return *this;
        }
        constexpr const_repeated_chars_iterator operator-- (int) noexcept
        {
            const_repeated_chars_iterator retval = *this;
            --*this;
            return retval;
        }
        constexpr const_repeated_chars_iterator & operator-= (difference_type n) noexcept
        {
            difference_type const size = last_ - first_;
            difference_type const repetitions = n / size;
            count_ -= repetitions;
            n = n % size;
            difference_type const remaining = curr_ - first_;
            if (!n && curr_ == last_) {
                curr_ = first_;
            } else if (n <= remaining) {
                curr_ -= n;
            } else {
                curr_ = last_ - (n - remaining);
                --count_;
            }
            return *this;
        }

        friend constexpr bool operator== (const_repeated_chars_iterator lhs, const_repeated_chars_iterator rhs) noexcept
        { return lhs.first_ == rhs.first_ && lhs.last_ == rhs.last_ && lhs.count_ == rhs.count_ && lhs.curr_ == rhs.curr_; }
        friend constexpr bool operator!= (const_repeated_chars_iterator lhs, const_repeated_chars_iterator rhs) noexcept
        { return !(lhs == rhs); }
        // TODO: Document wonky behavior of the inequalities when rhs.{frst,last}_ != rhs.{first,last}_.
        friend constexpr bool operator< (const_repeated_chars_iterator lhs, const_repeated_chars_iterator rhs) noexcept
        {
            return
                lhs.first_ == rhs.first_ && lhs.last_ == rhs.last_ &&
                (lhs.count_ < rhs.count_ || (lhs.count_ == rhs.count_ && lhs.curr_ < rhs.curr_));
        }
        friend constexpr bool operator<= (const_repeated_chars_iterator lhs, const_repeated_chars_iterator rhs) noexcept
        { return lhs == rhs || lhs < rhs; }
        friend constexpr bool operator> (const_repeated_chars_iterator lhs, const_repeated_chars_iterator rhs) noexcept
        { return rhs < lhs; }
        friend constexpr bool operator>= (const_repeated_chars_iterator lhs, const_repeated_chars_iterator rhs) noexcept
        { return lhs <= rhs; }

        friend constexpr const_repeated_chars_iterator operator+ (const_repeated_chars_iterator lhs, difference_type rhs) noexcept
        { return lhs += rhs; }
        friend constexpr const_repeated_chars_iterator operator+ (difference_type lhs, const_repeated_chars_iterator rhs) noexcept
        { return rhs += lhs; }
        friend constexpr const_repeated_chars_iterator operator- (const_repeated_chars_iterator lhs, difference_type rhs) noexcept
        { return lhs -= rhs; }
        friend constexpr const_repeated_chars_iterator operator- (difference_type lhs, const_repeated_chars_iterator rhs) noexcept
        { return rhs -= lhs; }
        friend constexpr difference_type operator- (const_repeated_chars_iterator lhs, const_repeated_chars_iterator rhs) noexcept
        {
            // TODO: Document this precondition!
            assert(lhs.first_ == rhs.first_ && lhs.last_ == rhs.last_);
            difference_type const l_offset = lhs.curr_ - lhs.first_;
            difference_type const r_offset = rhs.curr_ - rhs.first_;
            difference_type retval = l_offset - r_offset;
            difference_type const size = lhs.last_ - lhs.first_;
            retval += size * (lhs.count_ - rhs.count_);
            if (lhs.curr_ == lhs.last_)
                retval -= size;
            if (rhs.curr_ == rhs.last_)
                retval += size;
            return retval;
        }

    private:
        char const * first_;
        char const * curr_;
        char const * last_;
        difference_type count_;
    };

    // TODO: Test this one.
    struct const_reverse_repeated_chars_iterator
    {
        using value_type = char const;
        using difference_type = std::ptrdiff_t;
        using pointer = char const *;
        using reference = char const;
        using iterator_category = std::random_access_iterator_tag;

        constexpr const_reverse_repeated_chars_iterator () noexcept :
            base_ ()
        {}
        constexpr const_reverse_repeated_chars_iterator (const_repeated_chars_iterator it) noexcept :
            base_ (it)
        {}

        constexpr const_repeated_chars_iterator base () const
        { return base_; }

        constexpr reference operator* () const noexcept
        { return *(base_ - 1); }
        constexpr value_type operator[] (difference_type n) const noexcept
        { return base_[-n - 1]; }

        constexpr const_reverse_repeated_chars_iterator & operator++ () noexcept
        { --base_; return *this; }
        constexpr const_reverse_repeated_chars_iterator operator++ (int) noexcept
        {
            const_reverse_repeated_chars_iterator retval = *this;
            --base_;
            return retval;
        }
        constexpr const_reverse_repeated_chars_iterator & operator+= (difference_type n) noexcept
        { base_ -= n; return *this; }

        constexpr const_reverse_repeated_chars_iterator & operator-- () noexcept
        { ++base_; return *this; }
        constexpr const_reverse_repeated_chars_iterator operator-- (int) noexcept
        {
            const_reverse_repeated_chars_iterator retval = *this;
            ++base_;
            return retval;
        }
        constexpr const_reverse_repeated_chars_iterator & operator-= (difference_type n) noexcept
        { base_ += n; return *this; }

        friend constexpr bool operator== (const_reverse_repeated_chars_iterator lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return lhs.base_ == rhs.base_; }
        friend constexpr bool operator!= (const_reverse_repeated_chars_iterator lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return !(lhs == rhs); }
        // TODO: Document wonky behavior of the inequalities when rhs.{frst,last}_ != rhs.{first,last}_.
        friend constexpr bool operator< (const_reverse_repeated_chars_iterator lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return rhs.base_ < lhs.base_; }
        friend constexpr bool operator<= (const_reverse_repeated_chars_iterator lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return rhs.base_ <= lhs.base_; }
        friend constexpr bool operator> (const_reverse_repeated_chars_iterator lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return rhs.base_ > lhs.base_; }
        friend constexpr bool operator>= (const_reverse_repeated_chars_iterator lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return rhs.base_ >= lhs.base_; }

        friend constexpr const_reverse_repeated_chars_iterator operator+ (const_reverse_repeated_chars_iterator lhs, difference_type rhs) noexcept
        { return lhs += rhs; }
        friend constexpr const_reverse_repeated_chars_iterator operator+ (difference_type lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return rhs += lhs; }
        friend constexpr const_reverse_repeated_chars_iterator operator- (const_reverse_repeated_chars_iterator lhs, difference_type rhs) noexcept
        { return lhs -= rhs; }
        friend constexpr const_reverse_repeated_chars_iterator operator- (difference_type lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return rhs -= lhs; }
        friend constexpr difference_type operator- (const_reverse_repeated_chars_iterator lhs, const_reverse_repeated_chars_iterator rhs) noexcept
        { return lhs.base_ - rhs.base_; }

    private:
        const_repeated_chars_iterator base_;
    };

} } }

#endif
