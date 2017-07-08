#ifndef BOOST_TEXT_DETAIL_ROPE_HPP
#define BOOST_TEXT_DETAIL_ROPE_HPP

#include <boost/text/text_view.hpp>
#include <boost/text/text.hpp>

// TODO: Profile both ways.
#ifndef BOOST_TEXT_THREAD_UNSAFE
#include <boost/atomic.hpp>
#endif
#include <boost/align/align.hpp>
#include <boost/align/aligned_alloc.hpp>
#include <boost/align/aligned_delete.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>


namespace boost { namespace text { namespace detail {

    // TODO: Experiment with collapsing adjacent text_views, references, etc.,
    // when inserting, erasing, etc.

    struct node_t;
    struct leaf_node_t;
    struct interior_node_t;
    struct node_ptr;

    void dump_tree (node_ptr const & root, int key = -1, int indent = 0);

    struct mutable_node_ptr
    {
        ~mutable_node_ptr () noexcept;

        explicit operator bool () const noexcept
        { return ptr_; }

        node_t * operator-> () noexcept
        { return ptr_; }

        leaf_node_t * as_leaf () noexcept;
        interior_node_t * as_interior () noexcept;

    private:
        mutable_node_ptr (node_ptr & np, node_t * ptr) noexcept :
            node_ptr_ (np), ptr_ (ptr)
        {}

        node_ptr & node_ptr_;
        node_t * ptr_;
        friend node_ptr;
    };

    struct node_ptr
    {
        node_ptr () noexcept {}
        explicit node_ptr (node_t const * node) noexcept : ptr_ (node) {}

        explicit operator bool () const noexcept
        { return ptr_.get(); }

        node_t const * operator-> () const noexcept
        { return ptr_.get(); }

        leaf_node_t const * as_leaf () const noexcept;
        interior_node_t const * as_interior () const noexcept;

        node_t const * get () const noexcept
        { return ptr_.get(); }

        mutable_node_ptr write () const;

        void swap (node_ptr & rhs) noexcept
        { ptr_.swap(rhs.ptr_); }

        bool operator== (node_ptr const & rhs) const noexcept
        { return ptr_ == rhs.ptr_; }

    private:
        intrusive_ptr<node_t const> ptr_;
        friend mutable_node_ptr;
    };

    struct reference
    {
        reference (node_ptr const & text_node, text_view ref) noexcept;

        node_ptr text_;
        text_view ref_;
    };

    constexpr int node_buf_size () noexcept
    {
        return
            max_(alignof(text),
                 max_(alignof(text_view),
                      max_(alignof(repeated_text_view),
                           alignof(reference))))
            +
            max_(sizeof(text),
                 max_(sizeof(text_view),
                      max_(sizeof(repeated_text_view),
                           sizeof(reference))))
            ;
    }

    template <typename T>
    void * placement_address (void * buf, std::size_t buf_size) noexcept
    {
        std::size_t const alignment = alignof(T);
        std::size_t const size = sizeof(T);
        return alignment::align(alignment, size, buf, buf_size);
    }

    struct node_t
    {
        enum class which : char { t, tv, rtv, ref };

        explicit node_t (bool leaf) noexcept : refs_ (0), leaf_ (leaf) {}
        node_t (node_t const & rhs) noexcept : refs_ (0), leaf_ (rhs.leaf_) {}
        node_t & operator= (node_t const & rhs) = delete;

#ifdef BOOST_TEXT_THREAD_UNSAFE
        mutable int refs_;
#else
        mutable atomic<int> refs_;
#endif
        bool leaf_;
    };

    constexpr int min_children = 8;
    constexpr int max_children = 16;
    constexpr int text_insert_max = 512;

    inline std::ptrdiff_t size (node_t const * node) noexcept;

    using keys_t = container::static_vector<std::ptrdiff_t, max_children>;
    using children_t = container::static_vector<node_ptr, max_children>;

    static_assert(sizeof(std::ptrdiff_t) * 8 <= 64, "");
    static_assert(sizeof(node_ptr) * 8 <= 64, "");

    struct interior_node_t : node_t
    {
        interior_node_t () noexcept : node_t (false) {}

        void * operator new (std::size_t) = delete;

        alignas(64) keys_t keys_;
        children_t children_;
    };

    inline interior_node_t * new_interior_node ()
    {
        void * ptr =
            alignment::aligned_alloc(alignof(interior_node_t), sizeof(interior_node_t));
        return ::new (ptr) interior_node_t;
    }

    inline interior_node_t * new_interior_node (interior_node_t const & other)
    {
        void * ptr =
            alignment::aligned_alloc(alignof(interior_node_t), sizeof(interior_node_t));
        return ::new (ptr) interior_node_t(other);
    }

