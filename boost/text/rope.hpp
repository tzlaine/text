#ifndef BOOST_TEXT_ROPE_HPP
#define BOOST_TEXT_ROPE_HPP

#include <boost/text/text_view.hpp>
#include <boost/text/text.hpp>

#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/container/static_vector.hpp>

#include <vector>


namespace boost { namespace text {

    namespace detail {

        struct node;

        using node_ptr = boost::intrusive_ptr<node>;

        struct concatenation
        {
            concatenation (node_ptr left, node_ptr right) noexcept :
                left_ (left),
                right_ (right)
            {
                assert(left_);
                assert(right_);
            }

            node_ptr left_;
            node_ptr right_;
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
            alignment = alignment < alignof(concatenation) ? alignof(concatenation) : alignment;
            int size = sizeof(text);
            size = size < sizeof(text_view) ? alignof(text_view) : size;
            size = size < sizeof(repeated_text_view) ? alignof(repeated_text_view) : size;
            size = size < sizeof(reference) ? alignof(reference) : size;
            size = size < sizeof(concatenation) ? alignof(concatenation) : size;
            return alignment + size;
        }

        template <typename T>
        constexpr void * placement_address (void * buf, std::size_t buf_size) noexcept
        {
            std::size_t const alignment = alignof(T);
            std::size_t const size = sizeof(T);
            return std::align(alignment, size, buf, buf_size);
        }

        struct node
        {
            enum class which : char { t, tv, rtv, ref, cat };

            node () :
                buf_ptr_ (nullptr),
                size_ (0),
                parent_ (nullptr),
                refs_ (0),
                depth_ (0)
            {}

            ~node ()
            {
                if (!buf_ptr_)
                    return;

                switch (which_) {
                case which::t: static_cast<text *>(buf_ptr_)->~text(); break;
                case which::tv: static_cast<text_view *>(buf_ptr_)->~text_view (); break;
                case which::rtv: static_cast<repeated_text_view *>(buf_ptr_)->~repeated_text_view(); break;
                case which::ref: static_cast<reference *>(buf_ptr_)->~reference(); break;
                case which::cat: static_cast<concatenation *>(buf_ptr_)->~concatenation(); break;
                default: assert(!"unhandled rope node case"); break;
                }
            }

            char buf_[node_buf_size()];
            void * buf_ptr_;
            std::ptrdiff_t size_;
            node const * parent_;
            int refs_;
            char depth_;
            which which_;
        };

        void intrusive_ptr_add_ref (node * node)
        { ++node->refs_; }
        void intrusive_ptr_release (node * node)
        {
            if (!--node->refs_)
                delete node;
        }

        inline reference::reference (node_ptr text_node, text_view ref) noexcept :
            text_ (text_node),
            ref_ (ref)
        {
            assert(text_node);
            assert(text_node->which_ == node::which::t);
        }

        constexpr int max_depth = 91;
        constexpr int text_max_size = 512;

        constexpr std::ptrdiff_t min_size_of_balanced_tree (int depth) noexcept
        {
            constexpr std::ptrdiff_t min_sizes[max_depth + 1] = {
                1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584,
                4181, 6765, 10946, 17711, 28657, 46368, 75025, 121393,
                196418, 317811, 514229, 832040, 1346269, 2178309, 3524578,
                5702887, 9227465, 14930352, 24157817, 39088169, 63245986,
                102334155, 165580141, 267914296, 433494437, 701408733,
                1134903170, 1836311903, 2971215073,
                4807526976, 7778742049, 12586269025, 20365011074, 32951280099,
                53316291173, 86267571272, 139583862445, 225851433717, 365435296162,
                591286729879, 956722026041, 1548008755920, 2504730781961,
                4052739537881, 6557470319842, 10610209857723, 17167680177565,
                27777890035288, 44945570212853, 72723460248141, 117669030460994,
                190392490709135, 308061521170129, 498454011879264, 806515533049393,
                1304969544928657, 2111485077978050, 3416454622906707,
                5527939700884757, 8944394323791464, 14472334024676221,
                23416728348467685, 37889062373143906, 61305790721611591,
                99194853094755497, 160500643816367088, 259695496911122585,
                420196140727489673, 679891637638612258, 1100087778366101931,
                1779979416004714189, 2880067194370816120, 4660046610375530309,
                7540113804746346429
            };
            return min_sizes[depth];
        }

