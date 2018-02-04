#ifndef BOOST_TEXT_TRIE_MAP_HPP
#define BOOST_TEXT_TRIE_MAP_HPP

#include <boost/text/trie.hpp>


namespace boost { namespace trie {

    template<typename Key, typename Value>
    struct trie_map_iterator;

    template<typename Key, typename Value>
    struct const_trie_map_iterator;

    template<typename Key, typename Value>
    struct reverse_trie_map_iterator;

    template<typename Key, typename Value>
    struct const_reverse_trie_map_iterator;

    template<typename Key, typename Value>
    struct trie_map_element
    {
        trie_map_element() {}
        trie_map_element(Key k, Value v) : key(k), value(v) {}

        template<typename KeyT, typename ValueT>
        trie_map_element(trie_map_element<KeyT, ValueT> const & rhs) :
            key(rhs.key),
            value(rhs.value)
        {}

        template<typename KeyT, typename ValueT>
        trie_map_element(trie_map_element<KeyT, ValueT> && rhs) :
            key(std::move(rhs.key)),
            value(std::move(rhs.value))
        {}

        template<typename KeyT, typename ValueT>
        trie_map_element & operator=(trie_map_element<KeyT, ValueT> const & rhs)
        {
            key = rhs.key;
            value = rhs.value;
            return *this;
        }

        template<typename KeyT, typename ValueT>
        trie_map_element & operator=(trie_map_element<KeyT, ValueT> && rhs)
        {
            key = std::move(rhs.key);
            value = std::move(rhs.value);
            return *this;
        }

        Key key;
        Value value;

        friend bool
        operator==(trie_map_element const & lhs, trie_map_element const & rhs)
        {
            return lhs.key == rhs.key && lhs.value == rhs.value;
        }
        friend bool
        operator!=(trie_map_element const & lhs, trie_map_element const & rhs)
        {
            return !(lhs == rhs);
        }
    };

    template<typename Iter>
    struct trie_range
    {
        using iterator = Iter;

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

    template<typename Iter>
    struct const_trie_range
    {
        using const_iterator = Iter;

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

    template<typename Iter>
    struct trie_insert_result
    {
        using iterator = Iter;

        trie_insert_result() : iter(), inserted(false) {}
        trie_insert_result(iterator it, bool ins) : iter(it), inserted(ins) {}

        iterator iter;
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

        struct index_within_parent_t
        {
            index_within_parent_t() : value_(-1) {}

            std::size_t value() const noexcept { return value_; }

            template<typename Key, typename Value, typename Iter>
            void insert_at(
                std::unique_ptr<
                    trie_node_t<index_within_parent_t, Key, Value>> const &
                    child,
                std::ptrdiff_t offset,
                Iter it,
                Iter end)
            {
                child->index_within_parent_.value_ = offset;
                for (; it != end; ++it) {
                    ++(*it)->index_within_parent_.value_;
                }
            }

            template<typename Key, typename Value>
            void
            insert_ptr(std::unique_ptr<
                       trie_node_t<index_within_parent_t, Key, Value>> const &
                           child)
            {
                child->index_within_parent_.value_ = 0;
            }

            template<typename Iter>
            void erase(Iter it, Iter end)
            {
                for (; it != end; ++it) {
                    --(*it)->index_within_parent_.value_;
                }
            }

        private:
            std::size_t value_;
        };

        template<typename Key, typename Value>
        struct trie_iterator_state_t
        {
            trie_node_t<index_within_parent_t, Key, Value> const * parent_;
            std::size_t index_;
        };

        template<typename Key, typename Value>
        trie_iterator_state_t<Key, Value>
        parent_state(trie_iterator_state_t<Key, Value> state)
        {
            return {state.parent_->parent(),
                    state.parent_->index_within_parent()};
        }

        template<typename Key, typename Value>
        Key reconstruct_key(trie_iterator_state_t<Key, Value> state)
        {
            Key retval;
            while (state.parent_->parent()) {
                retval.insert(retval.end(), state.parent_->key(state.index_));
                state = parent_state(state);
            }
            std::reverse(retval.begin(), retval.end());
            return retval;
        }

        template<typename Key, typename Value>
        trie_node_t<index_within_parent_t, Key, Value> const *
        to_node(trie_iterator_state_t<Key, Value> state)
        {
            if (state.index_ < state.parent_->size())
                return state.parent_->child(state.index_);
            return nullptr;
        }
    }

