// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_DETAIL_VECTOR_ITERATOR_ITERATOR_HPP
#define BOOST_TEXT_DETAIL_VECTOR_ITERATOR_ITERATOR_HPP

#include <boost/stl_interfaces/iterator_interface.hpp>

#include <iterator>


namespace boost { namespace text { inline namespace v1 {

    template<typename T>
    struct segmented_vector;

}}}

namespace boost { namespace text { inline namespace v1 { namespace detail {

    template<typename T>
    struct const_vector_iterator : stl_interfaces::iterator_interface<
                                       const_vector_iterator<T>,
                                       std::random_access_iterator_tag,
                                       T,
                                       T const &,
                                       T const *>
    {
        const_vector_iterator() noexcept :
            vec_(nullptr),
            n_(-1),
            leaf_(nullptr),
            leaf_start_(-1)
        {}

        const_vector_iterator(
            segmented_vector<T> const & v, std::ptrdiff_t n) noexcept :
            vec_(&v),
            n_(n),
            leaf_(nullptr),
            leaf_start_(0)
        {}

        T const & operator*() const noexcept
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

        const_vector_iterator & operator+=(std::ptrdiff_t n) noexcept
        {
            n_ += n;
            leaf_ = nullptr;
            return *this;
        }

        friend std::ptrdiff_t
        operator-(const_vector_iterator lhs, const_vector_iterator rhs) noexcept
        {
            BOOST_ASSERT(lhs.vec_ == rhs.vec_);
            return lhs.n_ - rhs.n_;
        }

    private:
        const_vector_iterator(
            segmented_vector<T> const * v, std::ptrdiff_t n) noexcept :
            vec_(v),
            n_(n),
            leaf_(nullptr),
            leaf_start_(0)
        {}

        T const & deref() const
        {
            switch (leaf_->which_) {
            default: BOOST_ASSERT(!"unhandled leaf node case");
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
        std::ptrdiff_t n_;
        mutable leaf_node_t<T> const * leaf_;
        mutable std::ptrdiff_t leaf_start_;
    };

}}}}

#endif