        inline node_ptr make_node (text const & t)
        {
            node_ptr retval(new node);
            retval->size_ = t.size();
            retval->which_ = node::which::t;
            auto at = placement_address<text>(retval->buf_, sizeof(retval->buf_));
            assert(at);
            retval->buf_ptr_ = new (at) text(t);
            return retval;
        }

        inline node_ptr make_node (text && t)
        {
            node_ptr retval(new node);
            retval->size_ = t.size();
            retval->which_ = node::which::t;
            auto at = placement_address<text>(retval->buf_, sizeof(retval->buf_));
            assert(at);
            retval->buf_ptr_ = new (at) text(std::move(t));
            return retval;
        }

        inline node_ptr make_node (text_view tv)
        {
            node_ptr retval(new node);
            retval->size_ = tv.size();
            retval->which_ = node::which::tv;
            auto at = placement_address<text_view>(retval->buf_, sizeof(retval->buf_));
            assert(at);
            retval->buf_ptr_ = new (at) text_view(tv);
            return retval;
        }

        inline node_ptr make_node (repeated_text_view rtv)
        {
            node_ptr retval(new node);
            retval->size_ = rtv.size();
            retval->which_ = node::which::rtv;
            auto at = placement_address<repeated_text_view>(retval->buf_, sizeof(retval->buf_));
            assert(at);
            retval->buf_ptr_ = new (at) repeated_text_view(rtv);
            return retval;
        }

        inline node_ptr make_ref (node_ptr t, std::ptrdiff_t lo, std::ptrdiff_t hi)
        {
            assert(t->which_ == node::which::t);
            text_view const tv = (*static_cast<text *>(t->buf_ptr_))(lo, hi);
            node_ptr retval(new node);
            retval->size_ = tv.size();
            retval->which_ = node::which::ref;
            auto at = placement_address<reference>(retval->buf_, sizeof(retval->buf_));
            assert(at);
            retval->buf_ptr_ = new (at) reference(t, tv);
            return retval;
        }

        inline node_ptr make_cat (node_ptr lhs, node_ptr rhs)
        {
            node_ptr retval(new node);
            retval->size_ = lhs->size_ + rhs->size_;
            retval->depth_ = (std::max)(lhs->depth_, rhs->depth_) + 1;
            retval->which_ = node::which::cat;
            auto at = placement_address<concatenation>(retval->buf_, sizeof(retval->buf_));
            assert(at);
            retval->buf_ptr_ = new (at) concatenation(lhs, rhs);
            if (lhs)
                lhs->parent_ = retval.get();
            if (rhs)
                rhs->parent_ = retval.get();
            return retval;
        }

        struct found_char
        {
            node_ptr node_;
            std::ptrdiff_t offset_;
            char c_;
        };

        inline found_char find_char (node_ptr node, std::ptrdiff_t n)
        {
            assert(node);
            if (node->which_ == node::which::cat) {
                concatenation * cat = static_cast<concatenation *>(node->buf_ptr_);
                if (n < cat->left_->size_)
                    return find_char(cat->left_, n);
                else
                    return find_char(cat->right_, n - cat->left_->size_);
            } else if (n < node->size_) {
                char c = '\0';
                switch (node->which_) {
                case node::which::t:
                    c = *(static_cast<text *>(node->buf_ptr_)->cbegin() + n);
                    break;
                case node::which::tv:
                    c = *(static_cast<text_view *>(node->buf_ptr_)->begin() + n);
                    break;
                case node::which::rtv:
                    c = *(static_cast<repeated_text_view *>(node->buf_ptr_)->begin() + n);
                    break;
                case node::which::ref:
                    c = *(static_cast<reference *>(node->buf_ptr_)->ref_.begin() + n);
                    break;
                case node::which::cat:
                default: assert(!"unhandled rope node case"); break;
                }
                return found_char{node, n, c};
            } else {
                return found_char{nullptr, -1, '\0'};
            }
        }