    struct leaf_node_t : node_t
    {
        leaf_node_t () noexcept : leaf_node_t (text_view()) {}

        leaf_node_t (text const & t) :
            node_t (true),
            buf_ptr_ (nullptr),
            which_ (which::t)
        {
            auto at = placement_address<text>(buf_, sizeof(buf_));
            assert(at);
            buf_ptr_ = new (at) text(t);
        }

        leaf_node_t (text && t) noexcept :
            node_t (true),
            buf_ptr_ (nullptr),
            which_ (which::t)
        {
            auto at = placement_address<text>(buf_, sizeof(buf_));
            assert(at);
            buf_ptr_ = new (at) text(std::move(t));
        }

        leaf_node_t (text_view tv) noexcept :
            node_t (true),
            buf_ptr_ (nullptr),
            which_ (which::tv)
        {
            auto at = placement_address<text_view>(buf_, sizeof(buf_));
            assert(at);
            buf_ptr_ = new (at) text_view(tv);
        }

        leaf_node_t (repeated_text_view rtv) noexcept :
            node_t (true),
            buf_ptr_ (nullptr),
            which_ (which::rtv)
        {
            auto at = placement_address<repeated_text_view>(buf_, sizeof(buf_));
            assert(at);
            buf_ptr_ = new (at) repeated_text_view(rtv);
        }

        leaf_node_t (leaf_node_t const & rhs) :
            node_t (true),
            buf_ptr_ (rhs.buf_ptr_),
            which_ (rhs.which_)
        {
            switch (which_) {
            case which::t: {
                auto at = placement_address<text>(buf_, sizeof(buf_));
                assert(at);
                buf_ptr_ = new (at) text(rhs.as_text());
                break;
            }
            case which::tv: {
                auto at = placement_address<text_view>(buf_, sizeof(buf_));
                assert(at);
                buf_ptr_ = new (at) text_view(rhs.as_text_view());
                break;
            }
            case which::rtv: {
                auto at = placement_address<repeated_text_view>(buf_, sizeof(buf_));
                assert(at);
                buf_ptr_ = new (at) repeated_text_view(rhs.as_repeated_text_view());
                break;
            }
            case which::ref: {
                auto at = placement_address<reference>(buf_, sizeof(buf_));
                assert(at);
                buf_ptr_ = new (at) reference(rhs.as_reference());
                break;
            }
            default: assert(!"unhandled rope node case"); break;
            }
        }

        leaf_node_t & operator= (leaf_node_t const &) = delete;
        leaf_node_t (leaf_node_t &&) = delete;
        leaf_node_t & operator= (leaf_node_t &&) = delete;

        ~leaf_node_t () noexcept
        {
            if (!buf_ptr_)
                return;

            switch (which_) {
            case which::t: as_text().~text(); break;
            case which::tv: as_text_view().~text_view (); break;
            case which::rtv: as_repeated_text_view().~repeated_text_view(); break;
            case which::ref: as_reference().~reference(); break;
            default: assert(!"unhandled rope node case"); break;
            }
        }

        int size () const noexcept
        {
            switch (which_) {
            case which::t: return as_text().size(); break;
            case which::tv: return as_text_view().size(); break;
            case which::rtv: return as_repeated_text_view().size(); break;
            case which::ref: return as_reference().ref_.size(); break;
            default: assert(!"unhandled rope node case"); break;
            }
            return -(1 << 30); // This should never execute.
        }

        text const & as_text () const noexcept
        {
            assert(which_ == which::t);
            return *static_cast<text *>(buf_ptr_);
        }

        text_view const & as_text_view () const noexcept
        {
            assert(which_ == which::tv);
            return *static_cast<text_view *>(buf_ptr_);
        }

        repeated_text_view const & as_repeated_text_view () const noexcept
        {
            assert(which_ == which::rtv);
            return *static_cast<repeated_text_view *>(buf_ptr_);
        }

        reference const & as_reference () const noexcept
        {
            assert(which_ == which::ref);
            return *static_cast<reference *>(buf_ptr_);
        }

        text & as_text () noexcept
        {
            assert(which_ == which::t);
            return *static_cast<text *>(buf_ptr_);
        }

        text_view & as_text_view () noexcept
        {
            assert(which_ == which::tv);
            return *static_cast<text_view *>(buf_ptr_);
        }

        repeated_text_view & as_repeated_text_view () noexcept
        {
            assert(which_ == which::rtv);
            return *static_cast<repeated_text_view *>(buf_ptr_);
        }

        reference & as_reference () noexcept
        {
            assert(which_ == which::ref);
            return *static_cast<reference *>(buf_ptr_);
        }

