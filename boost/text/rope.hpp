#ifndef BOOST_TEXT_ROPE_HPP
#define BOOST_TEXT_ROPE_HPP

#include <boost/text/text_view.hpp>
#include <boost/text/text.hpp>

#include <boost/container/static_vector.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>

#include <vector>


namespace boost { namespace text {

    namespace detail {

        // TODO: const_cast audit.

        struct node_t;
        struct leaf_node_t;
        struct interior_node_t;
        struct node_ptr;

        struct node_ptr
        {
            node_ptr () {}
            node_ptr (node_t * node) : ptr_ (node) {}

            explicit operator bool () const
            { return ptr_.get(); }

            node_t const * operator-> () const
            { return ptr_.get(); }

            leaf_node_t const * as_leaf () const;

            interior_node_t const * as_interior () const;

            node_ptr clone ();
            node_ptr write ();

            void swap (node_ptr & rhs)
            { ptr_.swap(rhs.ptr_); }

        private:
            boost::intrusive_ptr<node_t> ptr_;
        };

        struct reference
        {
            reference (node_ptr text_node, text_view ref) noexcept;

            node_ptr text_;
            text_view ref_;
        };

        constexpr int node_buf_size () noexcept
        {
            int alignment = alignof(text);
            alignment = alignment < alignof(text_view) ? alignof(text_view) : alignment;
            alignment = alignment < alignof(repeated_text_view) ? alignof(repeated_text_view) : alignment;
            alignment = alignment < alignof(reference) ? alignof(reference) : alignment;
            int size = sizeof(text);
            size = size < sizeof(text_view) ? alignof(text_view) : size;
            size = size < sizeof(repeated_text_view) ? alignof(repeated_text_view) : size;
            size = size < sizeof(reference) ? alignof(reference) : size;
            return alignment + size;
        }

        template <typename T>
        constexpr void * placement_address (void * buf, std::size_t buf_size) noexcept
        {
            std::size_t const alignment = alignof(T);
            std::size_t const size = sizeof(T);
            return std::align(alignment, size, buf, buf_size);
        }

        struct node_t
        {
            enum class which : char { t, tv, rtv, ref };

            explicit node_t (bool leaf) : refs_ (0), leaf_ (leaf) {}
            node_t (node_t const & rhs) : refs_ (0), leaf_ (rhs.leaf_) {}
            node_t & operator= (node_t const & rhs) = delete;

            int refs_;
            bool leaf_;
        };

        constexpr int min_children = 4;
        constexpr int max_children = 8;
        constexpr int text_insert_max = 512;

        struct interior_node_t : node_t
        {
            interior_node_t () : node_t (false) {}

            bool full () const
            { return children_.size() == max_children; }

            // TODO: experiment with alignas(64)
            boost::container::static_vector<std::ptrdiff_t, max_children> child_sizes_;
            boost::container::static_vector<node_ptr, max_children> children_;
        };

        struct leaf_node_t : node_t
        {
            leaf_node_t () :
                node_t (false),
                buf_ptr_ (nullptr),
                prev_ (nullptr),
                next_ (nullptr),
                size_ (0)
            {}

            ~leaf_node_t ()
            {
                if (!buf_ptr_)
                    return;

                switch (which_) {
                case which::t: static_cast<text *>(buf_ptr_)->~text(); break;
                case which::tv: static_cast<text_view *>(buf_ptr_)->~text_view (); break;
                case which::rtv: static_cast<repeated_text_view *>(buf_ptr_)->~repeated_text_view(); break;
                case which::ref: static_cast<reference *>(buf_ptr_)->~reference(); break;
                default: assert(!"unhandled rope node case"); break;
                }
            }

            char buf_[node_buf_size()];
            void * buf_ptr_;
            leaf_node_t * prev_; // TODO
            leaf_node_t * next_;
            int size_;
            which which_;
        };

        leaf_node_t const * node_ptr::as_leaf () const
        {
            assert(ptr_);
            assert(ptr_->leaf_);
            return static_cast<leaf_node_t const *>(ptr_.get());
        }