        inline node_ptr slice_leaf (node_ptr node, std::ptrdiff_t lo, std::ptrdiff_t hi)
        {
            assert(lo <= hi);

            if (lo == 0 && hi == node->size_)
                return node;

            switch (node->which_) {
            case detail::node::which::t:
                return make_ref(node, lo, hi);
            case detail::node::which::tv:
                return make_node((*static_cast<text_view *>(node->buf_ptr_))(lo, hi));
            case detail::node::which::rtv: {
                repeated_text_view const & rtv =
                    *static_cast<repeated_text_view *>(node->buf_ptr_);
                std::ptrdiff_t const lo_segment = lo / rtv.view().size();
                std::ptrdiff_t const hi_segment = hi / rtv.view().size();
                if (lo_segment == hi_segment) {
                    return make_node(rtv.view()(lo % rtv.view().size(), hi % rtv.view().size()));
                } else {
                    node_ptr prefix = make_node(rtv.view()(lo % rtv.view().size(), rtv.view().size()));
                    node_ptr suffix = make_node(rtv.view()(0, hi % rtv.view().size()));
                    if (2 < hi_segment - lo_segment) {
                        node_ptr middle =
                            make_node(repeated_text_view(rtv.view(), hi_segment - lo_segment - 1));
                        return make_cat(make_cat(prefix, middle), suffix);
                    } else if (hi_segment - lo_segment == 2) {
                        node_ptr middle = make_node(rtv.view());
                        return make_cat(make_cat(prefix, middle), suffix);
                    } else {
                        return make_cat(prefix, suffix);
                    }
                }
                break;
            }
            case detail::node::which::ref:
                return make_node(static_cast<detail::reference *>(node->buf_ptr_)->ref_(lo, hi));
            case detail::node::which::cat:
            default: assert(!"unhandled rope node case"); break;
            }
            return nullptr; // This should never execute.
        }

        inline node_ptr common_ancestor (node_ptr lhs, node_ptr rhs)
        {
            assert(lhs);
            assert(rhs);

            if (lhs == rhs)
                return lhs;

            container::static_vector<node const *, max_depth + 1> lhs_ancestors;
            node const * ptr = lhs.get();
            lhs_ancestors.push_back(ptr);
            while (ptr->parent_) {
                if (ptr == rhs)
                    return rhs;
                lhs_ancestors.push_back(ptr->parent_);
                ptr = ptr->parent_;
            }

            std::sort(lhs_ancestors.begin(), lhs_ancestors.end());

            ptr = rhs.get();
            while (ptr->parent_) {
                auto const it = std::lower_bound(
                    lhs_ancestors.begin(), lhs_ancestors.end(),
                    ptr->parent_
                );
                if (it != lhs_ancestors.end() && *it == ptr->parent_)
                    return const_cast<node *>(*it);
                ptr = ptr->parent_;
            }

            return nullptr;
        }

        inline node_ptr copy_tree_replace_nodes (
            node_ptr root,
            node_ptr replacee_1, node_ptr replacer_1,
            node_ptr replacee_2, node_ptr replacer_2
        ) {
            if (root->which_ != node::which::cat) {
                if (root == replacee_1)
                    return replacer_1;
                if (root == replacee_2)
                    return replacer_2;
                return root;
            } else  {
                concatenation const * const cat = static_cast<concatenation *>(root->buf_ptr_);
                return make_cat(
                    copy_tree_replace_nodes(cat->left_, replacee_1, replacer_1, replacee_2, replacer_2),
                    copy_tree_replace_nodes(cat->right_, replacee_1, replacer_1, replacee_2, replacer_2)
                );
            }
        }