    // TODO: KeyIter, KeyRange, and Range concepts.
    // TODO: Key concept specifying that Key is a container.
    // TODO: Compare concept specifying that Compare compares Key::value_types.
    // Don't forget to mention that Compare must be statically polymorphic.
    template<typename Key, typename Value, typename Compare = less>
    struct trie_map
    {
    private:
        using node_t =
            detail::trie_node_t<detail::index_within_parent_t, Key, Value>;
        using iter_state_t = detail::trie_iterator_state_t<Key, Value>;

    public:
        using key_type = Key;
        using mapped_type = Value;
        using value_type = trie_map_element<Key, Value>;
        using key_compare = Compare;
        using key_element_type = typename Key::value_type;

        using reference = value_type &;
        using const_reference = value_type const &;
        using iterator = trie_map_iterator<key_type, mapped_type>;
        using const_iterator = const_trie_map_iterator<key_type, mapped_type>;
        using reverse_iterator =
            reverse_trie_map_iterator<key_type, mapped_type>;
        using const_reverse_iterator =
            const_reverse_trie_map_iterator<key_type, mapped_type>;
        using size_type = std::ptrdiff_t;
        using difference_type = std::ptrdiff_t;

        using range = trie_range<iterator>;
        using const_range = const_trie_range<const_iterator>;
        using insert_result = trie_insert_result<iterator>;
        using match_result = trie_match_result;

        trie_map() : size_(0) {}

        trie_map(Compare const & comp) : size_(0), comp_(comp) {}

        template<typename Iter>
        trie_map(Iter first, Iter last, Compare const & comp = Compare()) :
            size_(0),
            comp_(comp)
        {
            insert(first, last);
        }
        template<typename Range>
        explicit trie_map(Range r, Compare const & comp = Compare()) :
            size_(0),
            comp_(comp)
        {
            using std::begin;
            using std::end;
            insert(begin(r), end(r));
        }
        trie_map(std::initializer_list<value_type> il) : size_(0)
        {
            insert(il);
        }

