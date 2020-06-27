// Copyright (C) 2020 T. Zachary Laine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_TEXT_SEGMENTED_VECTOR_HPP
#define BOOST_TEXT_SEGMENTED_VECTOR_HPP

#include <boost/text/detail/btree.hpp>
#include <boost/text/detail/iterator.hpp>
#include <boost/text/detail/vector_iterator.hpp>

#include <initializer_list>


namespace boost { namespace text { inline namespace v1 {

    namespace detail {

        constexpr int vec_insert_max = 512;
    }

    /** A sequence of T with discontiguous storage.  This data strucutre is a
        generalization of unencoded_rope to non-char sequences. */
    template<typename T>
    struct segmented_vector
    {
        using iterator = detail::const_vector_iterator<T>;
        using const_iterator = detail::const_vector_iterator<T>;
        using reverse_iterator = stl_interfaces::reverse_iterator<iterator>;
        using const_reverse_iterator =
            stl_interfaces::reverse_iterator<const_iterator>;
        using value_type = T;

        using size_type = std::ptrdiff_t;

        /** Default ctor.

            \post size() == 0 && begin() == end() */
        segmented_vector() noexcept {}

        segmented_vector(segmented_vector const & rhs) = default;
        segmented_vector(segmented_vector && rhs) noexcept = default;

        template<typename Iter, typename Sentinel>
        segmented_vector(Iter first, Sentinel last)
        {
            for (; first != last; ++first) {
                push_back(*first);
            }
        }

        segmented_vector(std::initializer_list<T> il)
        {
            for (auto && x : il) {
                push_back(std::move(x));
            }
        }

        segmented_vector & operator=(segmented_vector const & rhs) = default;
        segmented_vector &
        operator=(segmented_vector && rhs) noexcept = default;

        segmented_vector & operator=(std::initializer_list<T> il)
        {
            clear();
            for (auto && x : il) {
                push_back(std::move(x));
            }
        }

        const_iterator begin() const noexcept
        {
            return const_iterator(*this, 0);
        }
        const_iterator end() const noexcept
        {
            return const_iterator(*this, size());
        }

        const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator(end());
        }
        const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator(begin());
        }

        bool empty() const noexcept { return size() == 0; }

        size_type size() const noexcept { return detail::size(ptr_.get()); }

        /** Returns a const reference to the *element of this at index n, or
            the char at index -n when n < 0.

            \pre 0 <= n && n <= size() || 0 <= -n && -n <= size()  */
        T const & operator[](size_type n) const noexcept
        {
            BOOST_ASSERT(ptr_);
            if (n < 0)
                n += size();
            BOOST_ASSERT(0 <= n && n < size());
            detail::found_element<T> found;
            find_element(ptr_, n, found);
            return *found.element_;
        }

        /** Returns the maximum size a segmented_vector can have. */
        size_type max_size() const noexcept { return PTRDIFF_MAX; }

        /** Lexicographical compare.  Returns a value < 0 when *this is
            lexicographically less than rhs, 0 if *this == rhs, and a value >
            0 if *this is lexicographically greater than rhs. */
        int compare(segmented_vector rhs) const noexcept
        {
            if (empty())
                return rhs.empty() ? 0 : -1;

            auto const iters =
                algorithm::mismatch(begin(), end(), rhs.begin(), rhs.end());
            if (iters.first == end()) {
                if (iters.second == rhs.end())
                    return 0;
                else
                    return -1;
            } else if (iters.second == rhs.end()) {
                return 1;
            } else if (*iters.first < *iters.second) {
                return -1;
            } else {
                return 1;
            }
        }

        bool operator==(segmented_vector rhs) const noexcept
        {
            return size() == rhs.size() &&
                   std::equal(begin(), end(), rhs.begin());
        }

        bool operator!=(segmented_vector rhs) const noexcept
        {
            return !(*this == rhs);
        }

        bool operator<(segmented_vector rhs) const noexcept
        {
            return compare(rhs) < 0;
        }

