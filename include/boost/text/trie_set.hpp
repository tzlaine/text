#ifndef BOOST_TEXT_TRIE_SET_HPP
#define BOOST_TEXT_TRIE_SET_HPP

#include <boost/text/trie_map.hpp>


namespace boost { namespace trie {

    template<typename Key>
    struct trie_set_iterator;

    template<typename Key>
    struct const_trie_set_iterator;

    template<typename Key>
    struct reverse_trie_set_iterator;

    template<typename Key>
    struct const_reverse_trie_set_iterator;

    template<typename Key, typename Compare = less>
    struct trie_set
    {
    private:
        using trie_map_t = trie_map<Key, detail::void_>;
        using iter_state_t = detail::trie_iterator_state_t<Key, detail::void_>;

        trie_map_t trie_;

    public:
        using key_type = Key;
        using value_type = Key;
        using key_compare = Compare;
        using key_element_type = typename Key::value_type;

        using reference = value_type &;
        using const_reference = value_type const &;
        using iterator = trie_set_iterator<key_type>;
        using const_iterator = const_trie_set_iterator<key_type>;
        using reverse_iterator = reverse_trie_set_iterator<key_type>;
        using const_reverse_iterator =
            const_reverse_trie_set_iterator<key_type>;
        using size_type = std::ptrdiff_t;
        using difference_type = std::ptrdiff_t;

        using range = trie_range<iterator>;
        using const_range = const_trie_range<const_iterator>;
        using insert_result = trie_insert_result<iterator>;
        using match_result = trie_match_result;

        trie_set() : trie_() {}

        trie_set(Compare const & comp) : trie_(comp) {}

        template<typename Iter>
        trie_set(Iter first, Iter last, Compare const & comp = Compare()) :
            trie_(comp)
        {
            insert(first, last);
        }
        template<typename Range>
        explicit trie_set(Range r, Compare const & comp = Compare()) :
            trie_(comp)
        {
            using std::begin;
            using std::end;
            insert(begin(r), end(r));
        }
        trie_set(std::initializer_list<value_type> il) : trie_() { insert(il); }

        trie_set & operator=(std::initializer_list<value_type> il)
        {
            clear();
            for (auto const & x : il) {
                insert(x);
            }
            return *this;
        }

        bool empty() const noexcept { return trie_.empty(); }
        size_type size() const noexcept { return trie_.size(); }
        size_type max_size() const noexcept { return trie_.max_size(); }

        const_iterator begin() const noexcept
        {
            return const_iterator(trie_.begin().state_);
        }
        const_iterator end() const noexcept
        {
            return const_iterator(trie_.end().state_);
        }
        const_iterator cbegin() const noexcept
        {
            return const_iterator(trie_.begin().state_);
        }
        const_iterator cend() const noexcept
        {
            return const_iterator(trie_.end().state_);
        }