        template <typename Text>
        node_ptr append (node_ptr node, Text && rhs)
        {
            if (node) {
                switch (node->which_) {
                case node::which::t: {
                    if (1 < node->refs_)
                        return make_cat(node, make_node(static_cast<Text &&>(rhs)));
                    text * t = static_cast<text *>(node->buf_ptr_);
                    bool const space_available = rhs.size() < t->capacity() - t->size();
                    if (space_available || t->size() + rhs.size() <= text_max_size) {
                        *t += rhs;
                        return node;
                    } else {
                        return make_cat(node, make_node(static_cast<Text &&>(rhs)));
                    }
                }
                case node::which::tv:
                case node::which::rtv:
                case node::which::ref:
                    // TODO: Collapse where possible, instead of making new nodes.
                    node = make_cat(node, make_node(static_cast<Text &&>(rhs)));
                    return node;
                case node::which::cat: {
                    if (1 < node->refs_)
                        return make_cat(node, make_node(static_cast<Text &&>(rhs)));
                    concatenation * cat = static_cast<concatenation *>(node->buf_ptr_);
                    node_ptr new_right = nullptr;
                    if (cat->right_ && cat->right_->which_ == node::which::t)
                        new_right = append(cat->right_, static_cast<Text &&>(rhs));
                    else
                        new_right = make_cat(cat->right_, make_node(static_cast<Text &&>(rhs)));
                    cat->right_->parent_ = node.get();
                    cat->right_ = new_right;
                    return node;
                }
                default: assert(!"unhandled rope node case"); break;
                }
            }
            return make_node(static_cast<Text &&>(rhs));
        }

        inline node_ptr concatenate (node_ptr lhs, node_ptr rhs)
        {
            if (!lhs)
                return rhs;
            else if (!rhs)
                return lhs;
            return make_cat(lhs, rhs);
        }

        inline bool balanced (node_ptr node) noexcept
        { return min_size_of_balanced_tree(node->depth_) <= node->size_; }

        inline void add_leaf_to_forest (node_ptr node, node_ptr * forest)
        {
            assert(forest);

            std::ptrdiff_t i;
            node_ptr insertee = nullptr;

            {
                node_ptr too_small = nullptr;

                std::ptrdiff_t size = node->size_;
                for (i = 0; min_size_of_balanced_tree(i + 1) <= size; ++i) {
                    if (forest[i]) {
                        too_small = concatenate(forest[i], too_small);
                        forest[i] = 0;
                    }
                }

                insertee = concatenate(too_small, node);
            }

            assert(insertee->depth_ <= node->depth_ + 1);
            for (;; ++i) {
                if (forest[i]) {
                    insertee = concatenate(forest[i], insertee);
                    forest[i] = 0;
                }
                assert(min_size_of_balanced_tree(i) <= insertee->size_);
                assert(!forest[i]);
                if (i == max_depth ||
                    insertee->size_ < min_size_of_balanced_tree(i + 1)) {
                    forest[i] = insertee;
                    return;
                }
            }
        }

        inline void add_to_forest (node_ptr node, node_ptr * forest)
        {
            assert(node);
            assert(forest);
            if (balanced(node)) {
                add_leaf_to_forest(node, forest);
            } else {
                assert(node->which_ == node::which::cat);
                concatenation * cat = static_cast<concatenation *>(node->buf_ptr_);
                add_to_forest(cat->left_, forest);
                add_to_forest(cat->right_, forest);
            }
        }

        struct const_rope_iterator;

    }

    struct rope
    {
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
#if 0
        const_iterator begin () const noexcept { return data_; }
        const_iterator end () const noexcept { return data_ + size_; }

        const_iterator cbegin () const noexcept { return begin(); }
        const_iterator cend () const noexcept { return end(); }

        const_reverse_iterator rbegin () const noexcept { return reverse_iterator(end()); }
        const_reverse_iterator rend () const noexcept { return reverse_iterator(begin()); }

        const_reverse_iterator crbegin () const noexcept { return rbegin(); }
        const_reverse_iterator crend () const noexcept { return rend(); }
#endif

        bool empty () const noexcept
        { return !ptr_; }