        trie_map & operator=(std::initializer_list<value_type> il)
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
            iter_state_t state{&header_, 0};
            if (size_) {
                while (!state.parent_->min_value()) {
                    state.parent_ = state.parent_->min_child();
                }
            }
            return const_iterator(state);
        }
        const_iterator end() const noexcept
        {
            iter_state_t state{&header_, 0};
            if (size_) {
                node_t const * node = nullptr;
                while ((node = to_node(state))) {
                    state.parent_ = node;
                    state.index_ = state.parent_->size() - 1;
                }
                state.parent_ = state.parent_->parent();
                state.index_ = state.parent_->size();
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

#define BOOST_TRIE_MAP_C_STR_OVERLOAD(rtype, func, quals)                      \
    template<typename Char, std::size_t N>                                     \
    rtype func(Char const(&chars)[N]) quals                                    \
    {                                                                          \
        static_assert(                                                         \
            std::is_same<Char, key_element_type>::value,                       \
            "Only well-formed when Char is Key::value_type.");                 \
        return func(detail::char_range<Char const>{chars, chars + N - 1});     \
    }

        template<typename KeyRange>
        bool contains(KeyRange const & key) const noexcept
        {
            return find(key) != end();
        }

        BOOST_TRIE_MAP_C_STR_OVERLOAD(bool, contains, const noexcept)

        template<typename KeyRange>
        const_iterator find(KeyRange const & key) const noexcept
        {
            using std::begin;
            using std::end;
            auto first = begin(key);
            auto const last = end(key);
            auto match = longest_match_impl(first, last);
            if (first == last && match.match) {
                return const_iterator(iter_state_t{
                    to_node_ptr(match.node)->parent(),
                    to_node_ptr(match.node)->index_within_parent()});
            }
            return this->end();
        }

        BOOST_TRIE_MAP_C_STR_OVERLOAD(const_iterator, find, const noexcept)

        template<typename KeyRange>
        const_iterator lower_bound(KeyRange const & key) const noexcept
        {
            return bound_impl<true>(key);
        }

        BOOST_TRIE_MAP_C_STR_OVERLOAD(
            const_iterator, lower_bound, const noexcept)

        template<typename KeyRange>
        const_iterator upper_bound(KeyRange const & key) const noexcept
        {
            return bound_impl<false>(key);
        }

        BOOST_TRIE_MAP_C_STR_OVERLOAD(
            const_iterator, upper_bound, const noexcept)

        template<typename KeyRange>
        const_range equal_range(KeyRange const & key) const noexcept
        {
            return {lower_bound(key), upper_bound(key)};
        }

        BOOST_TRIE_MAP_C_STR_OVERLOAD(const_range, equal_range, const noexcept)

        template<typename KeyIter>
        match_result longest_subsequence(KeyIter first, KeyIter last) const
            noexcept
        {
            return longest_match_impl(first, last);
        }

        template<typename KeyRange>
        match_result longest_subsequence(KeyRange const & key) const noexcept
        {
            using std::begin;
            using std::end;
            return longest_subsequence(begin(key), end(key));
        }

        BOOST_TRIE_MAP_C_STR_OVERLOAD(
            match_result, longest_subsequence, const noexcept)

        template<typename KeyIter>
        match_result longest_match(KeyIter first, KeyIter last) const noexcept
        {
            auto retval = longest_match_impl(first, last);
            auto node = to_node_ptr(retval.node);
            while (node->parent() && !node->value()) {
                retval.node = node = node->parent();
                --retval.size;
            }
            if (!!node->value())
                retval.match = true;
            return retval;
        }

        template<typename KeyRange>
        match_result longest_match(KeyRange const & key) const noexcept
        {
            using std::begin;
            using std::end;
            return longest_match(begin(key), end(key));
        }

        BOOST_TRIE_MAP_C_STR_OVERLOAD(
            match_result, longest_match, const noexcept)

        template<typename KeyElementT>
        match_result extend_subsequence(match_result prev, KeyElementT e) const
            noexcept
        {
            auto e_ptr = &e;
            return extend_subsequence_impl(prev, e_ptr, e_ptr + 1);
        }

        template<typename KeyIter>
        match_result
        extend_subsequence(match_result prev, KeyIter first, KeyIter last) const
            noexcept
        {
            return extend_subsequence_impl(prev, first, last);
        }

        template<typename OutIter>
        OutIter copy_next_key_elements(match_result prev, OutIter out) const
        {
            auto node = to_node_ptr(prev.node);
            return std::copy(node->key_begin(), node->key_end(), out);
        }

        template<typename KeyRange>
        optional_ref<mapped_type const> operator[](KeyRange const & key) const
            noexcept
        {
            auto it = find(key);
            if (it == end())
                return {};
            return it->value;
        }

        BOOST_TRIE_MAP_C_STR_OVERLOAD(
            optional_ref<mapped_type const>, operator[], const noexcept)

        iterator begin() noexcept { return iterator(const_this()->begin()); }
        iterator end() noexcept { return iterator(const_this()->end()); }

        reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }
        reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }

        void clear() noexcept
        {
            header_ = node_t();
            size_ = 0;
        }

        template<typename KeyRange>
        iterator find(KeyRange const & key) noexcept
        {
            return iterator(const_this()->find(key));
        }

        BOOST_TRIE_MAP_C_STR_OVERLOAD(iterator, find, noexcept)

        template<typename KeyRange>
        iterator lower_bound(KeyRange const & key) noexcept
        {
            return iterator(const_this()->lower_bound(key));
        }

        BOOST_TRIE_MAP_C_STR_OVERLOAD(iterator, lower_bound, noexcept)

        template<typename KeyRange>
        iterator upper_bound(KeyRange const & key) noexcept
        {
            return iterator(const_this()->upper_bound(key));
        }

        BOOST_TRIE_MAP_C_STR_OVERLOAD(iterator, upper_bound, noexcept)

        template<typename KeyRange>
        range equal_range(KeyRange const & key) noexcept
        {
            return {lower_bound(key), upper_bound(key)};
        }

        BOOST_TRIE_MAP_C_STR_OVERLOAD(range, equal_range, noexcept)

        template<typename KeyRange>
        optional_ref<mapped_type> operator[](KeyRange const & key) noexcept
        {
            auto it = find(key);
            if (it == end())
                return {};
            return it->value;
        }

        BOOST_TRIE_MAP_C_STR_OVERLOAD(
            optional_ref<mapped_type>, operator[], noexcept)

