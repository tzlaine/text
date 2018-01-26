#ifndef BOOST_TEXT_SEGMENTED_VECTOR_HPP
#define BOOST_TEXT_SEGMENTED_VECTOR_HPP

#include <boost/text/detail/btree.hpp>
#include <boost/text/detail/vector_iterator.hpp>

#include <initializer_list>


namespace boost { namespace text {

    namespace detail {

        constexpr int vec_insert_max = 512;
    }

    template<typename T>
    struct segmented_vector
    {
        using iterator = detail::const_vector_iterator<T>;
        using const_iterator = detail::const_vector_iterator<T>;
        using reverse_iterator = detail::const_reverse_vector_iterator<T>;
        using const_reverse_iterator = detail::const_reverse_vector_iterator<T>;

        using size_type = std::ptrdiff_t;

        /** Default ctor.

            \post size() == 0 && begin() == end() */
        segmented_vector() noexcept : ptr_(nullptr) {}

        segmented_vector(segmented_vector const & rhs) = default;
        segmented_vector(segmented_vector && rhs) noexcept = default;

        template <typename Iter>
        segmented_vector(Iter first, Iter last)
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
            return const_reverse_iterator(const_iterator(*this, size() - 1));
        }
        const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator(const_iterator(*this, -1));
        }

        bool empty() const noexcept { return size() == 0; }

        size_type size() const noexcept { return detail::size(ptr_.get()); }

        /** Returns a const reference to the i-th element of *this.

            \pre 0 <= i && i < size() */
        T const & operator[](size_type n) const noexcept
        {
            assert(ptr_);
            assert(n < size());
            detail::found_element<T> found;
            find_element(ptr_, n, found);
            return *found.element_;
        }

        /** Returns the maximum size a segmented_vector can have. */
        size_type max_size() const noexcept { return PTRDIFF_MAX; }

        /** Visits each segment s of *this and calls f(s).  Each segment is
            presented as a pair of T const * pointers.  Depending of the
            operation performed on each segment, this may be more efficient
            than iterating over [begin(), end()).

            \pre Fn is an Invocable accepting a pair of T const *
            parameters. */
        template<typename Fn>
        void foreach_segment(Fn && f) const
        {
            detail::foreach_leaf(
                ptr_, [&](detail::leaf_node_t<T> const * leaf) {
                    switch (leaf->which_) {
                    case detail::leaf_node_t<T>::which::vec:
                        f(&*leaf->as_vec().begin(), &*leaf->as_vec().end());
                        break;
                    case detail::leaf_node_t<T>::which::ref:
                        f(&*leaf->as_reference()
                                .vec_.as_leaf()
                                ->as_vec()
                                .begin(),
                          &*leaf->as_reference()
                                .vec_.as_leaf()
                                ->as_vec()
                                .end());
                        break;
                    default: assert(!"unhandled leaf node case"); break;
                    }
                    return true;
                });
        }

        /** Lexicographical compare.  Returns a value < 0 when *this is
            lexicographically less than rhs, 0 if *this == rhs, and a value >
            0 if *this is lexicographically greater than rhs. */
        int compare(segmented_vector rhs) const noexcept
        {
            if (empty())
                return rhs.empty() ? 0 : -1;

            // TODO: This could probably be optimized quite a bit by doing
            // something equivalent to mismatch, segment-wise.
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
            return compare(rhs) == 0;
        }

        bool operator!=(segmented_vector rhs) const noexcept
        {
            return compare(rhs) != 0;
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
            return insert(end(), std::move(t));
        }

        /** Inserts t into *this at offset at by moving t. */
        segmented_vector & insert(const_iterator at, T t)
        {
            assert(begin() <= at && at <= end());

#if 0
            std::cerr << "size() " << size() << " at " << (at - begin())
                      << "\n";
#endif
#if 0
            if (vec_insertion insertion =
                    mutable_insertion_leaf(at, 1, would_allocate)) {
                for (auto node : insertion.found_.path_) {
                    auto from = detail::find_child(node, at - begin());
                    detail::bump_keys(
                        const_cast<detail::interior_node_t<T> *>(node),
                        from,
                        1);
                }
#if 0
                std::cerr << "insertion vec size() " << insertion.vec_->size()
                          << " at " << insertion.found_.offset_ << "\n";
#endif
                insertion.vec_->insert(
                    insertion.vec_->begin() + insertion.found_.offset_,
                    std::move(t));
            } else
#endif
            {
                ptr_ = detail::btree_insert(
                    ptr_,
                    at - begin(),
                    detail::make_node(std::vector<T>(1, std::move(t))),
                    0);
            }

            return *this;
        }

