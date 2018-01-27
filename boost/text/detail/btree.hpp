#ifndef BOOST_TEXT_DETAIL_BTREE_HPP
#define BOOST_TEXT_DETAIL_BTREE_HPP

#include <boost/text/detail/utility.hpp>

#ifndef BOOST_TEXT_THREAD_UNSAFE
#    include <boost/atomic.hpp>
#endif
#include <boost/align/align.hpp>
#include <boost/align/aligned_alloc.hpp>
#include <boost/align/aligned_delete.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>

#include <vector>


namespace boost { namespace text { namespace detail {

    template<typename T>
    struct node_t;
    template<typename T>
    struct leaf_node_t;
    template<typename T>
    struct interior_node_t;
    template<typename T>
    struct node_ptr;

#ifdef BOOST_TEXT_TESTING
    template<typename T>
    void dump_tree(node_ptr<T> const & root, int key = -1, int indent = 0);
#endif

    template<typename T>
    struct mutable_node_ptr
    {
        ~mutable_node_ptr() noexcept;

        explicit operator bool() const noexcept { return ptr_; }

        node_t<T> * operator->() noexcept { return ptr_; }

        leaf_node_t<T> * as_leaf() noexcept;
        interior_node_t<T> * as_interior() noexcept;

    private:
        mutable_node_ptr(node_ptr<T> & np, node_t<T> * ptr) noexcept :
            node_ptr_(np),
            ptr_(ptr)
        {}

        node_ptr<T> & node_ptr_;
        node_t<T> * ptr_;
        friend node_ptr<T>;
    };

    template<typename T>
    struct node_ptr
    {
        node_ptr() noexcept : ptr_() {}
        explicit node_ptr(node_t<T> const * node) noexcept : ptr_(node) {}

        explicit operator bool() const noexcept { return ptr_.get(); }

        node_t<T> const * operator->() const noexcept { return ptr_.get(); }

        leaf_node_t<T> const * as_leaf() const noexcept;
        interior_node_t<T> const * as_interior() const noexcept;

        node_t<T> const * get() const noexcept { return ptr_.get(); }

        mutable_node_ptr<T> write() const;

        void swap(node_ptr & rhs) noexcept { ptr_.swap(rhs.ptr_); }

        bool operator==(node_ptr const & rhs) const noexcept
        {
            return ptr_ == rhs.ptr_;
        }

    private:
        intrusive_ptr<node_t<T> const> ptr_;
        friend mutable_node_ptr<T>;
    };

    template<typename T>
    struct reference
    {
        reference(
            node_ptr<T> const & node,
            std::ptrdiff_t lo,
            std::ptrdiff_t hi) noexcept;

        node_ptr<T> vec_;
        std::ptrdiff_t lo_;
        std::ptrdiff_t hi_;
    };

    template<typename T>
    void * placement_address(void * buf, std::size_t buf_size) noexcept
    {
        std::size_t const alignment = alignof(T);
        std::size_t const size = sizeof(T);
        return alignment::align(alignment, size, buf, buf_size);
    }

    template<typename T>
    struct node_t
    {
        explicit node_t(bool leaf) noexcept : refs_(0), leaf_(leaf) {}
        node_t(node_t const & rhs) noexcept : refs_(0), leaf_(rhs.leaf_) {}
        node_t & operator=(node_t const & rhs) = delete;

#ifdef BOOST_TEXT_THREAD_UNSAFE
        mutable int refs_;
#else
        mutable atomic<int> refs_;
#endif
        bool leaf_;
    };

    constexpr int min_children = 8;
    constexpr int max_children = 16;

    template<typename T>
    inline std::ptrdiff_t size(node_t<T> const * node) noexcept;

    using keys_t = container::static_vector<std::ptrdiff_t, max_children>;

    template<typename T>
    using children_t = container::static_vector<node_ptr<T>, max_children>;

    static_assert(sizeof(std::ptrdiff_t) * 8 <= 64, "");

    template<typename T>
    struct interior_node_t : node_t<T>
    {
        interior_node_t() noexcept : node_t<T>(false) {}

        void * operator new(std::size_t) = delete;

        alignas(64) keys_t keys_;
        children_t<T> children_;
    };

    template<typename T>
    inline interior_node_t<T> * new_interior_node()
    {
        void * ptr = alignment::aligned_alloc(
            alignof(interior_node_t<T>), sizeof(interior_node_t<T>));
        return ::new (ptr) interior_node_t<T>;
    }