        template<typename KeyIter>
        insert_result insert(KeyIter first, KeyIter last, Value value)
        {
            if (empty()) {
                std::unique_ptr<node_t> new_node(new node_t(&header_));
                header_.insert(std::move(new_node));
            }

            auto match = longest_match_impl(first, last);
            if (first == last && match.match) {
                return {iterator(iter_state_t{
                            to_node_ptr(match.node)->parent(),
                            to_node_ptr(match.node)->index_within_parent()}),
                        false};
            }
            auto node = create_children(
                const_cast<node_t *>(to_node_ptr(match.node)), first, last);
            node->value() = std::move(value);
            ++size_;
            return {iterator(iter_state_t{node->parent(), 0}), true};
        }

        template<typename KeyRange>
        insert_result insert(KeyRange const & key, Value value)
        {
            using std::begin;
            using std::end;
            return insert(begin(key), end(key), std::move(value));
        }

        template<typename Char, std::size_t N>
        insert_result insert(Char const (&chars)[N], Value value)
        {
            static_assert(
                std::is_same<Char, key_element_type>::value,
                "Only well-formed when Char is Key::value_type.");
            return insert(
                detail::char_range<Char const>{chars, chars + N - 1},
                std::move(value));
        }

        insert_result insert(value_type e)
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

        template<typename KeyIter>
        insert_result insert_or_assign(KeyIter first, KeyIter last, Value value)
        {
            auto it = first;
            auto match = longest_match_impl(it, last);
            if (it == last && match.match) {
                const_cast<Value &>(*to_node_ptr(match.node)->value()) =
                    std::move(value);
                return insert_result{
                    iterator(iter_state_t{
                        to_node_ptr(match.node)->parent(),
                        to_node_ptr(match.node)->index_within_parent()}),
                    false};
            }
            return insert(first, last, std::move(value));
        }

        template<typename KeyRange>
        insert_result insert_or_assign(KeyRange const & key, Value value)
        {
            using std::begin;
            using std::end;
            return insert_or_assign(begin(key), end(key), std::move(value));
        }

        template<typename Char, std::size_t N>
        insert_result insert_or_assign(Char const (&chars)[N], Value value)
        {
            static_assert(
                std::is_same<Char, key_element_type>::value,
                "Only well-formed when Char is Key::value_type.");
            return insert_or_assign(
                detail::char_range<Char const>{chars, chars + N - 1},
                std::move(value));
        }

        template<typename KeyRange>
        bool erase(KeyRange const & key) noexcept
        {
            auto it = find(key);
            if (it == end())
                return false;
            erase(it);
            return true;
        }

        BOOST_TRIE_MAP_C_STR_OVERLOAD(bool, erase, noexcept)

        iterator erase(iterator it)
        {
            auto state = it.it_.state_;

            --size_;

            auto node =
                const_cast<node_t *>(state.parent_->child(state.index_));
            if (!node->empty()) {
                // node has a value, but also children.  Remove the value and
                // return the next-iterator.
                ++it;
                node->value() = optional<Value>();
                return it;
            }

            // node has a value, *and* no children.  Remove it and all its
            // singular predecessors.
            const_cast<node_t *>(state.parent_)->erase(state.index_);
            while (state.parent_->parent() && state.parent_->empty() &&
                   !state.parent_->value()) {
                state = parent_state(state);
                const_cast<node_t *>(state.parent_)->erase(state.index_);
            }

            if (state.parent_->parent())
                state = parent_state(state);
            auto retval = iterator(state);
            if (!empty())
                ++retval;

            return retval;
        }
        iterator erase(iterator first, iterator last)
        {
            auto retval = last;
            if (first == last)
                return retval;
            --last;
            while (last != first) {
                erase(last--);
            }
            erase(last);
            return retval;
        }

        void swap(trie_map & other)
        {
            header_.swap(other.header_);
            std::swap(size_, other.size_);
            std::swap(comp_, other.comp_);
        }

        friend bool operator==(trie_map const & lhs, trie_map const & rhs)
        {
            return lhs.size() == rhs.size() &&
                   std::equal(lhs.begin(), lhs.end(), rhs.begin());
        }
        friend bool operator!=(trie_map const & lhs, trie_map const & rhs)
        {
            return !(lhs == rhs);
        }

    private:
        trie_map const * const_this()
        {
            return const_cast<trie_map const *>(this);
        }
        static node_t const * to_node_ptr(void const * ptr)
        {
            return static_cast<node_t const *>(ptr);
        }

