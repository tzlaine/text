#ifndef BOOST_TEXT_DETAIL_VECTOR_ITERATOR_ITERATOR_HPP
#define BOOST_TEXT_DETAIL_VECTOR_ITERATOR_ITERATOR_HPP

#include <iterator>


namespace boost { namespace text {

    template<typename T>
    struct segmented_vector;

}}

namespace boost { namespace text { namespace detail {

    template<typename T>
    struct const_vector_iterator
    {
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T const *;
        using reference = T const &;
        using iterator_category = std::random_access_iterator_tag;

        const_vector_iterator() noexcept :
            vec_(nullptr),
            n_(-1),
            leaf_(nullptr),
            leaf_start_(-1)
        {}

        const_vector_iterator(
            segmented_vector<T> const & v, difference_type n) noexcept :
            vec_(&v),
            n_(n),
            leaf_(nullptr),
            leaf_start_(0)
        {}

        reference operator*() const noexcept
        {
            if (leaf_) {
                return deref();
            } else {
                found_element<T> found;
                find_element(vec_->ptr_, n_, found);
                leaf_ = found.leaf_.leaf_->as_leaf();
                leaf_start_ = n_ - found.leaf_.offset_;
                return *found.element_;
            }
        }

        pointer operator->() const noexcept { return &**this; }

        value_type operator[](difference_type n) const noexcept
        {
            auto it = *this;
            if (0 <= n)
                it += n;
            else
                it -= -n;
            return *it;
        }

        const_vector_iterator & operator++() noexcept
        {
            ++n_;
            if (leaf_ && n_ == leaf_start_ + leaf_->size())
                leaf_ = nullptr;
            return *this;
        }
        const_vector_iterator operator++(int)noexcept
        {
            const_vector_iterator retval = *this;
            ++*this;
            return retval;
        }
        const_vector_iterator & operator+=(difference_type n) noexcept
        {
            n_ += n;
            leaf_ = nullptr;
            return *this;
        }

        const_vector_iterator & operator--() noexcept
        {
            if (leaf_ && n_ == leaf_start_)
                leaf_ = nullptr;
            --n_;
            return *this;
        }
        const_vector_iterator operator--(int)noexcept
        {
            const_vector_iterator retval = *this;
            --*this;
            return retval;
        }
        const_vector_iterator & operator-=(difference_type n) noexcept
        {
            n_ -= n;
            leaf_ = nullptr;
            return *this;
        }

        friend bool operator==(
            const_vector_iterator lhs, const_vector_iterator rhs) noexcept
        {
            return lhs.vec_ == rhs.vec_ && lhs.n_ == rhs.n_;
        }
        friend bool operator!=(
            const_vector_iterator lhs, const_vector_iterator rhs) noexcept
        {
            return !(lhs == rhs);
        }
        friend bool
        operator<(const_vector_iterator lhs, const_vector_iterator rhs) noexcept
        {
            return lhs.vec_ == rhs.vec_ && lhs.n_ < rhs.n_;
        }
        friend bool operator<=(
            const_vector_iterator lhs, const_vector_iterator rhs) noexcept
        {
            return lhs == rhs || lhs < rhs;
        }
        friend bool
        operator>(const_vector_iterator lhs, const_vector_iterator rhs) noexcept
        {
            return rhs < lhs;
        }
        friend bool operator>=(
            const_vector_iterator lhs, const_vector_iterator rhs) noexcept
        {
            return rhs <= lhs;
        }

        friend const_vector_iterator
        operator+(const_vector_iterator lhs, difference_type rhs) noexcept
        {
            return lhs += rhs;
        }
        friend const_vector_iterator
        operator+(difference_type lhs, const_vector_iterator rhs) noexcept
        {
            return rhs += lhs;
        }
        friend const_vector_iterator
        operator-(const_vector_iterator lhs, difference_type rhs) noexcept
        {
            return lhs -= rhs;
        }
        friend const_vector_iterator
        operator-(difference_type lhs, const_vector_iterator rhs) noexcept
        {
            return rhs -= lhs;
        }
        friend difference_type
        operator-(const_vector_iterator lhs, const_vector_iterator rhs) noexcept
        {
            assert(lhs.vec_ == rhs.vec_);
            return lhs.n_ - rhs.n_;
        }

    private:
        const_vector_iterator(
            segmented_vector<T> const * v, difference_type n) noexcept :
            vec_(v),
            n_(n),
            leaf_(nullptr),
            leaf_start_(0)
        {}