        char buf_[node_buf_size()];
        void * buf_ptr_;
        which which_;
    };

    inline mutable_node_ptr::~mutable_node_ptr () noexcept
    { node_ptr_.ptr_ = ptr_; }

    inline leaf_node_t * mutable_node_ptr::as_leaf () noexcept
    {
        assert(ptr_);
        assert(ptr_->leaf_);
        return static_cast<leaf_node_t *>(ptr_);
    }

    inline interior_node_t * mutable_node_ptr::as_interior () noexcept
    {
        assert(ptr_);
        assert(!ptr_->leaf_);
        return static_cast<interior_node_t *>(ptr_);
    }

    inline leaf_node_t const * node_ptr::as_leaf () const noexcept
    {
        assert(ptr_);
        assert(ptr_->leaf_);
        return static_cast<leaf_node_t const *>(ptr_.get());
    }

    inline interior_node_t const * node_ptr::as_interior () const noexcept
    {
        assert(ptr_);
        assert(!ptr_->leaf_);
        return static_cast<interior_node_t const *>(ptr_.get());
    }

    inline mutable_node_ptr node_ptr::write () const
    {
        auto & this_ref = const_cast<node_ptr &>(*this);
        if (ptr_->refs_ == 1)
            return mutable_node_ptr(this_ref, const_cast<node_t *>(ptr_.get()));
        if (ptr_->leaf_)
            return mutable_node_ptr(this_ref, new leaf_node_t(*as_leaf()));
        else
            return mutable_node_ptr(this_ref, new_interior_node(*as_interior()));
    }

#ifdef BOOST_TEXT_THREAD_UNSAFE

    inline void intrusive_ptr_add_ref (node_t const * node)
    { ++node->refs_; }
    inline void intrusive_ptr_release (node_t const * node)
    {
        if (!--node->refs_) {
            if (node->leaf_)
                delete static_cast<leaf_node_t const *>(node);
            else
                alignment::aligned_delete{}((interior_node_t *)(node));
        }
    }

#else

    // These functions were implemented following the "Reference counting"
    // example from Boost.Atomic.

    inline void intrusive_ptr_add_ref (node_t const * node)
    { node->refs_.fetch_add(1, boost::memory_order_relaxed); }

    inline void intrusive_ptr_release (node_t const * node)
    {
        if (node->refs_.fetch_sub(1, boost::memory_order_release) == 1) {
            boost::atomic_thread_fence(boost::memory_order_acquire);
            if (node->leaf_)
                delete static_cast<leaf_node_t const *>(node);
            else
                alignment::aligned_delete{}((interior_node_t *)(node));
        }
    }

#endif

    inline std::ptrdiff_t size (node_t const * node) noexcept
    {
        if (!node) {
            return 0;
        } else if (node->leaf_) {
            return static_cast<leaf_node_t const *>(node)->size();
        } else {
            auto int_node = static_cast<interior_node_t const *>(node);
            if (int_node->keys_.empty())
                return 0;
            return int_node->keys_.back();
        }
    }

    inline children_t const & children (node_ptr const & node) noexcept
    { return node.as_interior()->children_; }

    inline children_t & children (mutable_node_ptr & node) noexcept
    { return node.as_interior()->children_; }

    inline keys_t const & keys (node_ptr const & node) noexcept
    { return node.as_interior()->keys_; }

    inline keys_t & keys (mutable_node_ptr & node) noexcept
    { return node.as_interior()->keys_; }

    inline int num_children (node_ptr const & node) noexcept
    { return static_cast<int>(children(node).size()); }

    inline int num_children (mutable_node_ptr & node) noexcept
    { return static_cast<int>(children(node).size()); }

    inline int num_keys (node_ptr const & node) noexcept
    { return static_cast<int>(keys(node).size()); }

    inline int num_keys (mutable_node_ptr & node) noexcept
    { return static_cast<int>(keys(node).size()); }

    inline bool full (node_ptr const & node) noexcept
    { return num_children(node) == max_children; }

    inline bool almost_full (node_ptr const & node) noexcept
    { return num_children(node) == max_children - 1; }

    inline bool leaf_children (node_ptr const & node)
    { return children(node)[0]->leaf_; }

    inline std::ptrdiff_t offset (interior_node_t const * node, int i) noexcept
    {
        assert(0 <= i);
        assert(i <= (int)node->keys_.size());
        if (i == 0)
            return 0;
        return node->keys_[i - 1];
    }

    inline std::ptrdiff_t offset (node_ptr const & node, int i) noexcept
    { return offset(node.as_interior(), i); }

    inline std::ptrdiff_t offset (mutable_node_ptr const & node, int i) noexcept
    { return offset(const_cast<mutable_node_ptr &>(node).as_interior(), i); }