        const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator(trie_.rbegin().state_);
        }
        const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator(trie_.rend().state_);
        }
        const_reverse_iterator crbegin() const noexcept
        {
            return const_reverse_iterator(trie_.rbegin().state_);
        }
        const_reverse_iterator crend() const noexcept
        {
            return const_reverse_iterator(trie_.rend().state_);
        }

#define BOOST_TRIE_SET_C_STR_OVERLOAD(rtype, func, quals)                      \
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
            return trie_.contains(key);
        }

        BOOST_TRIE_SET_C_STR_OVERLOAD(bool, contains, const noexcept)

        template<typename KeyRange>
        const_iterator find(KeyRange const & key) const noexcept
        {
            return const_iterator(trie_.find(key).state_);
        }

        BOOST_TRIE_SET_C_STR_OVERLOAD(const_iterator, find, const noexcept)

        template<typename KeyRange>
        const_iterator lower_bound(KeyRange const & key) const noexcept
        {
            return const_iterator(trie_.lower_bound(key).state_);
        }

        BOOST_TRIE_SET_C_STR_OVERLOAD(
            const_iterator, lower_bound, const noexcept)

        template<typename KeyRange>
        const_iterator upper_bound(KeyRange const & key) const noexcept
        {
            return const_iterator(trie_.upper_bound(key).state_);
        }

        BOOST_TRIE_SET_C_STR_OVERLOAD(
            const_iterator, upper_bound, const noexcept)

        template<typename KeyRange>
        const_range equal_range(KeyRange const & key) const noexcept
        {
            return {lower_bound(key), upper_bound(key)};
        }

        BOOST_TRIE_SET_C_STR_OVERLOAD(const_range, equal_range, const noexcept)

        template<typename KeyIter>
        match_result longest_subsequence(KeyIter first, KeyIter last) const
            noexcept
        {
            return trie_.longest_subsequence(first, last);
        }

        template<typename KeyRange>
        match_result longest_subsequence(KeyRange const & key) const noexcept
        {
            return trie_.longest_subsequence(key);
        }

        BOOST_TRIE_SET_C_STR_OVERLOAD(
            match_result, longest_subsequence, const noexcept)

        template<typename KeyIter>
        match_result longest_match(KeyIter first, KeyIter last) const noexcept
        {
            return trie_.longest_match(first, last);
        }

        template<typename KeyRange>
        match_result longest_match(KeyRange const & key) const noexcept
        {
            return trie_.longest_match(key);
        }

        BOOST_TRIE_SET_C_STR_OVERLOAD(
            match_result, longest_match, const noexcept)

        template<typename KeyElementT>
        match_result extend_subsequence(match_result prev, KeyElementT e) const
            noexcept
        {
            return trie_.extend_subsequence(prev, e);
        }

        template<typename KeyIter>
        match_result
        extend_subsequence(match_result prev, KeyIter first, KeyIter last) const
            noexcept
        {
            return trie_.extend_subsequence(prev, first, last);
        }

        template<typename OutIter>
        OutIter copy_next_key_elements(match_result prev, OutIter out) const
        {
            return trie_.copy_next_key_elements(prev, out);
        }

        iterator begin() noexcept { return iterator(const_this()->begin()); }
        iterator end() noexcept { return iterator(const_this()->end()); }

        reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
        reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

        void clear() noexcept { trie_.clear(); }

        template<typename KeyRange>
        iterator find(KeyRange const & key) noexcept
        {
            return iterator(const_this()->find(key));
        }

        BOOST_TRIE_SET_C_STR_OVERLOAD(iterator, find, noexcept)

        template<typename KeyRange>
        iterator lower_bound(KeyRange const & key) noexcept
        {
            return iterator(const_this()->lower_bound(key));
        }

        BOOST_TRIE_SET_C_STR_OVERLOAD(iterator, lower_bound, noexcept)

        template<typename KeyRange>
        iterator upper_bound(KeyRange const & key) noexcept
        {
            return iterator(const_this()->upper_bound(key));
        }

        BOOST_TRIE_SET_C_STR_OVERLOAD(iterator, upper_bound, noexcept)

        template<typename KeyRange>
        range equal_range(KeyRange const & key) noexcept
        {
            return {lower_bound(key), upper_bound(key)};
        }

        BOOST_TRIE_SET_C_STR_OVERLOAD(range, equal_range, noexcept)

        template<typename KeyIter>
        auto insert(KeyIter first, KeyIter last) -> decltype(
            translate_insert_result(trie_.insert(first, last, detail::void_{})))
        {
            auto const trie_result = trie_.insert(first, last, detail::void_{});
            return translate_insert_result(trie_result);
        }

        template<typename KeyRange>
        insert_result insert(KeyRange const & key)
        {
            using std::begin;
            using std::end;
            auto const trie_result =
                trie_.insert(begin(key), end(key), detail::void_{});
            return translate_insert_result(trie_result);
        }

        BOOST_TRIE_SET_C_STR_OVERLOAD(insert_result, insert, /**/)

        insert_result insert(Key const & key)
        {
            using std::begin;
            using std::end;
            return insert(begin(key), end(key));
        }
        template<typename Iter>
        auto insert(Iter first, Iter last)
            -> decltype(trie_.insert(first, last))
        {
            trie_.insert(first, last);
        }