        std::ptrdiff_t size () const noexcept
        { return ptr_ ? ptr_->size_ : 0; }

        char operator[] (std::ptrdiff_t n) const noexcept
        {
            assert(ptr_);
            assert(n < size());
            detail::found_char found = find_char(ptr_, n);
            return found.c_;
        }

        constexpr std::ptrdiff_t max_size () const noexcept
        { return PTRDIFF_MAX; }

        rope substr (std::ptrdiff_t lo, std::ptrdiff_t hi) const
        {
            assert(ptr_);
            assert(0 <= lo && lo <= size());
            assert(0 <= hi && hi <= size());
            assert(lo <= hi);

            detail::found_char const found_lo = find_char(ptr_, lo);
            if (found_lo.offset_ + (hi - lo) < found_lo.node_->size_) {
                rope retval;
                retval.ptr_ = slice_leaf(found_lo.node_, found_lo.offset_, found_lo.offset_ + hi - lo);
                return retval;
            } else {
                detail::found_char const found_hi = find_char(ptr_, hi);

                detail::node_ptr root = common_ancestor(found_lo.node_, found_hi.node_);

                detail::node_ptr const lo_slice =
                    slice_leaf(found_lo.node_, found_lo.offset_, found_lo.node_->size_);
                detail::node_ptr const hi_slice =
                    slice_leaf(found_hi.node_, 0, found_hi.offset_);

                detail::node_ptr ptr = copy_tree_replace_nodes(
                    root,
                    found_lo.node_, lo_slice,
                    found_hi.node_, hi_slice
                );

                rope retval;
                retval.ptr_ = ptr;
                return retval;
            }
        }

        rope substr (std::ptrdiff_t cut) const
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

#if 0
        int compare (rope rhs) const noexcept
        { return TODO; }

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

        friend iterator begin (rope const & r) noexcept
        { return v.begin(); }
        friend iterator end (rope const & r) noexcept
        { return v.end(); }
        friend iterator cbegin (rope const & r) noexcept
        { return v.cbegin(); }
        friend iterator cend (rope const & r) noexcept
        { return v.cend(); }

        friend reverse_iterator rbegin (rope const & r) noexcept
        { return v.rbegin(); }
        friend reverse_iterator rend (rope const & r) noexcept
        { return v.rend(); }
        friend reverse_iterator crbegin (rope const & r) noexcept
        { return v.crbegin(); }
        friend reverse_iterator crend (rope const & r) noexcept
        { return v.crend(); }

        friend std::ostream & operator<< (std::ostream & os, rope const & r)
        { TODO; }
#endif

        void clear ()
        { ptr_ = nullptr; }

        rope & append (rope const & r)
        {
            if (r.empty())
                return *this;
            if (empty()) {
                ptr_ = r.ptr_;
                return *this;
            }
                
            ptr_ = detail::make_cat(ptr_, r.ptr_);
            if (detail::max_depth <= ptr_->depth_)
                rebalance();

            return *this;
        }

        rope & append (text_view tv)
        {
            ptr_ = detail::append(ptr_, tv);
            if (detail::max_depth <= ptr_->depth_)
                rebalance();
            return *this;
        }

        rope & append (repeated_text_view rtv)
        {
            ptr_ = detail::append(ptr_, rtv);
            if (detail::max_depth <= ptr_->depth_)
                rebalance();
            return *this;
        }

        rope & append (text const & t)
        {
            ptr_ = detail::append(ptr_, t);
            if (detail::max_depth <= ptr_->depth_)
                rebalance();
            return *this;
        }

        rope & append (text && t)
        {
            ptr_ = detail::append(ptr_, std::move(t));
            if (detail::max_depth <= ptr_->depth_)
                rebalance();
            return *this;
        }

        void swap (rope & rhs)
        { ptr_.swap(rhs.ptr_); }