        interior_node_t const * node_ptr::as_interior () const
        {
            assert(ptr_);
            assert(!ptr_->leaf_);
            return static_cast<interior_node_t const *>(ptr_.get());
        }

        node_ptr node_ptr::clone ()
        {
            if (ptr_->leaf_)
                return node_ptr(new leaf_node_t(*as_leaf()));
            else
                return node_ptr(new interior_node_t(*as_interior()));
        }

        node_ptr node_ptr::write ()
        {
            if (ptr_->refs_ == 1)
                return *this;
            return clone();
        }

        inline void intrusive_ptr_add_ref (node_t * node)
        { ++node->refs_; }
        inline void intrusive_ptr_release (node_t * node)
        {
            if (!--node->refs_) {
                if (node->leaf_)
                    delete static_cast<leaf_node_t *>(node);
                else
                    delete static_cast<interior_node_t *>(node);
            }
        }

        inline std::ptrdiff_t size (node_ptr node)
        {
            if (node->leaf_)
                return node.as_leaf()->size_;
            else
                return node.as_interior()->child_sizes_.back();
        }

        struct found_leaf
        {
            boost::container::static_vector<interior_node_t *, 24> path_;
            leaf_node_t * leaf_;
            std::ptrdiff_t offset_;
        };

        inline std::ptrdiff_t find_child (interior_node_t const * node, std::ptrdiff_t n)
        {
            int i = 0;
            int const sizes = node->child_sizes_.size();
            while (i < sizes && node->child_sizes_[i] <= n) {
                ++i;
            }
            assert(i < sizes);
            return i;
        }

        inline void find_leaf (node_ptr node, std::ptrdiff_t n, found_leaf & retval)
        {
            if (node->leaf_) {
                retval.leaf_ = const_cast<leaf_node_t *>(node.as_leaf());
                retval.offset_ = n;
                return;
            }

            interior_node_t const * int_node = node.as_interior();

            int const i = find_child(int_node, n);

            node_ptr next_node = int_node->children_[i];
            if (0 < i)
                n -= int_node->child_sizes_[i - 1];

            retval.path_.push_back(const_cast<interior_node_t *>(int_node));

            find_leaf(next_node, n, retval);
        }

        struct found_char
        {
            found_leaf leaf_;
            char c_;
        };

        inline void find_char (node_ptr node, std::ptrdiff_t n, found_char & retval)
        {
            assert(node);
            find_leaf(node, n, retval.leaf_);

            leaf_node_t * leaf = retval.leaf_.leaf_;
            char c = '\0';
            switch (leaf->which_) {
            case node_t::which::t:
                c = *(static_cast<text *>(leaf->buf_ptr_)->cbegin() + n);
                break;
            case node_t::which::tv:
                c = *(static_cast<text_view *>(leaf->buf_ptr_)->begin() + n);
                break;
            case node_t::which::rtv:
                c = *(static_cast<repeated_text_view *>(leaf->buf_ptr_)->begin() + n);
                break;
            case node_t::which::ref:
                c = *(static_cast<reference *>(leaf->buf_ptr_)->ref_.begin() + n);
                break;
            default: assert(!"unhandled rope node case"); break;
            }
            retval.c_ = c;
        }

        inline reference::reference (node_ptr text_node, text_view ref) noexcept :
            text_ (text_node),
            ref_ (ref)
        {
            assert(text_node);
            assert(text_node->leaf_);
            assert(text_node.as_leaf()->which_ == node_t::which::t);
        }

        inline node_ptr make_node (text const & t)
        {
            leaf_node_t * leaf = nullptr;
            node_ptr retval(leaf = new leaf_node_t);
            leaf->size_ = t.size();
            leaf->which_ = node_t::which::t;
            auto at = placement_address<text>(leaf->buf_, sizeof(leaf->buf_));
            assert(at);
            leaf->buf_ptr_ = new (at) text(t);
            return retval;
        }