    inline std::ptrdiff_t find_child (interior_node_t const * node, std::ptrdiff_t n) noexcept
    {
        int i = 0;
        auto const sizes = static_cast<int>(node->keys_.size());
        while (i < sizes - 1 && node->keys_[i] <= n) {
            ++i;
        }
        assert(i < sizes);
        return i;
    }

    struct found_leaf
    {
        node_ptr const * leaf_;
        std::ptrdiff_t offset_;
        alignas(64) container::static_vector<interior_node_t const *, 24> path_;

        static_assert(sizeof(interior_node_t const *) * 8 <= 64, "");
    };

    inline void find_leaf (
        node_ptr const & node,
        std::ptrdiff_t n,
        found_leaf & retval
    ) noexcept {
        assert(node);
        assert(n <= size(node.get()));
        if (node->leaf_) {
            retval.leaf_ = &node;
            retval.offset_ = n;
            return;
        }
        retval.path_.push_back(node.as_interior());
        auto const i = find_child(node.as_interior(), n);
        node_ptr const & child = children(node)[i];
        auto const offset_ = offset(node, i);
        find_leaf(child, n - offset_, retval);
    }

    struct found_char
    {
        found_leaf leaf_;
        char c_;
    };

    inline void find_char (node_ptr const & node, std::ptrdiff_t n, found_char & retval) noexcept
    {
        assert(node);
        find_leaf(node, n, retval.leaf_);

        leaf_node_t const * leaf = retval.leaf_.leaf_->as_leaf(); // Heh.
        char c = '\0';
        switch (leaf->which_) {
        case node_t::which::t:
            c = *(leaf->as_text().cbegin() + retval.leaf_.offset_);
            break;
        case node_t::which::tv:
            c = *(leaf->as_text_view().begin() + retval.leaf_.offset_);
            break;
        case node_t::which::rtv:
            c = *(leaf->as_repeated_text_view().begin() + retval.leaf_.offset_);
            break;
        case node_t::which::ref:
            c = *(leaf->as_reference().ref_.begin() + retval.leaf_.offset_);
            break;
        default: assert(!"unhandled rope node case"); break;
        }
        retval.c_ = c;
    }

    inline reference::reference (node_ptr const & text_node, text_view ref) noexcept :
        text_ (text_node),
        ref_ (ref)
    {
        assert(text_node);
        assert(text_node->leaf_);
        assert(text_node.as_leaf()->which_ == node_t::which::t);
    }

    enum encoding_note_t { check_encoding_breakage, encoding_breakage_ok };

    inline node_ptr make_node (text const & t)
    { return node_ptr(new leaf_node_t(t)); }

    inline node_ptr make_node (text && t)
    { return node_ptr(new leaf_node_t(std::move(t))); }

    inline node_ptr make_node (text_view tv)
    { return node_ptr(new leaf_node_t(tv)); }

    inline node_ptr make_node (repeated_text_view rtv)
    { return node_ptr(new leaf_node_t(rtv)); }

    inline node_ptr make_ref (
        leaf_node_t const * t,
        std::ptrdiff_t lo,
        std::ptrdiff_t hi,
        encoding_note_t encoding_note = check_encoding_breakage
    ) {
        assert(t->which_ == node_t::which::t);
        text_view const tv =
            encoding_note == encoding_breakage_ok ?
            text_view(t->as_text().begin() + lo, hi - lo, utf8::unchecked) :
            t->as_text()(lo, hi);

        leaf_node_t * leaf = nullptr;
        node_ptr retval(leaf = new leaf_node_t);
        leaf->which_ = node_t::which::ref;
        auto at = placement_address<reference>(leaf->buf_, sizeof(leaf->buf_));
        assert(at);
        leaf->buf_ptr_ = new (at) reference(node_ptr(t), tv);
        return retval;
    }

    inline node_ptr make_ref (
        reference const & t,
        std::ptrdiff_t lo,
        std::ptrdiff_t hi,
        encoding_note_t encoding_note = check_encoding_breakage
    ) {
        auto const offset = t.ref_.begin() - t.text_.as_leaf()->as_text().begin();
        node_ptr retval = make_ref(t.text_.as_leaf(), lo + offset, hi + offset, encoding_note);
        return retval;
    }

    template <typename Fn>
    void foreach_leaf (node_ptr const & root, Fn && f)
    {
        if (!root)
            return;

        std::ptrdiff_t offset = 0;
        while (true) {
            found_leaf found;
            find_leaf(root, offset, found);
            leaf_node_t const * leaf = found.leaf_->as_leaf();

            if (!f(leaf))
                break;

            if ((offset += size(leaf)) == size(root.get()))
                break;
        }
    }