        template<typename KeyIter>
        match_result longest_match_impl(KeyIter & first, KeyIter last) const
            noexcept
        {
            return extend_subsequence_impl(
                match_result{&header_, 0, false, true}, first, last);
        }

        template<typename KeyIter>
        match_result extend_subsequence_impl(
            match_result prev, KeyIter & first, KeyIter last) const noexcept
        {
            if (to_node_ptr(prev.node) == &header_) {
                if (header_.empty())
                    return prev;
                prev.node = header_.child(0);
            }

            if (first == last) {
                prev.match = !!to_node_ptr(prev.node)->value();
                prev.leaf = to_node_ptr(prev.node)->empty();
                return prev;
            }

            node_t const * node = to_node_ptr(prev.node);
            size_type size = prev.size;
            while (first != last) {
                auto const it = node->find(*first, comp_);
                if (it == node->end())
                    break;
                ++first;
                ++size;
                node = it->get();
            }

            return match_result{node, size, !!node->value(), node->empty()};
        }

        template<typename KeyIter>
        node_t * create_children(node_t * node, KeyIter first, KeyIter last)
        {
            auto retval = node;
            for (; first != last; ++first) {
                std::unique_ptr<node_t> new_node(new node_t(retval));
                retval =
                    retval->insert(*first, comp_, std::move(new_node))->get();
            }
            return retval;
        }

        template<bool LowerBound, typename KeyRange>
        const_iterator bound_impl(KeyRange const & key) const noexcept
        {
            using std::begin;
            using std::end;
            auto first = begin(key);
            auto const last = end(key);
            auto match = longest_match_impl(first, last);
            if (first == last && match.match) {
                auto retval = const_iterator(iter_state_t{
                    to_node_ptr(match.node)->parent(),
                    to_node_ptr(match.node)->index_within_parent()});
                if (!LowerBound)
                    ++retval;
                return retval;
            }

            auto node = to_node_ptr(match.node);
            if (node->before_child_subtree(*first)) {
                // If the next element of the key would be before this node's
                // children, use the successor of this node; let
                // const_iterator::operator++() figure out for us which node
                // that is.
                return ++const_iterator(
                    iter_state_t{node->parent(), node->index_within_parent()});
            }

            auto const it = node->lower_bound(*first, comp_);
            if (it == node->end()) {
                // Find the max value in this subtree, and go one past that.
                do {
                    node = to_node_ptr(node->max_child());
                } while (!node->value());
                return ++const_iterator(
                    iter_state_t{node->parent(), node->parent()->size() - 1});
            }

            // Otherwise, find the min value within the child found above.
            std::size_t parent_index = it - node->begin();
            node = to_node_ptr(it->get());
            while (!node->value()) {
                node = to_node_ptr(node->min_child());
                parent_index = 0;
            }
            return const_iterator(iter_state_t{node->parent(), parent_index});
        }

        node_t header_;
        size_type size_;
        key_compare comp_;
    };

    template<typename Key, typename Compare>
    struct trie_set;

    template<typename Key>
    struct const_trie_set_iterator;

    namespace detail {
        template<typename Key, typename Value>
        struct arrow_proxy
        {
            trie_map_element<Key, Value &> * operator->() const noexcept
            {
                return &value_;
            }

        private:
            friend struct const_trie_map_iterator<
                Key,
                typename std::remove_const<Value>::type>;
            friend struct trie_map_iterator<
                Key,
                typename std::remove_const<Value>::type>;

            arrow_proxy(Key && key, Value & value) :
                value_{std::move(key), value}
            {}

            mutable trie_map_element<Key, Value &> value_;
        };
    }

    template<typename Key, typename Value>
    struct const_trie_map_iterator
    {
        using value_type = trie_map_element<Key, Value>;
        using pointer = detail::arrow_proxy<Key, Value const>;
        using reference = trie_map_element<Key, Value const &>;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        const_trie_map_iterator() noexcept : state_{nullptr, 0} {}

        const_trie_map_iterator(trie_match_result match_result) noexcept
        {
            assert(match_result.node);
            assert(match_result.match);
            auto node = static_cast<detail::trie_node_t<
                detail::index_within_parent_t,
                Key,
                Value> const *>(match_result.node);
            state_.parent_ = node->parent();
            state_.index_ = node->index_within_parent();
        }