    template<typename T>
    inline interior_node_t<T> *
    new_interior_node(interior_node_t<T> const & other)
    {
        void * ptr = alignment::aligned_alloc(
            alignof(interior_node_t<T>), sizeof(interior_node_t<T>));
        return ::new (ptr) interior_node_t<T>(other);
    }

    template<typename T>
    constexpr int node_buf_size() noexcept
    {
        return max_(alignof(std::vector<T>), alignof(reference<T>)) +
               max_(sizeof(std::vector<T>), sizeof(reference<T>));
    }

    template<typename T>
    struct leaf_node_t : node_t<T>
    {
        enum class which : char { vec, ref };

        leaf_node_t() noexcept : leaf_node_t(std::vector<T>()) {}

        leaf_node_t(std::vector<T> const & t) :
            node_t<T>(true),
            buf_ptr_(nullptr),
            which_(which::vec)
        {
            auto at = placement_address<std::vector<T>>(buf_, sizeof(buf_));
            assert(at);
            buf_ptr_ = new (at) std::vector<T>(t);
        }

        leaf_node_t(std::vector<T> && t) noexcept :
            node_t<T>(true),
            buf_ptr_(nullptr),
            which_(which::vec)
        {
            auto at = placement_address<std::vector<T>>(buf_, sizeof(buf_));
            assert(at);
            buf_ptr_ = new (at) std::vector<T>(std::move(t));
        }

        leaf_node_t(leaf_node_t const & rhs) :
            node_t<T>(true),
            buf_ptr_(rhs.buf_ptr_),
            which_(rhs.which_)
        {
            switch (which_) {
            case which::vec: {
                auto at = placement_address<std::vector<T>>(buf_, sizeof(buf_));
                assert(at);
                buf_ptr_ = new (at) std::vector<T>(rhs.as_vec());
                break;
            }
            case which::ref: {
                auto at = placement_address<reference<T>>(buf_, sizeof(buf_));
                assert(at);
                buf_ptr_ = new (at) reference<T>(rhs.as_reference());
                break;
            }
            default: assert(!"unhandled leaf node case"); break;
            }
        }

        leaf_node_t & operator=(leaf_node_t const &) = delete;
        leaf_node_t(leaf_node_t &&) = delete;
        leaf_node_t & operator=(leaf_node_t &&) = delete;

        ~leaf_node_t() noexcept
        {
            if (!buf_ptr_)
                return;

            switch (which_) {
            case which::vec: as_vec().~vector(); break;
            case which::ref: as_reference().~reference(); break;
            default: assert(!"unhandled leaf node case"); break;
            }
        }

        int size() const noexcept
        {
            switch (which_) {
            case which::vec: return as_vec().size(); break;
            case which::ref:
                return as_reference().hi_ - as_reference().lo_;
                break;
            default: assert(!"unhandled leaf node case"); break;
            }
            return -(1 << 30); // This should never execute.
        }

        std::vector<T> const & as_vec() const noexcept
        {
            assert(which_ == which::vec);
            return *static_cast<std::vector<T> *>(buf_ptr_);
        }

        reference<T> const & as_reference() const noexcept
        {
            assert(which_ == which::ref);
            return *static_cast<reference<T> *>(buf_ptr_);
        }

        std::vector<T> & as_vec() noexcept
        {
            assert(which_ == which::vec);
            return *static_cast<std::vector<T> *>(buf_ptr_);
        }

        reference<T> & as_reference() noexcept
        {
            assert(which_ == which::ref);
            return *static_cast<reference<T> *>(buf_ptr_);
        }

        char buf_[node_buf_size<T>()];
        void * buf_ptr_;
        which which_;
    };

    template<typename T>
    inline mutable_node_ptr<T>::~mutable_node_ptr() noexcept
    {
        node_ptr_.ptr_ = ptr_;
    }

    template<typename T>
    inline leaf_node_t<T> * mutable_node_ptr<T>::as_leaf() noexcept
    {
        assert(ptr_);
        assert(ptr_->leaf_);
        return static_cast<leaf_node_t<T> *>(ptr_);
    }

    template<typename T>
    inline interior_node_t<T> * mutable_node_ptr<T>::as_interior() noexcept
    {
        assert(ptr_);
        assert(!ptr_->leaf_);
        return static_cast<interior_node_t<T> *>(ptr_);
    }

    template<typename T>
    inline leaf_node_t<T> const * node_ptr<T>::as_leaf() const noexcept
    {
        assert(ptr_);
        assert(ptr_->leaf_);
        return static_cast<leaf_node_t<T> const *>(ptr_.get());
    }