        void rebalance()
        {
            if (empty() || ptr_->which_ == detail::node::which::t)
                return;

            detail::node_ptr forest[detail::max_depth + 1] = {
                0,0,0,0,0, 0,0,0,0,0,
                0,0,0,0,0, 0,0,0,0,0,
                0,0,0,0,0, 0,0,0,0,0,
                0,0,0,0,0, 0,0,0,0,0,
                0,0,0,0,0, 0,0,0,0,0,
                0,0,0,0,0, 0,0,0,0,0,
                0,0,0,0,0, 0,0,0,0,0,
                0,0,0,0,0, 0,0,0,0,0,
                0,0,0,0,0, 0,0,0,0,0,
                0,0
            };

            detail::add_to_forest(ptr_, forest);
            ptr_ = 0;
            for (int i = 0; i <= detail::max_depth; ++i) {
                if (forest[i])
                    prepend(forest[i]);
            }

            assert(ptr_->depth_ < detail::max_depth);
        }

    private:
        void prepend (detail::node_ptr node)
        {
            if (empty())
                ptr_ = node;
            else
                ptr_ = detail::make_cat(node, ptr_);
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
            char_iter_ (false)
        {}

        const_rope_iterator (rope const & r, std::ptrdiff_t n) noexcept :
            rope_ (&r),
            n_ (n),
            leaf_ (nullptr),
            char_iter_ (false)
        {}

        reference operator* () const noexcept
        {
            if (!leaf_) {
                found_char const found = find_char(rope_->ptr_, n_);
                switch (found.node_->which_) {
                case node::which::t: {
                    text const * t = static_cast<text *>(found.node_->buf_ptr_);
                    first_.char_it_ = t->begin();
                    it_.char_it_ = first_.char_it_;
                    last_.char_it_ = t->end();
                    char_iter_ = true;
                    break;
                }
                case node::which::tv: {
                    text_view const * tv = static_cast<text_view *>(found.node_->buf_ptr_);
                    first_.char_it_ = tv->begin();
                    it_.char_it_ = first_.char_it_;
                    last_.char_it_ = tv->end();
                    char_iter_ = true;
                    break;
                }
                case node::which::rtv: {
                    repeated_text_view const * rtv = static_cast<repeated_text_view *>(found.node_->buf_ptr_);
                    first_.repeated_it_ = rtv->begin();
                    it_.repeated_it_ = first_.repeated_it_;
                    last_.repeated_it_ = rtv->end();
                    char_iter_ = true;
                    break;
                }
                case node::which::ref: {
                    detail::reference const * ref = static_cast<detail::reference *>(found.node_->buf_ptr_);
                    first_.char_it_ = ref->ref_.begin();
                    it_.char_it_ = first_.char_it_;
                    last_.char_it_ = ref->ref_.end();
                    char_iter_ = false;
                    break;
                }
                case node::which::cat:
                default: assert(!"unhandled rope node case"); break;
                }
                return found.c_;
            }
            if (char_iter_)
                return *it_.char_it_;
            else
                return *it_.repeated_it_;
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
            if (leaf_) {
                if (char_iter_) {
                    if (++it_.char_it_ == last_.char_it_)
                        leaf_ = nullptr; // TODO: Go to next leaf.
                } else {
                    if (++it_.repeated_it_ == last_.repeated_it_)
                        leaf_ = nullptr;
                }
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
            // TODO
            return *this;
        }

        const_rope_iterator & operator-- () noexcept
        {
            --n_;
            if (leaf_) {
                if (char_iter_) {
                    if (it_.char_it_ == first_.char_it_)
                        leaf_ = nullptr; // TODO: Go to next leaf.
                    else
                        --it_.char_it_;
                } else {
                    if (it_.repeated_it_ == first_.repeated_it_)
                        leaf_ = nullptr;
                    else
                        --it_.repeated_it_;
                }
            }
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
            // TODO
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
        union iter_t
        {
            iter_t () {}

            char const * char_it_;
            const_repeated_chars_iterator repeated_it_;
        };

        rope const * rope_;
        difference_type n_;
        mutable node const * leaf_;
        mutable iter_t first_;
        mutable iter_t it_;
        mutable iter_t last_;
        mutable bool char_iter_;
    };

    }

} }

#endif