        inline node_ptr make_node (text && t)
        {
            leaf_node_t * leaf = nullptr;
            node_ptr retval(leaf = new leaf_node_t);
            leaf->size_ = t.size();
            leaf->which_ = node_t::which::t;
            auto at = placement_address<text>(leaf->buf_, sizeof(leaf->buf_));
            assert(at);
            leaf->buf_ptr_ = new (at) text(std::move(t));
            return retval;
        }

        inline node_ptr make_node (text_view tv)
        {
            leaf_node_t * leaf = nullptr;
            node_ptr retval(leaf = new leaf_node_t);
            leaf->size_ = tv.size();
            leaf->which_ = node_t::which::tv;
            auto at = placement_address<text_view>(leaf->buf_, sizeof(leaf->buf_));
            assert(at);
            leaf->buf_ptr_ = new (at) text_view(tv);
            return retval;
        }

        inline node_ptr make_node (repeated_text_view rtv)
        {
            leaf_node_t * leaf = nullptr;
            node_ptr retval(leaf = new leaf_node_t);
            leaf->size_ = rtv.size();
            leaf->which_ = node_t::which::rtv;
            auto at = placement_address<repeated_text_view>(leaf->buf_, sizeof(leaf->buf_));
            assert(at);
            leaf->buf_ptr_ = new (at) repeated_text_view(rtv);
            return retval;
        }

        inline node_ptr make_ref (leaf_node_t * t, std::ptrdiff_t lo, std::ptrdiff_t hi)
        {
            assert(t->which_ == node_t::which::t);
            text_view const tv = (*static_cast<text *>(t->buf_ptr_))(lo, hi);

            leaf_node_t * leaf = nullptr;
            node_ptr retval(leaf = new leaf_node_t);
            leaf->size_ = tv.size();
            leaf->which_ = node_t::which::ref;
            auto at = placement_address<reference>(leaf->buf_, sizeof(leaf->buf_));
            assert(at);
            leaf->buf_ptr_ = new (at) reference(t, tv);
            return retval;
        }

        inline node_ptr make_ref (reference * t, std::ptrdiff_t lo, std::ptrdiff_t hi)
        {
            leaf_node_t * t_leaf = const_cast<leaf_node_t *>(t->text_.as_leaf());
            int const offset = t->ref_.begin() - static_cast<text *>(t_leaf->buf_ptr_)->begin();
            return make_ref(t_leaf, lo + offset, hi + offset);
        }

        inline node_ptr slice_leaf (leaf_node_t * node, std::ptrdiff_t lo, std::ptrdiff_t hi)
        {
            assert(lo <= hi);

            if (lo == 0 && hi == node->size_)
                return node;

            switch (node->which_) {
            case node_t::which::t:
                return make_ref(node, lo, hi);
            case node_t::which::tv:
                return make_node((*static_cast<text_view *>(node->buf_ptr_))(lo, hi));
            case node_t::which::rtv: {
                repeated_text_view const & rtv =
                    *static_cast<repeated_text_view *>(node->buf_ptr_);
                std::ptrdiff_t const lo_segment = lo / rtv.view().size();
                std::ptrdiff_t const hi_segment = hi / rtv.view().size();
                if (lo_segment == hi_segment)
                    return make_node(rtv.view()(lo % rtv.view().size(), hi % rtv.view().size()));
                else
                    return make_node(text(rtv.begin() + lo, rtv.begin() + hi));
                break;
            }
            case node_t::which::ref:
                return make_ref(static_cast<reference *>(node->buf_ptr_), lo, hi);
            default: assert(!"unhandled rope node case"); break;
            }
            return nullptr; // This should never execute.
        }

        struct const_rope_iterator;
        struct const_reverse_rope_iterator;

        template <typename Container>
        auto reverse (Container const & c)
        {
            return boost::iterator_range<typename Container::const_reverse_iterator>(
                c.rbegin(), c.rend()
            );
        }