        /** Inserts the sequence of T from t into *this starting at offset at,
            by moving the contents of t. */
        segmented_vector & insert(const_iterator at, std::vector<T> t)
        {
            return insert_impl(at, std::move(t), would_not_allocate);
        }

        /** Inserts the T sequence [first, last) into *this starting at
            position at. */
        template<typename Iter>
        segmented_vector & insert(const_iterator at, Iter first, Iter last)
        {
            assert(begin() <= at && at <= end());

            if (first == last)
                return *this;

            ptr_ = detail::btree_insert(
                ptr_,
                at - begin(),
                detail::make_node(std::vector<T>(first, last)),
                0);

            return *this;
        }

        /** Erases the element at position at.

            \pre begin() <= at && at < end() */
        segmented_vector & erase(const_iterator at)
        {
            assert(begin() <= at && at < end());

            auto const lo = at - begin();
            ptr_ = btree_erase(ptr_, lo, lo + 1, 0);

            return *this;
        }

        /** Erases the portion of *this delimited by [first, last).

            \pre first <= last */
        segmented_vector & erase(const_iterator first, const_iterator last)
        {
            assert(first <= last);
            assert(begin() <= first && last <= end());

            if (first == last)
                return *this;

            auto const lo = first - begin();
            auto const hi = last - begin();
            ptr_ = btree_erase(ptr_, lo, hi, 0);

            return *this;
        }

        /** Replaces the element at position at with t, by moving t.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        segmented_vector & replace(const_iterator at, T t)
        {
            assert(begin() <= at && at <= end());

#if 0
            if (vec_insertion insertion =
                    mutable_insertion_leaf(at, 0, would_allocate)) {
                (*insertion.vec_)[insertion.found_.offset_] = std::move(t);
            } else
#endif
            {
                auto const offset = at - begin();
                erase(at);
                ptr_ = detail::btree_insert(
                    ptr_,
                    offset,
                    detail::make_node(std::vector<T>(1, std::move(t))),
                    0);
            }

            return *this;
        }

        /** Replaces the portion of *this delimited by old_substr with the
            sequence of T from t by moving the contents of t.

            \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        segmented_vector &
        replace(const_iterator first, const_iterator last, std::vector<T> t)
        {
            return erase(first, last).insert(first, t);
        }

        /** Replaces the portion of *this delimited by [old_first, old_last)
            with the T sequence [new_first, new_last).

           \pre begin() <= old_substr.begin() && old_substr.end() <= end() */
        template<typename Iter>
        segmented_vector & replace(
            const_iterator old_first,
            const_iterator old_last,
            Iter new_first,
            Iter new_last)
        {
            return erase(old_first, old_last).insert(new_first, new_last);
        }

        /** Swaps *this with rhs. */
        void swap(segmented_vector & rhs) { ptr_.swap(rhs.ptr_); }

    private:
        enum allocation_note_t { would_allocate, would_not_allocate };

        struct vec_insertion
        {
            explicit operator bool() const { return vec_ != nullptr; }

            std::vector<T> * vec_;
            detail::found_leaf<T> found_;
        };

        vec_insertion mutable_insertion_leaf(
            iterator at, size_type size, allocation_note_t allocation_note)
        {
            if (!ptr_)
                return vec_insertion{nullptr};

            detail::found_leaf<T> found;
            find_leaf(ptr_, at - begin(), found);

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
                auto const inserted_size = v.size() + size;
                if (inserted_size <= v.capacity() ||
                    (allocation_note == would_allocate &&
                     inserted_size <= detail::vec_insert_max)) {
                    return vec_insertion{&v, found};
                }
            }

            return vec_insertion{nullptr};
        }

        template<typename U>
        segmented_vector &
        insert_impl(iterator at, U && u, allocation_note_t allocation_note)
        {
            assert(begin() <= at && at <= end());

            if (u.empty())
                return *this;

#if 0
            if (vec_insertion insertion =
                    mutable_insertion_leaf(at, u.size(), allocation_note)) {
                auto const u_size = u.size();
                for (auto node : insertion.found_.path_) {
                    auto from = detail::find_child(node, at - begin());
                    detail::bump_keys(
                        const_cast<detail::interior_node_t<T> *>(node),
                        from,
                        u_size);
                }
                insertion.vec_->insert(
                    insertion.found_.offset_, u.begin(), u.end());
            } else
#endif
            {
                ptr_ = detail::btree_insert(
                    ptr_,
                    at - begin(),
                    detail::make_node(std::forward<T &&>(u)),
                    0);
            }

            return *this;
        }

        detail::node_ptr<T> ptr_;

        friend struct detail::const_vector_iterator<T>;
    };

}}

#endif
