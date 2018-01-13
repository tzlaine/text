#ifndef BOOST_TEXT_TRIE_HPP
#define BOOST_TEXT_TRIE_HPP

#include <boost/optional.hpp>

#include <memory>
#include <type_traits>
#include <vector>


namespace boost { namespace trie {

    // TODO: KeyIter, KeyRange, and Range concepts.

    // TODO: Document assumption that Compare is statically polymorphic.
    // TODO: Document that Key::value_type must be default constructible. No!
    // make the key element a part of the vector of childre.
    // TODO: Privatize trie_node_t data members.

    template<typename Key, typename Value>
    struct trie_iterator;

    template<typename Key, typename Value>
    struct const_trie_iterator;

    template<typename Key, typename Value>
    struct reverse_trie_iterator;

    template<typename Key, typename Value>
    struct const_reverse_trie_iterator;

    template<typename Key, typename Value>
    struct trie_element
    {
        // TODO: Either this should be const, or iterators should only provide
        // a const reference to it.
        Key key;
        Value value;

        friend bool
        operator==(trie_element const & lhs, trie_element const & rhs)
        {
            return lhs.key == rhs.key && lhs.value == rhs.value;
        }
        friend bool
        operator!=(trie_element const & lhs, trie_element const & rhs)
        {
            return !(lhs == rhs);
        }
    };

    template<typename Key, typename Value>
    struct trie_range
    {
        using iterator = trie_iterator<Key, Value>;

        iterator first;
        iterator last;

        iterator begin() const { return first; }
        iterator end() const { return last; }

        friend bool operator==(trie_range const & lhs, trie_range const & rhs)
        {
            return lhs.first == rhs.first && lhs.last == rhs.last;
        }
        friend bool operator!=(trie_range const & lhs, trie_range const & rhs)
        {
            return !(lhs == rhs);
        }
    };

    template<typename Key, typename Value>
    struct const_trie_range
    {
        using const_iterator = const_trie_iterator<Key, Value>;

        const_iterator first;
        const_iterator last;

        const_iterator begin() const { return first; }
        const_iterator end() const { return last; }

        friend bool
        operator==(const_trie_range const & lhs, const_trie_range const & rhs)
        {
            return lhs.first == rhs.first && lhs.last == rhs.last;
        }
        friend bool
        operator!=(const_trie_range const & lhs, const_trie_range const & rhs)
        {
            return !(lhs == rhs);
        }
    };

    template<typename Key, typename Value>
    struct trie_insert_result
    {
        trie_iterator<Key, Value> iter;
        bool inserted;

        friend bool operator==(
            trie_insert_result const & lhs, trie_insert_result const & rhs)
        {
            return lhs.iter == rhs.iter && lhs.inserted == rhs.inserted;
        }
        friend bool operator!=(
            trie_insert_result const & lhs, trie_insert_result const & rhs)
        {
            return !(lhs == rhs);
        }
    };

    namespace detail {

        template<typename Key, typename Value>
        struct trie_node_t;

        template<typename Key, typename Value>
        using trie_node_children_t =
            std::vector<std::unique_ptr<trie_node_t<Key, Value>>>;

        template<typename Key, typename Value>
        struct trie_iterator_state_t
        {
            trie_node_t<Key, Value> const * parent_;
            std::size_t index_;
        };

        template<typename Key, typename Value>
        trie_iterator_state_t<Key, Value>
        parent_state(trie_iterator_state_t<Key, Value> state)
        {
            auto const new_parent = state.parent_->parent_;
            auto const it = std::find_if(
                new_parent->children_.begin(),
                new_parent->children_.end(),
                [state](std::unique_ptr<trie_node_t<Key, Value>> const & ptr) {
                    return ptr.get() == state.parent_;
                });
            std::size_t const index = it - new_parent->children_.begin();
            return {new_parent, index};
        }