    template<typename T>
    inline interior_node_t<T> const * node_ptr<T>::as_interior() const noexcept
    {
        assert(ptr_);
        assert(!ptr_->leaf_);
        return static_cast<interior_node_t<T> const *>(ptr_.get());
    }

    template<typename T>
    inline mutable_node_ptr<T> node_ptr<T>::write() const
    {
        auto & this_ref = const_cast<node_ptr<T> &>(*this);
        if (ptr_->refs_ == 1)
            return mutable_node_ptr<T>(
                this_ref, const_cast<node_t<T> *>(ptr_.get()));
        if (ptr_->leaf_)
            return mutable_node_ptr<T>(
                this_ref, new leaf_node_t<T>(*as_leaf()));
        else
            return mutable_node_ptr<T>(
                this_ref, new_interior_node(*as_interior()));
    }

#ifdef BOOST_TEXT_THREAD_UNSAFE

    template<typename T>
    inline void intrusive_ptr_add_ref(node_t<T> const * node)
    {
        ++node->refs_;
    }

    template<typename T>
    inline void intrusive_ptr_release(node_t<T> const * node)
    {
        if (!--node->refs_) {
            if (node->leaf_)
                delete static_cast<leaf_node_t<T> const *>(node);
            else
                alignment::aligned_delete{}((interior_node_t<T> *)(node));
        }
    }

#else

    // These functions were implemented following the "Reference counting"
    // example from Boost.Atomic.

    template<typename T>
    inline void intrusive_ptr_add_ref(node_t<T> const * node)
    {
        node->refs_.fetch_add(1, boost::memory_order_relaxed);
    }

    template<typename T>
    inline void intrusive_ptr_release(node_t<T> const * node)
    {
        if (node->refs_.fetch_sub(1, boost::memory_order_release) == 1) {
            boost::atomic_thread_fence(boost::memory_order_acquire);
            if (node->leaf_)
                delete static_cast<leaf_node_t<T> const *>(node);
            else
                alignment::aligned_delete{}((interior_node_t<T> *)(node));
        }
    }

#endif

    template<typename T>
    inline std::ptrdiff_t size(node_t<T> const * node) noexcept
    {
        if (!node) {
            return 0;
        } else if (node->leaf_) {
            return static_cast<leaf_node_t<T> const *>(node)->size();
        } else {
            auto int_node = static_cast<interior_node_t<T> const *>(node);
            if (int_node->keys_.empty())
                return 0;
            return int_node->keys_.back();
        }
    }

    template<typename T>
    inline children_t<T> const & children(node_ptr<T> const & node) noexcept
    {
        return node.as_interior()->children_;
    }

    template<typename T>
    inline children_t<T> & children(mutable_node_ptr<T> & node) noexcept
    {
        return node.as_interior()->children_;
    }

    template<typename T>
    inline keys_t const & keys(node_ptr<T> const & node) noexcept
    {
        return node.as_interior()->keys_;
    }

    template<typename T>
    inline keys_t & keys(mutable_node_ptr<T> & node) noexcept
    {
        return node.as_interior()->keys_;
    }

    template<typename T>
    inline int num_children(node_ptr<T> const & node) noexcept
    {
        return static_cast<int>(children(node).size());
    }

    template<typename T>
    inline int num_children(mutable_node_ptr<T> & node) noexcept
    {
        return static_cast<int>(children(node).size());
    }

    template<typename T>
    inline int num_keys(node_ptr<T> const & node) noexcept
    {
        return static_cast<int>(keys(node).size());
    }

    template<typename T>
    inline int num_keys(mutable_node_ptr<T> & node) noexcept
    {
        return static_cast<int>(keys(node).size());
    }

    template<typename T>
    inline bool full(node_ptr<T> const & node) noexcept
    {
        return num_children(node) == max_children;
    }

    template<typename T>
    inline bool almost_full(node_ptr<T> const & node) noexcept
    {
        return num_children(node) == max_children - 1;
    }

    template<typename T>
    inline bool leaf_children(node_ptr<T> const & node)
    {
        return children(node)[0]->leaf_;
    }

    template<typename T>
    inline std::ptrdiff_t
    offset(interior_node_t<T> const * node, int i) noexcept
    {
        assert(0 <= i);
        assert(i <= (int)node->keys_.size());
        if (i == 0)
            return 0;
        return node->keys_[i - 1];
    }

    template<typename T>
    inline std::ptrdiff_t offset(node_ptr<T> const & node, int i) noexcept
    {
        return offset(node.as_interior(), i);
    }

