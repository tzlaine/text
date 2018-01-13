#ifndef BOOST_TEXT_TRIE_HPP
#define BOOST_TEXT_TRIE_HPP

#include <memory>
#include <type_traits>
#include <vector>


namespace boost { namespace trie {

    // TODO: KeyIter concept.

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
        Key const key;
        Value value;
    };

    template<typename Key, typename Value>
    struct trie_range
    {
        using iterator = trie_iterator<Key, Value>;

        iterator first;
        iterator last;

        iterator begin() const { return first; }
        iterator end() const { return last; }
    };

    template<typename Key, typename Value>
    struct const_trie_range
    {
        using const_iterator = const_trie_iterator<Key, Value>;

        const_iterator first;
        const_iterator last;

        const_iterator begin() const { return first; }
        const_iterator end() const { return last; }
    };

    template<typename Key, typename Value>
    struct trie_insert_result
    {
        trie_iterator<Key, Value> iter;
        bool inserted;
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
            auto const it = new_parent->lower_bound(state.parent_->e_);
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

        trie(Compare const & comp) : size_(0), comp_(comp) {}

        template<typename Iter>
        trie(Iter first, Iter last, Compare const & comp = Compare()) :
            size_(0),
            comp_(comp)
        {
            insert(first, last);
        }
        template<typename Range>
        explicit trie(Range r, Compare const & comp = Compare()) :
            size_(0),
            comp_(comp)
        {
            using std::begin;
            using std::end;
            insert(begin(r), end(r));
        }
        trie(std::initializer_list<value_type> il) : size_(0) { insert(il); }
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
        }

        bool empty() const noexcept { return size_ == 0; }
        size_type size() const noexcept { return size_; }
        size_type max_size() const noexcept { return PTRDIFF_MAX; }

        const_iterator begin() const noexcept
        {
            iter_state_t state{&root_, 0};
            if (size_) {
                while (!state.root_->children_.front()->value_) {
                    state.root_ = state.root_->children_.front().get();
                }
            }
            return const_iterator(state);
        }
        const_iterator end() const noexcept
        {
            iter_state_t state{&root_, root_.children_.size()};
            if (size_) {
                while (!state.root_->children_.back()->value_) {
                    state.index_ = state.root_->children_.size() - 1;
                    state.root_ = state.root_->children_.back().get();
                }
            }
            return const_iterator(state);
        }
        const_iterator cbegin() const noexcept { return begin(); }
        const_iterator cend() const noexcept { return end(); }

        const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator{begin()};
        }
        const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator{rend()};
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
                node = node->children_.front();
            }
            return const_iterator(iter_state_t{node->parent_, 0});
        }

        template<typename KeyT>
        range equal_range(KeyT const & key) const noexcept
        {
            return {lower_bound(key), upper_bound(key)};
        }

        optional<mapped_type> operator[](key_type const & k) const noexcept
        {
            auto it = find(k);
            if (it == end())
                return {};
            return it->value;
        }

        void clear() noexcept
        {
            root_ = node_t();
            size_ = 0;
        }

        template<typename KeyT>
        iterator find(KeyT const & key) noexcept
        {
            return iterator(const_cast<trie const &>(*this).find(key));
        }

        template<typename KeyT>
        iterator lower_bound(KeyT const & key) noexcept
        {
            return iterator(const_cast<trie const &>(*this).lower_bound(key));
        }

        template<typename KeyT>
        iterator upper_bound(KeyT const & key) noexcept
        {
            return iterator(const_cast<trie const &>(*this).upper_bound(key));
        }

        template<typename KeyT>
        const_range equal_range(KeyT const & key) noexcept
        {
            return {lower_bound(key), upper_bound(key)};
        }

        template<typename KeyIter>
        insert_result insert(KeyIter first, KeyIter last, Value value)
        {
            auto match = longest_match_impl(first, last);
            if (first == last && match.node->value_) {
                return {iterator(iter_state_t{match.result_.node->parent_,
                                              match.index_within_parent_}),
                        false};
            }
            auto node = create_children(
                const_cast<node_t *>(match.result_.node), first, last);
            node->value_ = element{mapped_type{first, last}, std::move(value)};
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
            children->erase(children->begin() + state.index_);
            --state.index_;
            --size_;
            while (state.parent_ && children->empty()) {
                state = parent_state(state);
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

        template<typename KeyIter>
        match_result longest_match(KeyIter first, KeyIter last)
        {
            return longest_match_impl(first, last).result_;
        }

        template<typename KeyT>
        match_result longest_match(KeyT const & key)
        {
            using std::begin;
            using std::end;
            return longest_match(begin(key), end(key)).result_;
        }

        template<typename KeyElementT>
        match_result extend_match(match_result prev, KeyElementT e)
        {
            return extend_match_impl(priv_match_result{prev}, &e, &e + 1)
                .result_;
        }

        template<typename KeyIter>
        match_result
        extend_match(match_result prev, KeyIter first, KeyIter last)
        {
            return extend_match_impl(priv_match_result{prev}, first, last)
                .result_;
        }

    private:
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
        node_t * create_children(
            node_t * node,
            std::size_t index_within_node,
            KeyIter first,
            KeyIter last)
        {
            auto retval = node;
            for (; first != last; ++first) {
                std::unique_ptr<node_t> new_node(new node_t(*first));
                auto at = retval->children_.begin();
                if (retval == node)
                    at = retval->lower_bound(*first, comp_);
                retval =
                    retval->children_.insert(at, std::move(new_node))->get();
                index_within_node = 0;
            }
            return node;
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
        using iterator_category = std::bidirectional_iterator_tag;

        const_trie_iterator() noexcept : state_{nullptr, 0} {}

        reference operator*() const noexcept
        {
            return *state_.parent_->children_[state_.index_]->value_;
        }

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
            if (!state_.parent->parent_ &&
                state_.index_ == state_.parent_->children_.size()) {
                state_ = first_state;
                return *this;
            }

            auto node = state_.parent_->children_[state_.index_];
            while (!node->value_) {
                auto i = 0;
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
            if (!state_.parent->parent_ &&
                state_.parent_->children_.size() <= state_.index_) {
                state_ = first_state;
                return *this;
            }

            auto node = state_.parent_->children_[state_.index_];
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
    };

    template<typename Key, typename Value>
    struct trie_iterator
    {
        using value_type = trie_element<Key, Value>;
        using pointer = value_type *;
        using reference = value_type &;
        using iterator_category = std::bidirectional_iterator_tag;

        trie_iterator() {}

        reference operator*() const noexcept
        {
            return const_cast<reference>(*it_);
        }

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
        using iterator_category = std::bidirectional_iterator_tag;

        reverse_trie_iterator() noexcept {}
        explicit reverse_trie_iterator(trie_iterator<Key, Value> it) noexcept :
            it_(--it)
        {}

        reference operator*() const noexcept { return *it_; }

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

            trie_node_t(trie_node_t * parent, element e) :
                parent_(parent),
                e_(std::move(e))
            {}
            trie_node_t(
                trie_node_t * parent,
                element e,
                trie_element<Key, Value> value) :
                value_(std::move(value)),
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
            trie_node_t *
            child(element const & e, Compare const & comp) const noexcept
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