    template <typename Iter>
    struct reversed_range
    {
        Iter first_;
        Iter last_;

        Iter begin () const noexcept { return first_; }
        Iter end () const noexcept { return last_; }
    };

    template <typename Container>
    reversed_range<typename Container::const_reverse_iterator>
    reverse (Container const & c) noexcept
    { return {c.rbegin(), c.rend()}; }

    inline void bump_keys (interior_node_t * node, int from, std::ptrdiff_t bump)
    {
        for (int i = from, size = (int)node->keys_.size(); i < size; ++i) {
            node->keys_[i] += bump;
        }
    }

    inline void insert_child (interior_node_t * node, int i, node_ptr && child) noexcept
    {
        auto const child_size = size(child.get());
        node->children_.insert(node->children_.begin() + i, std::move(child));
        node->keys_.insert(node->keys_.begin() + i, offset(node, i));
        bump_keys(node, i, child_size);
    }

    enum erasure_adjustments { adjust_keys, dont_adjust_keys };

    inline void erase_child (interior_node_t * node, int i, erasure_adjustments adj = adjust_keys) noexcept
    {
        auto const child_size = size(node->children_[i].get());
        node->children_.erase(node->children_.begin() + i);
        node->keys_.erase(node->keys_.begin() + i);
        if (adj == adjust_keys)
            bump_keys(node, i, -child_size);
    }

    inline node_ptr slice_leaf (
        node_ptr const & node,
        std::ptrdiff_t lo,
        std::ptrdiff_t hi,
        bool immutable,
        encoding_note_t encoding_note
    ) {
        assert(node);
        assert(0 <= lo && lo <= size(node.get()));
        assert(0 <= hi && hi <= size(node.get()));
        assert(lo < hi);

        bool const leaf_mutable = !immutable && node->refs_ == 1;

        switch (node.as_leaf()->which_) {
        case node_t::which::t:
            if (!leaf_mutable)
                return make_ref(node.as_leaf(), lo, hi, encoding_note);
            {
                auto mut_node = node.write();
                text & t = mut_node.as_leaf()->as_text();
                if (encoding_note == encoding_breakage_ok)
                    t = text_view(t.begin() + lo, hi - lo, utf8::unchecked);
                else
                    t = t(lo, hi);
            }
            return node;
        case node_t::which::tv: {
            text_view const old_tv = node.as_leaf()->as_text_view();
            text_view const new_tv =
                encoding_note == encoding_breakage_ok ?
                text_view(old_tv.begin() + lo, hi - lo, utf8::unchecked) :
                old_tv(lo, hi);

            if (!leaf_mutable)
                return make_node(new_tv);

            {
                auto mut_node = node.write();
                text_view & tv = mut_node.as_leaf()->as_text_view();
                tv = new_tv;
            }
            return node;
        }
        case node_t::which::rtv: {
            repeated_text_view const & crtv = node.as_leaf()->as_repeated_text_view();
            int const mod_lo = lo % crtv.view().size();
            int const mod_hi = hi % crtv.view().size();
            if (mod_lo != 0 || mod_hi != 0) {
                if (encoding_note == check_encoding_breakage) {
                    text_view const tv = crtv.view()(
                        (std::min)(mod_lo, mod_hi),
                        (std::max)(mod_lo, mod_hi)
                    );
                    (void)tv;
                }
                return make_node(text(crtv.begin() + lo, crtv.begin() + hi));
            } else {
                auto const count = (hi - lo) / crtv.view().size();
                if (!leaf_mutable)
                    return make_node(repeated_text_view(crtv.view(), count));
                auto mut_node = node.write();
                repeated_text_view & rtv = mut_node.as_leaf()->as_repeated_text_view();
                rtv = repeated_text_view(rtv.view(), count);
            }
            return node;
        }
        case node_t::which::ref: {
            if (!leaf_mutable)
                return make_ref(node.as_leaf()->as_reference(), lo, hi, encoding_note);
            {
                auto mut_node = node.write();
                reference & ref = mut_node.as_leaf()->as_reference();
                ref.ref_ =
                    encoding_note == encoding_breakage_ok ?
                    text_view(ref.ref_.begin() + lo, hi - lo, utf8::unchecked) :
                    ref.ref_(lo, hi);
            }
            return node;
        }
        default: assert(!"unhandled rope node case"); break;
        }
        return node_ptr(); // This should never execute.
    }

    struct leaf_slices
    {
        node_ptr slice;
        node_ptr other_slice;
    };