    template<typename T>
    inline std::ptrdiff_t
    offset(mutable_node_ptr<T> const & node, int i) noexcept
    {
        return offset(const_cast<mutable_node_ptr<T> &>(node).as_interior(), i);
    }

    template<typename T>
    inline std::ptrdiff_t
    find_child(interior_node_t<T> const * node, std::ptrdiff_t n) noexcept
    {
        int i = 0;
        auto const sizes = static_cast<int>(node->keys_.size());
        while (i < sizes - 1 && node->keys_[i] <= n) {
            ++i;
        }
        assert(i < sizes);
        return i;
    }

    template<typename T>
    struct found_leaf
    {
        node_ptr<T> const * leaf_;
        std::ptrdiff_t offset_;
        alignas(
            64) container::static_vector<interior_node_t<T> const *, 24> path_;

        static_assert(sizeof(interior_node_t<T> const *) * 8 <= 64, "");
    };

    template<typename T>
    inline void find_leaf(
        node_ptr<T> const & node,
        std::ptrdiff_t n,
        found_leaf<T> & retval) noexcept
    {
        assert(node);
        assert(n <= size(node.get()));
        if (node->leaf_) {
            retval.leaf_ = &node;
            retval.offset_ = n;
            return;
        }
        retval.path_.push_back(node.as_interior());
        auto const i = find_child(node.as_interior(), n);
        node_ptr<T> const & child = children(node)[i];
        auto const offset_ = offset(node, i);
        find_leaf(child, n - offset_, retval);
    }

    template<typename T>
    struct found_element
    {
        found_leaf<T> leaf_;
        T const * element_;
    };

    template<typename T>
    inline void find_element(
        node_ptr<T> const & node,
        std::ptrdiff_t n,
        found_element<T> & retval) noexcept
    {
        assert(node);
        find_leaf(node, n, retval.leaf_);

        leaf_node_t<T> const * leaf = retval.leaf_.leaf_->as_leaf();
        T const * e = nullptr;
        switch (leaf->which_) {
        case leaf_node_t<T>::which::vec:
            e = &leaf->as_vec()[retval.leaf_.offset_];
            break;
        case leaf_node_t<T>::which::ref:
            e = &leaf->as_reference().vec_.as_leaf()->as_vec()
                     [leaf->as_reference().lo_ + retval.leaf_.offset_];
            break;
        default: assert(!"unhandled leaf node case"); break;
        }
        retval.element_ = e;
    }

    template<typename T>
    inline reference<T>::reference(
        node_ptr<T> const & vec_node,
        std::ptrdiff_t lo,
        std::ptrdiff_t hi) noexcept :
        vec_(vec_node),
        lo_(lo),
        hi_(hi)
    {
        assert(vec_node);
        assert(vec_node->leaf_);
        assert(vec_node.as_leaf()->which_ == leaf_node_t<T>::which::vec);
    }

    template<typename T>
    inline node_ptr<T> make_node(std::vector<T> const & t)
    {
        return node_ptr<T>(new leaf_node_t<T>(t));
    }

    template<typename T>
    inline node_ptr<T> make_node(std::vector<T> && t)
    {
        return node_ptr<T>(new leaf_node_t<T>(std::move(t)));
    }

    template<typename T>
    inline node_ptr<T>
    make_ref(leaf_node_t<T> const * v, std::ptrdiff_t lo, std::ptrdiff_t hi)
    {
        assert(v->which_ == leaf_node_t<T>::which::vec);
        leaf_node_t<T> * leaf = nullptr;
        node_ptr<T> retval(leaf = new leaf_node_t<T>);
        leaf->which_ = leaf_node_t<T>::which::ref;
        auto at =
            placement_address<reference<T>>(leaf->buf_, sizeof(leaf->buf_));
        assert(at);
        leaf->buf_ptr_ = new (at) reference<T>(node_ptr<T>(v), lo, hi);
        return retval;
    }

    template<typename T>
    inline node_ptr<T>
    make_ref(reference<T> const & r, std::ptrdiff_t lo, std::ptrdiff_t hi)
    {
        auto const offset = r.lo_;
        return make_ref(r.vec_.as_leaf(), lo + offset, hi + offset);
    }

    template<typename T, typename Fn>
    void foreach_leaf(node_ptr<T> const & root, Fn && f)
    {
        if (!root)
            return;

        std::ptrdiff_t offset = 0;
        while (true) {
            found_leaf<T> found;
            find_leaf(root, offset, found);
            leaf_node_t<T> const * leaf = found.leaf_->as_leaf();

            if (!f(leaf))
                break;

            if ((offset += size(leaf)) == size(root.get()))
                break;
        }
    }