        template<typename Key, typename Value, typename Compare>
        trie_iterator_state_t<Key, Value> parent_state(
            trie_iterator_state_t<Key, Value> state, Compare const & comp)
        {
            auto const new_parent = state.parent_->parent_;
            auto const it = new_parent->lower_bound(state.parent_->e_, comp);
            return {new_parent, it - new_parent->children_.begin()};
        }

        // TODO
        template<typename T>
        struct is_pair_iterator : std::false_type
        {
        };
    }

    template<typename Key, typename Value>
    struct trie_match_result
    {
        detail::trie_node_t<Key, Value> const * node;
        std::ptrdiff_t size;
        bool match;

        friend bool
        operator==(trie_match_result const & lhs, trie_match_result const & rhs)
        {
            return lhs.node == rhs.node && lhs.size == rhs.size &&
                   lhs.match == rhs.match;
        }
        friend bool
        operator!=(trie_match_result const & lhs, trie_match_result const & rhs)
        {
            return !(lhs == rhs);
        }
    };

    // TODO: Key concept specifying that Key is a container.
    // TODO: Compare concept specifying that Compare compares Key::value_types.
    template<typename Key, typename Value, typename Compare = std::less<>>
    struct trie
    {
    private:
        using node_t = detail::trie_node_t<Key, Value>;
        using iter_state_t = detail::trie_iterator_state_t<Key, Value>;
        using node_children_t = detail::trie_node_children_t<Key, Value>;

    public:
        using element = trie_element<Key, Value>;
        using range = trie_range<Key, Value>;
        using const_range = const_trie_range<Key, Value>;
        using insert_result = trie_insert_result<Key, Value>;
        using match_result = trie_match_result<Key, Value>;

        using key_type = Key;
        using mapped_type = Value;
        using value_type = element;
        using key_compare = Compare;
        using key_element_type = typename Key::value_type;

        using reference = value_type &;
        using const_reference = value_type const &;
        using iterator = trie_iterator<key_type, mapped_type>;
        using const_iterator = const_trie_iterator<key_type, mapped_type>;
        using reverse_iterator = reverse_trie_iterator<key_type, mapped_type>;
        using const_reverse_iterator =
            const_reverse_trie_iterator<key_type, mapped_type>;
        using size_type = std::ptrdiff_t;
        using difference_type = std::ptrdiff_t;

        trie() : size_(0) {}

        trie(Compare const & comp) : comp_(comp), size_(0) {}

        template<typename Iter>
        trie(Iter first, Iter last, Compare const & comp = Compare()) :
            comp_(comp),
            size_(0)
        {
            insert(first, last);
        }
        template<typename Range>
        explicit trie(Range r, Compare const & comp = Compare()) :
            comp_(comp),
            size_(0)
        {
            using std::begin;
            using std::end;
            insert(begin(r), end(r));
        }
        trie(std::initializer_list<value_type> il) : size_(0) { insert(il); }
        // TODO: Needs implementation and testing.
        template<typename PairIter>
        trie(
            PairIter first,
            PairIter last,
            Compare const & comp = Compare(),
            std::enable_if<detail::is_pair_iterator<PairIter>::value> * =
                nullptr) :
            size_(0),
            comp_(comp)
        {
            for (; first != last; ++first) {
                insert(first->first, first->second);
            }
        }

        trie & operator=(std::initializer_list<value_type> il)
        {
            clear();
            for (auto const & x : il) {
                insert(x);
            }
            return *this;
        }

        bool empty() const noexcept { return size_ == 0; }
        size_type size() const noexcept { return size_; }
        size_type max_size() const noexcept { return PTRDIFF_MAX; }