        T const & deref() const
        {
            switch (leaf_->which_) {
            default: assert(!"unhandled leaf node case");
            case leaf_node_t<T>::which::vec: {
                std::vector<T> const * v =
                    static_cast<std::vector<T> *>(leaf_->buf_ptr_);
                return *(v->begin() + (n_ - leaf_start_));
            }
            case leaf_node_t<T>::which::ref: {
                detail::reference<T> const * ref =
                    static_cast<detail::reference<T> *>(leaf_->buf_ptr_);
                return *(
                    ref->vec_.as_leaf()->as_vec().begin() + ref->lo_ +
                    (n_ - leaf_start_));
            }
            }
        }

        segmented_vector<T> const * vec_;
        difference_type n_;
        mutable leaf_node_t<T> const * leaf_;
        mutable difference_type leaf_start_;
    };

    template<typename T>
    struct const_reverse_vector_iterator
    {
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T const *;
        using reference = T const &;
        using iterator_category = std::random_access_iterator_tag;

        const_reverse_vector_iterator() noexcept : base_() {}
        explicit const_reverse_vector_iterator(
            const_vector_iterator<T> it) noexcept :
            base_(it)
        {}

        const_vector_iterator<T> base() const { return base_ + 1; }

        reference operator*() const noexcept { return *base_; }
        pointer operator->() const noexcept { return &**this; }

        value_type operator[](difference_type n) const noexcept
        {
            return base_[-n];
        }

        const_reverse_vector_iterator & operator++() noexcept
        {
            --base_;
            return *this;
        }
        const_reverse_vector_iterator operator++(int)noexcept
        {
            const_reverse_vector_iterator retval = *this;
            --base_;
            return retval;
        }
        const_reverse_vector_iterator & operator+=(difference_type n) noexcept
        {
            base_ -= n;
            return *this;
        }

        const_reverse_vector_iterator & operator--() noexcept
        {
            ++base_;
            return *this;
        }
        const_reverse_vector_iterator operator--(int)noexcept
        {
            const_reverse_vector_iterator retval = *this;
            ++base_;
            return retval;
        }
        const_reverse_vector_iterator & operator-=(difference_type n) noexcept
        {
            base_ += n;
            return *this;
        }

        friend bool operator==(
            const_reverse_vector_iterator lhs,
            const_reverse_vector_iterator rhs) noexcept
        {
            return lhs.base_ == rhs.base_;
        }
        friend bool operator!=(
            const_reverse_vector_iterator lhs,
            const_reverse_vector_iterator rhs) noexcept
        {
            return !(lhs == rhs);
        }
        friend bool operator<(
            const_reverse_vector_iterator lhs,
            const_reverse_vector_iterator rhs) noexcept
        {
            return rhs.base_ < lhs.base_;
        }
        friend bool operator<=(
            const_reverse_vector_iterator lhs,
            const_reverse_vector_iterator rhs) noexcept
        {
            return rhs.base_ <= lhs.base_;
        }
        friend bool operator>(
            const_reverse_vector_iterator lhs,
            const_reverse_vector_iterator rhs) noexcept
        {
            return rhs.base_ > lhs.base_;
        }
        friend bool operator>=(
            const_reverse_vector_iterator lhs,
            const_reverse_vector_iterator rhs) noexcept
        {
            return rhs.base_ >= lhs.base_;
        }

        friend const_reverse_vector_iterator operator+(
            const_reverse_vector_iterator lhs, difference_type rhs) noexcept
        {
            return lhs += rhs;
        }
        friend const_reverse_vector_iterator operator+(
            difference_type lhs, const_reverse_vector_iterator rhs) noexcept
        {
            return rhs += lhs;
        }
        friend const_reverse_vector_iterator operator-(
            const_reverse_vector_iterator lhs, difference_type rhs) noexcept
        {
            return lhs -= rhs;
        }
        friend const_reverse_vector_iterator operator-(
            difference_type lhs, const_reverse_vector_iterator rhs) noexcept
        {
            return rhs -= lhs;
        }
        friend difference_type operator-(
            const_reverse_vector_iterator lhs,
            const_reverse_vector_iterator rhs) noexcept
        {
            return rhs.base_ - lhs.base_;
        }

    private:
        const_vector_iterator<T> base_;
    };

}}}

#endif