        reference operator*() const noexcept
        {
            return reference{detail::reconstruct_key(state_),
                             state_.parent_->child_value(state_.index_)};
        }

        pointer operator->() const noexcept
        {
            reference && deref_result = **this;
            return pointer(std::move(deref_result.key), deref_result.value);
        }

        const_trie_map_iterator & operator++() noexcept
        {
            auto node = to_node(state_);
            if (node && !node->empty()) {
                state_.parent_ = node;
                state_.index_ = 0;
            } else {
                // Try the next sibling node.
                ++state_.index_;
                auto const first_state = state_;
                while (state_.parent_->parent() &&
                       state_.parent_->parent()->parent() &&
                       state_.parent_->size() <= state_.index_) {
                    state_ = parent_state(state_);
                    ++state_.index_;
                }

                // If we went all the way up, incrementing indices, and they
                // were all at size() for each node, the first increment above
                // must have taken us to the end; use that.
                if ((!state_.parent_->parent() ||
                     !state_.parent_->parent()->parent()) &&
                    state_.parent_->size() <= state_.index_) {
                    state_ = first_state;
                    return *this;
                }
            }

            node = state_.parent_->child(state_.index_);
            while (!node->value()) {
                auto i = 0u;
                node = node->child(i);
                state_ = state_t{node->parent(), i};
            }

            return *this;
        }
        const_trie_map_iterator operator++(int)noexcept
        {
            auto const retval = *this;
            ++*this;
            return retval;
        }
        const_trie_map_iterator & operator--() noexcept
        {
            // Decrement-from-end case.
            if (state_.index_ == state_.parent_->size()) {
                --state_.index_;
                return *this;
            }

            // Back up one node at a time until we find an ancestor with a
            // value or a previous sibling.
            while (state_.parent_->parent() && state_.index_ == 0) {
                state_ = parent_state(state_);
                if (state_.parent_->child(state_.index_)->value())
                    return *this;
            }

            // If we get found no value above, go down the maximum subtree of
            // the previous sibling.
            if (state_.index_)
                --state_.index_;
            auto node = state_.parent_->child(state_.index_);
            while (!node->empty()) {
                auto i = node->size() - 1;
                node = node->child(i);
                state_ = state_t{node->parent(), i};
            }

            return *this;
        }
        const_trie_map_iterator operator--(int)noexcept
        {
            auto const retval = *this;
            --*this;
            return retval;
        }

        friend bool operator==(
            const_trie_map_iterator lhs, const_trie_map_iterator rhs) noexcept
        {
            return lhs.state_.parent_ == rhs.state_.parent_ &&
                   lhs.state_.index_ == rhs.state_.index_;
        }
        friend bool operator!=(
            const_trie_map_iterator lhs, const_trie_map_iterator rhs) noexcept
        {
            return !(lhs == rhs);
        }

    private:
        using state_t = detail::trie_iterator_state_t<Key, Value>;

        explicit const_trie_map_iterator(state_t state) : state_(state) {}

        state_t state_;

        template<typename KeyT, typename ValueT, typename Compare>
        friend struct trie_map;
        template<typename KeyT, typename Compare>
        friend struct trie_set;
        template<typename KeyT, typename ValueT>
        friend struct trie_map_iterator;
        template<typename KeyT>
        friend struct const_trie_set_iterator;
    };

    template<typename Key, typename Value>
    struct trie_map_iterator
    {
        using value_type = trie_map_element<Key, Value>;
        using pointer = detail::arrow_proxy<Key, Value>;
        using reference = trie_map_element<Key, Value &>;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        trie_map_iterator() {}

        reference operator*() const noexcept
        {
            return reference{
                detail::reconstruct_key(it_.state_),
                it_.state_.parent_->child_value(it_.state_.index_)};
        };

        pointer operator->() const noexcept
        {
            reference && deref_result = **this;
            return pointer(std::move(deref_result.key), deref_result.value);
        }

        trie_map_iterator & operator++() noexcept
        {
            ++it_;
            return *this;
        }
        trie_map_iterator operator++(int)noexcept
        {
            auto const retval = *this;
            ++*this;
            return retval;
        }
        trie_map_iterator & operator--() noexcept
        {
            --it_;
            return *this;
        }
        trie_map_iterator operator--(int)noexcept
        {
            auto const retval = *this;
            --*this;
            return retval;
        }