        const_iterator begin() const noexcept
        {
            iter_state_t state{&root_, 0};
            if (size_) {
                while (!state.parent_->children_.front()->value_) {
                    state.parent_ = state.parent_->children_.front().get();
                }
            }
            return const_iterator(state);
        }
        const_iterator end() const noexcept
        {
            iter_state_t state{&root_, root_.children_.size()};
            if (size_) {
                while (!state.parent_->children_.back()->value_) {
                    state.index_ = state.parent_->children_.size() - 1;
                    state.parent_ = state.parent_->children_.back().get();
                }
            }
            return const_iterator(state);
        }
        const_iterator cbegin() const noexcept { return begin(); }
        const_iterator cend() const noexcept { return end(); }

        const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator{end()};
        }
        const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator{begin()};
        }
        const_reverse_iterator crbegin() const noexcept { return rbegin(); }
        const_reverse_iterator crend() const noexcept { return rend(); }

        template<typename KeyT>
        bool contains(KeyT const & key) const noexcept
        {
            return find(key) != end();
        }

        template<typename KeyT>
        const_iterator find(KeyT const & key) const noexcept
        {
            using std::begin;
            using std::end;
            auto first = begin(key);
            auto const last = end(key);
            auto match = longest_match_impl(first, last);
            if (match.result_.match) {
                return const_iterator(iter_state_t{match.result_.node->parent_,
                                                   match.index_within_parent_});
            }
            return this->end();
        }

        template<typename KeyT>
        const_iterator lower_bound(KeyT const & key) const noexcept
        {
            using std::begin;
            using std::end;
            auto first = begin(key);
            auto const last = end(key);
            auto match = longest_match_impl(first, last);
            if (match.result_.match) {
                return const_iterator(iter_state_t{match.result_.node->parent_,
                                                   match.index_within_parent_});
            }
            auto node = match.result_.node;
            while (!node->value_) {
                node = node->children_.front().get();
            }
            return const_iterator(iter_state_t{node->parent_, 0});
        }

        template<typename KeyT>
        const_iterator upper_bound(KeyT const & key) const noexcept
        {
            using std::begin;
            using std::end;
            auto first = begin(key);
            auto const last = end(key);
            auto match = longest_match_impl(first, last);
            if (match.result_.match) {
                return ++const_iterator(iter_state_t{
                    match.result_.node->parent_, match.index_within_parent_});
            }
            auto node = match.result_.node;
            while (!node->value_) {
                node = node->children_.front().get();
            }
            return const_iterator(iter_state_t{node->parent_, 0});
        }

        template<typename KeyT>
        const_range equal_range(KeyT const & key) const noexcept
        {
            return {lower_bound(key), upper_bound(key)};
        }

        template<typename KeyIter>
        match_result longest_match(KeyIter first, KeyIter last) const noexcept
        {
            return longest_match_impl(first, last).result_;
        }

        template<typename KeyT>
        match_result longest_match(KeyT const & key) const noexcept
        {
            using std::begin;
            using std::end;
            return longest_match(begin(key), end(key));
        }

        template<typename KeyElementT>
        match_result extend_match(match_result prev, KeyElementT e) const
            noexcept
        {
            auto e_ptr = &e;
            return extend_match_impl(priv_match_result{prev}, e_ptr, e_ptr + 1)
                .result_;
        }

        template<typename KeyIter>
        match_result
        extend_match(match_result prev, KeyIter first, KeyIter last) const
            noexcept
        {
            return extend_match_impl(priv_match_result{prev}, first, last)
                .result_;
        }

        template<typename KeyT>
        optional<mapped_type> operator[](KeyT const & key) const noexcept
        {
            auto it = find(key);
            if (it == end())
                return {};
            return it->value;
        }

        iterator begin() noexcept { return iterator(const_this()->begin()); }
        iterator end() noexcept { return iterator(const_this()->end()); }
        iterator cbegin() noexcept { return begin(); }
        iterator cend() noexcept { return end(); }

        reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }
        reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }
        reverse_iterator crbegin() noexcept { return rbegin(); }
        reverse_iterator crend() noexcept { return rend(); }

        void clear() noexcept
        {
            root_ = node_t();
            size_ = 0;
        }

        template<typename KeyT>
        iterator find(KeyT const & key) noexcept
        {
            return iterator(const_this()->find(key));
        }

        template<typename KeyT>
        iterator lower_bound(KeyT const & key) noexcept
        {
            return iterator(const_this()->lower_bound(key));
        }

        template<typename KeyT>
        iterator upper_bound(KeyT const & key) noexcept
        {
            return iterator(const_this()->upper_bound(key));
        }

        template<typename KeyT>
        range equal_range(KeyT const & key) noexcept
        {
            return {lower_bound(key), upper_bound(key)};
        }

        template<typename KeyT>
        optional<mapped_type &> operator[](KeyT const & key) noexcept
        {
            auto it = find(key);
            if (it == end())
                return {};
            return it->value;
        }

        template<typename KeyIter>
        insert_result insert(KeyIter first, KeyIter last, Value value)
        {
            auto match = longest_match_impl(first, last);
            if (first == last && match.result_.node->value_) {
                return {iterator(iter_state_t{match.result_.node->parent_,
                                              match.index_within_parent_}),
                        false};
            }
            auto node = create_children(
                const_cast<node_t *>(match.result_.node), first, last);
            node->value_ = element{key_type{first, last}, std::move(value)};
            ++size_;
            return {iterator(iter_state_t{node->parent_, 0}), true};
        }
        template<typename KeyRange>
        insert_result insert(KeyRange const & key, Value value)
        {
            using std::begin;
            using std::end;
            return insert(begin(key), end(key), std::move(value));
        }
        insert_result insert(element e)
        {
            using std::begin;
            using std::end;
            return insert(begin(e.key), end(e.key), std::move(e.value));
        }
        template<typename Iter>
        void insert(Iter first, Iter last)
        {
            for (; first != last; ++first) {
                insert(first->key, first->value);
            }
        }
        template<typename Range>
        insert_result insert(Range const & r)
        {
            using std::begin;
            using std::end;
            return insert(begin(r), end(r));
        }
        void insert(std::initializer_list<value_type> il)
        {
            for (auto const & x : il) {
                insert(x);
            }
        }

        template<typename KeyT>
        bool erase(KeyT const & key)
        {
            auto it = find(key);
            if (it == end())
                return false;
            erase(it);
            return true;
        }
        iterator erase(iterator it)
        {
            auto state = it->it_.state_;
            node_children_t * children = &state.parent_->children_;

            auto node = (*children)[state.index_];
            if (!node->children_.empty()) {
                // node has a value and children.  Remove the value and find
                // the next-iterator.
                node->value_.reset();
                while (!node->value_) {
                    node = node->children_.begin();
                    return iterator(iter_state_t{node->parent_, 0});
                }
            }

            // node has a value, but no children.  Remove it and all its
            // singular predecessors.
            children->erase(children->begin() + state.index_);
            --state.index_;
            --size_;
            while (state.parent_ && children->empty()) {
                state = parent_state(state, comp_);
                children = &state.parent_->children_;
                children->erase(children->begin() + state.index_);
                --state.index_;
                --size_;
            }

            auto retval = iterator(state);
            if (state.index_ < children->size())
                ++retval;

            return retval;
        }
        iterator erase(iterator first, iterator last)
        {
            iterator retval = first;
            while (last-- > first) {
                retval = erase(last);
            }
            return retval;
        }

        void swap(trie & other)
        {
            std::swap(root_, other.root_);
            std::swap(size_, other.size_);
            std::swap(comp_, other.comp_);
        }

        friend bool operator==(trie const & lhs, trie const & rhs)
        {
            return lhs.size() == rhs.size() &&
                   std::equal(lhs.begin(), lhs.end(), rhs.begin());
        }
        friend bool operator!=(trie const & lhs, trie const & rhs)
        {
            return !(lhs == rhs);
        }

    private:
        trie const * const_this() { return const_cast<trie const *>(this); }

        struct priv_match_result
        {
            match_result result_;
            std::size_t index_within_parent_;
        };

        template<typename KeyIter>
        priv_match_result
        longest_match_impl(KeyIter & first, KeyIter last) const noexcept
        {
            return extend_match_impl(
                priv_match_result{match_result{&root_, 0}}, first, last);
        }

        template<typename KeyIter>
        priv_match_result extend_match_impl(
            priv_match_result prev, KeyIter & first, KeyIter last) const
            noexcept
        {
            if (first == last)
                return prev;

            node_t const * node = prev.result_.node;
            size_type size = prev.result_.size;
            std::size_t parent_index = 0;
            while (first != last) {
                auto const it = node->lower_bound(*first, comp_);
                if (it == node->children_.end() || comp_((*it)->e_, *first))
                    break;
                ++first;
                ++size;
                node = it->get();
                parent_index = it - node->children_.begin();
            }

            return {match_result{node, size, !!node->value_}, parent_index};
        }

        template<typename KeyIter>
        node_t * create_children(node_t * node, KeyIter first, KeyIter last)
        {
            auto retval = node;
            for (; first != last; ++first) {
                std::unique_ptr<node_t> new_node(new node_t(retval, *first));
                auto at = retval->children_.begin();
                if (retval == node)
                    at = retval->lower_bound(*first, comp_);
                retval =
                    retval->children_.insert(at, std::move(new_node))->get();
            }
            return retval;
        }

        node_t root_;
        key_compare comp_;
        size_type size_;
    };

    template<typename Key, typename Value>
    struct const_trie_iterator
    {
        using value_type = trie_element<Key, Value>;
        using pointer = value_type *;
        using reference = value_type &;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        const_trie_iterator() noexcept : state_{nullptr, 0} {}

        reference operator*() const noexcept
        {
            return *state_.parent_->children_[state_.index_]->value_;
        }

        pointer operator->() const noexcept { return &**this; }

        const_trie_iterator & operator++() noexcept
        {
            ++state_.index_;
            auto const first_state = state_;
            while (state_.parent_->parent_ &&
                   state_.index_ == state_.parent_->children_.size()) {
                state_ = parent_state(state_);
                ++state_.index_;
            }

            // If we went all the way up, incrementing indices, and they were
            // all at size() for each node, this is the end.
            if (!state_.parent_->parent_ &&
                state_.index_ == state_.parent_->children_.size()) {
                state_ = first_state;
                return *this;
            }

            auto node = state_.parent_->children_[state_.index_].get();
            while (!node->value_) {
                auto i = 0u;
                node = node->children_[i].get();
                state_ = state_t{node, i};
            }

            return *this;
        }
        const_trie_iterator operator++(int)noexcept
        {
            auto const retval = *this;
            ++*this;
            return retval;
        }
        const_trie_iterator & operator--() noexcept
        {
            // This crazy underflow behavior makes --my_trie.begin() work, and
            // thus makes my_trie.rend() easy to write.
            --state_.index_;
            auto const first_state = state_;
            while (state_.parent_->parent_ &&
                   state_.parent_->children_.size() <= state_.index_) {
                state_ = parent_state(state_);
                --state_.index_;
            }

            // If we went all the way up, incrementing indices, and they were
            // all at size() for each node, this is one-minus-begin.
            if (!state_.parent_->parent_ &&
                state_.parent_->children_.size() <= state_.index_) {
                state_ = first_state;
                return *this;
            }

            auto node = state_.parent_->children_[state_.index_].get();
            while (!node->value_) {
                auto i = node->children_.size() - 1;
                node = node->children_[i].get();
                state_ = state_t{node, i};
            }

            return *this;
        }
        const_trie_iterator operator--(int)noexcept
        {
            auto const retval = *this;
            --*this;
            return retval;
        }

        friend bool
        operator==(const_trie_iterator lhs, const_trie_iterator rhs) noexcept
        {
            return lhs.state_.parent_ == rhs.state_.parent_ &&
                   lhs.state_.index_ == rhs.state_.index_;
        }
        friend bool
        operator!=(const_trie_iterator lhs, const_trie_iterator rhs) noexcept
        {
            return !(lhs == rhs);
        }

    private:
        using state_t = detail::trie_iterator_state_t<Key, Value>;

        explicit const_trie_iterator(state_t state) : state_(state) {}

        state_t state_;

        template<typename KeyT, typename ValueT, typename Compare>
        friend struct trie;
        template<typename KeyT, typename ValueT>
        friend struct trie_iterator;
    };

    template<typename Key, typename Value>
    struct trie_iterator
    {
        using value_type = trie_element<Key, Value>;
        using pointer = value_type *;
        using reference = value_type &;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        trie_iterator() {}

        reference operator*() const noexcept
        {
            return const_cast<reference>(*it_);
        }

        pointer operator->() const noexcept { return &**this; }

        trie_iterator & operator++() noexcept
        {
            --it_;
            return *this;
        }
        trie_iterator operator++(int)noexcept
        {
            auto const retval = *this;
            ++*this;
            return retval;
        }
        trie_iterator & operator--() noexcept
        {
            ++it_;
            return *this;
        }
        trie_iterator operator--(int)noexcept
        {
            auto const retval = *this;
            --*this;
            return retval;
        }

        friend bool operator==(trie_iterator lhs, trie_iterator rhs) noexcept
        {
            return lhs.it_ == rhs.it_;
        }
        friend bool operator!=(trie_iterator lhs, trie_iterator rhs) noexcept
        {
            return lhs.it_ != lhs.it_;
        }

    private:
        explicit trie_iterator(
            detail::trie_iterator_state_t<Key, Value> state) :
            it_(state)
        {}
        explicit trie_iterator(const_trie_iterator<Key, Value> it) : it_(it) {}

        const_trie_iterator<Key, Value> it_;

        template<typename KeyT, typename ValueT, typename Compare>
        friend struct trie;
    };

    template<typename Key, typename Value>
    struct reverse_trie_iterator
    {
        using value_type = trie_element<Key, Value>;
        using pointer = value_type *;
        using reference = value_type &;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        reverse_trie_iterator() noexcept {}
        explicit reverse_trie_iterator(trie_iterator<Key, Value> it) noexcept :
            it_(--it)
        {}

        reference operator*() const noexcept { return *it_; }

        pointer operator->() const noexcept { return &**this; }

        reverse_trie_iterator & operator++() noexcept
        {
            --it_;
            return *this;
        }
        reverse_trie_iterator operator++(int)noexcept
        {
            auto const retval = *this;
            ++*this;
            return retval;
        }
        reverse_trie_iterator & operator--() noexcept
        {
            ++it_;
            return *this;
        }
        reverse_trie_iterator operator--(int)noexcept
        {
            auto const retval = *this;
            --*this;
            return retval;
        }

        trie_iterator<Key, Value> base() const noexcept { return it_; }

        friend bool operator==(
            reverse_trie_iterator lhs, reverse_trie_iterator rhs) noexcept
        {
            return lhs.it_ == rhs.it_;
        }
        friend bool operator!=(
            reverse_trie_iterator lhs, reverse_trie_iterator rhs) noexcept
        {
            return lhs.it_ != lhs.it_;
        }

    private:
        trie_iterator<Key, Value> it_;
    };

    template<typename Key, typename Value>
    struct const_reverse_trie_iterator
    {
        using value_type = trie_element<Key, Value>;
        using pointer = value_type const *;
        using reference = value_type const &;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        const_reverse_trie_iterator() noexcept {}
        explicit const_reverse_trie_iterator(
            const_trie_iterator<Key, Value> it) noexcept :
            it_(--it)
        {}
        explicit const_reverse_trie_iterator(
            reverse_trie_iterator<Key, Value> it) noexcept :
            it_(it.it_)
        {}

        reference operator*() const noexcept { return *it_; }

        pointer operator->() const noexcept { return &**this; }

        const_reverse_trie_iterator & operator++() noexcept
        {
            --it_;
            return *this;
        }
        const_reverse_trie_iterator operator++(int)noexcept
        {
            auto const retval = *this;
            ++*this;
            return retval;
        }
        const_reverse_trie_iterator & operator--() noexcept
        {
            ++it_;
            return *this;
        }
        const_reverse_trie_iterator operator--(int)noexcept
        {
            auto const retval = *this;
            --*this;
            return retval;
        }

        const_trie_iterator<Key, Value> base() const noexcept { return it_; }

        friend bool operator==(
            const_reverse_trie_iterator lhs,
            const_reverse_trie_iterator rhs) noexcept
        {
            return lhs.it_ == rhs.it_;
        }
        friend bool operator!=(
            const_reverse_trie_iterator lhs,
            const_reverse_trie_iterator rhs) noexcept
        {
            return lhs.it_ != lhs.it_;
        }

    private:
        const_trie_iterator<Key, Value> it_;
    };

    namespace detail {

        template<typename Key, typename Value>
        struct trie_node_t
        {
            using iterator =
                typename trie_node_children_t<Key, Value>::iterator;
            using const_iterator =
                typename trie_node_children_t<Key, Value>::const_iterator;
            using element = typename Key::value_type;

            trie_node_t() : parent_(nullptr) {}
            trie_node_t(trie_node_t const & other) :
                value_(other.value_),
                parent_(other.parent_),
                e_(other.e_)
            {
                children_.reserve(other.children_.size());
                for (auto const & node : other.children_) {
                    std::unique_ptr<trie_node_t> new_node(
                        new trie_node_t(*node));
                    children_.push_back(std::move(new_node));
                }
            }
            trie_node_t(trie_node_t && other) = default;
            trie_node_t & operator=(trie_node_t const & rhs)
            {
                trie_node_t temp(rhs);
                std::swap(temp, *this);
                return *this;
            }

            trie_node_t(trie_node_t * parent, element e) :
                parent_(parent),
                e_(std::move(e))
            {}

            template<typename Compare>
            const_iterator
            lower_bound(element const & e, Compare const & comp) const noexcept
            {
                return std::lower_bound(
                    children_.begin(),
                    children_.end(),
                    e,
                    [&comp](
                        std::unique_ptr<trie_node_t> const & ptr,
                        element const & e) { return comp(ptr->e_, e); });
            }
            template<typename Compare>
            iterator
            lower_bound(element const & e, Compare const & comp) noexcept
            {
                auto const it =
                    const_cast<trie_node_t const &>(*this).lower_bound(e, comp);
                return children_.begin() +
                       (it - const_iterator(children_.begin()));
            }
            template<typename Compare>
            trie_node_t * child(element const & e, Compare const & comp) const
                noexcept
            {
                auto const it = lower_bound(e, comp);
                if (it == children_.end() || comp((*it)->e_, e))
                    return nullptr;
                return it->get();
            }

            template<typename Compare>
            trie_node_t *
            child(element const & e, Compare const & comp) noexcept
            {
                return const_cast<trie_node_t *>(
                    const_cast<trie_node_t const &>(*this).child(e, comp));
            }

            using children_t = trie_node_children_t<Key, Value>;

            children_t children_;
            optional<trie_element<Key, Value>> value_;
            trie_node_t * parent_;
            element e_;

            template<typename KeyT, typename ValueT, typename Compare>
            friend struct trie;
            friend struct const_trie_iterator<Key, Value>;
        };
    }

}}

#endif