        inline node_ptr btree_split_child (node_ptr parent, int i)
        {
            assert(parent.as_interior()->full());

            interior_node_t * new_node = nullptr;
            node_ptr new_node_ptr(new_node = new interior_node_t);

            assert(!parent.as_interior()->children_[i]->leaf_);
            node_ptr child = parent.as_interior()->children_[i];

            {
                interior_node_t const * int_child = child.as_interior();
                new_node->children_.resize(min_children - 1);
                std::copy(
                    int_child->children_.begin() + min_children, int_child->children_.end(),
                    new_node->children_.begin()
                );
                new_node->child_sizes_.resize(min_children - 1);
                std::copy(
                    int_child->child_sizes_.begin() + min_children, int_child->child_sizes_.end(),
                    new_node->child_sizes_.begin()
                );
            }

            // .write() clones the node only if refs_ > 1
            node_ptr new_parent = parent.write();
            parent = new_parent;
            child = parent.as_interior()->children_[i];
            node_ptr new_child = child.write();
            child = new_child;

            {
                interior_node_t * int_parent = const_cast<interior_node_t *>(child.as_interior());
                interior_node_t * int_child = const_cast<interior_node_t *>(child.as_interior());
                int_parent->children_.insert(int_parent->children_.begin() + i + 1, new_node);
                int_parent->child_sizes_.insert(
                    int_parent->child_sizes_.begin() + i,
                    int_child->child_sizes_[min_children]
                );
                int_child->children_.resize(min_children);
            }

            return parent;
        }

        inline node_ptr btree_insert_nonfull (
            node_ptr parent,
            std::ptrdiff_t at,
            node_ptr node
        ) {
            node_ptr new_parent = parent.write();
            parent = new_parent;

            int i = find_child(parent.as_interior(), at);
            if (parent.as_interior()->children_[i]->leaf_) {
                interior_node_t * int_parent =
                    const_cast<interior_node_t *>(parent.as_interior());
                int_parent->children_.insert(
                    int_parent->children_.begin() + i,
                    node
                );
                std::ptrdiff_t size = node.as_leaf()->size_;
                auto it = int_parent->child_sizes_.begin() + i;
                auto const end = int_parent->child_sizes_.end();
                if (0 < i)
                    size += *(it - 1);
                int_parent->child_sizes_.insert(it++, size);
                std::ptrdiff_t delta = size - *it;
                while (it != end) {
                    *it++ += delta;
                }
            } else {
                if (parent.as_interior()->children_[i].as_interior()->full()) {
                    parent = btree_split_child(parent, i);
                    if (parent.as_interior()->child_sizes_[i] < at)
                        ++i;
                }
                interior_node_t * int_parent =
                    const_cast<interior_node_t *>(parent.as_interior());
                int_parent->children_[i] =
                    btree_insert_nonfull(int_parent->children_[i], at, node);
            }
            return parent;
        }

        inline node_ptr btree_insert (node_ptr root, std::ptrdiff_t at, node_ptr node)
        {
            assert(node->leaf_);
            if (root.as_interior()->full()) {
                interior_node_t * new_root = nullptr;
                node_ptr new_root_ptr(new_root = new interior_node_t);
                new_root->children_.push_back(root);
                new_root->child_sizes_.push_back(root.as_interior()->child_sizes_.back());
                btree_split_child(new_root_ptr, 0);
                return btree_insert_nonfull(new_root_ptr, at, node);
            } else {
                return btree_insert_nonfull(root, at, node);
            }
        }

    }

    struct rope
    {
        using iterator = detail::const_rope_iterator;
        using const_iterator = detail::const_rope_iterator;
        using reverse_iterator = detail::const_reverse_rope_iterator;
        using const_reverse_iterator = detail::const_reverse_rope_iterator;

        using size_type = std::ptrdiff_t;

        rope () : ptr_ (nullptr) {}

        rope (text const & t) : ptr_ (detail::make_node(t)) {}
        rope (text && t) : ptr_ (detail::make_node(std::move(t))) {}
        rope (text_view tv) : ptr_ (detail::make_node(tv)) {}
        rope (repeated_text_view rtv) : ptr_ (detail::make_node(rtv)) {}

        rope & operator= (text const & t)
        {
            rope temp(t);
            swap(temp);
            return *this;
        }