    template<typename Iter>
    struct reversed_range
    {
        Iter first_;
        Iter last_;

        Iter begin() const noexcept { return first_; }
        Iter end() const noexcept { return last_; }
    };

    template<typename Container>
    reversed_range<typename Container::const_reverse_iterator>
    reverse(Container const & c) noexcept
    {
        return {c.rbegin(), c.rend()};
    }

    template<typename T>
    inline void
    bump_keys(interior_node_t<T> * node, int from, std::ptrdiff_t bump)
    {
        for (int i = from, size = (int)node->keys_.size(); i < size; ++i) {
            node->keys_[i] += bump;
        }
    }

    template<typename T>
    inline void insert_child(
        interior_node_t<T> * node, int i, node_ptr<T> && child) noexcept
    {
        auto const child_size = size(child.get());
        node->children_.insert(node->children_.begin() + i, std::move(child));
        node->keys_.insert(node->keys_.begin() + i, offset(node, i));
        bump_keys(node, i, child_size);
    }

    enum erasure_adjustments { adjust_keys, dont_adjust_keys };

    template<typename T>
    inline void erase_child(
        interior_node_t<T> * node,
        int i,
        erasure_adjustments adj = adjust_keys) noexcept
    {
        auto const child_size = size(node->children_[i].get());
        node->children_.erase(node->children_.begin() + i);
        node->keys_.erase(node->keys_.begin() + i);
        if (adj == adjust_keys)
            bump_keys(node, i, -child_size);
    }

    template<typename T, typename LeafDatum>
    inline node_ptr<T> slice_leaf(
        node_ptr<T> const & node,
        std::ptrdiff_t lo,
        std::ptrdiff_t hi,
        bool immutable,
        LeafDatum)
    {
        assert(node);
        assert(0 <= lo && lo <= size(node.get()));
        assert(0 <= hi && hi <= size(node.get()));
        assert(lo < hi);

        bool const leaf_mutable = !immutable && node->refs_ == 1;

        switch (node.as_leaf()->which_) {
        case leaf_node_t<T>::which::vec:
            if (!leaf_mutable)
                return make_ref(node.as_leaf(), lo, hi);
            {
                auto mut_node = node.write();
                std::vector<T> & v = mut_node.as_leaf()->as_vec();
                v.erase(v.begin() + hi, v.end());
                v.erase(v.begin(), v.begin() + lo);
            }
            return node;
        case leaf_node_t<T>::which::ref: {
            if (!leaf_mutable)
                return make_ref(node.as_leaf()->as_reference(), lo, hi);
            {
                auto mut_node = node.write();
                reference<T> & ref = mut_node.as_leaf()->as_reference();
                ref.hi_ = ref.lo_ + hi;
                ref.lo_ = ref.lo_ + lo;
            }
            return node;
        }
        default: assert(!"unhandled leaf node case"); break;
        }
        return node_ptr<T>(); // This should never execute.
    }

    template<typename T>
    struct leaf_slices
    {
        node_ptr<T> slice;
        node_ptr<T> other_slice;
    };

    template<typename T, typename LeafDatum>
    inline leaf_slices<T> erase_leaf(
        node_ptr<T> & node,
        std::ptrdiff_t lo,
        std::ptrdiff_t hi,
        LeafDatum datum)
    {
        assert(node);
        assert(0 <= lo && lo <= size(node.get()));
        assert(0 <= hi && hi <= size(node.get()));
        assert(lo < hi);

        bool const leaf_mutable = node.as_leaf()->refs_ == 1;
        auto const leaf_size = size(node.get());

        leaf_slices<T> retval;

        if (lo == 0 && hi == leaf_size)
            return retval;

        if (leaf_mutable &&
            node.as_leaf()->which_ == leaf_node_t<T>::which::vec) {
            {
                auto mut_node = node.write();
                std::vector<T> & v = mut_node.as_leaf()->as_vec();
                v.erase(v.begin() + lo, v.begin() + hi);
            }
            retval.slice = node;
            return retval;
        }

        if (hi != leaf_size)
            retval.other_slice = slice_leaf(node, hi, leaf_size, true, datum);
        if (lo != 0)
            retval.slice = slice_leaf(node, 0, lo, false, datum);

        if (!retval.slice)
            retval.slice.swap(retval.other_slice);

        return retval;
    }

