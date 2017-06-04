#ifndef BOOST_TEXT_ROPE_HPP
#define BOOST_TEXT_ROPE_HPP

#include <boost/text/text_view.hpp>
#include <boost/text/text.hpp>

#include <boost/smart_ptr/intrusive_ptr.hpp>

#include <vector>


namespace boost { namespace text {

    namespace detail {

        struct node;

        using node_ptr = boost::intrusive_ptr<node>;

        struct concatenation
        {
            concatenation (
                node_ptr const & left,
                node_ptr const & right
            ) noexcept :
                left_ (left),
                right_ (right)
            {}

            node_ptr left_;
            node_ptr right_;
        };

        constexpr int node_buf_size () noexcept
        {
            int alignment = alignof(text);
            alignment = alignment < alignof(text_view) ? alignof(text_view) : alignment;
            alignment = alignment < alignof(repeated_text_view) ? alignof(repeated_text_view) : alignment;
            alignment = alignment < alignof(concatenation) ? alignof(concatenation) : alignment;
            int size = sizeof(text);
            size = size < sizeof(text_view) ? alignof(text_view) : size;
            size = size < sizeof(repeated_text_view) ? alignof(repeated_text_view) : size;
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
            enum class which : char { t, tv, rtv, cat };

            node () : buf_ptr_ (nullptr) {}
            ~node ()
            {
                if (!buf_ptr_)
                    return;

                switch (which_) {
                case which::t: static_cast<text *>(buf_ptr_)->~text(); break;
                case which::tv: static_cast<text_view *>(buf_ptr_)->~text_view (); break;
                case which::rtv: static_cast<repeated_text_view *>(buf_ptr_)->~repeated_text_view(); break;
                case which::cat: static_cast<concatenation *>(buf_ptr_)->~concatenation(); break;
                default: assert(!"unhandled rope node case"); break;
                }
            }

            char buf_[node_buf_size()];
            void * buf_ptr_;
            std::ptrdiff_t size_;
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
            retval->depth_ = 0;
            retval->which_ = detail::node::which::t;
            auto at = detail::placement_address<text>(retval->buf_, sizeof(retval->buf_));
            assert(at);
            retval->buf_ptr_ = new (at) text(t);
            return retval;
        }

        inline node_ptr make_node (text && t)
        {
            node_ptr retval(new node);
            retval->size_ = t.size();
            retval->depth_ = 0;
            retval->which_ = detail::node::which::t;
            auto at = detail::placement_address<text>(retval->buf_, sizeof(retval->buf_));
            assert(at);
            retval->buf_ptr_ = new (at) text(std::move(t));
            return retval;
        }

        inline node_ptr make_node (text_view tv)
        {
            node_ptr retval(new node);
            retval->size_ = tv.size();
            retval->depth_ = 0;
            retval->which_ = detail::node::which::tv;
            auto at = detail::placement_address<text_view>(retval->buf_, sizeof(retval->buf_));
            assert(at);
            retval->buf_ptr_ = new (at) text_view(tv);
            return retval;
        }

        inline node_ptr make_node (repeated_text_view rtv)
        {
            node_ptr retval(new node);
            retval->size_ = rtv.size();
            retval->depth_ = 0;
            retval->which_ = detail::node::which::rtv;
            auto at = detail::placement_address<repeated_text_view>(retval->buf_, sizeof(retval->buf_));
            assert(at);
            retval->buf_ptr_ = new (at) repeated_text_view(rtv);
            return retval;
        }

        inline node_ptr make_cat (node_ptr lhs, node_ptr rhs)
        {
            node_ptr retval(new node);
            retval->size_ = lhs->size_ + rhs->size_;
            retval->depth_ = (std::max)(lhs->depth_, rhs->depth_) + 1;
            retval->which_ = detail::node::which::cat;
            auto at = detail::placement_address<detail::concatenation>(retval->buf_, sizeof(retval->buf_));
            assert(at);
            retval->buf_ptr_ = new (at) detail::concatenation(lhs, rhs);
            return retval;
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
#if 0
                    bool const rhs_is_text = std::is_same<
                        typename std::remove_cv<typename std::remove_reference<T>::type>::type,
                        text
                    >::value;
#endif
                    if (space_available || t->size() + rhs.size() <= text_max_size) {
                        *t += rhs;
                        return node;
                    } else {
                        return make_cat(node, make_node(static_cast<Text &&>(rhs)));
                    }
                }
                case node::which::tv:
                case node::which::rtv:
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
                    cat->right_ = new_right;
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
        constexpr const_iterator begin () const noexcept { return data_; }
        constexpr const_iterator end () const noexcept { return data_ + size_; }

        constexpr const_iterator cbegin () const noexcept { return begin(); }
        constexpr const_iterator cend () const noexcept { return end(); }

        constexpr const_reverse_iterator rbegin () const noexcept { return reverse_iterator(end()); }
        constexpr const_reverse_iterator rend () const noexcept { return reverse_iterator(begin()); }

        constexpr const_reverse_iterator crbegin () const noexcept { return rbegin(); }
        constexpr const_reverse_iterator crend () const noexcept { return rend(); }
#endif

        bool empty () const noexcept
        { return !ptr_; }

        std::ptrdiff_t size () const noexcept
        { return ptr_ ? ptr_->size_ : 0; }

        char operator[] (std::ptrdiff_t n) const noexcept
        {
            // TODO
            return 'c';
        }

        constexpr std::ptrdiff_t max_size () const noexcept
        { return PTRDIFF_MAX; }

#if 0
        constexpr int compare (rope rhs) const noexcept
        { return detail::compare_impl(begin(), end(), rhs.begin(), rhs.end()); }

        constexpr bool operator== (rope rhs) const noexcept
        { return compare(rhs) == 0; }

        constexpr bool operator!= (rope rhs) const noexcept
        { return compare(rhs) != 0; }

        constexpr bool operator< (rope rhs) const noexcept
        { return compare(rhs) < 0; }

        constexpr bool operator<= (rope rhs) const noexcept
        { return compare(rhs) <= 0; }

        constexpr bool operator> (rope rhs) const noexcept
        { return compare(rhs) > 0; }

        constexpr bool operator>= (rope rhs) const noexcept
        { return compare(rhs) >= 0; }

        friend constexpr iterator begin (rope r) noexcept
        { return v.begin(); }
        friend constexpr iterator end (rope r) noexcept
        { return v.end(); }
        friend constexpr iterator cbegin (rope r) noexcept
        { return v.cbegin(); }
        friend constexpr iterator cend (rope r) noexcept
        { return v.cend(); }

        friend constexpr reverse_iterator rbegin (rope r) noexcept
        { return v.rbegin(); }
        friend constexpr reverse_iterator rend (rope r) noexcept
        { return v.rend(); }
        friend constexpr reverse_iterator crbegin (rope r) noexcept
        { return v.crbegin(); }
        friend constexpr reverse_iterator crend (rope r) noexcept
        { return v.crend(); }

        friend std::ostream & operator<< (std::ostream & os, rope r)
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

            ptr_t forest[detail::max_depth + 1] = {
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
        using ptr_t = detail::node_ptr;

        void concatenate_forest (ptr_t * forest, int total_size)
        {
            int i = 0;
            while (size() != total_size) {
                if (forest[i])
                    prepend(forest[i]);
                ++i;
            }
        }

        void prepend (ptr_t const & node)
        {
            if (empty())
                ptr_ = node;
            else
                ptr_ = detail::make_cat(node, ptr_);
        }

        ptr_t ptr_;
    };

} }

#endif