        rope & operator= (text && t)
        {
            rope temp(std::move(t));
            swap(temp);
            return *this;
        }

        rope & operator= (text_view tv)
        {
            rope temp(tv);
            swap(temp);
            return *this;
        }

        rope & operator= (repeated_text_view rtv)
        {
            rope temp(rtv);
            swap(temp);
            return *this;
        }

        const_iterator begin () const noexcept;
        const_iterator end () const noexcept;

        const_reverse_iterator rbegin () const noexcept;
        const_reverse_iterator rend () const noexcept;

        bool empty () const noexcept
        { return size() == 0; }

        size_type size () const noexcept
        { return detail::size(ptr_); }

        char operator[] (size_type n) const noexcept
        {
            assert(ptr_);
            assert(n < size());
            detail::found_char found;
            find_char(ptr_, n, found);
            return found.c_;
        }

        constexpr size_type max_size () const noexcept
        { return PTRDIFF_MAX; }

#if 0
        rope substr (size_type lo, size_type hi) const
        {
            assert(ptr_);
            assert(0 <= lo && lo <= size());
            assert(0 <= hi && hi <= size());
            assert(lo <= hi);

            // TODO:

            // Take an extra ref to the root, which will force all a clone of
            // all the nodes to teh insertion point.
            node_ptr extra_ref = ptr_;

            // TODO: insert slice at leftmost substr leaf.
            // TODO: Go back through nodes exterior to (left of) path to leaf
            // and remove them.
            // TODO: Same thing on the right.

            rope retval;
            retval.ptr_ = new_root;
            return retval;
        }

        rope substr (size_type cut) const
        {
            int lo = 0;
            int hi = cut;
            if (cut < 0) {
                lo = cut + size();
                hi = size();
            }
            assert(0 <= lo && lo <= size());
            assert(0 <= hi && hi <= size());
            return substr(lo, hi);
        }
#endif

        int compare (rope rhs) const noexcept;

        bool operator== (rope rhs) const noexcept
        { return compare(rhs) == 0; }

        bool operator!= (rope rhs) const noexcept
        { return compare(rhs) != 0; }

        bool operator< (rope rhs) const noexcept
        { return compare(rhs) < 0; }

        bool operator<= (rope rhs) const noexcept
        { return compare(rhs) <= 0; }

        bool operator> (rope rhs) const noexcept
        { return compare(rhs) > 0; }

        bool operator>= (rope rhs) const noexcept
        { return compare(rhs) >= 0; }

        friend const_iterator begin (rope const & r) noexcept;
        friend const_iterator end (rope const & r) noexcept;

        friend const_reverse_iterator rbegin (rope const & r) noexcept;
        friend const_reverse_iterator rend (rope const & r) noexcept;

        friend std::ostream & operator<< (std::ostream & os, rope const & r)
        {
            if (!r.ptr_)
                return os;

            detail::found_leaf found;
            detail::find_leaf(r.ptr_, 0, found);

            detail::leaf_node_t const * leaf = found.leaf_;
            while (leaf) {
                switch (leaf->which_) {
                case detail::node_t::which::t:
                    os << *static_cast<text *>(leaf->buf_ptr_);
                    break;
                case detail::node_t::which::tv:
                    os << *static_cast<text_view *>(leaf->buf_ptr_);
                    break;
                case detail::node_t::which::rtv:
                    os << *static_cast<repeated_text_view *>(leaf->buf_ptr_);
                    break;
                case detail::node_t::which::ref:
                    os << static_cast<detail::reference *>(leaf->buf_ptr_)->ref_;
                    break;
                default: assert(!"unhandled rope node case"); break;
                }
                leaf = leaf->next_;
            }

            return os;
        }

        void clear ()
        { ptr_ = nullptr; }

        rope & insert (size_type at, rope const & r)
        {
            if (r.empty())
                return *this;

            detail::found_leaf found;
            find_leaf(r.ptr_, 0, found);

            detail::leaf_node_t * leaf = found.leaf_;
            while (leaf) {
                ptr_ = detail::btree_insert(ptr_, at, leaf);
                leaf = leaf->next_;
            }

            return *this;
        }