    // Follows CLRS.
    template<typename T>
    inline node_ptr<T> btree_split_child(node_ptr<T> const & parent, int i)
    {
        assert(0 <= i && i < num_children(parent));
        assert(!full(parent));
        assert(full(children(parent)[i]) || almost_full(children(parent)[i]));

        interior_node_t<T> * new_node = nullptr;
        node_ptr<T> new_node_ptr(new_node = new_interior_node<T>());

        assert(!leaf_children(parent));
        node_ptr<T> const & child = children(parent)[i];

        {
            int const elements = min_children - (full(child) ? 0 : 1);
            new_node->children_.resize(elements);
            std::copy(
                children(child).begin() + min_children,
                children(child).end(),
                new_node->children_.begin());
            new_node->keys_.resize(elements);
            auto it = new_node->children_.begin();
            std::ptrdiff_t sum = 0;
            for (auto & key : new_node->keys_) {
                sum += size(it->get());
                key = sum;
                ++it;
            }
        }

        {
            auto mut_parent = parent.write();
            children(mut_parent)
                .insert(children(mut_parent).begin() + i + 1, new_node_ptr);
        }

        {
            auto mut_child = children(parent)[i].write();
            children(mut_child).resize(min_children);
            keys(mut_child).resize(min_children);
        }
        {
            auto mut_parent = parent.write();
            keys(mut_parent)
                .insert(
                    keys(mut_parent).begin() + i,
                    offset(mut_parent, i) + size(children(parent)[i].get()));
        }

        return parent;
    }

    template<typename T>
    bool child_immutable(node_ptr<T> const & node)
    {
        return false;
    }

    // Analogous to btree_split_child(), for leaf nodes.
    template<typename T, typename LeafDatum>
    inline void btree_split_leaf(
        node_ptr<T> const & parent, int i, std::ptrdiff_t at, LeafDatum datum)
    {
        assert(0 <= i && i < num_children(parent));
        assert(0 <= at && at <= size(parent.get()));
        assert(!full(parent));

        node_ptr<T> const & child = children(parent)[i];

        auto const child_size = child.as_leaf()->size();
        auto const offset_at_i = offset(parent, i);
        auto const cut = static_cast<int>(at - offset_at_i);

        if (cut == 0 || cut == child_size)
            return;

        node_ptr<T> right = slice_leaf(child, cut, child_size, true, datum);
        node_ptr<T> left =
            slice_leaf(child, 0, cut, child_immutable(child), datum);

        auto mut_parent = parent.write();
        children(mut_parent)[i] = left;
        children(mut_parent)
            .insert(children(mut_parent).begin() + i + 1, right);
        keys(mut_parent)
            .insert(keys(mut_parent).begin() + i, offset_at_i + cut);
    }

    // Follows CLRS.
    template<typename T, typename LeafDatum>
    inline node_ptr<T> btree_insert_nonfull(
        node_ptr<T> & parent,
        std::ptrdiff_t at,
        node_ptr<T> && node,
        LeafDatum datum)
    {
        assert(!parent->leaf_);
        assert(0 <= at && at <= size(parent.get()));
        assert(node->leaf_);

        int i = find_child(parent.as_interior(), at);
        if (leaf_children(parent)) {
            // Note that this split may add a node to parent, for a
            // maximum of two added nodes in the leaf code path.
            btree_split_leaf(parent, i, at, datum);
            if (keys(parent)[i] <= at)
                ++i;

            auto mut_parent = parent.write();
            insert_child(mut_parent.as_interior(), i, std::move(node));
        } else {
            {
                node_ptr<T> const & child = children(parent)[i];
                bool const child_i_needs_split =
                    full(child) || (leaf_children(child) && almost_full(child));
                if (child_i_needs_split) {
                    parent = btree_split_child(parent, i);
                    if (keys(parent)[i] <= at)
                        ++i;
                }
            }
            auto mut_parent = parent.write();
            auto delta = -size(children(mut_parent)[i].get());
            node_ptr<T> new_child = btree_insert_nonfull(
                children(mut_parent)[i],
                at - offset(mut_parent, i),
                std::move(node),
                datum);
            delta += size(new_child.get());
            children(mut_parent)[i] = new_child;
            for (int j = i, size = num_keys(mut_parent); j < size; ++j) {
                keys(mut_parent)[j] += delta;
            }
        }

        return parent;
    }