#if 0 // TODO: SFINAE required to disambiguate from insert(KeyRange).
        template<typename Range>
        insert_result insert(Range const & r)
        {
            using std::begin;
            using std::end;
            trie_.insert(begin(r), end(r));
        }
#endif

        void insert(std::initializer_list<value_type> il)
        {
            for (auto const & x : il) {
                insert(x);
            }
        }

        template<typename KeyRange>
        bool erase(KeyRange const & key) noexcept
        {
            return trie_.erase(key);
        }

        BOOST_TRIE_SET_C_STR_OVERLOAD(bool, erase, noexcept)

        iterator erase(iterator it)
        {
            auto const trie_it = typename trie_map_t::iterator(it.state_);
            return iterator(trie_.erase(trie_it).state_);
        }
        iterator erase(iterator first, iterator last)
        {
            auto const trie_first = typename trie_map_t::iterator(first.state_);
            auto const trie_last = typename trie_map_t::iterator(last.state_);
            return iterator(trie_.erase(trie_first, trie_last).state_);
        }

        void swap(trie_set & other) { trie_.swap(other.trie_); }

        friend bool operator==(trie_set const & lhs, trie_set const & rhs)
        {
            return lhs.trie_ == rhs.trie_;
        }
        friend bool operator!=(trie_set const & lhs, trie_set const & rhs)
        {
            return !(lhs == rhs);
        }

    private:
        trie_set const * const_this()
        {
            return const_cast<trie_set const *>(this);
        }

        insert_result
        translate_insert_result(typename trie_map_t::insert_result trie_result)
        {
            return insert_result{iterator(trie_result.iter.it_.state_),
                                 trie_result.inserted};
        }
    };

    namespace detail {
        template<typename Key>
        struct set_arrow_proxy
        {
            Key * operator->() const noexcept { return &key_; }

        private:
            friend struct const_trie_set_iterator<Key>;
            friend struct trie_set_iterator<Key>;

            set_arrow_proxy(Key && key) : key_{std::move(key)} {}

            mutable Key key_;
        };
    }

    template<typename Key>
    struct const_trie_set_iterator
    {
        using value_type = Key;
        using pointer = detail::set_arrow_proxy<Key>;
        using reference = Key;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        const_trie_set_iterator() noexcept : it_() {}

        const_trie_set_iterator(trie_match_result match_result) noexcept :
            it_(match_result)
        {}

        reference operator*() const noexcept
        {
            return detail::reconstruct_key(it_.state_);
        }

        pointer operator->() const noexcept { return pointer(**this); }

        const_trie_set_iterator & operator++() noexcept
        {
            ++it_;
            return *this;
        }
        const_trie_set_iterator operator++(int)noexcept
        {
            auto const retval = *this;
            ++*this;
            return retval;
        }
        const_trie_set_iterator & operator--() noexcept
        {
            --it_;
            return *this;
        }
        const_trie_set_iterator operator--(int)noexcept
        {
            auto const retval = *this;
            --*this;
            return retval;
        }

        friend bool operator==(
            const_trie_set_iterator lhs, const_trie_set_iterator rhs) noexcept
        {
            return lhs.it_ == rhs.it_;
        }
        friend bool operator!=(
            const_trie_set_iterator lhs, const_trie_set_iterator rhs) noexcept
        {
            return !(lhs == rhs);
        }

    private:
        using state_t = detail::trie_iterator_state_t<Key, detail::void_>;

        explicit const_trie_set_iterator(state_t state) : it_(state) {}

        const_trie_map_iterator<Key, detail::void_> it_;

        template<typename KeyT, typename Compare>
        friend struct trie_set;
        template<typename KeyT>
        friend struct trie_set_iterator;
    };

    template<typename Key>
    struct trie_set_iterator
    {
        using value_type = Key;
        using pointer = detail::set_arrow_proxy<Key>;
        using reference = Key;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        trie_set_iterator() {}

        reference operator*() const noexcept { return *it_; }

        pointer operator->() const noexcept { return pointer(**this); }

        trie_set_iterator & operator++() noexcept
        {
            ++it_;
            return *this;
        }
        trie_set_iterator operator++(int)noexcept
        {
            auto const retval = *this;
            ++*this;
            return retval;
        }
        trie_set_iterator & operator--() noexcept
        {
            --it_;
            return *this;
        }
        trie_set_iterator operator--(int)noexcept
        {
            auto const retval = *this;
            --*this;
            return retval;
        }

        friend bool
        operator==(trie_set_iterator lhs, trie_set_iterator rhs) noexcept
        {
            return lhs.it_ == rhs.it_;
        }
        friend bool
        operator!=(trie_set_iterator lhs, trie_set_iterator rhs) noexcept
        {
            return lhs.it_ != rhs.it_;
        }

    private:
        explicit trie_set_iterator(
            detail::trie_iterator_state_t<Key, detail::void_> state) :
            it_(state)
        {}
        explicit trie_set_iterator(const_trie_set_iterator<Key> it) : it_(it) {}

        const_trie_set_iterator<Key> it_;

        template<typename KeyT, typename Compare>
        friend struct trie_set;
    };

    template<typename Key>
    struct reverse_trie_set_iterator
    {
        using value_type = Key;
        using pointer = detail::set_arrow_proxy<Key>;
        using reference = Key;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        reverse_trie_set_iterator() noexcept {}
        explicit reverse_trie_set_iterator(trie_set_iterator<Key> it) noexcept :
            it_(it)
        {}

        reference operator*() const noexcept { return *std::prev(it_); }

        pointer operator->() const noexcept
        {
            return std::prev(it_).operator->();
        }

        reverse_trie_set_iterator & operator++() noexcept
        {
            --it_;
            return *this;
        }
        reverse_trie_set_iterator operator++(int)noexcept
        {
            auto const retval = *this;
            --*this;
            return retval;
        }
        reverse_trie_set_iterator & operator--() noexcept
        {
            ++it_;
            return *this;
        }
        reverse_trie_set_iterator operator--(int)noexcept
        {
            auto const retval = *this;
            ++*this;
            return retval;
        }

        trie_set_iterator<Key> base() const noexcept { return it_; }

        friend bool operator==(
            reverse_trie_set_iterator lhs,
            reverse_trie_set_iterator rhs) noexcept
        {
            return lhs.it_ == rhs.it_;
        }
        friend bool operator!=(
            reverse_trie_set_iterator lhs,
            reverse_trie_set_iterator rhs) noexcept
        {
            return lhs.it_ != rhs.it_;
        }

    private:
        trie_set_iterator<Key> it_;
    };

    template<typename Key>
    struct const_reverse_trie_set_iterator
    {
        using value_type = Key;
        using pointer = detail::set_arrow_proxy<Key>;
        using reference = Key;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        const_reverse_trie_set_iterator() noexcept {}
        explicit const_reverse_trie_set_iterator(
            const_trie_set_iterator<Key> it) noexcept :
            it_(it)
        {}
        explicit const_reverse_trie_set_iterator(
            reverse_trie_set_iterator<Key> it) noexcept :
            it_(it.it_)
        {}

        reference operator*() const noexcept { return *std::prev(it_); }

        pointer operator->() const noexcept
        {
            return std::prev(it_).operator->();
        }

        const_reverse_trie_set_iterator & operator++() noexcept
        {
            --it_;
            return *this;
        }
        const_reverse_trie_set_iterator operator++(int)noexcept
        {
            auto const retval = *this;
            --*this;
            return retval;
        }
        const_reverse_trie_set_iterator & operator--() noexcept
        {
            ++it_;
            return *this;
        }
        const_reverse_trie_set_iterator operator--(int)noexcept
        {
            auto const retval = *this;
            ++*this;
            return retval;
        }

        const_trie_set_iterator<Key> base() const noexcept { return it_; }

        friend bool operator==(
            const_reverse_trie_set_iterator lhs,
            const_reverse_trie_set_iterator rhs) noexcept
        {
            return lhs.it_ == rhs.it_;
        }
        friend bool operator!=(
            const_reverse_trie_set_iterator lhs,
            const_reverse_trie_set_iterator rhs) noexcept
        {
            return lhs.it_ != rhs.it_;
        }

    private:
        const_trie_set_iterator<Key> it_;
    };

}}

#endif