        bool operator<=(segmented_vector rhs) const noexcept
        {
            return compare(rhs) <= 0;
        }

        bool operator>(segmented_vector rhs) const noexcept
        {
            return compare(rhs) > 0;
        }

        bool operator>=(segmented_vector rhs) const noexcept
        {
            return compare(rhs) >= 0;
        }

        /** Returns true if *this and rhs contain the same root node pointer.
            This is useful when you want to check for equality between two
            segmented_vectors that are likely to have originated from the same
           initial segmented_vector, and may have since been mutated. */
        bool equal_root(segmented_vector rhs) const noexcept
        {
            return ptr_ == rhs.ptr_;
        }

        void clear() { ptr_ = detail::node_ptr<T>(); }

        /** Inserts t into *this at offset size() by moving t. */
        segmented_vector & push_back(T t)
        {
            insert(end(), std::move(t));
            return *this;
        }

        /** Inserts t into *this at offset at by moving t. */
        const_iterator insert(const_iterator at, T t)
        {
            BOOST_ASSERT(begin() <= at && at <= end());

            int const offset = at - begin();

            if (vec_insertion insertion =
                    mutable_insertion_leaf(at, 1, would_allocate)) {
                detail::bump_along_path_to_leaf(ptr_, at - begin(), 1);
                insertion.vec_->insert(
                    insertion.vec_->begin() + insertion.found_.offset_,
                    std::move(t));
            } else {
                ptr_ = detail::btree_insert(
                    ptr_,
                    offset,
                    detail::make_node(std::vector<T>(1, std::move(t))));
            }

            return begin() + offset;
        }

        /** Inserts the sequence of T from t into *this starting at offset at,
            by moving the contents of t. */
        const_iterator insert(const_iterator at, std::vector<T> t)
        {
            return insert_impl(at, std::move(t), would_not_allocate);
        }

        /** Inserts the T sequence [first, last) into *this starting at
            position at. */
        template<typename Iter, typename Sentinel>
        const_iterator insert(const_iterator at, Iter first, Sentinel last)
        {
            BOOST_ASSERT(begin() <= at && at <= end());

            if (first == last)
                return at;

            int const offset = at - begin();

            std::vector<T> vec;
            for (; first != last; ++first) {
                vec.push_back(*first);
            }

            ptr_ = detail::btree_insert(
                ptr_, offset, detail::make_node(std::move(vec)));

            return begin() + offset;
        }

        /** Erases the element at position at.

            \pre begin() <= at && at < end() */
        const_iterator erase(const_iterator at)
        {
            BOOST_ASSERT(begin() <= at && at < end());

            int const offset = at - begin();

            auto const lo = at - begin();
            if (vec_insertion insertion =
                    mutable_insertion_leaf(at, -1, would_not_allocate)) {
                detail::bump_along_path_to_leaf(ptr_, lo, -1);
                insertion.vec_->erase(
                    insertion.vec_->begin() + insertion.found_.offset_);
            } else {
                ptr_ = btree_erase(ptr_, lo, lo + 1);
            }

            return begin() + offset;
        }

        /** Erases the portion of *this delimited by [first, last).

            \pre first <= last */
        const_iterator erase(const_iterator first, const_iterator last)
        {
            BOOST_ASSERT(first <= last);
            BOOST_ASSERT(begin() <= first && last <= end());

            if (first == last)
                return first;

            int const offset = first - begin();

            auto const lo = first - begin();
            auto const hi = last - begin();
            auto const size = hi - lo;
            if (vec_insertion insertion =
                    mutable_insertion_leaf(first, -size, would_not_allocate)) {
                detail::bump_along_path_to_leaf(ptr_, lo, -size);
                insertion.vec_->erase(
                    insertion.vec_->begin() + insertion.found_.offset_,
                    insertion.vec_->begin() + insertion.found_.offset_ + size);
            } else {
                ptr_ = btree_erase(ptr_, lo, hi);
            }

            return begin() + offset;
        }