    // Follows CLRS.
    template<typename T, typename LeafDatum>
    inline node_ptr<T> btree_insert(
        node_ptr<T> & root,
        std::ptrdiff_t at,
        node_ptr<T> && node,
        LeafDatum datum)
    {
        assert(0 <= at && at <= size(root.get()));
        assert(node->leaf_);

        if (!root) {
            return node;
        } else if (root->leaf_) {
            interior_node_t<T> * new_root = nullptr;
            node_ptr<T> new_root_ptr(new_root = new_interior_node<T>());
            auto const root_size = size(root.get());
            new_root->children_.push_back(std::move(root));
            new_root->keys_.push_back(root_size);
            return btree_insert_nonfull(
                new_root_ptr, at, std::move(node), datum);
        } else if (full(root) || (leaf_children(root) && almost_full(root))) {
            interior_node_t<T> * new_root = nullptr;
            node_ptr<T> new_root_ptr(new_root = new_interior_node<T>());
            auto const root_size = size(root.get());
            new_root->children_.push_back(std::move(root));
            new_root->keys_.push_back(root_size);
            new_root_ptr = btree_split_child(new_root_ptr, 0);
            return btree_insert_nonfull(
                new_root_ptr, at, std::move(node), datum);
        } else {
            return btree_insert_nonfull(root, at, std::move(node), datum);
        }
    }

    // Recursing top-to-bottom, pull nodes down the tree as necessary to
    // ensure that each node has min_children + 1 nodes in it *before*
    // recursing into it.  This enables the erasure to happen in a single
    // downward pass, with no backtracking.  This function only erases
    // entire segments; the segments must have been split appropriately
    // before this function is ever called.
    template<typename T, typename LeafDatum>
    inline node_ptr<T> btree_erase(
        node_ptr<T> const & node,
        std::ptrdiff_t at,
        leaf_node_t<T> const * leaf,
        LeafDatum datum)
    {
        assert(node);

        auto child_index = find_child(node.as_interior(), at);

        if (leaf_children(node)) {
            if (num_children(node) == 2)
                return children(node)[child_index ? 0 : 1];

            assert(children(node)[child_index].as_leaf() == leaf);

            {
                auto mut_node = node.write();
                erase_child(mut_node.as_interior(), child_index);
            }

            return node;
        }

        node_ptr<T> new_child;

        node_ptr<T> const & child = children(node)[child_index];
        // Due to the use of almost_full() in a few places, == does not actually
        // work here.  As unsatisfying as it is, the minimium possible number of
        // children is actually min_children - 1.
        if (num_children(child) <= min_children) {
            assert(child_index != 0 || child_index != num_children(node) - 1);

            if (child_index != 0 &&
                min_children + 1 <=
                    num_children(children(node)[child_index - 1])) {
                node_ptr<T> const & child_left_sib =
                    children(node)[child_index - 1];

                // Remove last element of left sibling.
                node_ptr<T> moved_node = children(child_left_sib).back();
                auto const moved_node_size = size(moved_node.get());

                {
                    auto mut_left = child_left_sib.write();
                    erase_child(
                        mut_left.as_interior(), num_children(mut_left) - 1);
                }

                // Prepend last element onto child; now child has min_children
                // + 1 children, and we can recurse.
                {
                    auto mut_child = child.write();
                    insert_child(
                        mut_child.as_interior(), 0, std::move(moved_node));
                }

                std::ptrdiff_t const offset_ = offset(node, child_index);
                new_child = btree_erase(
                    child, at - offset_ + moved_node_size, leaf, datum);
            } else if (
                child_index != num_children(node) - 1 &&
                min_children + 1 <=
                    num_children(children(node)[child_index + 1])) {
                node_ptr<T> const & child_right_sib =
                    children(node)[child_index + 1];

                // Remove first element of right sibling.
                node_ptr<T> moved_node = children(child_right_sib).front();

                {
                    auto mut_right = child_right_sib.write();
                    erase_child(mut_right.as_interior(), 0);
                }

                // Append first element onto child; now child has min_children
                // + 1 children, and we can recurse.
                {
                    auto mut_child = child.write();
                    insert_child(
                        mut_child.as_interior(),
                        num_children(mut_child),
                        std::move(moved_node));
                }

                std::ptrdiff_t const offset_ = offset(node, child_index);
                new_child = btree_erase(child, at - offset_, leaf, datum);
            } else {
                auto const right_index =
                    child_index == 0 ? child_index + 1 : child_index;
                auto const left_index = right_index - 1;

                node_ptr<T> const & left = children(node)[left_index];
                node_ptr<T> const & right = children(node)[right_index];

                {
                    auto mut_left = left.write();
                    auto mut_right = right.write();

                    children_t<T> & left_children = children(mut_left);
                    children_t<T> & right_children = children(mut_right);

                    left_children.insert(
                        left_children.end(),
                        right_children.begin(),
                        right_children.end());

                    keys_t & left_keys = keys(mut_left);
                    keys_t & right_keys = keys(mut_right);

                    auto const old_left_size = left_keys.back();
                    int const old_children = num_keys(mut_left);

                    left_keys.insert(
                        left_keys.end(), right_keys.begin(), right_keys.end());
                    for (int i = old_children, size = num_keys(mut_left);
                         i < size;
                         ++i) {
                        left_keys[i] += old_left_size;
                    }
                }

                std::ptrdiff_t const offset_ = offset(node, left_index);
                new_child = btree_erase(left, at - offset_, leaf, datum);

                // This can only happen if node is the root.
                if (num_children(node) == 2)
                    return new_child;

                auto mut_node = node.write();
                erase_child(
                    mut_node.as_interior(), right_index, dont_adjust_keys);

                if (right_index <= child_index)
                    --child_index;
            }
        } else {
            std::ptrdiff_t const offset_ = offset(node, child_index);
            new_child = btree_erase(
                children(node)[child_index], at - offset_, leaf, datum);
        }

        {
            auto mut_node = node.write();
            children(mut_node)[child_index] = new_child;
            std::ptrdiff_t prev_size = 0;
            for (int i = 0, size = num_keys(mut_node); i < size; ++i) {
                prev_size += detail::size(children(mut_node)[i].get());
                keys(mut_node)[i] = prev_size;
            }
        }

        return node;
    }