    inline leaf_slices erase_leaf (
        node_ptr & node,
        std::ptrdiff_t lo,
        std::ptrdiff_t hi,
        encoding_note_t encoding_note
    ) {
        assert(node);
        assert(0 <= lo && lo <= size(node.get()));
        assert(0 <= hi && hi <= size(node.get()));
        assert(lo < hi);

        bool const leaf_mutable = node.as_leaf()->refs_ == 1;
        auto const leaf_size = size(node.get());

        leaf_slices retval;

        if (lo == 0 && hi == leaf_size)
            return retval;

        if (leaf_mutable && node.as_leaf()->which_ == node_t::which::t) {
            {
                auto mut_node = node.write();
                text & t = mut_node.as_leaf()->as_text();
                if (encoding_note == encoding_breakage_ok)
                    t.erase(text_view(t.begin() + lo, hi - lo, utf8::unchecked));
                else
                    t.erase(t(lo, hi));
            }
            retval.slice = node;
            return retval;
        }

        if (hi != leaf_size)
            retval.other_slice = slice_leaf(node, hi, leaf_size, true, encoding_note);
        if (lo != 0)
            retval.slice = slice_leaf(node, 0, lo, false, encoding_note);

        if (!retval.slice)
            retval.slice.swap(retval.other_slice);

        return retval;
    }

    // Follows CLRS.
    inline node_ptr btree_split_child (node_ptr const & parent, int i)
    {
        assert(0 <= i && i < num_children(parent));
        assert(!full(parent));
        assert(full(children(parent)[i]) || almost_full(children(parent)[i]));

        interior_node_t * new_node = nullptr;
        node_ptr new_node_ptr(new_node = new_interior_node());

        assert(!leaf_children(parent));
        node_ptr const & child = children(parent)[i];

        {
            int const elements = min_children - (full(child) ? 0 : 1);
            new_node->children_.resize(elements);
            std::copy(
                children(child).begin() + min_children, children(child).end(),
                new_node->children_.begin()
            );
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
            children(mut_parent).insert(
                children(mut_parent).begin() + i + 1,
                new_node_ptr
            );
        }

        {
            auto mut_child = children(parent)[i].write();
            children(mut_child).resize(min_children);
            keys(mut_child).resize(min_children);
        }
        {
            auto mut_parent = parent.write();
            keys(mut_parent).insert(
                keys(mut_parent).begin() + i,
                offset(mut_parent, i) + size(children(parent)[i].get())
            );
        }

        return parent;
    }

    // Analogous to btree_split_child(), for leaf nodes.
    inline void btree_split_leaf (
        node_ptr const & parent,
        int i,
        std::ptrdiff_t at,
        encoding_note_t encoding_note
    ) {
        assert(0 <= i && i < num_children(parent));
        assert(0 <= at && at <= size(parent.get()));
        assert(!full(parent));

        node_ptr const & child = children(parent)[i];

        auto const child_size = child.as_leaf()->size();
        auto const offset_at_i = offset(parent, i);
        auto const cut = static_cast<int>(at - offset_at_i);

        if (cut == 0 || cut == child_size)
            return;

        node_ptr right = slice_leaf(child, cut, child_size, true, encoding_note);
        node_ptr left = slice_leaf(
            child,
            0,
            cut,
            child.as_leaf()->which_ == node_t::which::t,
            encoding_note
        );

        auto mut_parent = parent.write();
        children(mut_parent)[i] = left;
        children(mut_parent).insert(
            children(mut_parent).begin() + i + 1,
            right
        );
        keys(mut_parent).insert(
            keys(mut_parent).begin() + i,
            offset_at_i + cut
        );
    }

