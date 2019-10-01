#ifndef BOOST_TEXT_TRIE_FWD_HPP
#define BOOST_TEXT_TRIE_FWD_HPP


namespace boost { namespace trie {

    /** A statically polymorphic less-than compariason object type.  This is
        only necessary for pre-C++14 portablility. */
    struct less
    {
        template<typename T>
        bool operator()(T const & lhs, T const & rhs) const
            noexcept(noexcept(std::less<T>{}(lhs, rhs)))
        {
            return std::less<T>{}(lhs, rhs);
        }
    };

    template<
        typename Key,
        typename Value,
        typename Compare = less,
        std::size_t KeySize = 0>
    struct trie;

    template<typename Key, typename Value, typename Compare = less>
    struct trie_map;

    template<typename Key, typename Compare = less>
    struct trie_set;

}}

#endif
