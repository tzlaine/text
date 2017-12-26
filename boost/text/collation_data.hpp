#ifndef BOOST_TEXT_COLLATION_DATA_HPP
#define BOOST_TEXT_COLLATION_DATA_HPP

#include <boost/text/normalization_data.hpp>
#include <boost/text/collation_weights.hpp>

#include <unordered_map>

#include <cstdint>


namespace boost { namespace text {

    /** */
    struct collation_element
    {
        uint16_t l1() const noexcept { return l1_; }
        uint8_t l2() const noexcept { return uint8_t(l2_bias + biased_l2_); }
        uint8_t l3() const noexcept { return l3_; }

        static constexpr uint8_t l2_bias =
            static_cast<uint8_t>(static_cast<int>(collation_weights::min_l2));

        uint16_t l1_;
        uint8_t biased_l2_;
        uint8_t l3_;
    };

    static_assert(
        sizeof(collation_element) == 4,
        "Oops!  collation_element should be 32 bits.");

    namespace detail {
        extern collation_element const * g_collation_elements_first;
    }

    struct collation_elements
    {
        using iterator = collation_element const *;

        iterator begin() const noexcept
        {
            return detail::g_collation_elements_first + first_;
        }
        iterator end() const noexcept
        {
            return detail::g_collation_elements_first + last_;
        }

        explicit operator bool() const noexcept { return first_ != last_; }

        uint16_t first_;
        uint16_t last_;
    };

    /** TODO */
    struct longest_collation_t
    {
        collation_elements collation_elements_;
        int match_length_;
        uint16_t trie_node_index_;

        static const uint16_t invalid_trie_node_index = 0xffff;
    };

    namespace detail {
        struct collation_trie_node;

        extern collation_trie_node const * g_collation_trie_start_nodes_first;
        extern collation_trie_node const * g_collation_trie_start_nodes_last;

        struct collation_trie_node
        {
            using iterator = collation_trie_node const *;

            iterator begin() const noexcept
            {
                return g_collation_trie_start_nodes_first + first_child_;
            }
            iterator end() const noexcept
            {
                return g_collation_trie_start_nodes_first + last_child_;
            }

            bool match() const noexcept
            {
                return static_cast<bool>(collation_elements_);
            }

            bool leaf() const noexcept { return first_child_ == last_child_; }

            uint32_t cp_;

            uint16_t first_child_;
            uint16_t last_child_;

            // Only nonempty when this is the end of a match.
            collation_elements collation_elements_;
        };

        inline collation_trie_node const * find_trie_node(
            collation_trie_node const * first,
            collation_trie_node const * last,
            uint32_t cp) noexcept
        {
            collation_trie_node const to_find{cp};
            auto it = std::lower_bound(
                first,
                last,
                to_find,
                [](collation_trie_node const & lhs,
                   collation_trie_node const & rhs) {
                    return lhs.cp_ < rhs.cp_;
                });
            if (it != last && it->cp_ != cp)
                it = last;
            return it;
        }

        inline longest_collation_t
        extend_collation(longest_collation_t prev, uint32_t cp) noexcept
        {
            auto const node =
                g_collation_trie_start_nodes_first[prev.trie_node_index_];
            auto first = g_collation_trie_start_nodes_first + node.first_child_;
            auto last = g_collation_trie_start_nodes_first + node.last_child_;
            auto const node_it = find_trie_node(first, last, cp);
            if (node_it == last || !node_it->match())
                return prev;
            return longest_collation_t{
                node_it->collation_elements_,
                prev.match_length_ + 1,
                uint16_t(node_it - g_collation_trie_start_nodes_first)};
        }
    }

    /** TODO */
    template<typename Iter>
    longest_collation_t longest_collation(Iter first, Iter last) noexcept
    {
        auto it = first;
        auto const starter = *it++;
        auto node_it = find_trie_node(
            detail::g_collation_trie_start_nodes_first,
            detail::g_collation_trie_start_nodes_last,
            starter);
        if (node_it == detail::g_collation_trie_start_nodes_last)
            return longest_collation_t{{0, 0}, 0, 0xffff};

        longest_collation_t retval{
            node_it->collation_elements_,
            1,
            node_it - detail::g_collation_trie_start_nodes_first};
        while (!node_it->leaf()) {
            auto const node_it2 = find_trie_node(
                node_it->first_child_, node_it->last_child_, *it++);
            if (node_it2 == node_it->last_child_)
                break;
            node_it = node_it2;
            if (node_it->match()) {
                retval.collation_elements_ = node_it->collation_elements_;
                retval.match_length_ = it - first;
                retval.trie_node_index_ =
                    node_it - detail::g_collation_trie_start_nodes_first;
            }
        }
        return retval;
    }

    namespace detail {
        extern const std::unordered_map<uint32_t, collation_elements>
            g_collation_element_table_1;
        extern const std::unordered_map<uint32_t, collation_elements>
            g_collation_element_table_2;
    }

    /** Returns the collation elements for code point cp, if it exists. */
    inline longest_collation_t collation(uint32_t cp) noexcept
    {
        if (cp < static_cast<int>(
                     collation_weights::median_sigleton_collation_key)) {
            auto const it = detail::g_collation_element_table_1.find(cp);
            if (it == detail::g_collation_element_table_1.end())
                return longest_collation_t{{0, 0}, 0, 0xffff};
            return longest_collation_t{it->second, 1, 0xffff};
        } else {
            auto const it = detail::g_collation_element_table_2.find(cp);
            if (it == detail::g_collation_element_table_2.end())
                return longest_collation_t{{0, 0}, 0, 0xffff};
            return longest_collation_t{it->second, 1, 0xffff};
        }
    }

}}

#endif