    // Follows CLRS.
    inline node_ptr btree_insert_nonfull (
        node_ptr & parent,
        std::ptrdiff_t at,
        node_ptr && node,
        encoding_note_t encoding_note
    ) {
        assert(!parent->leaf_);
        assert(0 <= at && at <= size(parent.get()));
        assert(node->leaf_);

        int i = find_child(parent.as_interior(), at);
        if (leaf_children(parent)) {
            // Note that this split may add a node to parent, for a
            // maximum of two added nodes in the leaf code path.
            btree_split_leaf(parent, i, at, encoding_note);
            if (keys(parent)[i] <= at)
                ++i;

            auto mut_parent = parent.write();
            insert_child(mut_parent.as_interior(), i, std::move(node));
        } else {
            {
                node_ptr const & child = children(parent)[i];
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
            node_ptr new_child = btree_insert_nonfull(
                children(mut_parent)[i],
                at - offset(mut_parent, i),
                std::move(node),
                encoding_note
            );
            delta += size(new_child.get());
            children(mut_parent)[i] = new_child;
            for (int j = i, size = num_keys(mut_parent); j < size; ++j) {
                keys(mut_parent)[j] += delta;
            }
        }

        return parent;
    }

    // Follows CLRS.
    inline node_ptr btree_insert (
        node_ptr & root,
        std::ptrdiff_t at,
        node_ptr && node,
        encoding_note_t encoding_note = check_encoding_breakage
    ) {
        assert(0 <= at && at <= size(root.get()));
        assert(node->leaf_);

        if (!root) {
            return node;
        } else if (root->leaf_) {
            interior_node_t * new_root = nullptr;
            node_ptr new_root_ptr(new_root = new_interior_node());
            auto const root_size = size(root.get());
            new_root->children_.push_back(std::move(root));
            new_root->keys_.push_back(root_size);
            return btree_insert_nonfull(new_root_ptr, at, std::move(node), encoding_note);
        } else if (full(root) || (leaf_children(root) && almost_full(root))) {
            interior_node_t * new_root = nullptr;
            node_ptr new_root_ptr(new_root = new_interior_node());
            auto const root_size = size(root.get());
            new_root->children_.push_back(std::move(root));
            new_root->keys_.push_back(root_size);
            new_root_ptr = btree_split_child(new_root_ptr, 0);
            return btree_insert_nonfull(new_root_ptr, at, std::move(node), encoding_note);
        } else {
            return btree_insert_nonfull(root, at, std::move(node), encoding_note);
        }
    }

    // Recursing top-to-bottom, pull nodes down the tree as necessary to
    // ensure that each node has min_children + 1 nodes in it *before*
    // recursing into it.  This enables the erasure to happen in a single
    // downward pass, with no backtracking.  This function only erases
    // entire segments; the segments must have been split appropriately
    // before this function is ever called.
    inline node_ptr btree_erase (
        node_ptr const & node,
        std::ptrdiff_t at,
        leaf_node_t const * leaf,
        encoding_note_t encoding_note
    ) {
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

        node_ptr new_child;

        node_ptr const & child = children(node)[child_index];
        if (num_children(child) == min_children) {
            assert(child_index != 0 || child_index != num_children(node) - 1);

            if (child_index != 0 &&
                min_children + 1 <= num_children(children(node)[child_index - 1])) {
                node_ptr const & child_left_sib = children(node)[child_index - 1];

                // Remove last element of left sibling.
                node_ptr moved_node = children(child_left_sib).back();
                auto const moved_node_size = size(moved_node.get());

                {
                    auto mut_left = child_left_sib.write();
                    erase_child(mut_left.as_interior(), num_children(mut_left) - 1);
                }

                // Prepend last element onto child; now child has min_children
                // + 1 children, and we can recurse.
                {
                    auto mut_child = child.write();
                    insert_child(mut_child.as_interior(), 0, std::move(moved_node));
                }

                std::ptrdiff_t const offset_ = offset(node, child_index);
                new_child = btree_erase(child, at - offset_ + moved_node_size, leaf, encoding_note);
            } else if (child_index != num_children(node) - 1 &&
                       min_children + 1 <= num_children(children(node)[child_index + 1])) {
                node_ptr const & child_right_sib = children(node)[child_index + 1];

                // Remove first element of right sibling.
                node_ptr moved_node = children(child_right_sib).front();

                {
                    auto mut_right = child_right_sib.write();
                    erase_child(mut_right.as_interior(), 0);
                }

                // Append first element onto child; now child has min_children
                // + 1 children, and we can recurse.
                {
                    auto mut_child = child.write();
                    insert_child(mut_child.as_interior(), num_children(mut_child), std::move(moved_node));
                }

                std::ptrdiff_t const offset_ = offset(node, child_index);
                new_child = btree_erase(child, at - offset_, leaf, encoding_note);
            } else {
                auto const right_index = child_index == 0 ? child_index + 1 : child_index;
                auto const left_index = right_index - 1;

                node_ptr const & left = children(node)[left_index];
                node_ptr const & right = children(node)[right_index];

                {
                    auto mut_left = left.write();
                    auto mut_right = right.write();

                    children_t & left_children = children(mut_left);
                    children_t & right_children = children(mut_right);

                    left_children.insert(
                        left_children.end(),
                        right_children.begin(),
                        right_children.end()
                    );

                    keys_t & left_keys = keys(mut_left);
                    keys_t & right_keys = keys(mut_right);

                    auto const old_left_size = left_keys.back();
                    int const old_children = num_keys(mut_left);

                    left_keys.insert(
                        left_keys.end(),
                        right_keys.begin(),
                        right_keys.end()
                    );
                    for (int i = old_children, size = num_keys(mut_left); i < size; ++i) {
                        left_keys[i] += old_left_size;
                    }
                }

                std::ptrdiff_t const offset_ = offset(node, left_index);
                new_child = btree_erase(left, at - offset_, leaf, encoding_note);

                // This can only happen if node is the root.
                if (num_children(node) == 2)
                    return new_child;

                auto mut_node = node.write();
                erase_child(mut_node.as_interior(), right_index, dont_adjust_keys);

                if (right_index <= child_index)
                    --child_index;
            }
        } else {
            std::ptrdiff_t const offset_ = offset(node, child_index);
            new_child = btree_erase(children(node)[child_index], at - offset_, leaf, encoding_note);
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

    inline node_ptr btree_erase (
        node_ptr & root,
        std::ptrdiff_t lo,
        std::ptrdiff_t hi,
        encoding_note_t encoding_note = check_encoding_breakage
    ) {
        assert(root);
        assert(0 <= lo && lo <= size(root.get()));
        assert(0 <= hi && hi <= size(root.get()));
        assert(lo < hi);

        assert(root);

        if (lo == 0 && hi == size(root.get())) {
            return node_ptr();
        } else if (root->leaf_) {
            leaf_slices slices;
            slices = erase_leaf(root, lo, hi, encoding_note);
            if (!slices.other_slice) {
                return slices.slice;
            } else {
                interior_node_t * new_root = nullptr;
                node_ptr new_root_ptr(new_root = new_interior_node());
                new_root->keys_.push_back(size(slices.slice.get()));
                new_root->keys_.push_back(new_root->keys_[0] + size(slices.other_slice.get()));
                new_root->children_.push_back(std::move(slices.slice));
                new_root->children_.push_back(std::move(slices.other_slice));
                return new_root_ptr;
            }
        } else {
            auto const final_size = size(root.get()) - (hi - lo);

            // Right after the hi-segment, insert the suffix of the
            // hi-segment that's not being erased (if there is one).
            detail::found_leaf found_hi;
            detail::find_leaf(root, hi, found_hi);
            auto const hi_leaf_size = size(found_hi.leaf_->get());
            if (found_hi.offset_ != 0 && found_hi.offset_ != hi_leaf_size) {
                node_ptr suffix =
                    slice_leaf(*found_hi.leaf_, found_hi.offset_, hi_leaf_size, true, encoding_note);
                auto const suffix_at = hi - found_hi.offset_ + hi_leaf_size;
                root = btree_insert(root, suffix_at, std::move(suffix), encoding_note);
                detail::find_leaf(root, suffix_at, found_hi);
            }

            // Right before the lo-segment, insert the prefix of the
            // lo-segment that's not being erased (if there is one).
            detail::found_leaf found_lo;
            detail::find_leaf(root, lo, found_lo);
            if (found_lo.offset_ != 0) {
                auto const lo_leaf_size = size(found_lo.leaf_->get());
                node_ptr prefix =
                    slice_leaf(*found_lo.leaf_, 0, found_lo.offset_, true, encoding_note);
                if (prefix.get() == found_lo.leaf_->get())
                    hi -= lo_leaf_size;
                root = btree_insert(root, lo - found_lo.offset_, std::move(prefix), encoding_note);
                detail::find_leaf(root, lo, found_lo);
            }

            assert(found_lo.offset_ == 0);
            assert(found_hi.offset_ == 0 || found_hi.offset_ == hi_leaf_size);

            leaf_node_t const * leaf_lo = found_lo.leaf_->as_leaf();
            while (true) {
                root = btree_erase(root, lo, leaf_lo, encoding_note);
                if (size(root.get()) == final_size)
                    break;
                found_leaf found;
                find_leaf(root, lo, found);
                leaf_lo = found.leaf_->as_leaf();
            }
        }

        return root;
    }

    struct segment_inserter
    {
        template <typename Segment>
        void operator() (Segment const & s) const
        {
            if (os_.good())
                os_ << s;
        }

        std::ostream & os_;
    };

    template <typename Segment>
    bool encoded (Segment const & segment)
    { return utf8::encoded(segment.begin(), segment.end()); }

    inline bool encoded (repeated_text_view rtv)
    { return utf8::encoded(rtv.view().begin(), rtv.view().end()); }

    struct segment_encoding_checker
    {
        template <typename Segment>
        void operator() (Segment const & s) const
        {
            if (!encoded(s))
                throw std::invalid_argument("Invalid UTF-8 encoding");
        }
    };

    struct repeated_range
    {
        repeated_text_view::const_iterator first, last;
        repeated_text_view::const_iterator begin () const { return first; }
        repeated_text_view::const_iterator end () const { return last; }
    };

    inline std::ostream & operator<< (std::ostream & os, repeated_range rr)
    {
        for (char c : rr) {
            os << c;
        }
        return os;
    }

} } }

#endif
