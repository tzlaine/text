#ifndef BOOST_TEXT_DETAIL_COLLATION_DATA_HPP
#define BOOST_TEXT_DETAIL_COLLATION_DATA_HPP

#include <boost/text/string_view.hpp>
#include <boost/text/detail/collation_constants.hpp>
#include <boost/text/detail/normalization_data.hpp>

#include <boost/optional.hpp>

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include <cstdint>


namespace boost { namespace text { namespace detail {

    struct compressed_collation_element
    {
        uint32_t l1() const noexcept { return l1_; }
        uint16_t l2() const noexcept { return l2_; }
        uint8_t l3() const noexcept { return l3_; }

        uint32_t l1_;
        uint16_t l2_;
        uint8_t l3_;
    };

    inline bool operator==(
        compressed_collation_element lhs, compressed_collation_element rhs)
    {
        return lhs.l1_ == rhs.l1_ && lhs.l2_ == rhs.l2_ && lhs.l3_ == rhs.l3_;
    }
    inline bool operator!=(
        compressed_collation_element lhs, compressed_collation_element rhs)
    {
        return !(lhs == rhs);
    }
    inline bool operator<(
        compressed_collation_element lhs, compressed_collation_element rhs)
    {
        if (rhs.l1_ < lhs.l1_)
            return false;
        if (lhs.l1_ < rhs.l1_)
            return true;

        if (rhs.l2_ < lhs.l2_)
            return false;
        if (lhs.l2_ < rhs.l2_)
            return true;

        return lhs.l3_ < rhs.l3_;
    }
    inline bool operator<=(
        compressed_collation_element lhs, compressed_collation_element rhs)
    {
        return lhs == rhs || lhs < rhs;
    }

    static_assert(
        sizeof(compressed_collation_element) == 8,
        "Oops!  compressed_collation_element should be 64 bits.");

    struct collation_element
    {
        uint32_t l1_;
        uint16_t l2_;
        uint8_t l3_;
        uint32_t l4_;
    };

    extern compressed_collation_element const * g_collation_elements_first;

    struct compressed_collation_elements
    {
        using iterator = compressed_collation_element const *;

        iterator begin(compressed_collation_element const * elements) const
            noexcept
        {
            return elements + first_;
        }
        iterator end(compressed_collation_element const * elements) const
            noexcept
        {
            return elements + last_;
        }

        int size() const noexcept { return last_ - first_; }
        explicit operator bool() const noexcept { return first_ != last_; }

        uint16_t first_;
        uint16_t last_;
    };

    inline bool operator==(
        compressed_collation_elements lhs, compressed_collation_elements rhs)
    {
        return lhs.first_ == rhs.first_ && lhs.last_ == rhs.last_;
    }
    inline bool operator!=(
        compressed_collation_elements lhs, compressed_collation_elements rhs)
    {
        return !(lhs == rhs);
    }

    struct collation_trie_node;

    extern collation_trie_node const * g_collation_trie_nodes;

    struct collation_trie_node
    {
        using iterator = collation_trie_node const *;

        iterator begin(collation_trie_node const * nodes) const noexcept
        {
            return nodes + first_child_;
        }
        iterator end(collation_trie_node const * nodes) const noexcept
        {
            return nodes + last_child_;
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
        compressed_collation_elements collation_elements_;
    };

    inline bool operator==(collation_trie_node lhs, collation_trie_node rhs)
    {
        return lhs.cp_ == rhs.cp_;
    }
    inline bool operator!=(collation_trie_node lhs, collation_trie_node rhs)
    {
        return !(lhs == rhs);
    }

}}}

namespace std {
    template<>
    struct hash<boost::text::detail::collation_trie_node>
    {
        typedef boost::text::detail::collation_trie_node argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const & n) const noexcept
        {
            return n.cp_;
        }
    };
}

namespace boost { namespace text { namespace detail {

    struct longest_collation_t
    {
        collation_trie_node node_;
        int match_length_ = 0;

        static const uint16_t invalid_trie_node_index = 0xffff;
    };

    extern std::unordered_set<collation_trie_node> const
        g_collation_initial_nodes;

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
               collation_trie_node const & rhs) { return lhs.cp_ < rhs.cp_; });
        if (it != last && it->cp_ != cp)
            it = last;
        return it;
    }

    inline longest_collation_t extend_collation(
        longest_collation_t prev,
        uint32_t cp,
        collation_trie_node const * collation_trie_nodes) noexcept
    {
        auto first = collation_trie_nodes + prev.node_.first_child_;
        auto last = collation_trie_nodes + prev.node_.last_child_;
        auto const node_it = find_trie_node(first, last, cp);
        if (node_it == last || !node_it->match())
            return prev;
        return longest_collation_t{*node_it, prev.match_length_ + 1};
    }

    inline longest_collation_t
    default_extend_collation(longest_collation_t prev, uint32_t cp) noexcept
    {
        return extend_collation(prev, cp, g_collation_trie_nodes);
    }

    template<typename Iter>
    longest_collation_t longest_collation(
        Iter first,
        Iter last,
        std::unordered_set<collation_trie_node> const & collation_initial_nodes,
        collation_trie_node const * collation_trie_nodes) noexcept
    {
        auto it = first;
        auto const starter = *it++;
        collation_trie_node node{starter};
        auto hash_it = collation_initial_nodes.find(node);
        if (hash_it == collation_initial_nodes.end())
            return longest_collation_t{{}, 0};
        node = *hash_it;

        longest_collation_t retval{node, node.collation_elements_ ? 1 : 0};
        while (it != last && !node.leaf()) {
            auto const node_it = find_trie_node(
                collation_trie_nodes + node.first_child_,
                collation_trie_nodes + node.last_child_,
                *it++);
            if (node_it == collation_trie_nodes + node.last_child_)
                break;
            node = *node_it;
            if (node.match()) {
                retval.node_ = node;
                retval.match_length_ = it - first;
            }
        }
        return retval;
    }

    template<typename Iter>
    longest_collation_t
    default_longest_collation(Iter first, Iter last) noexcept
    {
        return longest_collation(
            first, last, g_collation_initial_nodes, g_collation_trie_nodes);
    }

    struct reorder_group
    {
        string_view name_;
        compressed_collation_element first_;
        compressed_collation_element last_;
        bool simple_;
        bool compressible_;
    };

    inline bool operator==(reorder_group lhs, reorder_group rhs)
    {
        return lhs.name_ == rhs.name_ && lhs.first_ == rhs.first_ &&
               lhs.last_ == rhs.last_ && lhs.simple_ == rhs.simple_ &&
               lhs.compressible_ == rhs.compressible_;
    }

    extern std::array<reorder_group, 140> const g_reorder_groups;

    inline optional<reorder_group> find_reorder_group(string_view name)
    {
        if (name == "Hrkt")
            name = "Hira";
        if (name == "Kana")
            name = "Hira";
        if (name == "Hans")
            name = "Hani";
        if (name == "Hant")
            name = "Hani";
        for (auto group : g_reorder_groups) {
            if (group.name_ == name)
                return group;
        }
        return {};
    }

}}}

#endif