        rope & insert (size_type at, text const & t)
        {
            if (t.empty())
                return *this;

            if (text * leaf_t = mutable_insertion_leaf(at, t.size(), true))
                leaf_t->insert(leaf_t->size(), t);
            else
                ptr_ = detail::btree_insert(ptr_, at, detail::make_node(t));

            return *this;
        }

        rope & insert (size_type at, text && t)
        {
            if (t.empty())
                return *this;

            if (text * leaf_t = mutable_insertion_leaf(at, t.size(), false))
                leaf_t->insert(leaf_t->size(), t);
            else
                ptr_ = detail::btree_insert(ptr_, at, detail::make_node(std::move(t)));

            return *this;
        }

        rope & insert (size_type at, text_view tv)
        {
            if (tv.empty())
                return *this;

            if (text * leaf_t = mutable_insertion_leaf(at, tv.size(), false))
                leaf_t->insert(leaf_t->size(), tv);
            else
                ptr_ = detail::btree_insert(ptr_, at, detail::make_node(tv));

            return *this;
        }

        rope & insert (size_type at, repeated_text_view rtv)
        {
            if (rtv.empty())
                return *this;

            if (text * leaf_t = mutable_insertion_leaf(at, rtv.size(), false))
                leaf_t->insert(leaf_t->size(), rtv);
            else
                ptr_ = detail::btree_insert(ptr_, at, detail::make_node(rtv));

            return *this;
        }

        void swap (rope & rhs)
        { ptr_.swap(rhs.ptr_); }

    private:
        text * mutable_insertion_leaf (size_type at, size_type size, bool insertion_would_allocate)
        {
            detail::found_leaf found;
            find_leaf(ptr_, at, found);

            for (auto node : found.path_) {
                if (1 < node->refs_)
                    return nullptr;
            }

            if (found.leaf_->which_ == detail::node_t::which::t) {
                text * t = static_cast<text *>(found.leaf_->buf_ptr_);
                if (t->capacity() - t->size() < size)
                    return t;
                else if (insertion_would_allocate && t->size() + size < detail::text_insert_max)
                    return t;
            }

            return nullptr;
        }

        detail::node_ptr ptr_;

        friend struct detail::const_rope_iterator;
    };

    namespace detail {

        struct const_rope_iterator
        {
            using value_type = char const;
            using difference_type = std::ptrdiff_t;
            using pointer = char const *;
            using reference = char const;
            using iterator_category = std::random_access_iterator_tag;

            const_rope_iterator () noexcept :
                rope_ (nullptr),
                n_ (-1),
                leaf_ (nullptr),
                leaf_start_ (-1)
            {}

            const_rope_iterator (rope const & r, difference_type n) noexcept :
                rope_ (&r),
                n_ (n),
                leaf_ (nullptr),
                leaf_start_ (0)
            {}

            reference operator* () const noexcept
            {
                if (leaf_) {
                    return deref();
                } else {
                    found_char found;
                    find_char(rope_->ptr_, n_, found);
                    leaf_ = found.leaf_.leaf_;
                    leaf_start_ = n_ - found.leaf_.offset_;
                    return found.c_;
                }
            }

            value_type operator[] (difference_type n) const noexcept
            {
                auto it = *this;
                if (0 <= n)
                    it += n;
                else
                    it -= -n;
                return *it;
            }

            const_rope_iterator & operator++ () noexcept
            {
                ++n_;
                if (leaf_ && leaf_start_ + n_ == leaf_->size_) {
                    leaf_ = leaf_->next_;
                    leaf_start_ = n_;
                }
                return *this;
            }
            const_rope_iterator operator++ (int) noexcept
            {
                const_rope_iterator retval = *this;
                ++*this;
                return retval;
            }
            const_rope_iterator & operator+= (difference_type n) noexcept
            {
                n_ += n;
                leaf_ = nullptr;
                return *this;
            }