        /** Replaces the element at position at with t, by moving t.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        segmented_vector & replace(const_iterator at, T t)
        {
            BOOST_ASSERT(begin() <= at && at <= end());

            if (vec_insertion insertion =
                    mutable_insertion_leaf(at, 0, would_allocate)) {
                (*insertion.vec_)[insertion.found_.offset_] = std::move(t);
            } else {
                auto const offset = at - begin();
                ptr_ = detail::btree_erase(ptr_, offset, offset + 1);
                ptr_ = detail::btree_insert(
                    ptr_,
                    offset,
                    detail::make_node(std::vector<T>(1, std::move(t))));
            }

            return *this;
        }

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of T from t by moving the contents of t.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        segmented_vector &
        replace(const_iterator first, const_iterator last, std::vector<T> t)
        {
            auto const it = erase(first, last);
            insert(it, std::move(t));
            return *this;
        }

        /** Replaces the portion of *this delimited by [old_first, old_last)
            with the T sequence [new_first, new_last).

           \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template<typename Iter, typename Sentinel>
        segmented_vector & replace(
            const_iterator old_first,
            const_iterator old_last,
            Iter new_first,
            Sentinel new_last)
        {
            auto const it = erase(old_first, old_last);
            insert(it, new_first, new_last);
            return *this;
        }

        /** Swaps *this with rhs. */
        void swap(segmented_vector & rhs) { ptr_.swap(rhs.ptr_); }

#ifndef BOOST_TEXT_DOXYGEN

    private:
        enum allocation_note_t { would_allocate, would_not_allocate };

        struct vec_insertion
        {
            explicit operator bool() const { return vec_ != nullptr; }

            std::vector<T> * vec_;
            detail::found_leaf<T> found_;
        };

        vec_insertion mutable_insertion_leaf(
            iterator at, size_type delta, allocation_note_t allocation_note)
        {
            if (!ptr_)
                return vec_insertion{nullptr};

            detail::found_leaf<T> found;
            if (0 < delta && at == end()) {
                detail::find_leaf(ptr_, at - begin() - 1, found);
                ++found.offset_;
            } else {
                detail::find_leaf(ptr_, at - begin(), found);
            }

            for (auto node : found.path_) {
                if (1 < node->refs_)
                    return vec_insertion{nullptr};
            }

            if (1 < found.leaf_->get()->refs_)
                return vec_insertion{nullptr};

            if (found.leaf_->as_leaf()->which_ ==
                detail::leaf_node_t<T>::which::vec) {
                std::vector<T> & v = const_cast<std::vector<T> &>(
                    found.leaf_->as_leaf()->as_vec());
                auto const inserted_size = v.size() + delta;
                if (delta < 0 &&
                    (std::ptrdiff_t)v.size() < found.offset_ + -delta) {
                    return vec_insertion{nullptr};
                }
                if ((0 < inserted_size && inserted_size <= v.capacity()) ||
                    (allocation_note == would_allocate &&
                     inserted_size <= detail::vec_insert_max)) {
                    return vec_insertion{&v, found};
                }
            }

            return vec_insertion{nullptr};
        }

        template<typename U>
        const_iterator
        insert_impl(iterator at, U && u, allocation_note_t allocation_note)
        {
            BOOST_ASSERT(begin() <= at && at <= end());

            if (u.empty())
                return at;

            int const offset = at - begin();

            if (vec_insertion insertion =
                    mutable_insertion_leaf(at, u.size(), allocation_note)) {
                auto const u_size = u.size();
                detail::bump_along_path_to_leaf(ptr_, at - begin(), u_size);
                insertion.vec_->insert(
                    insertion.vec_->begin() + insertion.found_.offset_,
                    u.begin(),
                    u.end());
            } else {
                ptr_ = detail::btree_insert(
                    ptr_,
                    at - begin(),
                    detail::make_node(static_cast<U &&>(u)));
            }

            return begin() + offset;
        }

        detail::node_ptr<T> ptr_;

        friend struct detail::const_vector_iterator<T>;

#endif
    };

}}}

#endif