        friend bool
        operator==(trie_map_iterator lhs, trie_map_iterator rhs) noexcept
        {
            return lhs.it_ == rhs.it_;
        }
        friend bool
        operator!=(trie_map_iterator lhs, trie_map_iterator rhs) noexcept
        {
            return lhs.it_ != rhs.it_;
        }

    private:
        explicit trie_map_iterator(
            detail::trie_iterator_state_t<Key, Value> state) :
            it_(state)
        {}
        explicit trie_map_iterator(const_trie_map_iterator<Key, Value> it) :
            it_(it)
        {}

        const_trie_map_iterator<Key, Value> it_;

        template<typename KeyT, typename ValueT, typename Compare>
        friend struct trie_map;
        template<typename KeyT, typename Compare>
        friend struct trie_set;
    };

    template<typename Key, typename Value>
    struct reverse_trie_map_iterator
    {
        using value_type = trie_map_element<Key, Value>;
        using pointer = detail::arrow_proxy<Key, Value>;
        using reference = trie_map_element<Key, Value &>;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        reverse_trie_map_iterator() noexcept {}
        explicit reverse_trie_map_iterator(
            trie_map_iterator<Key, Value> it) noexcept :
            it_(it)
        {}

        reference operator*() const noexcept { return *std::prev(it_); }

        pointer operator->() const noexcept
        {
            return std::prev(it_).operator->();
        }

        reverse_trie_map_iterator & operator++() noexcept
        {
            --it_;
            return *this;
        }
        reverse_trie_map_iterator operator++(int)noexcept
        {
            auto const retval = *this;
            --*this;
            return retval;
        }
        reverse_trie_map_iterator & operator--() noexcept
        {
            ++it_;
            return *this;
        }
        reverse_trie_map_iterator operator--(int)noexcept
        {
            auto const retval = *this;
            ++*this;
            return retval;
        }

        trie_map_iterator<Key, Value> base() const noexcept { return it_; }

        friend bool operator==(
            reverse_trie_map_iterator lhs,
            reverse_trie_map_iterator rhs) noexcept
        {
            return lhs.it_ == rhs.it_;
        }
        friend bool operator!=(
            reverse_trie_map_iterator lhs,
            reverse_trie_map_iterator rhs) noexcept
        {
            return lhs.it_ != rhs.it_;
        }

    private:
        trie_map_iterator<Key, Value> it_;
    };

    template<typename Key, typename Value>
    struct const_reverse_trie_map_iterator
    {
        using value_type = trie_map_element<Key, Value>;
        using pointer = detail::arrow_proxy<Key, Value const>;
        using reference = trie_map_element<Key, Value const &>;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        const_reverse_trie_map_iterator() noexcept {}
        explicit const_reverse_trie_map_iterator(
            const_trie_map_iterator<Key, Value> it) noexcept :
            it_(it)
        {}
        explicit const_reverse_trie_map_iterator(
            reverse_trie_map_iterator<Key, Value> it) noexcept :
            it_(it.it_)
        {}

        reference operator*() const noexcept { return *std::prev(it_); }

        pointer operator->() const noexcept
        {
            return std::prev(it_).operator->();
        }

        const_reverse_trie_map_iterator & operator++() noexcept
        {
            --it_;
            return *this;
        }
        const_reverse_trie_map_iterator operator++(int)noexcept
        {
            auto const retval = *this;
            --*this;
            return retval;
        }
        const_reverse_trie_map_iterator & operator--() noexcept
        {
            ++it_;
            return *this;
        }
        const_reverse_trie_map_iterator operator--(int)noexcept
        {
            auto const retval = *this;
            ++*this;
            return retval;
        }

        const_trie_map_iterator<Key, Value> base() const noexcept
        {
            return it_;
        }

        friend bool operator==(
            const_reverse_trie_map_iterator lhs,
            const_reverse_trie_map_iterator rhs) noexcept
        {
            return lhs.it_ == rhs.it_;
        }
        friend bool operator!=(
            const_reverse_trie_map_iterator lhs,
            const_reverse_trie_map_iterator rhs) noexcept
        {
            return lhs.it_ != rhs.it_;
        }

    private:
        const_trie_map_iterator<Key, Value> it_;
    };

}}

#endif