            const_rope_iterator & operator-- () noexcept
            {
                if (leaf_ && n_ == leaf_start_) {
                    leaf_ = leaf_->prev_;
                    leaf_start_ -= leaf_->size_;
                }
                --n_;
                return *this;
            }
            const_rope_iterator operator-- (int) noexcept
            {
                const_rope_iterator retval = *this;
                --*this;
                return retval;
            }
            const_rope_iterator & operator-= (difference_type n) noexcept
            {
                n_ -= n;
                leaf_ = nullptr;
                return *this;
            }

            friend bool operator== (const_rope_iterator lhs, const_rope_iterator rhs) noexcept
            { return lhs.rope_ == rhs.rope_ && lhs.n_ == rhs.n_; }
            friend bool operator!= (const_rope_iterator lhs, const_rope_iterator rhs) noexcept
            { return !(lhs == rhs); }
            // TODO: Document wonky behavior of the inequalities when rhs.{frst,last}_ != rhs.{first,last}_.
            friend bool operator< (const_rope_iterator lhs, const_rope_iterator rhs) noexcept
            { return lhs.rope_ == rhs.rope_ && lhs.n_ < rhs.n_; }
            friend bool operator<= (const_rope_iterator lhs, const_rope_iterator rhs) noexcept
            { return lhs == rhs || lhs < rhs; }
            friend bool operator> (const_rope_iterator lhs, const_rope_iterator rhs) noexcept
            { return rhs < lhs; }
            friend bool operator>= (const_rope_iterator lhs, const_rope_iterator rhs) noexcept
            { return lhs <= rhs; }

            friend const_rope_iterator operator+ (const_rope_iterator lhs, difference_type rhs) noexcept
            { return lhs += rhs; }
            friend const_rope_iterator operator+ (difference_type lhs, const_rope_iterator rhs) noexcept
            { return rhs += lhs; }
            friend const_rope_iterator operator- (const_rope_iterator lhs, difference_type rhs) noexcept
            { return lhs -= rhs; }
            friend const_rope_iterator operator- (difference_type lhs, const_rope_iterator rhs) noexcept
            { return rhs -= lhs; }
            friend difference_type operator- (const_rope_iterator lhs, const_rope_iterator rhs) noexcept
            {
                // TODO: Document this precondition!
                assert(lhs.rope_ == rhs.rope_);
                return rhs.n_ - lhs.n_;
            }

        private:
            char deref () const
            {
                switch (leaf_->which_) {
                case node_t::which::t: {
                    text const * t = static_cast<text *>(leaf_->buf_ptr_);
                    return *(t->begin() + (n_ - leaf_start_));
                }
                case node_t::which::tv: {
                    text_view const * tv = static_cast<text_view *>(leaf_->buf_ptr_);
                    return *(tv->begin() + (n_ - leaf_start_));
                }
                case node_t::which::rtv: {
                    repeated_text_view const * rtv = static_cast<repeated_text_view *>(leaf_->buf_ptr_);
                    return *(rtv->begin() + (n_ - leaf_start_));
                }
                case node_t::which::ref: {
                    detail::reference const * ref = static_cast<detail::reference *>(leaf_->buf_ptr_);
                    return *(ref->ref_.begin() + (n_ - leaf_start_));
                }
                default: assert(!"unhandled rope node case"); break;
                }
                return '\0'; // This should never execute.
            }

            rope const * rope_;
            difference_type n_;
            mutable leaf_node_t const * leaf_;
            mutable difference_type leaf_start_;
        };

        struct const_reverse_rope_iterator
        {
            using value_type = char const;
            using difference_type = std::ptrdiff_t;
            using pointer = char const *;
            using reference = char const;
            using iterator_category = std::random_access_iterator_tag;

            const_reverse_rope_iterator () noexcept : base_ () {}
            explicit const_reverse_rope_iterator (const_rope_iterator it) noexcept : base_ (it) {}

            const_rope_iterator base () const { return base_ + 1; }

            reference operator* () const noexcept { return *base_; }
            value_type operator[] (difference_type n) const noexcept { return base_[n]; }