    template<typename T, typename LeafDatum>
    inline node_ptr<T> btree_erase(
        node_ptr<T> & root,
        std::ptrdiff_t lo,
        std::ptrdiff_t hi,
        LeafDatum datum)
    {
        assert(root);
        assert(0 <= lo && lo <= size(root.get()));
        assert(0 <= hi && hi <= size(root.get()));
        assert(lo < hi);

        assert(root);

        if (lo == 0 && hi == size(root.get())) {
            return node_ptr<T>();
        } else if (root->leaf_) {
            leaf_slices<T> slices;
            slices = erase_leaf(root, lo, hi, datum);
            if (!slices.other_slice) {
                return slices.slice;
            } else {
                interior_node_t<T> * new_root = nullptr;
                node_ptr<T> new_root_ptr(new_root = new_interior_node<T>());
                new_root->keys_.push_back(size(slices.slice.get()));
                new_root->keys_.push_back(
                    new_root->keys_[0] + size(slices.other_slice.get()));
                new_root->children_.push_back(std::move(slices.slice));
                new_root->children_.push_back(std::move(slices.other_slice));
                return new_root_ptr;
            }
        } else {
            auto const final_size = size(root.get()) - (hi - lo);

            // Right after the hi-segment, insert the suffix of the
            // hi-segment that's not being erased (if there is one).
            detail::found_leaf<T> found_hi;
            detail::find_leaf(root, hi, found_hi);
            auto const hi_leaf_size = size(found_hi.leaf_->get());
            if (found_hi.offset_ != 0 && found_hi.offset_ != hi_leaf_size) {
                node_ptr<T> suffix = slice_leaf(
                    *found_hi.leaf_,
                    found_hi.offset_,
                    hi_leaf_size,
                    true,
                    datum);
                auto const suffix_at = hi - found_hi.offset_ + hi_leaf_size;
                root = btree_insert(root, suffix_at, std::move(suffix), datum);
                detail::find_leaf(root, suffix_at, found_hi);
            }

            // Right before the lo-segment, insert the prefix of the
            // lo-segment that's not being erased (if there is one).
            detail::found_leaf<T> found_lo;
            detail::find_leaf(root, lo, found_lo);
            if (found_lo.offset_ != 0) {
                auto const lo_leaf_size = size(found_lo.leaf_->get());
                node_ptr<T> prefix = slice_leaf(
                    *found_lo.leaf_, 0, found_lo.offset_, true, datum);
                if (prefix.get() == found_lo.leaf_->get())
                    hi -= lo_leaf_size;
                root = btree_insert(
                    root, lo - found_lo.offset_, std::move(prefix), datum);
                detail::find_leaf(root, lo, found_lo);
            }

            assert(found_lo.offset_ == 0);
            assert(found_hi.offset_ == 0 || found_hi.offset_ == hi_leaf_size);

            leaf_node_t<T> const * leaf_lo = found_lo.leaf_->as_leaf();
            while (true) {
                root = btree_erase(root, lo, leaf_lo, datum);
                assert(final_size <= size(root.get()));
                if (size(root.get()) == final_size)
                    break;
                found_leaf<T> found;
                find_leaf(root, lo, found);
                leaf_lo = found.leaf_->as_leaf();
            }
        }

        return root;
    }

}}}

#endif
