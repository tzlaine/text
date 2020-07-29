// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_DETAIL_ROPE_ITERATOR_HPP
#define BOOST_TEXT_DETAIL_ROPE_ITERATOR_HPP

#include <boost/text/detail/iterator.hpp>


namespace boost { namespace text {

    struct unencoded_rope_view;
    struct rope_view;

}}

namespace boost { namespace text { namespace detail {

    struct const_rope_iterator : stl_interfaces::iterator_interface<
                                     const_rope_iterator,
                                     std::random_access_iterator_tag,
                                     char,
                                     char>
    {
        const_rope_iterator() noexcept :
            rope_(nullptr),
            n_(-1),
            leaf_(nullptr),
            leaf_start_(-1)
        {}

        const_rope_iterator(
            unencoded_rope const & r, std::ptrdiff_t n) noexcept :
            rope_(&r),
            n_(n),
            leaf_(nullptr),
            leaf_start_(0)
        {}

        char operator*() const noexcept
        {
            if (leaf_) {
                return deref();
            } else {
                found_char found;
                find_char(rope_->ptr_, n_, found);
                leaf_ = found.leaf_.leaf_->as_leaf();
                leaf_start_ = n_ - found.leaf_.offset_;
                return found.c_;
            }
        }

        const_rope_iterator & operator+=(std::ptrdiff_t n) noexcept
        {
            n_ += n;
            leaf_ = nullptr;
            return *this;
        }

        friend std::ptrdiff_t
        operator-(const_rope_iterator lhs, const_rope_iterator rhs) noexcept
        {
            BOOST_ASSERT(lhs.rope_ == rhs.rope_);
            return lhs.n_ - rhs.n_;
        }

    private:
        const_rope_iterator(
            unencoded_rope const * r, std::ptrdiff_t n) noexcept :
            rope_(r),
            n_(n),
            leaf_(nullptr),
            leaf_start_(0)
        {}

        char deref() const
        {
            switch (leaf_->which_) {
            case which::t: {
                std::string const * t =
                    static_cast<std::string *>(leaf_->buf_ptr_);
                return *(t->begin() + (n_ - leaf_start_));
            }
            case which::ref: {
                detail::reference<rope_tag> const * ref =
                    static_cast<detail::reference<rope_tag> *>(leaf_->buf_ptr_);
                return *(ref->ref_.begin() + (n_ - leaf_start_));
            }
            default: BOOST_ASSERT(!"unhandled rope node case"); break;
            }
            return '\0'; // This should never execute.
        }

        unencoded_rope const * rope_;
        std::ptrdiff_t n_;
        mutable leaf_node_t<rope_tag> const * leaf_;
        mutable std::ptrdiff_t leaf_start_;

        friend struct ::boost::text::unencoded_rope_view;
        friend struct ::boost::text::rope_view;
    };

    struct const_rope_view_iterator : stl_interfaces::iterator_interface<
                                          const_rope_view_iterator,
                                          std::random_access_iterator_tag,
                                          char,
                                          char>
    {
        const_rope_view_iterator() noexcept : which_(which::r) {}
        explicit const_rope_view_iterator(const_rope_iterator it) noexcept :
            r_(it),
            which_(which::r)
        {}
        explicit const_rope_view_iterator(char const * it) noexcept :
            tv_(it),
            which_(which::tv)
        {}

        const_rope_iterator as_rope_iter() const
        {
            BOOST_ASSERT(which_ == which::r);
            return r_;
        }

        char operator*() const noexcept
        {
            switch (which_) {
            case which::r: return *r_;
            case which::tv: return *tv_;
            }
            return '\0'; // This should never execute.
        }
        const_rope_view_iterator & operator+=(std::ptrdiff_t n) noexcept
        {
            switch (which_) {
            case which::r: r_ += n; break;
            case which::tv: tv_ += n; break;
            }
            return *this;
        }

        friend bool operator==(
            const_rope_view_iterator lhs, const_rope_view_iterator rhs) noexcept
        {
            if (lhs.which_ != rhs.which_)
                return false;
            switch (lhs.which_) {
            case which::r: return lhs.r_ == rhs.r_;
            case which::tv: return lhs.tv_ == rhs.tv_;
            }
            return false; // This should never execute.
        }
        friend bool operator!=(
            const_rope_view_iterator lhs, const_rope_view_iterator rhs) noexcept
        {
            return !(lhs == rhs);
        }
        friend bool operator<(
            const_rope_view_iterator lhs, const_rope_view_iterator rhs) noexcept
        {
            if (lhs.which_ != rhs.which_)
                return false;
            switch (lhs.which_) {
            case which::r: return lhs.r_ < rhs.r_;
            case which::tv: return lhs.tv_ < rhs.tv_;
            }
            return false; // This should never execute.
        }
        friend bool operator<=(
            const_rope_view_iterator lhs, const_rope_view_iterator rhs) noexcept
        {
            return lhs < rhs || lhs == rhs;
        }
        friend bool operator>(
            const_rope_view_iterator lhs, const_rope_view_iterator rhs) noexcept
        {
            return rhs < lhs;
        }
        friend bool operator>=(
            const_rope_view_iterator lhs, const_rope_view_iterator rhs) noexcept
        {
            return lhs > rhs || lhs == rhs;
        }

        friend std::ptrdiff_t operator-(
            const_rope_view_iterator lhs, const_rope_view_iterator rhs) noexcept
        {
            if (lhs.which_ != rhs.which_)
                return false;
            switch (lhs.which_) {
            case which::r: return lhs.r_ - rhs.r_;
            case which::tv: return lhs.tv_ - rhs.tv_;
            }
            return 0; // This should never execute.
        }

    private:
        enum class which { r, tv };

        const_rope_iterator r_;
        char const * tv_;

        which which_;
    };

}}}

#endif