            const_reverse_rope_iterator & operator++ () noexcept { --base_; return *this; }
            const_reverse_rope_iterator operator++ (int) noexcept
            {
                const_reverse_rope_iterator retval = *this;
                --base_;
                return retval;
            }
            const_reverse_rope_iterator & operator+= (difference_type n) noexcept { base_ -= n; return *this; }

            const_reverse_rope_iterator & operator-- () noexcept { ++base_; return *this; }
            const_reverse_rope_iterator operator-- (int) noexcept
            {
                const_reverse_rope_iterator retval = *this;
                ++base_;
                return retval;
            }
            const_reverse_rope_iterator & operator-= (difference_type n) noexcept { base_ += n; return *this; }

            friend bool operator== (const_reverse_rope_iterator lhs, const_reverse_rope_iterator rhs) noexcept
            { return lhs.base_ == rhs.base_; }
            friend bool operator!= (const_reverse_rope_iterator lhs, const_reverse_rope_iterator rhs) noexcept
            { return !(lhs == rhs); }
            friend bool operator< (const_reverse_rope_iterator lhs, const_reverse_rope_iterator rhs) noexcept
            { return rhs.base_ < lhs.base_; }
            friend bool operator<= (const_reverse_rope_iterator lhs, const_reverse_rope_iterator rhs) noexcept
            { return rhs.base_ <= lhs.base_; }
            friend bool operator> (const_reverse_rope_iterator lhs, const_reverse_rope_iterator rhs) noexcept
            { return rhs.base_ > lhs.base_; }
            friend bool operator>= (const_reverse_rope_iterator lhs, const_reverse_rope_iterator rhs) noexcept
            { return rhs.base_ >= lhs.base_; }

            friend const_reverse_rope_iterator operator+ (const_reverse_rope_iterator lhs, difference_type rhs) noexcept
            { return lhs += rhs; }
            friend const_reverse_rope_iterator operator+ (difference_type lhs, const_reverse_rope_iterator rhs) noexcept
            { return rhs += lhs; }
            friend const_reverse_rope_iterator operator- (const_reverse_rope_iterator lhs, difference_type rhs) noexcept
            { return lhs -= rhs; }
            friend const_reverse_rope_iterator operator- (difference_type lhs, const_reverse_rope_iterator rhs) noexcept
            { return rhs -= lhs; }
            friend difference_type operator- (const_reverse_rope_iterator lhs, const_reverse_rope_iterator rhs) noexcept
            { return lhs.base_ - rhs.base_; }

        private:
            const_rope_iterator base_;
        };

    }

    int rope::compare (rope rhs) const noexcept
    {
        // TODO: This could probably be optimized quite a bit by doing
        // something equivalent to mismatch, segment-wise.
        auto const iters = std::mismatch(begin(), end(), rhs.begin(), rhs.end());
        if (iters.first == end()) {
            if (iters.second == rhs.end())
                return 0;
            else
                return -1;
        } else if (iters.second == rhs.end()) {
            return 1;
        } else if (*iters.first == *iters.second) {
            return 0;
        } else if (*iters.first < *iters.second) {
            return -1;
        } else {
            return 1;
        }
    }

    rope::const_iterator rope::begin () const noexcept
    { return const_iterator(*this, 0); }
    rope::const_iterator rope::end () const noexcept
    { return const_iterator(*this, size()); }

    rope::const_reverse_iterator rope::rbegin () const noexcept
    { return const_reverse_iterator(const_iterator(*this, size() - 1)); }
    rope::const_reverse_iterator rope::rend () const noexcept
    { return const_reverse_iterator(const_iterator(*this, -1)); }

    rope::const_iterator begin (rope const & r) noexcept
    { return r.begin(); }
    rope::const_iterator end (rope const & r) noexcept
    { return r.end(); }

    rope::const_reverse_iterator rbegin (rope const & r) noexcept
    { return r.rbegin(); }
    rope::const_reverse_iterator rend (rope const & r) noexcept
    { return r.rend(); }

} }

#endif
